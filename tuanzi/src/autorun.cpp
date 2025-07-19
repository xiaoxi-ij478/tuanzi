#include "all.h"
#include "cmdutil.h"
#include "psutil.h"
#include "autorun.h"

bool AddAutoRun(const char *arg)
{
    std::string path;
    get_exe_name(path);
    path = path.substr(path.rfind('/'), std::string::npos);
    path.insert(0, "/usr/bin");
    return !auto_run(path.c_str(), AFTER_LOGIN_ENABLE, arg);
}

bool DelAutoRun(const char *arg)
{
    std::string path;
    get_exe_name(path);
    path = path.substr(path.rfind('/'), std::string::npos);
    path.insert(0, "/usr/bin");
    return !auto_run(path.c_str(), AFTER_LOGIN_DISABLE, arg);
}

int auto_run(const char *exe, enum MODIFY_MODE mode, const char *arg)
{
    if (!exe || access(exe, F_OK))
        return -3;

    switch (mode) {
        case BEFOER_LOGIN_DISABLE:
            rj_printf_debug("auto_run BEFOER_LOGIN_DISABLE\n");
            return before_login_disable(exe);

        case BEFOER_LOGIN_ENABLE:
            rj_printf_debug("auto_run BEFOER_LOGIN_ENABLE\n");
            return before_login_enable(exe, arg);

        case AFTER_LOGIN_DISABLE:
            rj_printf_debug("auto_run AFTER_LOGIN_DISABLE\n");
            return profile_del(exe);

        case AFTER_LOGIN_ENABLE:
            rj_printf_debug("auto_run AFTER_LOGIN_ENABLE\n");
            return profile_add(exe, arg);

        default:
            rj_printf_debug("auto_run default\n");
            return -1;
    }
}

int before_login_enable(
    [[maybe_unused]] const char *exe,
    [[maybe_unused]] const char *arg
)
{
    // rm -rf "/etc/rc.d/init.d/${exe%.*}"
    // cp su.sh "/etc/rc.d/init.d/${exe%.*}"
    // chmod +x "/etc/rc.d/init.d/${exe%.*}"
    // chkconfig --add "/etc/rc.d/init.d/${exe%.*}"
    // sed -i 's#ACCTFILE=\"\"#ACCTFILE=\"$exe\"#' %s "/etc/rc.d/init.d/${exe%.*}"
    // sed -i 's#WORKPATH=\"\"#WORKPATH=\"${exe%/*}\"#' "/etc/rc.d/init.d/${exe%.*}"
    // sed -i 's#PARA=\"\"#PARA=\"$arg\"#' "/etc/rc.d/init.d/${exe%.*}"
    return 0;
}

int before_login_disable([[maybe_unused]] const char *exe)
{
    // rm -rf "/etc/rc.d/init.d/${exe%.*}"
    return 0;
}

int profile_add(
    [[maybe_unused]] const char *exe,
    [[maybe_unused]] const char *arg
)
{
    // if ! cat /etc/profile | grep "$exe"
    // then echo "$exe $arg & #$exe SU" >>/etc/profile
    // fi
    return 0;
}

int profile_del([[maybe_unused]] const char *exe)
{
    // sed -i "/$exe/d" /etc/profile
    return 0;
}
