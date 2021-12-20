#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <unistd.h>
#include <fcntl.h>

#include <knuutils.h>

static int prepare_KnuString(void** state) {
  KnuString* str = malloc(sizeof(*str));
  KnuString_init(str);
  *state = (void*)str;

  return 0;
}

static int clean_KnuString(void** state) {
  KnuString* str = (KnuString*)*state;
  if (str) {
    KnuString_destroy(str);
    free(str);
  }

  return 0;
}

static void test_KnuString_init(void** state) {
  KnuString* str = (KnuString*)*state;

  assert_string_equal(str->value, "");
  assert_int_equal(str->length, 0);
  assert_int_equal(str->capacity, 1);
}

static void test_KnuString_appendString(void** state) {
  KnuString* str = (KnuString*)*state;

  KnuString_appendString(str, "Hello");
  assert_string_equal(str->value, "Hello");
  assert_int_equal(str->length, strlen("Hello"));
  assert_int_equal(str->capacity, 8);

  KnuString_appendString(str, " ");
  assert_string_equal(str->value, "Hello ");
  assert_int_equal(str->length, strlen("Hello "));
  assert_int_equal(str->capacity, 8);

  KnuString_appendString(str, "World!");
  assert_string_equal(str->value, "Hello World!");
  assert_int_equal(str->length, strlen("Hello World!"));
  assert_int_equal(str->capacity, 16);
}

static void test_KnuString_empty(void** state) {
  KnuString* str = (KnuString*)*state;

  KnuString_appendString(str, "Hello World!");
  assert_true(KnuString_empty(str));
  assert_string_equal(str->value, "");
  assert_int_equal(str->length, 0);
}

static void test_KnuString_isEmpty(void** state) {
  KnuString* str = (KnuString*)*state;
  assert_true(KnuString_isEmpty(str));

  KnuString_appendString(str, "Hello World!");
  assert_false(KnuString_isEmpty(str));

  KnuString_empty(str);
  assert_true(KnuString_isEmpty(str));
}

static void test_KnuString_readAllFileFromFd(void** state) {
  KnuString* str = (KnuString*)*state;

  char buffer[FILENAME_MAX];
  strcpy(buffer, getenv("RES_PATH"));
  strcat(buffer, "/HelloWorld5.txt");

  int fd = open(buffer, O_RDONLY);
  assert_int_not_equal(fd, -1);
  KnuString_readAllFromFd(str, fd);
  close(fd); 

  buffer[0] = '\0';
  for (int i = 0; i < 5; i++) {
    strcat(buffer, "Hello World!\n");
  }

  assert_string_equal(str->value, buffer);
}

static void test_KnuString_readAllFromFile(void** state) {
  KnuString* str = (KnuString*)*state;

  char buffer[FILENAME_MAX];
  strcpy(buffer, getenv("RES_PATH"));
  strcat(buffer, "/HelloWorld5.txt");

  FILE* file = fopen(buffer, "r");
  assert_non_null(file);
  KnuString_readAllFromFile(str, file);
  fclose(file); 

  buffer[0] = '\0';
  for (int i = 0; i < 5; i++) {
    strcat(buffer, "Hello World!\n");
  }

  assert_string_equal(str->value, buffer);
}

static void test_KnuString_tokenize(void** state) {
  KnuString* str = (KnuString*)*state;

  const char* origin_text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
  const char* origin_text_tokenized[] = {"Lorem", "ipsum", "dolor", "sit", "amet,", "consectetur", "adipiscing", "elit,", "sed", "do", "eiusmod", "tempor", "incididunt", "ut", "labore", "et", "dolore", "magna", "aliqua."};

  KnuString_appendString(str, origin_text);

  const char* result;
  char* save_ptr = NULL;
  
  int i = 0;
  while (result = KnuString_tokenize(str, ' ', &save_ptr)) {
    assert_string_equal(result, origin_text_tokenized[i]);
    i++;
  }

  assert_int_equal(i, sizeof(origin_text_tokenized)/sizeof(*origin_text_tokenized));  
}

static void test_KnuString_collapse(void** state) {
  KnuString* str = (KnuString*)(*state);

  const char* prepared = "H e    llo World  !";
  const char* expected = "H e llo World !";

  KnuString_appendString(str, prepared);
  KnuString_collapse(str, ' ');

  assert_string_equal(str->value, expected);
  assert_int_equal(str->length, strlen(expected));
}

static void test_KnuString_collapseWhitespace(void** state) {
  KnuString* str = (KnuString*)(*state);

  const char* prepared = "H e  \n\t  llo World \r\n !";
  const char* expected = "H e llo World !";

  KnuString_appendString(str, prepared);
  KnuString_collapseWhitespace(str);

  assert_string_equal(str->value, expected);
  assert_int_equal(str->length, strlen(expected));
}

static void test_KnuString_destroy(void** state) {
  KnuString* str = (KnuString*)*state;

  KnuString_appendString(str, "Hello World!");
  KnuString_destroy(str);

  assert_null(str->value);
  assert_int_equal(str->length, -1);
  assert_int_equal(str->capacity, 0);
  free(str);

  *state = NULL;
}

const struct CMUnitTest* test_knustring() {
  static const struct CMUnitTest knuutils_tests[] = {
    cmocka_unit_test_setup_teardown(test_KnuString_init, prepare_KnuString, clean_KnuString),
    cmocka_unit_test_setup_teardown(test_KnuString_appendString, prepare_KnuString, clean_KnuString),
    cmocka_unit_test_setup_teardown(test_KnuString_readAllFileFromFd, prepare_KnuString, clean_KnuString),
    cmocka_unit_test_setup_teardown(test_KnuString_readAllFromFile, prepare_KnuString, clean_KnuString),
    cmocka_unit_test_setup_teardown(test_KnuString_destroy, prepare_KnuString, clean_KnuString),
    cmocka_unit_test_setup_teardown(test_KnuString_tokenize, prepare_KnuString, clean_KnuString),
    cmocka_unit_test_setup_teardown(test_KnuString_collapse, prepare_KnuString, clean_KnuString),
    cmocka_unit_test_setup_teardown(test_KnuString_collapseWhitespace, prepare_KnuString, clean_KnuString),
    {NULL, NULL, NULL, NULL, NULL},
  };

  return knuutils_tests;
}