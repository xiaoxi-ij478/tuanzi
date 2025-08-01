#include "all.h"
#include "changelanguage.h"
#include "psutil.h"

void killrjsu()
{
    if (checkorexitrjsu(true, false)) {
        for (int i = 0; i < 3; i++) {
            sleep(1);

            if (!checkorexitrjsu(false, false))
                break;
        }

        std::cout << CChangeLanguage::Instance().LoadString(2051) << std::endl;

    } else
        std::cout << CChangeLanguage::Instance().LoadString(2052) << std::endl;
}

bool checkorexitrjsu(bool kill_task, bool force_kill)
{
    // the original implementation uses shell
    // "pidof rjsupplicant 2>&-"
    std::ifstream ifs;
    std::string comm;
    DIR *proc_dir = nullptr;
    proc_dir = opendir("/proc");

    if (!proc_dir) {
//      perror("Could not open /proc for reading PID");
        return false;
    }

    for (struct dirent *dent = readdir(proc_dir); dent; dent = readdir(proc_dir)) {
        if (
            dent->d_type != DT_DIR ||
            strspn(dent->d_name, "1234567890") != strlen(dent->d_name)
        ) // not a directory or is not a PID
            continue;

        int pid = strtol(dent->d_name, nullptr, 10);
        ifs.open(std::string("/proc/").append(dent->d_name).append("/cmdline"));

        if (!ifs)
            continue;

        std::getline(ifs, comm, '\0');
        ifs.close();

        if (comm != "rjsupplicant")
            continue;

        if (kill_task)
            if (kill(pid, force_kill ? SIGKILL : SIGTERM) == -1) {
//              perror("Error while killing rjsupplicant");
                return false;
            }

        closedir(proc_dir);
        return true;
    }

    closedir(proc_dir);
    std::cerr << "rjsupplicant not found" << std::endl;
    return false;
}

void killProcess(const char *proc)
{
    // the original implementation use shell
    // kill -9 $(pidof $proc 2>&-)
    std::ifstream ifs;
    std::string comm;
    DIR *proc_dir = opendir("/proc");

    if (!proc_dir) {
//      perror("Could not open /proc for reading PID");
        return;
    }

    for (struct dirent *dent = readdir(proc_dir); dent; dent = readdir(proc_dir)) {
        if (
            dent->d_type != DT_DIR ||
            strspn(dent->d_name, "1234567890") != strlen(dent->d_name)
        ) // not a directory or is not a PID
            continue;

        int pid = strtol(dent->d_name, nullptr, 10);
        ifs.open(std::string("/proc/").append(dent->d_name).append("/cmdline"));

        if (!ifs)
            continue;

        std::getline(ifs, comm, '\0');
        ifs.close();

        if (comm != proc)
            continue;

        kill(pid, SIGKILL);
        closedir(proc_dir);
        return;
    }

    closedir(proc_dir);
}

int get_pid_byname(const char *proc)
{
    // the original implementation use shell
    // "pidof $proc 2>&-"
    std::ifstream ifs;
    std::string comm;
    DIR *proc_dir = opendir("/proc");

    if (!proc_dir) {
//      perror("Could not open /proc for reading PID");
        return -1;
    }

    for (struct dirent *dent = readdir(proc_dir); dent; dent = readdir(proc_dir)) {
        if (
            dent->d_type != DT_DIR ||
            strspn(dent->d_name, "1234567890") != strlen(dent->d_name)
        ) // not a directory or is not a PID
            continue;

        int pid = strtol(dent->d_name, nullptr, 10);
        ifs.open(std::string("/proc/").append(dent->d_name).append("/cmdline"));

        if (!ifs)
            continue;

        std::getline(ifs, comm, '\0');
        ifs.close();

        if (comm != proc)
            continue;

        closedir(proc_dir);
        return pid;
    }

    closedir(proc_dir);
    return -1;
}

bool check_process_run(const char *proc)
{
    // the original implementation use shell
    // "pidof $proc 2>&-"
    return get_pid_byname(proc) != -1;
}

void stop_dhclient_asyn()
{
    killProcess("dhclient");
}

void dhclient_exit()
{
    switch (fork()) {
        case 0:
            execlp("dhclient", "dhclient", "-x", nullptr);
            [[fallthrough]];

        case -1:
            return;

        default:
            wait(nullptr);
    }
}

void get_exe_name(std::string &dst)
{
    char s[1024] = {};
    int r = readlink("/proc/self/exe", s, sizeof(s));
    dst = r == -1 ? "" : s;
}
