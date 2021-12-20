#ifndef _KNU_UTILS_H_
#define _KNU_UTILS_H_

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>

#include "knustring.h"

/*
 * dynamically allocate string with format
 */
char* asprintf(const char* format, ...);
char* vasprintf(const char* format, va_list arg);


/**
 * @brief make the temporary file and save its path to filename
 * 
 * @param filename the filename that has XXXXXX
 * @return 0 if suceed, otherwise, nonzero value.
 */
int make_tmpfile(char filename[]);

/**
 * @brief copy file from source to destination
 * 
 * @param source the source filename
 * @param destination the destination filename
 * @return 0 if succeed, otherwise, nonzero value.
 */
int copy_file(const char* source, const char* destination);


int unzip(const char* source, const char* directory, const char* include_glob, const char* exclude_glob);

/*
 * unzip gzip file into its parent directory 
 */ 
int gunzip(const char* source, const char* destination);
/*
 * remove scripts tag from html file and normaize to well-formed xml
 */
int html_normalize(const char* source, const char* destination);
/*
 * css-select with hxselect.
 * - @return 1 if the pattern is founded, otherwise, 0.
 */
int css_select(const char* filename, const char* option, const char* pattern, KnuString* str);

#endif //!_KNU_UTILS_H_ 