#include <knulms.h>
#include <knuutils.h>
#include <knuapi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <curl/curl.h>
#include <json.h>



/*
 * initate global knulms library
 */
void knulms_global_init() {
  knuapi_global_init();
  mkdir("/tmp/knulms", 0777);
}
/*
 * clean up global knulms library
 */
void knulms_global_cleanup() {
  knuapi_global_cleanup();
  rmdir("/tmp/knulms");
}


/*
 * used to append KnuString in curl writing process 
 */
static size_t append_to_knustr(void* data, size_t size, size_t nmemb, void* knustr) {
  size_t whole_size = size * nmemb;
  
  KnuString* str = (KnuString*)knustr;
  KnuString_appendCharArray(str, data, whole_size);

  return whole_size;
}

/*
 * used to write to File in curl writing process
 */
static size_t write_to_file(void* data, size_t size, size_t nmemb, void* f) {
  FILE* file = (FILE*)f;
  fwrite(data, size, nmemb, file);
  return size * nmemb;
}


/* 
 * used for login process in knulms_login
 * - @return 1 if succeed, otherwise, 0.
 */
static int login(const char* id, const char* password, const char* cookie_filename) {
  int ret = 1;

  CURL* curl = curl_easy_init();
  if (!curl)
    return 0;
  
  // set cookie and url
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookie_filename);
  curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookie_filename);
  curl_easy_setopt(curl, CURLOPT_URL, GET_KNULMS_URL("/ilos/lo/login.acl"));

  // add headers
  struct curl_slist* headers = NULL;
  headers = curl_slist_append(headers, "Connection: keep-alive");
  headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
  headers = curl_slist_append(headers, "Accept: */*");
  headers = curl_slist_append(headers,  "Accept-Encoding: gzip");
  headers = curl_slist_append(headers,  "Accept-Language: ko-KR,ko;q=0.9");
  headers = curl_slist_append(headers,  "Host: lms.knu.ac.kr");
  headers = curl_slist_append(headers,  "Origin: https://lms.knu.ac.kr");
  headers = curl_slist_append(headers,  "Referer: https://lms.knu.ac.kr/ilos/main/member/login_form.acl");
  headers = curl_slist_append(headers,  "\"sec-ch-ua: \" Not A;Brand\";v=\"99\", \"Chromium\";v=\"96\", \"Google Chrome\";v=\"96\"");
  headers = curl_slist_append(headers,  "sec-ch-ua-mobile: ?0");
  headers = curl_slist_append(headers,  "sec-ch-ua-platfrom: \"Windows\"");
  headers = curl_slist_append(headers,  "Sec-Fetch-Dest: empty");
  headers = curl_slist_append(headers,  "Sec-Fetch-Mode: cors");
  headers = curl_slist_append(headers,  "Sec-Fetch-Site: same-origin");
  headers = curl_slist_append(headers,  "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/96.0.4664.93 Safari/537.36");
  headers = curl_slist_append(headers,  "X-Requested-With: XMLHttpRequest");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  // add request body(data)
  char* data = asprintf("usr_id=%s&usr_pwd=%s&encoding=utf-8", id, password);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

  // add writer callback for response's body
  KnuString body;
  KnuString_init(&body);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&body);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, append_to_knustr);

  // response and check
  CURLcode res = curl_easy_perform(curl);  
  if (res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    ret = 0;
    goto cleanup_login;
  }
  long response_code;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
  if (response_code != 200) {
    fprintf(stderr, "the reponse code is %ld: failed\n", response_code);
    ret = 0;
    goto cleanup_login;
  }

  json_object* json = json_tokener_parse(body.value);
  json_object* is_error = json_object_object_get(json, "isError");
  if (json_object_get_boolean(is_error)) {
    fprintf(stderr, "the response, isError is true\n");
    ret = 0;
  }

  json_object_put(json);

cleanup_login:
  KnuString_destroy(&body);
  curl_slist_free_all(headers);
  free(data);

  curl_easy_cleanup(curl);  

  return ret;
}

static char* get_student_number(const char* cookie_filename) {
  char* ret = NULL;

  CURL* curl = curl_easy_init();
  if (!curl)
    return NULL;
  
  // set cookie and url
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookie_filename);
  curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookie_filename);
  curl_easy_setopt(curl, CURLOPT_URL, GET_KNULMS_URL("/ilos/mp/myinfo_form.acl"));

  // add headers
  struct curl_slist* headers = NULL;
  headers = curl_slist_append(headers, "Connection: keep-alive");
  headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9");
  headers = curl_slist_append(headers,  "Accept-Encoding: gzip");
  headers = curl_slist_append(headers,  "Accept-Language: ko-KR,ko;q=0.9");
  headers = curl_slist_append(headers,  "Host: lms.knu.ac.kr");
  headers = curl_slist_append(headers,  "Origin: https://lms.knu.ac.kr");
  headers = curl_slist_append(headers,  "Referer: https://lms.knu.ac.kr/ilos/main/mp/myinfo_form.acl");
  headers = curl_slist_append(headers,  "\"sec-ch-ua: \" Not A;Brand\";v=\"99\", \"Chromium\";v=\"96\", \"Google Chrome\";v=\"96\"");
  headers = curl_slist_append(headers,  "sec-ch-ua-mobile: ?0");
  headers = curl_slist_append(headers,  "sec-ch-ua-platfrom: \"Windows\"");
  headers = curl_slist_append(headers,  "Sec-Fetch-Dest: document");
  headers = curl_slist_append(headers,  "Sec-Fetch-Mode: navigate");
  headers = curl_slist_append(headers,  "Sec-Fetch-Site: same-origin");
  headers = curl_slist_append(headers,  "Sec-Fetch-User: ?1");
  headers = curl_slist_append(headers,  "Upgrade-Insecure-Requests: 1");
  headers = curl_slist_append(headers,  "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/96.0.4664.93 Safari/537.36");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  // add writer callback for response's body
  char result_filename[] = "/tmp/knulms/resultXXXXXX.gz";
  int result_fd = mkstemps(result_filename,3);
  FILE* result_file = fdopen(result_fd, "w");
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)result_file);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_file);

  // response and check
  CURLcode res = curl_easy_perform(curl);  
  if (res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    ret = NULL;
    fclose(result_file);
    goto cleanup_get_student_number;
  }

  fclose(result_file);
  
  long response_code;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
  if (response_code != 200) {
    fprintf(stderr, "the reponse code is %ld: failed\n", response_code);
    ret = NULL;
    goto cleanup_get_student_number;
  }

cleanup_get_student_number:
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);  

  return ret;
}


/*
 * login to Kyungpook lms and return StudentSession. use it to other functions.
 * - !! todo set errno to identify the error
 * - @return nonNull object if succeed, otherwise, null.
 */
StudentSession* knulms_login(const char* id, const char* password) {
  char cookie_filename[] = "/tmp/knulms/cookieXXXXXX";
  int fd = mkstemp(cookie_filename);
  close(fd);
  
  int result = login(id, password, cookie_filename);
  if (!result)
    return NULL;

  get_student_number(cookie_filename);

  return (StudentSession*)1;
}
/*
 * select a Subject which predicate says true and return SubjectSession. use it to other functions.
 * - student_session loses the ownership of its members. do not destroy student_session.
 * - todo: set errno to identify the error
 * - @return nonNull object if succeed, otherwise, null
 */
SubjectSession* knulms_selectFirstSubject(StudentSession* student_session, int (*subject_predicate)(const char* subject_name, int idx, size_t size));
/*
 * download the files, which file_predicate says true, in the post which post_predicate says true. 
 * - todo: set errno to identify the error
 * - @return 0 if succeed, otherwise, non-zero value.
 */ 
int knulms_downloadFirstPost(SubjectSession* subject_session, const char* path, 
                              int (*post_predicate)(const char* postName, int idx, size_t size), 
                              int (*file_predicate)(const char* filename, int idx, size_t size));
/*
 * cancel the selection of subject.
 * call this before logout.
 * - @return nonNull object if succeed, otherwise null
 */
StudentSession* knulms_unselectSubject(SubjectSession* session);
/*
 * logout in Kyungpook lms and destroy session.
 * @return nonNull object if succeed, otherwise, null.
 */
int knulms_logout(StudentSession* session);
