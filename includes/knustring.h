#ifndef _KNU_STRING_H
#define _KNU_STRING_H

#include <stdio.h>
#include <stddef.h>

/*
 * KnuString
 * - used for dynamic-sizing string
 * - value: the buffer
 * - length: the length of string
 * - capacity: the size of buffer.
 */
typedef struct _KnuString {
  char* value;
  size_t length;
  size_t capacity;
} KnuString;

// initiate KnuString
void KnuString_init(KnuString* str);
// destory KnuString's members, not the one
void KnuString_destroy(KnuString* str);

/*
 * resize the buffer
 */
int KnuString_resize(KnuString* knustr, size_t capacity);
/*
 * append null-terminated string to KnuString.
 * - @return 1 if succeed, otherwise, 0.
 */
int KnuString_appendString(KnuString* knustr, const char* str);

/* 
 * append c-type char array to KnuString.
 * - @return 1 if succeed, otherwise, 0.
 */
int KnuString_appendCharArray(KnuString* knustr, const char* arr, size_t size);

/*
 * empty KnuString
 */
int KnuString_empty(KnuString* knustr);

/*
 * return if KnuString is empty
 */
int KnuString_isEmpty(KnuString* knustr);

/**
 * @brief read all charactor in the file descriptor. descriptor should be closed after the call
 * 
 * @return 1 if succeeed, else 0
 */
int KnuString_readAllFromFd(KnuString* knustr, int fd);

/**
 * @brief read all charator in the file. file should be closed after the call
 * 
 * @return 1 if succeed, else 0
 */
int KnuString_readAllFromFile(KnuString* knustr, FILE* file);

/**
 * @brief read all charator in the file. file should be closed after the call
 * 
 * @return 1 if succeed, else 0
 */
int KnuString_readAllFromFilename(KnuString* knustr, const char* filename);

int KnuString_substring(KnuString* knustr, int start, int end, KnuString* result);


/**
 * @brief tokenize KnuString with '\0'. at last, resotre the KnuString. if you want stop, give save_ptr as NULL.
 * 
 * @param knustr the KnuString
 * @param delim only one char
 * @param save_ptr the pointer of char* instance. it should be initialize with NULL.
 * @return the toknized string with '\0'
 */
const char* KnuString_tokenize(KnuString* knustr, const char delim, char** save_ptr);

/**
 * @brief tokenize KnuString with '\0'. at last, resotre the KnuString. if you want stop, give save_ptr as NULL.
 * 
 * @param knustr the KnuString
 * @param delim only one char
 * @param save_ptr the pointer of char* instance. it should be initialize with NULL.
 * @param array the pointer of KnuString* to contain the NULL-terminated array.
 * @return array size if succeed, else -1
 */
int KnuString_tokenizeToArray(KnuString* knustr, const char delim, KnuString** array);

#endif // !_KNU_STRING_H 