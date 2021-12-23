#ifndef _KNU_API_H_
#define _KNU_API_H_

#include <knudef.h>
#include <knuutils.h>
#include <curl/curl.h>

#define KNULMS_HOSTNAME "https://lms.knu.ac.kr"
#define GET_KNULMS_URL(subpath) KNULMS_HOSTNAME subpath

/**
 * @brief initiate knuapi library
 */
void knuapi_global_init();
/**
 * @brief clean up knuapi library
 */
void knuapi_global_cleanup();

/**
 * @brief an option used by knuapi_http_request. initialize it with memset(..., 0, ...); 
 *
 * @param headers the headers to send request with
 * @param referer the referer 
 * @param cookie_file a file to get cookie and save it.
 */
typedef struct _HttpRequestOptions {
  struct curl_slist* headers;
  const char* referer;
  const char* cookie_file;
  long int follow_redirects;
} HttpRequestOptions;

typedef struct _Material {
  KnuString id;
  KnuString title;
} Material;

/**
 * @brief send http request and save result to output_file.
 * 
 * @param url the url to send request
 * @param 
 * @param data_format the format of post data. the following arguments will be got into the format. if null, send request as GET
 * @return 0 if succeed, else non-zero value.
 */
int knuapi_http_request(const char* url, const HttpRequestOptions* options, const char* output_filename, const char* data_format, ...);

/** 
 * @brief login with id and password, and save the cookie to file.
 * 
 * @param user_id the user's id.
 * @param password the user's password.
 * @param cookie_filename the name of cookie that will contain cookie from libcurl
 * @return 0 if succeed, otherwise, non-zero value.
 */
int knuapi_login(const char* user_id, const char* password, const char* cookie_filename);

/**
 * @brief logout the user in cookie, and save the cookie to file.
 * 
 * @param cookie_filename the name of file that contains coookie from libcurl
 * @return 0 if succeed, otherwise, non-zero value.
 */
int knuapi_logout(const char* cookie_filename);


/**
 * @brief get the student number with cookie
 * 
 * @param student_number a KnuString that will contain the student number
 * @param cookie_filename the name of file that conatins cookie from libcurl
 * @return 0 if succeed, otherwise, non-zero value.
 */
int knuapi_get_student_number(KnuString* student_number, const char* cookie_filename);

/**
 * @brief get the subject names and keys with cookie
 * 
 * @param subject_name_list a pointer that will conatin subject names, which is a NULL-terminated array. free this after using
 * @param subject_key_list a pointer that will conatin subject keys, which is a NULL-terminated array. free this after using.
 * @param size the array size
 * @param cookie_filename the name of file that contains coookie from libcurl
 * @return 0 if succeed, otherwise, non-zero value.
 */
int knuapi_get_subject(KnuString** subjects_name_list, KnuString** subject_key_list, size_t* size, const char* cookie_filename);


/**
 * @brief set the subject of the user in cookie, and save it to the cookie of file.
 * 
 * @param subject_key the subject key
 * @param cookie_filename the name of file that contains coookie from libcurl
 * @return 0 if succeed, otherwise, non-zero value.
 */
int knuapi_set_subject(const char* subject_key, const char* cookie_filename);


/**
 * @brief get the material names and ids with cookie
 * 
 * @param material_name_list a pointer to conatin material names, which is a NULL-terminated array. free this after using
 * @param material_id_list a pointer to conatin material ids, which is a NULL-terminated array. free this after using.
 * @param size the array size
 * @param cookie_filename the name of file that contains coookie from libcurl
 * @return 0 if succeed, otherwise, non-zero value.
 */
int knuapi_get_materials(const char* student_number, const char* subject_key, Material** material_list, size_t* size, const char* cookie_filename);

/**
 * @brief download file with id of material files and cookie. ignore the files that match to glob pattern.
 * 
 * @param cookie_filename the name of file that contains coookie from libcurl
 * @return 0 if succeed, otherwise, non-zero value.
 */
int knuapi_download_material_files(const char* downlaod_filename, const char* material_id, const char* subject_key, const char* stduent_number, const char* cookie_filename);



#endif // !_KNU_API_H_ 