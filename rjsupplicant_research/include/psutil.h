#ifndef PSUTIL_H_INCLUDED
#define PSUTIL_H_INCLUDED

extern void killrjsu();
extern bool checkorexitrjsu(bool kill, bool force_kill);
extern void killProcess(const char *proc);
extern int get_pid_byname(const char *proc);
extern bool check_process_run(const char *proc);

#endif // PSUTIL_H_INCLUDED
