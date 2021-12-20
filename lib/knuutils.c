#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "knuutils.h"

char* asprintf(const char* format, ...) {
  va_list args;
  va_start(args, format);

  char* result = vasprintf(format, args);

  va_end(args);

  return result;
}

char* vasprintf(const char* format, va_list args) {
  va_list args2;
  va_copy(args2, args);

  size_t len = vsnprintf(NULL, 0, format, args);
  char* buffer = malloc(sizeof(*buffer) * len);

  if (buffer) {
    vsprintf(buffer, format, args2);
    va_end(args2);
  }

  return buffer;
}


/**
 * @brief make the temporary file and save its path to filename
 * 
 * @param filename the filename that has XXXXXX
 * @return 0 if suceeed, otherwise, non-zero value.
 */
int make_tmpfile(char filename[]) {
  const char* pattern = "XXXXXX";
  char* found = strstr(filename, pattern);
  if (!found)
    return -1;

  char* suffix = found + strlen(pattern);

  int fd = mkstemps(filename, strlen(suffix));
  close(fd);

  return 0;
}

/**
 * @brief copy file from source to destination
 * 
 * @param source the source filename
 * @param destination the destination filename
 * @return 0 if succeed, otherwise, nonzero value.
 */
int copy_file(const char* source, const char* destination) {
  FILE* file1, *file2;
  file1 = file2 = NULL;
  file1 = fopen(source, "r");
  file2 = fopen(destination, "w");

  if (!file1 || !file2) {
    if (file1) fclose(file1);
    if (file2) fclose(file2);
    return -1;
  }

  char buffer[BUFSIZ];
  size_t len;
  while ((len = fread(buffer, sizeof(char), sizeof(buffer), file1)) > 0) {
    fwrite(buffer, sizeof(char), len, file2);
  }  

  fclose(file1);
  fclose(file2);

  return 0;
}


/*
 * unzip gzip file into directory 
 * if dir is null, ungzip it directly.
 */ 
int gunzip(const char* source, const char* destination) {
  int fd_in = open(source, O_RDONLY);
  int fd_out = open(destination, O_WRONLY | O_CREAT | O_TRUNC);
  if (fd_in == -1 || fd_out == -1) {
    if (fd_in != -1) close(fd_in);
    if (fd_out != -1) close(fd_out);
    return -1;
  }

  int pid = fork();
  if (pid == 0) {

    dup2(fd_in, 0);
    dup2(fd_out, 1);

    execlp("gunzip", "gunzip", NULL);
  }
  else {
    close(fd_in);
    close(fd_out);
  }

  wait(NULL);

  return 1;
}

#define NORMALIZE_COMMAND "sed \"s/<script.*<\\/script>//g;/<script/,/<\\/script>/{/<script/!{/<\\/script>/!d}};s/<script.*//g;s/.*<\\/script>//g\" | hxnormalize -x | tr -d \"\\n\"" 
/*
 * remove scripts tag from html file and normaize to well-formed xml
 */
int html_normalize(const char* source, const char* destination) {
  char* command = NULL;
  FILE* process = NULL;
  FILE* output = NULL;
  char buffer[BUFSIZ];
  size_t len;

  command = asprintf("<%s %s", source, NORMALIZE_COMMAND);
  if (!command) goto html_normalize_fail;

  output = fopen(destination, "w");
  if (!output) goto html_normalize_fail;
  
  process = popen(command, "r");
  if (!process) goto html_normalize_fail;

  while ((len = fread(buffer, sizeof(*buffer), sizeof(buffer), process)) > 0) {
    fwrite(buffer, sizeof(*buffer), len, output);
  }

  free(command);
  fclose(output);
  pclose(process);
  return 1;

html_normalize_fail:
  if (command) free(command);
  if (output) fclose(output);
  if (process) pclose(process);
  return 0;
}

/*
 * css-select with hxselect.
 * - @return 1 if the pattern is founded, otherwise, 0.
 */
int css_select(const char* filename, const char* option, const char* pattern, KnuString* str) {
  int pipes[2];
  pipe(pipes);

  int pid = fork();
  if (pid == 0) {
    close(pipes[0]);
    int infd = open(filename, O_RDONLY);
    dup2(infd, 0);
    dup2(pipes[1], 1);

    if (option)
      execlp("hxselect", "hxselect", option, pattern, NULL);
    else
      execlp("hxselect", "hxselect", pattern, NULL);
    perror("excelp, hxselect error");
  }
  wait(NULL);

  close(pipes[1]);
  KnuString_empty(str);
  KnuString_readAllFromFd(str, pipes[0]);

  return !KnuString_isEmpty(str);
}

int unzip(const char* source, const char* directory, const char* include_glob, const char* exclude_glob) {
  pid_t pid = fork();
  if (pid == 0) {
    if (include_glob && exclude_glob)
      execlp("unzip", "unzip", "-q", "-o", source, "-d", directory, include_glob,"-x", exclude_glob,  NULL);
    else if (include_glob)
      execlp("unzip", "unzip", "-q", "-o", source, "-d", directory, include_glob, NULL);
    else if (exclude_glob)
      execlp("unzip", "unzip", "-q", "-o", source, "-d", directory, "-x", exclude_glob, NULL);
    else
      execlp("unzip", "unzip", "-q", "-o", source, "-d", directory,NULL);

    perror("excelp unzip error");
  }
  wait(NULL);

  return 1;
}