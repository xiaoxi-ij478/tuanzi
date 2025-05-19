#ifndef SYSUTIL_H_INCLUDED
#define SYSUTIL_H_INCLUDED

enum OS_TYPE {
    OS_INVALID = -1,
    OS_FEDORA,
    OS_UBUNTU,
    OS_CENTOS
};

[[maybe_unused]] extern void check_run_ibus();
[[maybe_unused]] extern void check_stop_ibus();
[[maybe_unused]] extern bool check_service_status(
    [[maybe_unused]] const char *service_name
);
[[maybe_unused]] extern bool check_service_status2(
    [[maybe_unused]] const char *service_name
);
[[maybe_unused]] extern bool service_start(
    [[maybe_unused]] const char *service_name
);
[[maybe_unused]] extern bool service_start2(
    [[maybe_unused]] const char *service_name
);
[[maybe_unused]] extern bool service_stop(
    [[maybe_unused]] const char *service_name
);
[[maybe_unused]] extern bool service_stop2(
    [[maybe_unused]] const char *service_name
);
extern enum OS_TYPE get_os_type();

#endif // SYSUTIL_H_INCLUDED
