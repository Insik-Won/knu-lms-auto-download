#include "knudef.h"

#include <stdlib.h>
#include <string.h>

/*
 * initialize StudentSession. 
 * strdup cookie_filename and student_number to session.
 */
void StudentSession_init(StudentSession* session, const char* cookie_filename, const char* student_number) {
  session->cookie_filename = strdup(cookie_filename);
  session->student_number = strdup(student_number);
}
/*
 * destroy StudentSession
 * free its members, not self.
 */
void StudentSession_destroy(StudentSession* session) {
  free((void*)session->cookie_filename);
  free((void*)session->student_number);
  session->cookie_filename = NULL;
  session->student_number = NULL;
}


/*
 * initialize SubjectSession
 * copy the value of parent and strdup current_subject and subject_key
 * the ownership of parent's member transfers to the SubjectSession.
 * do not destroy StudentSession parent.
 */
void SubjectSession_init(SubjectSession* session, StudentSession* parent, const char* current_subject, const char* subject_key) {
  session->parent = *parent;
  session->current_subject = strdup(current_subject);
  session->subject_key = strdup(subject_key);
}
/*
 * destroy SubjectSession
 * destroy parent and free its members, not self. 
 */
void SubjectSession_destroy(SubjectSession* session) {
  StudentSession_destroy(&session->parent);
  free((void*)session->current_subject);
  free((void*)session->subject_key);
  session->current_subject = NULL;
  session->subject_key = NULL;
}

/*
 * convert SubjectSession to StudentSession.
 * destroy subject_session's members.
 * - !! do not use directly. use knulms_unselectSubject
 */
void SubjectSession_toStudentSession(StudentSession* student_session, SubjectSession* subject_session) {
  free((void*)subject_session->current_subject);
  free((void*)subject_session->subject_key);
  subject_session->current_subject = NULL;
  subject_session->subject_key = NULL;

  *student_session = subject_session->parent;
}

