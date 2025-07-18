#ifndef AUTORUN_H_INCLUDED
#define AUTORUN_H_INCLUDED

extern bool AddAutoRun(const char *arg);
extern bool DelAutoRun(const char *arg);
extern int auto_run(const char *exe, enum MODIFY_MODE mode, const char *arg);
extern int before_login_enable(const char *exe, const char *arg);
extern int before_login_disable(const char *exe);
extern int profile_add(const char *exe, const char *arg);
extern int profile_del(const char *exe);

#endif // AUTORUN_H_INCLUDED
