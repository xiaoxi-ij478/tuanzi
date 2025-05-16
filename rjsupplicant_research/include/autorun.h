#ifndef AUTORUN_H_INCLUDED
#define AUTORUN_H_INCLUDED

enum MODIFY_MODE {
    BEFOER_LOGIN_DISABLE, // the source code writes as this, not typo
    BEFOER_LOGIN_ENABLE,
    AFTER_LOGIN_DISABLE,
    AFTER_LOGIN_ENABLE
};

bool AddAutoRun(const char *arg);
bool DelAutoRun(const char *arg);
int auto_run(const char *exe, enum MODIFY_MODE mode, const char *arg);
int before_login_enable(const char * /*exe*/, const char * /*arg*/);
int before_login_disable(const char * /*exe*/);
int profile_add(const char * /*exe*/, const char * /*arg*/);
int profile_del(const char * /*exe*/);

#endif // AUTORUN_H_INCLUDED
