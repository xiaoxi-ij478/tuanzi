#include "all.h"
#include "global.h"
#include "checkrunthread.h"

#define LOCK_FILE_PATH "/tmp/.rjsupplicant_lock"

int CCheckRunThread::lock_file_fd = -1;

bool CCheckRunThread::StartThread(int &result)
{
    if (started) {
        result = 8;
        return false;
    }

    unsigned flock_result = create_flock();

    if (flock_result) {
        result = flock_result;
        return false;
    }

    started = true;
    return true;
}

unsigned CCheckRunThread::StopThread()
{
    return delete_flock();
}

unsigned CCheckRunThread::create_flock()
{
    lock_file_fd = open(
                       LOCK_FILE_PATH,
                       O_WRONLY | O_CREAT,
                       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
                   );

    if (lock_file_fd == -1)
        return 2;

    if (flock(lock_file_fd, LOCK_EX | LOCK_NB) == EWOULDBLOCK)
        return 1;

    return 0;
}

unsigned CCheckRunThread::delete_flock()
{
    if (lock_file_fd == -1)
        return 2;

    flock(lock_file_fd, LOCK_UN | LOCK_NB);
    unlink(LOCK_FILE_PATH);
    return 0;
}
