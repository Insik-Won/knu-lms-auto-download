#ifndef _KNU_LMS_H_
#define _KNU_LMS_H_

#include <stddef.h>
#include <knudef.h>



/*
 * initate global knulms library
 */
void knulms_global_init();
/*
 * clean up global knulms library
 */
void knulms_global_cleanup();

/*
 * login to Kyungpook lms and return StudentSession. use it to other functions.
 * - !! todo set errno to identify the error
 * - @return nonNull object if succeed, otherwise, null.
 */
StudentSession* knulms_login(const char* id, const char* password);
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

#endif // !_KNU_LMS_H_
