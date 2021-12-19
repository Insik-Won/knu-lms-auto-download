#ifndef _TEST_UTIL_H
#define _TEST_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

int is_sentinel(const struct CMUnitTest* test);

size_t get_count(const struct CMUnitTest* tests);

static int load_dotenv_impl(const char* filename);

int load_dotenv(const char* directory);

int cmocka_run_group_tests_dynamically(const char* group_name, const struct CMUnitTest* tests, CMFixtureFunction group_setup, CMFixtureFunction group_teardown);

int prepare_file(void** state);

int clean_file(void** state);

void assert_file_equal(FILE* file1, FILE* file2);
void assert_filename_equal(const char* filename2, const char* filename1);



#endif // !_TEST_UTIL_H