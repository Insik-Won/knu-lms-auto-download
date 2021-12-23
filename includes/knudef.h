#ifndef _KNU_DEF_H_
#define _KNU_DEF_H_

#include <stddef.h>

/* 
 * Student Session 
 * - cookie_filename: the name of cookie file. its ownership belongs to StudentSession.
 * - student_number: the number of student. its ownership belongs to StudentSession.
 */
typedef struct _StudentSession {
  const char* cookie_filename;
  const char* student_number;
} StudentSession;

/*
 * initialize StudentSession. 
 * strdup cookie_filename and student_number to session.
 */
void StudentSession_init(StudentSession* session, const char* cookie_filename, const char* student_number);
/*
 * destroy StudentSession
 * free its members, not self.
 */
void StudentSession_destroy(StudentSession* session);

/*
 * Subject Session
 * - cookie_filename: the name of cookie file. its ownership belongs to SubjectSession.
 * - subject_name: the name of current subject. its owenership belongs to SubjectSession.
 * - subject_id: the id of subject_id; its ownership belongs to SubjectSession.
 */
typedef struct _SubjectSession {
  StudentSession parent;
  const char* subject_name;
  const char* subject_key;
} SubjectSession;

/*
 * initialize SubjectSession
 * copy the value of parent and strdup subject_name and subject_key
 * the ownership of parent's member transfers to the SubjectSession.
 * do not destroy StudentSession parent.
 */
void SubjectSession_init(SubjectSession* session, StudentSession* parent, const char* subject_name, const char* subject_key);
/*
 * destroy SubjectSession
 * destroy parent and free its members, not self. 
 */
void SubjectSession_destroy(SubjectSession* session);
/*
 * convert SubjectSession to StudentSession.
 * destroy subject_session's members.
 */
void SubjectSession_toStudentSession(StudentSession* student_session, SubjectSession* subject_session);

#endif // !_KNU_DEF_H_