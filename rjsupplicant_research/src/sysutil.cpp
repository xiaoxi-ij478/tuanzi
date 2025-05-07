#include "sysutil.h"

[[maybe_unused]] void check_run_ibus()
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

[[maybe_unused]] void check_stop_ibus()
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

bool check_service_status(const char * /* service_name */)
{
    // this function is used to check and kill services,
    // so we deliberately not implement it
    // service $service_name status 2>&- | grep pid
    return false;
}

bool check_service_status2(const char * /* service_name */)
{
    // same as above
    // systemctl status %s.service 2>&- | awk '{if($1~/Active/) print $2}' |
    // grep active
    return false;
}

