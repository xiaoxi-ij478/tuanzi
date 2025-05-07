#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#include "logfile.h"
#include "checkrunthread.h"
#include "contextcontrolthread.h"

enum AUTH_MODE { AUTH_INVALID, AUTH_WIRED, AUTH_WIRELESS };
enum DHCP_MODE { DHCP_INVALID, DHCP_LOCAL, DHCP_SERVER };
enum ADAPTER_STATUS {
    ADAPTER_INVALID = -1,
    ADAPTER_UP = 1,
    ADAPTER_DOWN,
    ADAPTER_DISABLE,
    ADAPTER_ENABLE,
    ADAPTER_ERROR
};
enum ADAPTER_TYPE { ADAPTER_WIRELESS, ADAPTER_WIRED };
enum OS_TYPE { OS_INVALID = -1, OS_FEDORA, OS_UBUNTU, OS_CENTOS };

extern std::string g_strAppPath;
extern std::string g_runLogFile;
extern std::string g_strNotify;
[[maybe_unused]] extern std::string g_strError;
extern bool bLoadLib; // librt loaded
// used in update procedure
extern bool updteParam; // update requested
// original name: qword_70D9A8
extern std::string update_message;
// original name: qword_70D9B0
// this is always set to empty
extern std::string alt_update_message;
extern int g_rwpipe[2];
extern timer_t g_runModetimer;
[[maybe_unused]] extern bool g_bDoRunIbus;

extern int (*my_timer_create)(
    clockid_t clockid,
    struct sigevent *sevp,
    timer_t *timerid
);
extern int (*my_timer_gettime)(
    timer_t timerid,
    struct itimerspec *curr_value
);
extern int (*my_timer_settime)(
    timer_t timerid,
    int flags,
    const struct itimerspec *new_value,
    struct itimerspec *old_value
);
extern int (*my_timer_delete)(timer_t timerid);
extern int (*my_timer_getoverrun)(timer_t timerid);


extern CLogFile logFile;
extern CLogFile logFile_debug;
extern CLogFile g_logFile_Ser;
extern CLogFile g_logFile_start;
extern CLogFile g_Logoff;
extern CLogFile g_Update;
extern CLogFile g_eapPeapLog;
extern CLogFile g_eapTlsLog;
extern CLogFile g_dhcpDug;
extern CLogFile g_log_Wireless;
extern CLogFile g_logFile_proxy;
extern CLogFile g_logSystem;
extern CLogFile g_logChkRun;
extern CLogFile g_rjPrivateParselog;
extern CLogFile g_uilog;
extern CLogFile g_WlanStateLog;
extern CLogFile g_logContextControl;
extern CLogFile g_logFile_dns;

extern CCheckRunThread chkRunThread;
extern CContextControlThread CtrlThread;

#endif // GLOBAL_H_INCLUDED
