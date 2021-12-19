#include "test_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include <knuutils.h>


int is_sentinel(const struct CMUnitTest* test) {
  return test->initial_state == NULL && test->name == NULL 
         && test->setup_func == NULL && test->teardown_func == NULL 
         && test->test_func == NULL;
}

size_t get_count(const struct CMUnitTest* tests) {
  size_t count = 0;
  while (!is_sentinel(tests++)) {
     count++;
  }
  return count;
}

static int load_dotenv_impl(const char* filename) {
  FILE* file = fopen(filename, "r");
  if (!file)
    return 0;

  char buffer[512];
  while (fgets(buffer, sizeof(buffer), file)) {
    // remove the newline charactor
    char* pch = strrchr(buffer, '\n');
    if (pch) {
      *pch = '\0';
    }

    if(putenv(strdup(buffer)) == -1) {
      fclose(file);
      return 0;
    }
  }

  fclose(file);
  return 1;
}

int load_dotenv(const char* directory) {
  DIR* dir = opendir(directory);
  if (!dir)
    return 0;

  struct dirent* entry;
  char buffer[FILENAME_MAX];
  while (entry = readdir(dir)) {
    if (DT_REG == entry->d_type && strcmp(".env", entry->d_name) == 0) {
      strcpy(buffer, directory);
      strcat(buffer, "/");
      strcat(buffer, entry->d_name);

      int result = load_dotenv_impl(buffer);
      closedir(dir);
      return result;
    }
  }

  closedir(dir);

  return 0;
}

int cmocka_run_group_tests_dynamically(const char* group_name, const struct CMUnitTest* tests, CMFixtureFunction group_setup, CMFixtureFunction group_teardown) {
  return _cmocka_run_group_tests(group_name, tests, get_count(tests), group_setup, group_teardown);
}

int prepare_file(void** state) {
  char* filename = (char*)*state;
  char* original_filename = NULL;
  if (filename) {
    original_filename = asprintf("%s/%s", getenv("RES_PATH"), filename);
  }

  char* extension = "";
  if (original_filename) {
    char* original_extension = strchr(original_filename, '.');
    if (original_extension) {
      extension = original_extension;
    }
  }

  char* temp_filename = asprintf("%s/tempXXXXXX%s", getenv("RES_PATH"), extension);

  int fd = mkstemps(temp_filename, strlen(extension));
  if (fd == -1)
    return fd;
  close(fd);

  if (original_filename) {
    FILE* original_file = fopen(original_filename, "r");
    FILE* temp_file = fopen(temp_filename, "w");
    assert_non_null(original_file);
    assert_non_null(temp_file);
    
    char buffer[BUFSIZ];
    size_t count;
    while ((count = fread(buffer, sizeof(*buffer), sizeof(buffer), original_file)) > 0)
      fwrite(buffer, sizeof(*buffer), count, temp_file);

    fclose(original_file);
    fclose(temp_file);
  }

  char** filenames = malloc(sizeof(char**) * 2);
  filenames[0] = original_filename;
  filenames[1] = temp_filename;

  *state = (void*)filenames;
  return 0;
}

int clean_file(void** state) {
  char** filenames = (char**)*state;  
  char* original_filename = filenames[0];
  char* temp_filename = filenames[1];

  if (original_filename)
    free(original_filename);

  if (remove(temp_filename) == -1)
    return -1;

  free(temp_filename);
  free(filenames);

  return 0;
}

void assert_file_equal(FILE* file1, FILE* file2) {
  char buffer1[BUFSIZ], buffer2[BUFSIZ];
  size_t len1, len2;
  
  while (1) {
    len1 = fread(buffer1, sizeof(*buffer1), sizeof(buffer1), file1);
    len2 = fread(buffer2, sizeof(*buffer2), sizeof(buffer2), file2);

    assert_int_equal(len1, len2);
    if (len1 == 0 && len2 == 0) {
      break;
    }

    assert_memory_equal(buffer1, buffer2, len1);
  }
}

void assert_filename_equal(const char* filename2, const char* filename1) {
  FILE* file1 = fopen(filename1, "r");
  FILE* file2 = fopen(filename2, "r");
  assert_non_null(file1);
  assert_non_null(file2);
  assert_file_equal(file1, file2);
  fclose(file1);
  fclose(file2);
}
