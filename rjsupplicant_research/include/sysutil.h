#ifndef SYSUTIL_H_INCLUDED
#define SYSUTIL_H_INCLUDED

[[maybe_unused]] void check_run_ibus();
[[maybe_unused]] void check_stop_ibus();
bool check_service_status(const char * /* service_name */);
bool check_service_status2(const char * /* service_name */);


#endif // SYSUTIL_H_INCLUDED
