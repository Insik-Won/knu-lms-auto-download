#include "knuapi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <unistd.h>

#include <curl/curl.h>
#include <json.h>
#include <knustring.h>
#include <knuutils.h>
#include <regex.h>
#include <sys/types.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>

#define TYPE1_ACCEPT "Accept: */*"
#define TYPE1_SEC_FETCH_DEST "Sec-Fetch-Dest: empty"
#define TYPE1_SEC_FETCH_MODE "Sec-Fetch-Mode: cors"
#define TYPE1_X_REQUESTED_WITH "X-Requested-With: XMLHttpRequest"

#define TYPE2_ACCEPT "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9"
#define TYPE2_SEC_FETCH_DEST "Sec-Fetch-Dest: document"
#define TYPE2_SEC_FETCH_MODE "Sec-Fetch-Mode: navigate"
#define TYPE2_SEC_FETCH_USER "Sec-Fetch-User: ?1"
#define TYPE2_UPGRADE_INSECURE_REQUESTS "Upgrade-Insecure-Requests: 1"

#define LOGIN_URL "https://lms.knu.ac.kr/ilos/lo/login.acl"
#define LOGIN_REFERER "https://lms.knu.ac.kr/ilos/main/member/login_form.acl"
#define LOGOUT_URL "https://lms.knu.ac.kr/ilos/lo/logout.acl"
#define LOGOUT_REFERER "https://lms.knu.ac.kr/ilos/st/course/lecture_material_list_form.acl"
#define STUDENT_NUMBER_URL "https://lms.knu.ac.kr/ilos/mp/myinfo_form.acl"
#define STUDENT_NUMBER_REFERER "https://lms.knu.ac.kr/ilos/mp/myinfo_form.acl"
#define MAIN_URL "https://lms.knu.ac.kr/ilos/main/main_form.acl"
#define MAIN_REFERER "https://lms.knu.ac.kr/ilos/main/member/login_form.acl"
#define SET_SUBJECT_URL "https://lms.knu.ac.kr/ilos/st/course/eclass_room2.acl"
#define SET_SUBJECT_REFERER "https://lms.knu.ac.kr/ilos/main/member/login_form.acl"
#define GET_MATERIAL_URL "https://lms.knu.ac.kr/ilos/st/course/lecture_material_list.acl"
#define GET_MATERIAL_REFERER "https://lms.knu.ac.kr/ilos/st/course/lecture_material_list_form.acl"


void knuapi_global_init() {
  curl_global_init(CURL_GLOBAL_DEFAULT);
}
void knuapi_global_cleanup() {
  curl_global_cleanup();
}

static struct curl_slist* append_default_headers(struct curl_slist* headers) {
  headers = curl_slist_append(headers, "Connection: keep-alive");
  headers = curl_slist_append(headers, "Accept-Language: ko-KR");
  headers = curl_slist_append(headers, "Host: lms.knu.ac.kr");
  headers = curl_slist_append(headers, "\"sec-ch-ua: \" Not A;Brand\";v=\"99\", \"Chromium\";v=\"96\", \"Google Chrome\";v=\"96\"");
  headers = curl_slist_append(headers, "sec-ch-ua-mobile: ?0");
  headers = curl_slist_append(headers, "sec-ch-ua-platform: \"Windows\"");
  headers = curl_slist_append(headers, "Sec-Fetch-Site: same-origin");
  headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/96.0.4664.93 Safari/537.36");

  return headers;
}

static struct curl_slist* append_type1_headers(struct curl_slist* headers) {
  headers = curl_slist_append(headers, TYPE1_ACCEPT);
  headers = curl_slist_append(headers, TYPE1_SEC_FETCH_DEST);
  headers = curl_slist_append(headers, TYPE1_SEC_FETCH_MODE);
  headers = curl_slist_append(headers, TYPE1_X_REQUESTED_WITH);

  return headers;
}

static struct curl_slist* append_type2_headers(struct curl_slist* headers) {
  headers = curl_slist_append(headers, TYPE2_ACCEPT);
  headers = curl_slist_append(headers, TYPE2_SEC_FETCH_DEST);
  headers = curl_slist_append(headers, TYPE2_SEC_FETCH_MODE);
  headers = curl_slist_append(headers, TYPE2_SEC_FETCH_USER);
  headers = curl_slist_append(headers, TYPE2_UPGRADE_INSECURE_REQUESTS);

  return headers;
}

static int is_error_json(const char* ouput_filename) {
  int return_value = 1;
  KnuString str;
  KnuString_init(&str);
  int result = KnuString_readAllFromFilename(&str, ouput_filename);
  
  if (result) {
    json_object* json = json_tokener_parse(str.value);
    json_object* is_error = json_object_object_get(json, "isError");
    if (json_object_get_boolean(is_error)) {
      return_value = 1;
    }
    else {
      return_value = 0;
    }
    json_object_put(json);
  }

  KnuString_destroy(&str);
  return return_value;
}

static void normazlie_html(const char* output_filename) {
  char tmp_filename[] = "/tmp/tmpXXXXXX";
  make_tmpfile(tmp_filename);
  html_normalize(output_filename, tmp_filename);
  copy_file(tmp_filename, output_filename);
  remove(tmp_filename);
}

static size_t detect_content_encoding(char* buffer, size_t size, size_t nitems, void* userdata) {
  KnuString* str = (KnuString*)userdata;
  (void)str;

  char name_buf[256];
  char value_buf[256];
  sscanf(buffer, "%255[^:] : %255[^\r\n]", name_buf, value_buf);
  
  if (strcmp(name_buf, "Content-Encoding") == 0) {
    KnuString_empty(str);
    KnuString_appendString(str, value_buf);
  }

  return size * nitems;
}

static size_t write_to_file(char* buffer, size_t size, size_t nitems, void* userdata) {
  FILE* file = (FILE*)userdata;
  if (file) 
    fwrite(buffer, size, nitems, file);
  return size * nitems;
}

/**
 * @brief send http request and save result to output_file.
 * 
 * @param url the url to send request
 * @param 
 * @param data_format the format of post data. the following arguments will be got into the format. if null, send request as GET
 * @return 0 if succeed, else non-zero value.
 */
int knuapi_http_request(const char* url, const HttpRequestOptions* options, const char* output_filename, const char* data_format, ...) {
  int return_value = 0;

  CURL* curl = curl_easy_init();
  char* postdata = NULL;
  if (!curl)
    return -1;

  //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(curl, CURLOPT_URL, url);
  if (options->headers)
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, options->headers);
  if (options->referer)
    curl_easy_setopt(curl, CURLOPT_REFERER, options->referer);
  if (options->cookie_file) {
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, options->cookie_file);
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, options->cookie_file);
  }
  if (options->follow_redirects) {
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
  }

  if (data_format) {
    va_list args;
    va_start(args, data_format);
    postdata = vasprintf(data_format, args);
    va_end(args);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(postdata) + 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
  }

  KnuString encoding;
  KnuString_init(&encoding);
  FILE* output = NULL;
 
  if (output_filename) { 
    output = fopen(output_filename, "w");
    if (!output) {
      return_value = -1;
      goto cleanup_knuapi_http_request;
    }

    curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*)&encoding);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, detect_content_encoding);
    curl_easy_setopt(curl ,CURLOPT_WRITEDATA, (void*)output);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_file);
  }
  else {
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_file);
  }

  CURLcode code = curl_easy_perform(curl);  
  if (code != CURLE_OK && code != CURLE_RECV_ERROR) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(code));
    return_value = code;
    goto cleanup_knuapi_http_request;
  }

  if (output_filename) {
    fclose(output);
    output = NULL;
  }

  long status_code;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
  if (400 <= status_code && status_code < 600) {
    fprintf(stderr, "the status code is %ld: failed\n", status_code);
    return_value = status_code;
    goto cleanup_knuapi_http_request;
  }

  if (output_filename == NULL) {
    // pass
  }
  else if (KnuString_isEmpty(&encoding)) {
    // pass
  } 
  else if (strcmp(encoding.value, "gzip") == 0) {
    char temp_filename[] = "/tmp/tmpXXXXXX";
    make_tmpfile(temp_filename);
    gunzip(output_filename, temp_filename);
    copy_file(temp_filename, output_filename);
  }
  else {
    fprintf(stderr, "unsupported Content Encoding: %s\n", encoding.value);
    return_value = -1;
    goto cleanup_knuapi_http_request;
  }

cleanup_knuapi_http_request:
  if (output)
    fclose(output);
  if (postdata)
    free(postdata);
  curl_easy_cleanup(curl);

  return return_value;
}

/** 
 * @brief login with id and password, and save the cookie to file.
 * 
 * @param user_id the user's id.
 * @param password the user's password.
 * @param cookie_filename the name of cookie that will contain cookie from libcurl
 * @return 0 if succeed, otherwise, non-zero value.
 */
int knuapi_login(const char* user_id, const char* password, const char* cookie_filename) {
  int return_value = 0;

  HttpRequestOptions options;
  memset(&options, 0, sizeof(options));
  
  options.headers = append_default_headers(options.headers);
  options.headers = append_type1_headers(options.headers);

  options.cookie_file = cookie_filename;
  options.referer = LOGIN_REFERER;
  options.follow_redirects = 0L;

  char output_filename[] = "/tmp/tmpXXXXXX";
  if (make_tmpfile(output_filename) != 0) {
    return_value = -1;
    goto cleanup_knuapi_login;
  }

  int result = knuapi_http_request(LOGIN_URL, &options, output_filename, "usr_id=%s&usr_pwd=%s&encoding=utf-8", user_id, password);
  if (result != 0) {
    return_value = -1;
    goto cleanup_knuapi_login;
  }

  if (is_error_json(output_filename)) {
    return_value = -1;
    goto cleanup_knuapi_login;
  }

cleanup_knuapi_login:
  if (options.headers) {
    curl_slist_free_all(options.headers);
  }
  remove(output_filename);

  return return_value;
}

/**
 * @brief logout the user in cookie, and save the cookie to file.
 * 
 * @param cookie_filename the name of file that contains coookie from libcurl
 * @return 0 if succeed, otherwise, non-zero value.
 */
int knuapi_logout(const char* cookie_filename) {
   int return_value = 0;

  HttpRequestOptions options;
  memset(&options, 0, sizeof(options));
  
  options.headers = append_default_headers(options.headers);
  options.headers = append_type2_headers(options.headers);

  options.cookie_file = cookie_filename;
  options.referer = LOGOUT_REFERER;
  options.follow_redirects = 1L;

  knuapi_http_request(LOGOUT_URL, &options, NULL, NULL);

  if (options.headers) {
    curl_slist_free_all(options.headers);
  }

  return return_value; 
}

/**
 * @brief get the student number with cookie
 * 
 * @param student_number a KnuString that will contain the student number
 * @param cookie_filename the name of file that conatins cookie from libcurl
 * @return 0 if succeed, otherwise, non-zero value.
 */
int knuapi_get_student_number(KnuString* student_number, const char* cookie_filename) {
  int return_value = 0;

  KnuString privacy;
  KnuString_init(&privacy);

  HttpRequestOptions options;
  memset(&options, 0, sizeof(options));
  
  options.headers = append_default_headers(options.headers);
  options.headers = append_type2_headers(options.headers);

  options.cookie_file = cookie_filename;
  options.referer = STUDENT_NUMBER_REFERER;
  options.follow_redirects = 0L;

  char output_filename[] = "/tmp/tmpXXXXXX";
  if (make_tmpfile(output_filename) != 0) {
    return_value = -1;
    goto cleanup_knuapi_get_student_number;
  }

  int result = knuapi_http_request(STUDENT_NUMBER_URL, &options, output_filename, NULL);
  if (result != 0) {
    return_value = -1;
    goto cleanup_knuapi_get_student_number;
  }

  normazlie_html(output_filename);
  result = css_select(output_filename, "-c", "#content_text > div:nth-child(1) > div:nth-child(6) > table > tbody > tr:nth-child(1) > td:nth-child(2)", &privacy);

  if (!result) {
    return_value = -1;
    goto cleanup_knuapi_get_student_number;
  }

  regex_t regex;
  const char* pattern = "[0-9]+";
  regcomp(&regex, pattern, REG_EXTENDED);  

  regmatch_t match[1];
  size_t nmatch =1;
  int status = regexec(&regex, privacy.value, nmatch, match, 0);
  (void)status;

  KnuString_substring(&privacy, match->rm_so, match->rm_eo, student_number);

  regfree(&regex);

cleanup_knuapi_get_student_number:
  if (options.headers) {
    curl_slist_free_all(options.headers);
  }
  remove(output_filename);

  return return_value;
}

/**
 * @brief get the subject names and keys with cookie
 * 
 * @param subject_name_list a pointer that will conatin subject names, which is a NULL-terminated array. free this after using
 * @param subject_key_list a pointer that will conatin subject keys, which is a NULL-terminated array. free this after using.
 * @param cookie_filename the name of file that contains coookie from libcurl
 * @return 0 if succeed, otherwise, non-zero value.
 */
int knuapi_get_subject(KnuString** subject_name_list, KnuString** subject_key_list, size_t* size, const char* cookie_filename) {
  int return_value = 0;

  KnuString subjects;
  KnuString_init(&subjects);

  HttpRequestOptions options;
  memset(&options, 0, sizeof(options));
  
  options.headers = append_default_headers(options.headers);
  options.headers = curl_slist_append(options.headers, TYPE1_SEC_FETCH_DEST);
  options.headers = curl_slist_append(options.headers, TYPE1_SEC_FETCH_MODE);
  options.headers = curl_slist_append(options.headers, TYPE2_SEC_FETCH_USER);
  options.headers = curl_slist_append(options.headers, TYPE2_UPGRADE_INSECURE_REQUESTS);

  options.cookie_file = cookie_filename;
  options.referer = MAIN_REFERER;
  options.follow_redirects = 0L;

  char output_filename[] = "/tmp/tmpXXXXXX";
  if (make_tmpfile(output_filename) != 0) {
    return_value = -1;
    goto cleanup_knuapi_get_student_number;
  }

  int result = knuapi_http_request(MAIN_URL, &options, output_filename, NULL);
  if (result != 0) {
    return_value = -1;
    goto cleanup_knuapi_get_student_number;
  }

  normazlie_html(output_filename);
  css_select(output_filename, "-s\n", "#contentsIndex > div.index-leftarea02 > div:nth-child(2) > ol > li > em", &subjects);
  
  KnuString* subject_list = NULL;
  int count = KnuString_tokenizeToArray(&subjects, '\n', &subject_list);

  KnuString* name_list = malloc(sizeof(*name_list) * (count + 1));
  KnuString* key_list = malloc(sizeof(*key_list) * (count + 1));

  xmlDocPtr doc = NULL;
  xmlNodePtr element = NULL;
  for (int i = 0; i < count; i++) {
    doc = xmlParseDoc((xmlChar*)subject_list[i].value);
    element = xmlDocGetRootElement(doc);
    xmlChar* kj = xmlGetProp(element, (xmlChar*)"kj");
    xmlChar* innerText = xmlNodeGetContent(element);
    
    KnuString_init(&name_list[i]);
    KnuString_init(&key_list[i]);
    KnuString_appendString(&name_list[i], (char*)innerText);
    KnuString_appendString(&key_list[i], (char*)kj);

    xmlFree(kj);
    xmlFree(innerText);
    xmlFreeDoc(doc);
  }

  *subject_name_list = name_list;
  *subject_key_list = key_list;
  *size = count;

  for (int i = 0; i < count; i++) {
    KnuString_destroy(&subject_list[i]);
  }
  free(subject_list);
  KnuString_destroy(&subjects);

cleanup_knuapi_get_student_number:
  if (options.headers) {
    curl_slist_free_all(options.headers);
  }
  remove(output_filename);

  return return_value;
}


/**
 * @brief set the subject of the user in cookie, and save it to the cookie of file.
 * 
 * @param subject_key the subject key
 * @param cookie_filename the name of file that contains coookie from libcurl
 * @return 0 if succeed, otherwise, non-zero value.
 */
int knuapi_set_subject(const char* subject_key, const char* cookie_filename) {
  int return_value = 0;

  HttpRequestOptions options;
  memset(&options, 0, sizeof(options));
  
  options.headers = append_default_headers(options.headers);
  options.headers = append_type1_headers(options.headers);

  options.cookie_file = cookie_filename;
  options.referer = SET_SUBJECT_REFERER;
  options.follow_redirects = 0L;

  char output_filename[] = "/tmp/tmpXXXXXX";
  if (make_tmpfile(output_filename) != 0) {
    return_value = -1;
    goto cleanup_knuapi_login;
  }

  int result = knuapi_http_request(SET_SUBJECT_URL, &options, output_filename, "KJKEY=%s&returnData=json&returnURI=/ilos/st/course/submain_form.acl&encoding=utf-8", subject_key);

  if (result != 0) {
    return_value = -1;
    goto cleanup_knuapi_login;
  }

  if (is_error_json(output_filename)) {
    return_value = -1;
    goto cleanup_knuapi_login;
  }

cleanup_knuapi_login:
  if (options.headers) {
    curl_slist_free_all(options.headers);
  }
  remove(output_filename);

  return return_value;
}

/**
 * @brief get the material names and ids with cookie
 * 
 * @param material_name_list a pointer to conatin material names, which is a NULL-terminated array. free this after using
 * @param material_id_list a pointer to conatin material ids, which is a NULL-terminated array. free this after using.
 * @param size the array size
 * @param cookie_filename the name of file that contains coookie from libcurl
 * @return 0 if succeed, otherwise, non-zero value.
 */
int knuapi_get_materials(const char* student_number, const char* subject_key, Material** material_list, size_t* size, const char* cookie_filename) {
  int return_value = 0;

  KnuString ids, titles, files_ids;
  KnuString_init(&titles);
  KnuString_init(&files_ids);
  KnuString_init(&ids);

  HttpRequestOptions options;
  memset(&options, 0, sizeof(options));
  
  options.headers = append_default_headers(options.headers);
  options.headers = append_type1_headers(options.headers);

  options.cookie_file = cookie_filename;
  options.referer = GET_MATERIAL_REFERER;
  options.follow_redirects = 0L;

  char output_filename[] = "/tmp/tmpXXXXXX";
  if (make_tmpfile(output_filename) != 0) {
    return_value = -1;
    goto cleanup_knuapi_get_student_number;
  }

  int result = knuapi_http_request(GET_MATERIAL_URL, &options, output_filename, "ud=%s&ky=%s&start=&display=1&SCH_VALUE=&encoding=utf-8", student_number, subject_key);
  if (result != 0) {
    return_value = -1;
    goto cleanup_knuapi_get_student_number;
  }

  normazlie_html(output_filename);
  css_select(output_filename, "-s\n", "body > table > tbody > tr:nth-child(1) > td.center.impt.impt_off", &ids);
  css_select(output_filename, "-s\n", "body > table > tbody > tr > td.left > div.subjt_top", &titles);
  css_select(output_filename, "-s\n", "body > table > tbody > tr > td:nth-child(4) > div > div", &files_ids);
  
  KnuString *id_list, *title_list, *files_id_list;
  int count = KnuString_tokenizeToArray(&ids, '\n', &id_list);
  KnuString_tokenizeToArray(&titles, '\n', &title_list);
  KnuString_tokenizeToArray(&files_ids, '\n', &files_id_list);

  Material* materials = malloc(sizeof(*materials) * count);
  for (int i = 0; i < count; i++) {
    KnuString_init(&materials[i].id);
    KnuString_init(&materials[i].title);
    KnuString_init(&materials[i].files_id);
  }

  xmlDocPtr doc = NULL;
  xmlNodePtr element = NULL; 
 for (int i = 0; i < count; i++) {
    doc = xmlParseDoc((xmlChar*)id_list[i].value);
    element = xmlDocGetRootElement(doc);
    xmlChar* impt_seq = xmlGetProp(element, (xmlChar*)"impt_seq");
    KnuString_appendString(&materials[i].id, (char*)impt_seq);
    xmlFree(impt_seq);
    xmlFreeDoc(doc);
  }
  for (int i = 0; i < count; i++) {
    doc = xmlParseDoc((xmlChar*)title_list[i].value);
    element = xmlDocGetRootElement(doc);
    xmlChar* innerText = xmlNodeGetContent(element);
    KnuString_appendString(&materials[i].title, (char*)innerText);
    xmlFree(innerText);
    xmlFreeDoc(doc);
  }
  for (int i = 0; i < count; i++) {
    doc = xmlParseDoc((xmlChar*)files_id_list[i].value);
    element = xmlDocGetRootElement(doc);
    xmlChar* id = xmlGetProp(element, (xmlChar*)"id");
    KnuString_appendString(&materials[i].files_id, strrchr((char*)id, '_') + 1);
    xmlFree(id);
    xmlFreeDoc(doc);
  }

  *material_list = materials;
  *size = count;

  for (int i = 0; i < count; i++) {
    KnuString_destroy(&id_list[i]);
    KnuString_destroy(&title_list[i]);
    KnuString_destroy(&files_id_list[i]);
  }
  free(id_list);
  free(title_list);
  free(files_id_list);
  KnuString_destroy(&ids);
  KnuString_destroy(&titles);
  KnuString_destroy(&files_ids);

cleanup_knuapi_get_student_number:
  if (options.headers) {
    curl_slist_free_all(options.headers);
  }
  remove(output_filename);

  return return_value;
}


