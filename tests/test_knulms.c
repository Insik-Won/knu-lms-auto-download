#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <dirent.h>
#include <unistd.h>
#include <fnmatch.h>

#include <knulms.h>
#include <test_util.h>

void test_knulms_login_and_logout(void** state) {
  StudentSession* session = knulms_login(getenv("USER_ID"), getenv("PASSWORD"));
  assert_non_null(session);
  assert_string_equal(session->student_number, getenv("STUDENT_NUMBER"));

  int result = knulms_logout(session);
  assert_int_equal(result, 0);
}

void test_knulms_login_expected_failed(void** state) {
  StudentSession* session = knulms_login("Hello World!", "Foo Goo");
  assert_null(session);
}

int select_system_programming(const char* subject_name, int idx, size_t size) {
  int result = fnmatch("시스템*", subject_name, 0);
  return result == 0;
}

int return_false(const char* subject_name, int idx, size_t size) {
  return 0;
}

void test_knulms_select_unselect_subject(void** state) {
  StudentSession* session = knulms_login(getenv("USER_ID"), getenv("PASSWORD"));
  assert_non_null(session);

  SubjectSession* subject_session = knulms_selectFirstSubject(session, select_system_programming);
  assert_non_null(subject_session);
  assert_string_equal(subject_session->subject_name, "시스템프로그래밍");

  session = knulms_unselectSubject(subject_session);
  assert_non_null(session);

  int result = knulms_logout(session);
  assert_int_equal(result, 0);
}

void test_knulms_select_subject_expect_failed(void** state) {
   StudentSession* session = knulms_login(getenv("USER_ID"), getenv("PASSWORD"));
  assert_non_null(session);

  SubjectSession* subject_session = knulms_selectFirstSubject(session, return_false);
  assert_null(subject_session);

  int result = knulms_logout(session);
  assert_int_equal(result, 0);
}

void test_knulms_download_materials(void** state) {
  StudentSession* session = knulms_login(getenv("USER_ID"), getenv("PASSWORD"));
  assert_non_null(session);

  SubjectSession* subject_session = knulms_selectFirstSubject(session, select_system_programming);
  assert_non_null(subject_session);

  char temp_directory[] = "/tmp/tmpXXXXXX";
  assert_non_null(mkdtemp(temp_directory));

  int result = knulms_downloadFirstMaterial(subject_session, temp_directory, NULL, NULL, NULL);
  assert_int_equal(result, 0);

  session = knulms_unselectSubject(subject_session);
  assert_non_null(session);

  result = knulms_logout(session);
  assert_int_equal(result, 0); 

  const char* expected_files[] = {"ch11.zip", "ch14.zip", "11. Connecting to Processes Near and Far Servers and Sockets (1).pdf", "14. Threads - Concurrent Functions.pdf"};
  int found[sizeof(expected_files)/sizeof(*expected_files)] = {0};

  DIR* dir = opendir(temp_directory);
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

    strcpy(full_path, temp_directory);
    strcat(full_path, "/");
    strcat(full_path, entry->d_name);
    assert_int_equal(remove(full_path), 0);
  }
  closedir(dir);
  assert_int_equal(rmdir(temp_directory), 0);

  for (int i = 0; i < 4; i++) {
    assert_true(found[i]);
  }
}

void test_knulms_download_materials_with_pattern(void** state) {
  StudentSession* session = knulms_login(getenv("USER_ID"), getenv("PASSWORD"));
  assert_non_null(session);

  SubjectSession* subject_session = knulms_selectFirstSubject(session, select_system_programming);
  assert_non_null(subject_session);

  char temp_directory[] = "/tmp/tmpXXXXXX";
  assert_non_null(mkdtemp(temp_directory));

  int result = knulms_downloadFirstMaterial(subject_session, temp_directory, NULL, "*.zip", "ch14.*");
  assert_int_equal(result, 0);

  session = knulms_unselectSubject(subject_session);
  assert_non_null(session);

  result = knulms_logout(session);
  assert_int_equal(result, 0); 

  const char* expected_files[] = {"ch11.zip"};
  int found[sizeof(expected_files)/sizeof(*expected_files)] = {0};

  DIR* dir = opendir(temp_directory);
  struct dirent* entry;
  char full_path[FILENAME_MAX];
  while ((entry = readdir(dir))) {
    if (entry->d_type != DT_REG)
      continue;

    for (int i = 0; i < 1; i++) {
      if (strcmp(expected_files[i], entry->d_name) == 0) {
        found[i] = 1;
        break;
      }
    }

    strcpy(full_path, temp_directory);
    strcat(full_path, "/");
    strcat(full_path, entry->d_name);
    assert_int_equal(remove(full_path), 0);
  }
  closedir(dir);
  assert_int_equal(rmdir(temp_directory), 0);

  for (int i = 0; i < 1; i++) {
    assert_true(found[i]);
  }
}

const struct CMUnitTest* test_knulms() {
  static const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_knulms_login_and_logout),
    cmocka_unit_test(test_knulms_login_expected_failed),
    cmocka_unit_test(test_knulms_select_unselect_subject),
    cmocka_unit_test(test_knulms_select_subject_expect_failed),
    cmocka_unit_test(test_knulms_download_materials),
    cmocka_unit_test(test_knulms_download_materials_with_pattern),
    { NULL, NULL, NULL, NULL, NULL}
  };

  return tests;
}
