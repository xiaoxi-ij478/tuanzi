#include "all.h"
#include "util.h"
#include "cmdutil.h"
#include "global.h"
#include "sysutil.h"

void check_run_ibus()
{
    char cmdbuf[1024] = {};
    char cmdretbuf[1024] = {};
    strcpy(cmdbuf, "whereis ibus-daemon|awk '{print $2}'");
    exec_cmd(cmdbuf, cmdretbuf, sizeof(cmdretbuf));

    if (!cmdretbuf[0] || cmdretbuf[0] == '\n') {
        rj_printf_debug("%s buf==NULL\n", "check_run_ibus");
        return;
    }

    strcpy(cmdbuf, "ps -C ibus-daemon -f|grep root");
    exec_cmd(cmdbuf, cmdretbuf, sizeof(cmdretbuf));

    if (!cmdretbuf[0] || cmdretbuf[0] == '\n') {
        rj_printf_debug("%s not run ibus by root,and run it\n", "check_run_ibus");
        system("ibus-daemon -d");

    } else
        rj_printf_debug("%s ibus alread run by root\n", "check_run_ibus");

    g_bDoRunIbus = true;
}

void check_stop_ibus()
{
    char cmdbuf[1032] = {};
    char cmdretbuf[1024] = {};

    if (!g_bDoRunIbus)
        return;

    strcpy(cmdbuf, "ps -C ibus-daemon -f|grep root | awk '{print $2}'");
    exec_cmd(cmdbuf, cmdretbuf, sizeof(cmdretbuf));

    if (!cmdretbuf[0] || cmdretbuf[0] == '\n')
        rj_printf_debug("%s not run ibus by root\n", "check_stop_ibus");

    else {
        sprintf(cmdbuf, "kill -9 %s", cmdretbuf);
        system(cmdbuf);
        rj_printf_debug("%s kill ibus by exit,cmd=%s\n", "check_stop_ibus", cmdretbuf);
    }

    g_bDoRunIbus = false;
}

bool check_service_status(const char *service_name)
{
    return false;
    char cmdbuf[512] = {};
    char cmdretbuf[512] = {};
    sprintf(cmdbuf, "service %s status 2>&-", service_name);
    exec_cmd(cmdbuf, cmdretbuf, sizeof(cmdretbuf));
    return !cmdbuf[0] || !strcasestr(cmdretbuf, "pid") ?
           check_service_status2(service_name) :
           true;
}

bool check_service_status2(const char *service_name)
{
    return false;
    char cmdbuf[512] = {};
    char cmdretbuf[512] = {};
    sprintf(
        cmdbuf,
        "systemctl status %s.service 2>&- | awk '{if($1~/Active/) print $2}'",
        service_name
    );
    exec_cmd(cmdbuf, cmdretbuf, sizeof(cmdretbuf));
    return !strcmp(cmdretbuf, "active");
}

bool service_start(const char *service_name)
{
    return true;
    char cmdbuf[512] = {};
    char cmdretbuf[512] = {};
    sprintf(cmdbuf, "service %s start 2>&-", service_name);
    exec_cmd(cmdbuf, cmdretbuf, sizeof(cmdretbuf));
    return !check_service_status(service_name) || service_start2(service_name);
}

bool service_start2(const char *service_name)
{
    return true;
    char cmdbuf[512] = {};
    char cmdretbuf[512] = {};
    sprintf(cmdbuf, "systemctl start %s.service 2>&-", service_name);
    exec_cmd(cmdbuf, cmdretbuf, sizeof(cmdretbuf));
    return !check_service_status(service_name);
}

void service_stop(const char *service_name)
{
    return;
    char cmdbuf[512] = {};
    sprintf(cmdbuf, "service %s stop 2>&-", service_name);
    system(cmdbuf);
    service_stop2(cmdbuf);
}

void service_stop2(const char *service_name)
{
    return;
    char cmdbuf[512] = {};
    sprintf(cmdbuf, "systemctl stop %s.service 2>&-", service_name);
    system(cmdbuf);
}

enum OS_TYPE get_os_type()
{
    // the original implementation uses /etc/issue
    // cat /etc/issue |awk 'NR==1 {print $1}'
    // we use /etc/os-release
    // I only use Debian, so code may be inaccurate
    // If you find any errors, notice me
    std::ifstream ifs("/etc/os-release");
    std::vector<std::string> val;
    std::string line;

    if (!ifs)
        return OS_INVALID;

    while (std::getline(ifs, line)) {
        ParseString(line, '=', val);

        if (val[0] == "ID")
            break;
    }

    ifs.close();

    if (val[1] == "debian" || val[1] == "ubuntu")
        return OS_UBUNTU;

    if (val[1] == "fedora")
        return OS_FEDORA;

    if (val[1] == "centos")
        return OS_CENTOS;

    return OS_INVALID;
}

// hey, would you see rjsupplicant.sh?
// this implementation is the same as the C version
//function is64BIT()
//{
//  os=$(getconf LONG_BIT);
//  if [ $os == "32" ];  then
//      return 1;
//  fi
//  return 0;
//}
bool Is64BIT()
{
    // but we'll use statically calculated value
    // maybe we'll support arm64 one day, so include it
#if defined(__x86_64__) || defined(__aarch64__)
    return true;
#elif defined(__i386__) || defined(__arm__)
    return false;
#else
#error Your platform is not supported
#endif
}

enum LANG GetSysLanguage()
{
    return strncmp(getenv("LANG"), "zh_", 3) ? LANG_ENGLISH : LANG_CHINESE;
}

float get_fedora_lib_version(const char *pkgname)
{
    // I'm a Debian fan, so I don't know how to use yum
    char cmdbuf[1024] = {};
    char cmdretbuf[1024] = {};
    sprintf(cmdbuf, "yum list installed |grep %s |awk 'NR==1 {print $2}'", pkgname);
    exec_cmd(cmdbuf, cmdretbuf, sizeof(cmdretbuf));
    rj_printf_debug("%s version=%s\n", "get_fedora_lib_version", cmdretbuf);
    return strtof(cmdretbuf, nullptr);
}
