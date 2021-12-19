#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <knuapi.h>
#include <test_util.h>

void test_knuapi_http_request(void** state) {
  char** filenames = (char**)*state;
  char* original_filename = filenames[0];
  char* temp_filename = filenames[1];

  HttpRequestOptions options;
  memset(&options, 0, sizeof(options));
  options.headers = curl_slist_append(options.headers, "Accept-Encoding: gzip");

  int result = knuapi_http_request("https://lms.knu.ac.kr/ilos/main/main_form.acl", &options, temp_filename, NULL);
  assert_int_equal(result, 0);

  assert_filename_equal(original_filename, temp_filename);

  curl_slist_free_all(options.headers);
}

int prepare_user_and_cookie(void** state) {
  char* cookie_filename = strdup("/tmp/cookieXXXXXX");
  if (make_tmpfile(cookie_filename) != 0) {
    free(cookie_filename);
    return -1;
  }

  int result = knuapi_login(getenv("USER_ID"), getenv("PASSWORD"), cookie_filename);
  if (result != 0) {
    remove(cookie_filename);
    free(cookie_filename);
    return -1;
  }

  *state = (void*)cookie_filename;

  return 0;
}

int prepare_user_and_subject(void** state) {
  char* subject_key = (char*)*state;

  int result = prepare_user_and_cookie(state);
  if (result != 0)
    return -1;

  char* cookie = (char*)*state;

  result = knuapi_set_subject(subject_key, cookie);
  if (result != 0)
    return -1;

  return 0;
}

int clean_user_and_cookie(void** state) {
  char* cookie_filename = (char*)*state;

  knuapi_logout(cookie_filename);

  remove(cookie_filename);
  free(cookie_filename);

  return 0;
}

void test_knuapi_login(void** state) {
  char** filenames = (char**)*state;
  char* temp_filename = filenames[1];

  free(filenames);
  *state = (void*)temp_filename;

  int result = knuapi_login(getenv("USER_ID"), getenv("PASSWORD"), temp_filename);
  assert_int_equal(result, 0);
}

void test_knuapi_logout(void** state) {
  const char* cookie = (const char*)*state;

  int result = knuapi_logout(cookie);
  assert_int_equal(result, 0);
}

void test_knuapi_get_student_number(void** state) {
  const char* cookie = (const char*)*state;

  KnuString str;
  KnuString_init(&str);
  int result = knuapi_get_student_number(&str, cookie);
  assert_int_equal(result, 0);
  assert_string_equal(str.value, getenv("STUDENT_NUMBER"));
}

void test_knuapi_get_subjects(void** state) {
  const char* cookie = (const char*)*state;

  const char* expected_names[] = {" 대학글쓰기 (001) ", " 서양의역사와문화 (007) ", " 시스템프로그래밍 (003) ", " 자료구조 (011) ", " 자료구조응용 (003) ", " 자료구조프로그래밍 (002) ", " 컴퓨터망프로그래밍 (001) ", " 2021년                  경북대학교 장애인식개선 교육(학생) (39) "};
  const char* expected_keys[] = {"A20213CLTR205001", "A20213CLTR043007", "A20213ELEC462003", "A20213COME331011", "A20213COMP216003", "A20213ITEC423002", "A20213GLSO217001", "N2021B2021sup3939"};

  KnuString* subject_keys = NULL;
  KnuString* subject_names = NULL;
  size_t size = 0;
  knuapi_get_subject(&subject_names, &subject_keys, &size, cookie);

  for (int i = 0; i < size; i++) {
    assert_string_equal(subject_keys[i].value, expected_keys[i]);
  }
  for (int i = 0; i <size; i++) {
    assert_string_equal(subject_names[i].value, expected_names[i]);
  }

  for (int i = 0; i < size; i++) {
    KnuString_destroy(&subject_keys[i]);
    KnuString_destroy(&subject_names[i]);
  }
  free(subject_keys);
  free(subject_names);
}

void test_knuapi_set_subject(void** state) {
  const char* cookie = (const char*)*state;
  const char* prepared_key = "A20213ELEC462003";
  
  int result = knuapi_set_subject(prepared_key, cookie);
  assert_int_equal(result, 0);
}

void test_knuapi_get_materials(void** state) {
  const char* cookie = (const char*)*state;
  const char* prepared_key = "A20213ELEC462003";

  Material* materials = NULL;
  size_t size = 0;
  knuapi_get_materials(getenv("STUDENT_NUMBER"), prepared_key, &materials, &size, cookie);

  for (int i = 0; i < size; i++) {
    printf("id[%d] : %s\n", i, materials[i].id.value);
    printf("title[%d] : %s\n", i, materials[i].title.value);
    printf("files_id[%d] : %s\n", i, materials[i].files_id.value);
  }

  for (int i = 0; i < size; i++) {
    KnuString_destroy(&materials[i].id);
    KnuString_destroy(&materials[i].title);
    KnuString_destroy(&materials[i].files_id);
  }
  free(materials);
}

const struct CMUnitTest* test_knuapi() {
  static const struct CMUnitTest tests[] = {
    //cmocka_unit_test_prestate_setup_teardown(test_knuapi_http_request, prepare_file, clean_file, "knulms_main.html"),
    cmocka_unit_test_setup_teardown(test_knuapi_login, prepare_file, clean_user_and_cookie),
    cmocka_unit_test_setup_teardown(test_knuapi_logout, prepare_user_and_cookie, clean_user_and_cookie),
    cmocka_unit_test_setup_teardown(test_knuapi_get_student_number, prepare_user_and_cookie, clean_user_and_cookie),
    cmocka_unit_test_setup_teardown(test_knuapi_get_subjects, prepare_user_and_cookie, clean_user_and_cookie),
    cmocka_unit_test_setup_teardown(test_knuapi_set_subject, prepare_user_and_cookie, clean_user_and_cookie),
    cmocka_unit_test_prestate_setup_teardown(test_knuapi_get_materials, prepare_user_and_subject, clean_user_and_cookie, "A20213ELEC462003"),
    {NULL, NULL, NULL, NULL, NULL},
  }; 

  return tests;
}