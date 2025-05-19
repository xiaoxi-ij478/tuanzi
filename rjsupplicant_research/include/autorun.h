#ifndef AUTORUN_H_INCLUDED
#define AUTORUN_H_INCLUDED

enum MODIFY_MODE {
    BEFOER_LOGIN_DISABLE, // the source code writes as this, not typo
    BEFOER_LOGIN_ENABLE,
    AFTER_LOGIN_DISABLE,
    AFTER_LOGIN_ENABLE
};

extern bool AddAutoRun(const char *arg);
extern bool DelAutoRun(const char *arg);
extern int auto_run(const char *exe, enum MODIFY_MODE mode, const char *arg);
extern int before_login_enable(
    [[maybe_unused]] const char *exe,
    [[maybe_unused]] const char *arg
);
extern int before_login_disable([[maybe_unused]] const char *exe);
extern int profile_add(
    [[maybe_unused]] const char *exe,
    [[maybe_unused]] const char *arg
);
extern int profile_del([[maybe_unused]] const char *exe);

#endif // AUTORUN_H_INCLUDED
