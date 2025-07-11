#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#include "logfile.h"
#include "checkrunthread.h"
#include "contextcontrolthread.h"
#include "supplicantapp.h"
#include "supf.h"

extern std::string g_strAppPath;
extern std::string g_runLogFile;
extern std::string g_strNotify;
extern std::string g_strError;
extern bool bLoadLib; // librt loaded
// used in update procedure
extern bool updteParam; // update requested
extern std::string update_message;
// this is always set to empty
extern std::string alt_update_message;
extern int g_rwpipe[2];
extern timer_t g_runModetimer;
extern bool g_bDoRunIbus;
extern pthread_rwlock_t g_fileLock;
extern struct SuPlatformParam *g_supf_param;
extern int g_supf_cmd_read_pipe;
extern int g_supf_cmd_write_pipe;
extern int g_supf_cb_read_pipe;
extern int g_supf_cb_write_pipe;
extern int g_conf_pipe_read;
extern pthread_t g_supf_thread;
// used for receiving callback datas from wpa_supplicant
extern pthread_t g_supf_callback_thread;
extern bool g_supf_exited;
extern char e_pMd5Chanllenge[16];
extern unsigned e_pHelloID[16];
extern const char g_pAppData[1820];
extern const char g_pDllData[2035];
extern const char cHeartBeatArray[6784];

#ifdef USE_EXTERNAL_LIBRT
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
#else
#define my_timer_gettime    timer_gettime
#define my_timer_create     timer_create
#define my_timer_settime    timer_settime
#define my_timer_delete     timer_delete
#define my_timer_getoverrun timer_getoverrun
#endif // USE_EXTERNAL_LIBRT

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

extern CSupplicantApp theApp;

extern CCheckRunThread chkRunThread;
// TODO
//extern CContextControlThread *CtrlThread;

#endif // GLOBAL_H_INCLUDED
