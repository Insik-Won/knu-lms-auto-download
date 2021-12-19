#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <curl/curl.h>
#include <knuapi.h>

int load_dotenv(const char* directory);
int cmocka_run_group_tests_dynamically(const char* group_name, const struct CMUnitTest* tests, CMFixtureFunction group_setup, CMFixtureFunction group_teardown);

const struct CMUnitTest* test_knustring();
const struct CMUnitTest* test_knuutils();
const struct CMUnitTest* test_knuapi();

int main(int argc, char* argv[]) {
  char buffer[FILENAME_MAX];
  strcpy(buffer, argv[0]);
  *strrchr(buffer, '/') = '\0';
  strcat(buffer, "/resources");

  int result = load_dotenv(buffer);
  setenv("RES_PATH", buffer, 0);

  /*
  test_knuapis();
  return 0;
  /**/

  return cmocka_run_group_tests_dynamically("KnuString test", test_knustring(), NULL, NULL)
          + cmocka_run_group_tests_dynamically("knuutils test", test_knuutils(), NULL, NULL)
          + cmocka_run_group_tests_dynamically("knuapi test", test_knuapi(), NULL, NULL);
}
