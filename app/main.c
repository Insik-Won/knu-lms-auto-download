#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <termios.h>
#include <getopt.h>
#include <fnmatch.h>

#include <knulms.h>

#define COMPLETE_MARK "[\033[0;32m\xE2\x9C\x94\033[0m ]"
#define FAIL_MARK "[\033[1;31m\xE2\x9C\x97\033[0m ]"

struct option long_options[] = {
  {"userid", required_argument, 0, 'U'},
  {"password", required_argument, 0, 'P'},
  {"directory", required_argument, 0, 'D'},
  {"posttitle", required_argument, 0, 't'},
  {"postnum", required_argument, 0, 'n'},
  {"postorder", required_argument, 0, 'o'},
  {"include", required_argument, 0, 'i'},
  {"exclude", required_argument, 0, 'e'},
  {"quiet", no_argument, 0, 'q'},
  {0, 0, 0 ,0},
};

const char* short_options = "U:P:D:t:n:o:i:e:zqf";

const char* id = NULL;
const char* password = NULL;
const char* subject_pattern = NULL;
const char* directory = NULL;
const char* post_title = NULL;
int         post_number = -1;
int         post_order = -1;
const char* include_pattern = NULL;
const char* exclude_pattern = NULL;
int         verbose_flag = 1;

int (*post_predicate)(const char*, int, size_t) = NULL;

time_t download_start_time = 0;
const char* wait_message = NULL;

const char* find_longopt(int shortopt);

int match_subject_pattern(const char* subject_name, int idx, size_t size);
int match_post_title(const char* material_name, int idx, size_t size);
int match_post_number(const char* material_name, int idx, size_t size);
int match_post_order(const char* material_name, int idx, size_t size);

int process_arguments(int argc, char* argv[]);
int fill_essential_argument();
int set_noecho_mode();
int set_echo_mode();
int signal_setup();
void print_wait_message(int signum);
void cleanup_on_interrupt(int signum);

int main(int argc, char* argv[]) {
  if (process_arguments(argc, argv) != 0)
    exit(EXIT_FAILURE);

  if (fill_essential_argument() != 0)
    exit(EXIT_FAILURE);

  knulms_global_init();

  StudentSession* student_session = NULL;
  SubjectSession* subject_session = NULL;
  struct itimerval interval = {0};
  interval.it_value.tv_usec = 1;
  interval.it_interval.tv_usec = 999999;
  struct itimerval old_timer = {0};

  student_session = knulms_login(id, password);
  if (student_session == NULL) {
    if (verbose_flag) {
      fprintf(stdout, "%s Download failed\n", FAIL_MARK);
      fprintf(stderr, "- error: logging in to KNU LMS\n");
      fprintf(stderr, "- check your id and password\n");
    }
    exit(EXIT_FAILURE);
  }

  subject_session = knulms_selectFirstSubject(student_session, match_subject_pattern);
  if (subject_session == NULL) {
    if (verbose_flag) {
      fprintf(stdout, "%s Download failed\n", FAIL_MARK);
      fprintf(stderr, "- error: selecting a subject\n");
      fprintf(stderr, "- check the subject name\n");
    }
    exit(EXIT_FAILURE);
  }

  download_start_time = time(NULL);

  pid_t pid = fork();
  if (pid == 0) {
    int result = knulms_downloadFirstMaterial(subject_session, directory, post_predicate, include_pattern, exclude_pattern);  
    if (result != 0) {
      if (verbose_flag) {
        fprintf(stdout, "%s Download failed\n", FAIL_MARK);
        fprintf(stderr, "- error: downloading the files.\n");
      }
      exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
  }

  if (signal_setup() != 0) {
    fprintf(stdout, "%s Download failed\n", FAIL_MARK);
    fprintf(stderr, "- an error occurred while setting signals.\n");
  }

  int status;
  wait_message = "Downloading";
  if (setitimer(ITIMER_REAL, &interval, &old_timer) != 0) {
    fprintf(stdout, "%s Download failed\n", FAIL_MARK);
    fprintf(stderr, "- an error occurred while setting a timer.\n");
  }

  waitpid(pid, &status, 0);
  if (setitimer(ITIMER_REAL, &old_timer, NULL) != 0) {
    fprintf(stdout, "%s Download failed\n", FAIL_MARK);
    fprintf(stderr, "- an error occurred while resetting a timer.\n");
  }

  if (WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS) {
    exit(WEXITSTATUS(status));
  }

  if (verbose_flag)  {
    printf("\r%s Donwload completed\n", COMPLETE_MARK);
  }

  knulms_global_cleanup();
  return 0;
}

int set_noecho_mode() {
  struct termios ttystate;
  tcgetattr(0, &ttystate);
  ttystate.c_lflag &= ~ECHO;
  tcsetattr(0, TCSANOW, &ttystate);
}
int set_echo_mode() {
  struct termios ttystate;
  tcgetattr(9, &ttystate);
  ttystate.c_lflag |= ECHO;
  tcsetattr(0, TCSANOW, &ttystate);
}

int signal_setup() {
  static struct sigaction inthandler;

  inthandler.sa_handler = cleanup_on_interrupt;
  sigemptyset(&inthandler.sa_mask);
  sigaddset(&inthandler.sa_mask, SIGINT);
  sigaddset(&inthandler.sa_mask, SIGQUIT);
  sigaddset(&inthandler.sa_mask, SIGALRM);
  inthandler.sa_flags = SA_NOCLDSTOP;

  if (sigaction(SIGINT, &inthandler, NULL) != 0)
    return -1;
  if (sigaction(SIGQUIT, &inthandler, NULL) != 0)
    return -1;

  signal(SIGALRM, print_wait_message);

  return 0;
}

int fill_essential_argument() {
  static char user_id[256];
  static char user_pwd[256];

  if (!id) {
    if (verbose_flag)
      printf("Input your KNU LMS id: ");

    if (scanf("%255s", user_id) == 0)
      return -1;
    id = user_id;
  }

  if (!password) {
    if (verbose_flag) 
      printf("Input your KNU LMS password: ");

    set_noecho_mode(); 
    if (scanf("%255s", user_pwd) == 0) {
      set_echo_mode();
      return -1;
    }
    set_echo_mode();
    putchar('\n');
    password = user_pwd;
  }

  return 0;
}

const char* find_longopt(int shortopt) {
  for (int i = 0; i < sizeof(long_options) / sizeof(*long_options); i++) {
    if (long_options[i].val == shortopt)
      return long_options[i].name;
  }

  return NULL;
}

int process_arguments(int argc, char* argv[]) {
  opterr = 0;

  int option;
  int option_idx = 0;
  while ((option = getopt_long(argc, argv, short_options, long_options, &option_idx)) != -1) {
    switch (option) {
      case 'U':
        id = optarg;
        break;
      case 'P':
        password = optarg;
        break;
      case 'D':
        directory = optarg;
        break;
      case 't':
        post_title = optarg;
        post_predicate = match_post_title;
        break;
      case 'n':
        post_number = strtol(optarg, NULL, 10);
        post_predicate = match_post_number;
        if (errno == EINVAL) {
          fprintf(stderr, "-%c or --%s requires a number\n", 'n', find_longopt('n'));
          return -1;
        }
        if (post_number <= 0) {
          fprintf(stderr, "-%c or --%s requires a number larger than 0\n", 'n', find_longopt('n'));
          return -1;
        }
        break;
      case 'o':
        post_order = strtol(optarg, NULL, 10);
        post_predicate = match_post_order;
        if (errno == EINVAL) {
          fprintf(stderr, "-%c or --%s requires a number\n", 'o', find_longopt('o'));
          return -1;
        }
        if (post_order <= 0) {
          fprintf(stderr, "-%c or --%s requires a number larger than 0\n", 'o', find_longopt('o'));
          return -1;
        }
        break;
      case 'i':
        include_pattern = optarg;
        break;
      case 'e':
        exclude_pattern = optarg;
        break;
      case 'q':
        verbose_flag = 0;
        break;
      case '?':
        if (find_longopt(optopt)) {
          fprintf(stderr, "-%c or --%s requires an argument\n", optopt, find_longopt(optopt));
        }
        else {
          fprintf(stderr, "unknown option: -%c\n", optopt);
        }
        return -1;
    }
  }

  int pos_argc = argc - optind;
  if (directory == NULL && pos_argc != 2) {
    fprintf(stderr, "usage: %s [options] [subject_name] [output_directory]\n", argv[0]);
    return -1;
  }
  
  if (directory != NULL && pos_argc != 1) {
    fprintf(stderr, "usage: %s [options] [subject_name] -D [output_directory]\n", argv[0]);
    return -1;
  }

  subject_pattern = argv[optind];
  if (directory == NULL)
    directory = argv[optind + 1];

  return 0;
}

void print_wait_message(int signum) {
  if (!verbose_flag)
    return;

  static int quote = 3;
  char quotes[4] = "";
  for (int i = 0; i < quote; i++) {
    strcat(quotes, ".");
  }
  printf("\r\e[0K%s%s", wait_message, quotes);
  fflush(stdout);
  
  quote++;
  if (quote > 3)
    quote = 0;
}

void cleanup_on_interrupt(int signum) {
  putchar('\n');

  if (download_start_time == 0)
    exit(EXIT_FAILURE);

  DIR* dir = opendir(directory);
  if (!dir)
    exit(EXIT_FAILURE);

  if (verbose_flag) {
    fprintf(stderr, "Interrupt detected. rollback starts.\n");
  }

  struct dirent* entry;
  struct stat statbuf;
  char full_path[FILENAME_MAX];

  while ((entry = readdir(dir))) {
    if (entry->d_type != DT_REG)
      continue;

    strcpy(full_path, directory);
    strcat(full_path, "/");
    strcat(full_path, entry->d_name);

    if (stat(full_path, &statbuf) != 0)
      continue;

    if (download_start_time < statbuf.st_ctim.tv_sec) {
      remove(full_path);
    }  
  }
  closedir(dir);

  rmdir(directory);

  if (verbose_flag) {
    fprintf(stderr, "rollback ended.\n");
  }

  exit(EXIT_FAILURE);
}

int match_subject_pattern(const char* subject_name, int idx, size_t size) {
  return strstr(subject_name, subject_pattern) != NULL;
}

int match_post_title(const char* material_name, int idx, size_t size) {
  return strstr(material_name, post_title) != NULL;
}
int match_post_number(const char* material_name, int idx, size_t size) {
  int num = size - idx + 1;
  return num == post_number;
}
int match_post_order(const char* material_name, int idx, size_t size) {
  int num = idx + 1;
  return num == post_order;
}
