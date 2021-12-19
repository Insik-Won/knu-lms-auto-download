#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <unistd.h>

#include <knuutils.h>
#include <test_util.h>


void test_asprintf(void** state) {
  char* str = asprintf("%c %d %s Hello World!", 'o', 0, "Everytime");

  assert_string_equal(str, "o 0 Everytime Hello World!");

  free(str);
}

void test_make_tmpfile(void** state) {
  char filename[] = "/tmp/tmpXXXXXX.txt";
  assert_int_equal(make_tmpfile(filename), 0);
  
  FILE* file = fopen(filename, "r");
  assert_non_null(file);
  fclose(file);
}

void test_copy_file(void** state) {
  char** filenames = (char**)*state;
  const char* original_filename = filenames[0];
  const char* temp_filename = filenames[1];

  // empty the file
  FILE* file = fopen(temp_filename, "w");
  fclose(file);

  assert_int_equal(copy_file(original_filename, temp_filename), 0);
  assert_filename_equal(original_filename, temp_filename);
}

void test_gunzip(void** state) { 
  char** filenames = (char**)*state;
  char* original_filename = filenames[0];
  char* temp_filename = filenames[1];

  assert_true(gunzip(original_filename, temp_filename));

  char succeed_filename[FILENAME_MAX];
  strcpy(succeed_filename, getenv("RES_PATH"));
  strcat(succeed_filename, "/gzip.jpg");

  assert_filename_equal(succeed_filename, temp_filename);
}

void test_html_normalize(void** state) {
  char** filenames = (char**)*state;
  char* original_filename = filenames[0];
  char* temp_filename = filenames[1];

  assert_true(html_normalize(original_filename, temp_filename));

  char succeed_filename[FILENAME_MAX];
  strcpy(succeed_filename, getenv("RES_PATH"));
  strcat(succeed_filename, "/naver_norm.html");

  assert_filename_equal(succeed_filename, temp_filename);
}

void test_css_select(void** state) {
  KnuString result;
  KnuString_init(&result);

  char filename[FILENAME_MAX];
  strcpy(filename, getenv("RES_PATH"));
  strcat(filename, "/naver_norm.html");

  assert_true(css_select(filename, "-cs\n",
    "#gnb > div.ly_service > div.group_service.NM_FAVORITE_ALL_LY > dl:nth-child(2) > dd > a", &result));

  const char* expected_tokens[] = {"밴드", "부동산", "북마크", "스포츠", "시리즈", "시리즈온"};
  int idx = 0;
  
  const char* token;
  char* save_ptr = NULL;
  while (token = KnuString_tokenize(&result, '\n', &save_ptr)) {
    assert_string_equal(token, expected_tokens[idx++]);
  }

  KnuString_destroy(&result);
}

const struct CMUnitTest* test_knuutils() {
  static const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_asprintf),
    cmocka_unit_test_prestate_setup_teardown(test_copy_file, prepare_file, clean_file, "naver.html"),
    cmocka_unit_test_prestate_setup_teardown(test_gunzip, prepare_file, clean_file, "gzip.jpg.gz"),
    cmocka_unit_test_prestate_setup_teardown(test_html_normalize, prepare_file, clean_file, "naver.html"),
    cmocka_unit_test(test_css_select),
    {NULL, NULL, NULL, NULL, NULL},
  }; 

  return tests;
}
