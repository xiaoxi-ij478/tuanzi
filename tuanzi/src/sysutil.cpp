#include "all.h"
#include "util.h"
#include "sysutil.h"

void check_run_ibus()
{
    // whereis ibus-daemon|awk '{print $2}'
    // if the above output is null then
    // rj_printf_debug("%s buf==NULL\n", __func__);
    // return
    // fi
    // ps -C ibus-daemon -f|grep root
    // if the above output is not null then
    // rj_printf_debug("%s ibus alread run by root\n", __func__);
    // return
    // fi
    // rj_printf_debug("%s not run ibus by root,and run it\n", __func__);
    // ibus-daemon -d
    // g_bDoRunIbus = 1;
}

void check_stop_ibus()
{
    // if not g_bDoRunIbus return;
    // ps -C ibus-daemon -f|grep root | awk '{print $2}'
    // if the above output is null then
    // rj_printf_debug("%s not run ibus by root\n", __func__);
    // g_bDoRunIbus = 0;
    // return
    // fi
    // kill -9 $(pidof ibus-daemon)
    // rj_printf_debug("%s kill ibus by exit,cmd=%s\n", __func__, $(pidof ibus-daemon));
}

bool check_service_status([[maybe_unused]] const char *service_name)
{
    // this function is used to check and kill services,
    // so we deliberately don't implement it
    // service $service_name status 2>&- | grep pid
    // || check_service_status2 $service_name
    return false;
}

bool check_service_status2([[maybe_unused]] const char *service_name)
{
    // same as above
    // systemctl status $service_name.service 2>&- | awk '{if($1~/Active/) print $2}' |
    // grep active
    return false;
}

bool service_start([[maybe_unused]] const char *service_name)
{
    // service $service_name start 2>&-
    // check_service_status $service_name || service_start2 $service_name
    return true;
}

bool service_start2([[maybe_unused]] const char *service_name)
{
    // systemctl start $service_name.service 2>&-
    return true;
}

bool service_stop([[maybe_unused]] const char *service_name)
{
    // service $service_name stop 2>&-
    return true;
}

bool service_stop2([[maybe_unused]] const char *service_name)
{
    // systemctl stop $service_name.service 2>&-
    return true;
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
