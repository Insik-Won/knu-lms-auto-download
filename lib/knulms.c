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
}
/*
 * clean up global knulms library
 */
void knulms_global_cleanup() {
  knuapi_global_cleanup();
}

/*
 * login to Kyungpook lms and return StudentSession. use it to other functions.
 * - !! todo set errno to identify the error
 * - @return nonNull object if succeed, otherwise, null.
 */
StudentSession* knulms_login(const char* id, const char* password) {
  StudentSession* session = NULL;
  KnuString student_number;
  KnuString_init(&student_number);

  char cookie[] = "/tmp/tmpXXXXXX";
  if (make_tmpfile(cookie) != 0)
    return NULL;

  if (knuapi_login(id, password, cookie) != 0)
    goto throw;

  if (knuapi_get_student_number(&student_number, cookie) != 0)
    goto throw;

  session = (StudentSession*)malloc(sizeof(*session));
  if (!session)
    goto throw;
  StudentSession_init(session, cookie, student_number.value);
  KnuString_destroy(&student_number);

  return session;

throw:
  if (session) {
    StudentSession_destroy(session);
    free(session);
  }
  KnuString_destroy(&student_number);
  return NULL;
}
/*
 * select a Subject which predicate says true and return SubjectSession. use it to other functions.
 * - student_session loses the ownership of its members. do not destroy student_session.
 * - todo: set errno to identify the error
 * - @return nonNull object if succeed, otherwise, null
 */
SubjectSession* knulms_selectFirstSubject(StudentSession* student_session, int (*subject_predicate)(const char* subject_name, int idx, size_t size)) {
  SubjectSession* subject_session = NULL;
  KnuString* subject_names = NULL;
  KnuString* subject_keys = NULL;
  size_t size = -1;

  if (student_session == NULL)
    goto throw;

  if (knuapi_get_subject(&subject_names, &subject_keys, &size, student_session->cookie_filename) != 0) 
    goto throw;

  if (size <= 0)
    goto throw;

  int found = -1;
  if (subject_predicate) {
    for (int i = 0; i < size; i++) {
      if (subject_predicate(subject_names[i].value, i, size)) {
        found = i;
        break;
      }
    }
  }
  else {
    found = 0;
  }

  if (found == -1)
    goto throw;

  if (knuapi_set_subject(subject_keys[found].value, student_session->cookie_filename) != 0)
    goto throw;

  subject_session = (SubjectSession*)malloc(sizeof(*subject_session));
  if (!subject_session)
    goto throw;

  SubjectSession_init(subject_session, student_session, subject_names[found].value, subject_keys[found].value);
  free(student_session);
 
  for (int i = 0; i < size; i++) {
    KnuString_destroy(&subject_names[i]);
    KnuString_destroy(&subject_keys[i]);
  }
  free(subject_names);
  free(subject_keys);

  return subject_session;

throw:
  if (subject_names) {
    for (int i = 0; i < size; i++)
      KnuString_destroy(&subject_names[i]);
    free(subject_names);
  }
  if (subject_keys) {
    for (int i = 0; i < size; i++)
      KnuString_destroy(&subject_keys[i]);
    free(subject_keys);
  }
  if (subject_session) {
    SubjectSession_destroy(subject_session);
    free(subject_session);
  }

  return NULL;
}

/*
 * download the files, which file_predicate says true, in the post which post_predicate says true. 
 * - todo: set errno to identify the error
 * - @return 0 if succeed, otherwise, non-zero value.
 */ 
int knulms_downloadFirstMaterial(SubjectSession* subject_session, const char* path, 
                              int (*post_predicate)(const char* title, int idx, size_t size),
                              const char* include_pattern, const char* exclude_pattern) {
  Material* materials = NULL;
  size_t size = -1;
  const char* material_id = NULL;
  StudentSession* stduent_session = &subject_session->parent;

  if (subject_session == NULL || path == NULL)
    goto throw;

  if (knuapi_get_materials(stduent_session->student_number, subject_session->subject_key,
    &materials, &size, stduent_session->cookie_filename) != 0)
    goto throw;

  if (size <= 0)
    goto throw;

  if (post_predicate) {
    for (int i = 0; i < size; i++) {
      if (post_predicate(materials[i].title.value, i, size)) {
        material_id = materials[i].id.value;
      }
    }
  }
  else {
    material_id = materials[0].id.value;
  }

  if (!material_id)
    goto throw;

  char download_fiename[] = "/tmp/tmpXXXXXX";
  if (make_tmpfile(download_fiename) != 0)
    goto throw;

  if (knuapi_download_material_files(download_fiename, material_id, subject_session->subject_key,
    stduent_session->student_number, stduent_session->cookie_filename) != 0) {
    remove(download_fiename);
    goto throw;
  }

  if (unzip(download_fiename, path, include_pattern, exclude_pattern) == 0) {
    remove(download_fiename);
    goto throw;
  }

  remove(download_fiename);
  return 0;
  
throw:
  if (materials) {
    for (int i = 0; i < size; i++) {
      KnuString_destroy(&materials[i].id);
      KnuString_destroy(&materials[i].title);
    }
    free(materials);
  }

  return -1;
}
/*
 * cancel the selection of subject.
 * call this before logout.
 * - @return nonNull object if succeed, otherwise null
 */
StudentSession* knulms_unselectSubject(SubjectSession* session) {
  if (session == NULL)
    return NULL;

  StudentSession* student_session = (StudentSession*)malloc(sizeof(*student_session));
  if (!student_session)
    return NULL;

  SubjectSession_toStudentSession(student_session, session);
  free(session);

  return student_session;
}
/*
 * logout in Kyungpook lms and destroy session.
 * @return 0 if succeed, otherwise, non-zero value.
 */
int knulms_logout(StudentSession* session) {
  if (session == NULL)
    return -1;

  if (knuapi_logout(session->cookie_filename) != 0) {
    remove(session->cookie_filename);
    StudentSession_destroy(session);
    free(session);
    return -1;
  }

  remove(session->cookie_filename);
  StudentSession_destroy(session);
  free(session);

  return 0;
}
