#ifndef PSUTIL_H_INCLUDED
#define PSUTIL_H_INCLUDED

void killrjsu();
bool checkorexitrjsu(bool kill, bool force_kill);
void killProcess(const char *proc);
void stop_dhclient_asyn();
int get_pid_byname(const char *proc);

#endif // PSUTIL_H_INCLUDED
