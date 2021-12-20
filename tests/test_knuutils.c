#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <unistd.h>
#include <dirent.h>

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

void test_unzip(void** state) {
  char** filenames = (char**)*state;
  char* temp_filename = filenames[1];

  char directory[] = "/tmp/tmpXXXXXX";
  mkdtemp(directory);

  const char* unwelcomes[] = {"11. Connecting to Process Near and Far Servers and Sockets (1).pdf", "14. Threads - Concurrent Functions.pdf", "ch14.zip"};
  const char* welcomes[] = {"ch11.zip"};

  unzip(temp_filename, directory, "*.zip", "ch14.zip");

  DIR* dir = opendir(directory);
  struct dirent* entry;
  char full_path1[sizeof(directory) + NAME_MAX + 1];
  char full_path2[sizeof(directory) + NAME_MAX + 1];

  while ((entry = readdir(dir))) {
    if (entry->d_type == DT_DIR)
      continue;

    for (int i = 0; i < sizeof(unwelcomes)/sizeof(*unwelcomes); i++)
      assert_string_not_equal(entry->d_name, unwelcomes[i]);
    
    int matched = -1;
    for (int i = 0; i < sizeof(welcomes)/sizeof(*welcomes); i++) {
      if (strcmp(entry->d_name, welcomes[i]) == 0) {
        matched = i;
        break;
      }
    }
    assert_int_not_equal(matched, -1);
    
    strcpy(full_path1, directory);
    strcpy(full_path2, getenv("RES_PATH"));
    strcat(full_path1, "/");
    strcat(full_path2, "/");
    strcat(full_path1, entry->d_name);
    strcat(full_path2, entry->d_name);
    assert_filename_equal(full_path1, full_path2);
  }
}

const struct CMUnitTest* test_knuutils() {
  static const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_asprintf),
    cmocka_unit_test_prestate_setup_teardown(test_copy_file, prepare_file, clean_file, "naver.html"),
    cmocka_unit_test_prestate_setup_teardown(test_gunzip, prepare_file, clean_file, "gzip.jpg.gz"),
    cmocka_unit_test_prestate_setup_teardown(test_html_normalize, prepare_file, clean_file, "naver.html"),
    cmocka_unit_test(test_css_select),
    cmocka_unit_test_prestate_setup_teardown(test_unzip, prepare_file, clean_file, "materials.zip"),
    {NULL, NULL, NULL, NULL, NULL},
  }; 

  return tests;
}
