#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <dirent.h>
#include <unistd.h>

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

  const char* expected_names[] = {"???????????????", "????????????????????????", "????????????????????????", "????????????", "??????????????????", "???????????????????????????", "???????????????????????????", "2021??? ??????????????? ?????????????????? ??????(??????)"};
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

  const char* expected_ids[] = {"6339205", "6294573", "6248637", "6206008", "6177258", "6132759", "6041723", "5989992", "5940093", "5905018", "5867069", "5837296", };
  const char* expected_titles[] = {"Connecting to Processes, Threads", "IO Redirection and Pipes", "A Programmable Shell", "Processes and Programs - Studying sh", "Event-Driven Programming (II)", "Event-Driven Programming (I)", "Terminal Control and Signals", "Connection Control : stty", "File Systems : writing pwd", "Directories and File Properties - Looking through ls", "Users, Files, and the Manual", "Unix System Programming - The Big Picture", };

  Material* materials = NULL;
  size_t size = 0;
  knuapi_get_materials(getenv("STUDENT_NUMBER"), prepared_key, &materials, &size, cookie);

  for (int i = 0; i < size; i++) {
   assert_string_equal(expected_ids[i], materials[i].id.value);
   assert_string_equal(expected_titles[i], materials[i].title.value);
  }

  for (int i = 0; i < size; i++) {
    KnuString_destroy(&materials[i].id);
    KnuString_destroy(&materials[i].title);
  }
  free(materials);
}

void test_knuapi_donload_material(void** state) {
  const char* cookie = (const char*)*state;

  const char* prepared_subject_key = "A20213ELEC462003";
  const char* prepared_material_id = "6339205";
  const char* expected_files[] = {"ch11.zip", "ch14.zip", "11. Connecting to Processes Near and Far Servers and Sockets (1).pdf", "14. Threads - Concurrent Functions.pdf"};
  int found[sizeof(expected_files)/sizeof(*expected_files)] = {0};

  char tmp_filename[] = "/tmp/tmpXXXXXX";
  assert_int_equal(make_tmpfile(tmp_filename), 0);

  int result = knuapi_download_material_files(tmp_filename, prepared_material_id, prepared_subject_key, getenv("STUDENT_NUMBER"), cookie);
  assert_int_equal(result, 0);

  //assert_filename_equal(tmp_filename, expected_filename); it differs by time ...

  char tmp_directory[] = "/tmp/tmpXXXXXX";
  mkdtemp(tmp_directory);

  assert_true(unzip(tmp_filename, tmp_directory, NULL, NULL));

  DIR* dir = opendir(tmp_directory);
  struct dirent* entry;
  char full_path[FILENAME_MAX];
  while ((entry = readdir(dir))) {
    if (entry->d_type != DT_REG)
      continue;

    for (int i = 0; i < 4; i++) {
      if (strcmp(expected_files[i], entry->d_name) == 0) {
        found[i] = 1;
        break;
      }
    }

    strcpy(full_path, tmp_directory);
    strcat(full_path, "/");
    strcat(full_path, entry->d_name);
    assert_int_equal(remove(full_path), 0);
  }
  closedir(dir);
  assert_int_equal(rmdir(tmp_directory), 0);

  for (int i = 0; i < 4; i++) {
    assert_true(found[i]);
  }

  remove(tmp_filename);
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
    cmocka_unit_test_prestate_setup_teardown(test_knuapi_donload_material, prepare_user_and_subject, clean_user_and_cookie, "A20213ELEC462003"),
    {NULL, NULL, NULL, NULL, NULL},
  }; 

  return tests;
}