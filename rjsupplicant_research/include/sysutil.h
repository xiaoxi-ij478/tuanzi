#ifndef SYSUTIL_H_INCLUDED
#define SYSUTIL_H_INCLUDED

enum OS_TYPE {
    OS_INVALID = -1,
    OS_FEDORA,
    OS_UBUNTU,
    OS_CENTOS
};

extern void check_run_ibus();
extern void check_stop_ibus();
extern bool check_service_status(const char *service_name);
extern bool check_service_status2(const char *service_name);
extern bool service_start(const char *service_name);
extern bool service_start2(const char *service_name);
extern bool service_stop(const char *service_name);
extern bool service_stop2(const char *service_name);
extern enum OS_TYPE get_os_type();

#endif // SYSUTIL_H_INCLUDED
