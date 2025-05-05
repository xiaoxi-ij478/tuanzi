#include <string>
#include <ctime>

#include "global.h"

std::string g_strAppPath;
std::string g_runLogFile;
std::string g_strNotify;
[[maybe_unused]] std::string g_strError;
bool bLoadLib = false;
bool updteParam = false;
std::string update_message;
std::string alt_update_message;

int (*my_timer_create)(
    clockid_t clockid,
    struct sigevent *sevp,
    timer_t *timerid
) = nullptr;
int (*my_timer_gettime)(
    timer_t timerid,
    struct itimerspec *curr_value
) = nullptr;
int (*my_timer_settime)(
    timer_t timerid,
    int flags,
    const struct itimerspec *new_value,
    struct itimerspec *old_value
) = nullptr;
int (*my_timer_delete)(timer_t timerid) = nullptr;
int (*my_timer_getoverrun)(timer_t timerid) = nullptr;

CLogFile logFile;
CLogFile logFile_debug;
CLogFile g_logFile_Ser;
CLogFile g_logFile_start;
CLogFile g_Logoff;
CLogFile g_Update;
CLogFile g_eapPeapLog;
CLogFile g_eapTlsLog;
CLogFile g_dhcpDug;
CLogFile g_log_Wireless;
CLogFile g_logFile_proxy;
CLogFile g_logSystem;
CLogFile g_logChkRun;
CLogFile g_rjPrivateParselog;
CLogFile g_uilog;
CLogFile g_WlanStateLog;
CLogFile g_logContextControl;
CLogFile g_logFile_dns;

CCheckRunThread chkRunThread;
CContextControlThread CtrlThread;
