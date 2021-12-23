#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "knustring.h"

// initiate KnuString
void KnuString_init(KnuString* str) {
  str->value = malloc(sizeof(char));
  str->value[0] = '\0';
  str->length = 0;
  str->capacity = 1;
}
// destory KnuString's members, not the one
void KnuString_destroy(KnuString* str) {
  if (!str) return;
  if (!str->value) return;

  free(str->value);
  str->value = NULL;
  str->length = -1;
  str->capacity = 0;
}

/*
 * empty KnuString
 */
int KnuString_empty(KnuString* knustr) {
  knustr->value[0] = '\0';
  knustr->length = 0;
  return 1;
}

/*
 * return if KnuString is empty
 */
int KnuString_isEmpty(KnuString* knustr) {
  return knustr->value[0] == '\0' && knustr->length == 0;
}

/*
 * resize the buffer
 */
int KnuString_resize(KnuString* knustr, size_t capacity) {
  if (capacity < knustr->capacity && knustr->length + 1 > capacity)
    return 0;

  char* new_buffer = realloc(knustr->value, sizeof(*new_buffer) * capacity);
  if (!new_buffer)
    return 0;

  knustr->value = new_buffer;
  knustr->capacity = capacity;
  return 1;
}
/*
 * append null-terminated string to KnuString.
 * - @return 1 if succeed, otherwise, 0.
 */
int KnuString_appendString(KnuString* knustr, const char* str) {
  return KnuString_appendCharArray(knustr, str, strlen(str));
}

/* 
 * append c-type char array to KnuString.
 * - return 1 if succeed, otherwise, 0.
 */
int KnuString_appendCharArray(KnuString* knustr, const char* arr, size_t n) {
  while (knustr->length + n + 1 > knustr->capacity) {
    int result = KnuString_resize(knustr, knustr->capacity * 2);
    if (!result)
      return 0;
  }

  memcpy(&knustr->value[knustr->length], arr, n);
  knustr->length += n;
  knustr->value[knustr->length] = '\0';

  return 1;
}

const char* KnuString_tokenize(KnuString* knustr, const char delim, char** save_ptr) {
  char delims[2] = { delim, '\0' };
  char* result;
  
  if (*save_ptr == NULL)
    result = strtok_r(knustr->value, delims, save_ptr);
  else
    result = strtok_r(NULL, delims, save_ptr);

  if (result == NULL || save_ptr == NULL) {
    for (int i = 0; i < knustr->length; i++) {
      if (knustr->value[i] == '\0') {
        knustr->value[i] = delim;
      }
    }

    return NULL;
  }

  return result;
}

/**
 * @brief tokenize KnuString with '\0'. at last, resotre the KnuString. if you want stop, give save_ptr as NULL.
 * 
 * @param knustr the KnuString
 * @param delim only one char
 * @param save_ptr the pointer of char* instance. it should be initialize with NULL.
 * @param array the pointer of KnuString* to contain the NULL-terminated array.
 * @return size of arry if succeed, else -1
 */
int KnuString_tokenizeToArray(KnuString* knustr, const char delim, KnuString** array) {
  char delims[2] = { delim, '\0' };
  char* result;
  char* save_ptr;

  int count = 0;
  result = strtok_r(knustr->value, delims, &save_ptr);
  if (result) {
    count++;
    while ((result = strtok_r(NULL, delims, &save_ptr))) {
      count++;
    }
  }

  KnuString* arr = malloc(sizeof(*arr) * (count + 1));
  int idx = 0;
  for (int i = 0; i < count; i++) {
    KnuString_init(&arr[i]);
    KnuString_appendString(&arr[i], &knustr->value[idx]);
    idx += arr[i].length + 1;
  }
  KnuString_init(&arr[count]);
  KnuString_empty(&arr[count]); 


  if (result == NULL || save_ptr == NULL) {
    for (int i = 0; i < knustr->length; i++) {
      if (knustr->value[i] == '\0') {
        knustr->value[i] = delim;
      }
    }
  }

  *array = arr;

  return count;
}

/**
 * @brief read all charactor in the file descriptor. descriptor should be closed after the call
 * 
 * @return 1 if succeeed, else 0
 */
int KnuString_readAllFromFd(KnuString* knustr, int fd) {
  char buf[BUFSIZ];
  size_t len;

  KnuString_empty(knustr);
  int ret = 1;
  while ((len = read(fd, buf, sizeof(buf))) > 0) {
    if(!KnuString_appendCharArray(knustr, buf, len)) {
      ret = 0;
      break;
    }
  }

  return ret;
}

/**
 * @brief read all charator in the file. file should be closed after the call
 * 
 * @return 1 if succeed, else 0
 */
int KnuString_readAllFromFile(KnuString* knustr, FILE* file) {
  char buf[BUFSIZ];
  size_t len;

  KnuString_empty(knustr);
  int ret = 1;
  while ((len = fread(buf, sizeof(*buf), sizeof(buf) / sizeof(*buf), file)) > 0) {
    if(!KnuString_appendCharArray(knustr, buf, len)) {
      ret = 0;
      break;
    }
  }

  return ret;
}

/**
 * @brief read all charator in the file. file should be closed after the call
 * 
 * @return 1 if succeed, else 0
 */
int KnuString_readAllFromFilename(KnuString* knustr, const char* filename) {
  FILE* file = fopen(filename, "r");
  if (!file)
    return 0;

  int result = KnuString_readAllFromFile(knustr, file);
  fclose(file);

  return result;
}

int KnuString_substring(KnuString* knustr, int start, int end, KnuString* result) {
  KnuString_empty(result);
  int length = end - start;
  if (length < 0)
    return 0;

  char replaced = knustr->value[end];
  knustr->value[end] = '\0';

  KnuString_appendString(result, knustr->value + start);
  knustr->value[end] = replaced;

  return 1;
}

int KnuString_collapse(KnuString* knustr, char to_collapse) {
  int i;
  for (i = 0; i < knustr->length; i++) {
    if (knustr->value[i] == to_collapse) {
      int idx = i;
      
      while (knustr->value[idx] == to_collapse)
        idx++;

      if (idx == i)
        continue;
      
      int gap = idx - i - 1;
      for (; idx < knustr->length; idx++) {
        knustr->value[idx - gap] = knustr->value[idx];
      }
      knustr->value[idx - gap] = '\0';
      knustr->length -= gap;
    }
  }

  return 0;
}

int KnuString_collapseWhitespace(KnuString* knustr) {
  int i;
  for (i = 0; i < knustr->length; i++) {
    if (isspace(knustr->value[i])) {
      int idx = i;
      
      while (isspace(knustr->value[idx]))
        idx++;

      if (idx == i)
        continue;
      
      int gap = idx - i - 1;
      knustr->value[idx - 1] = ' ';
      for (; idx < knustr->length; idx++) {
        knustr->value[idx - gap] = knustr->value[idx];
      }
      knustr->value[idx - gap] = '\0';
      knustr->length -= gap;
    }
  }

  return 0; 
}

int KnuString_trim(KnuString* knustr) {
  int start = 0;
  int end = knustr->length - 1;

  while (isspace(knustr->value[start])) {
    start++;
  }

  while (isspace(knustr->value[end])) {
    end--;
  }

  if (start == 0) {
    knustr->length = end - start + 1;
    knustr->value[knustr->length] = '\0';
  }
  else {
    knustr->length = end - start + 1;
    memcpy(knustr->value, knustr->value + start, sizeof(*knustr->value) * knustr->length);
    knustr->value[knustr->length] = '\0';
  }

  return 1;
}

int KnuString_trimEndUntil(KnuString* knustr, char sentinel) {
  char* pch = strrchr(knustr->value, sentinel);
  if (pch) {
    knustr->length = (pch - knustr->value - 1) / sizeof(char);
    *pch = '\0';
  }

  return 1;
}

