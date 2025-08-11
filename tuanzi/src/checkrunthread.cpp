#include "all.h"
#include "global.h"
#include "checkrunthread.h"

#ifdef _SEM_SEMUN_UNDEFINED
// I copied this from /usr/include/linux/sem.h
union semun {
    int val;                /* value for SETVAL */
    struct semid_ds *buf;   /* buffer for IPC_STAT & IPC_SET */
    unsigned short *array;  /* array for GETALL & SETALL */
    struct seminfo *__buf;  /* buffer for IPC_INFO */
    void *__pad;
};
#endif // _SEM_SEMUN_UNDEFINED

bool CCheckRunThread::started = 0;
int CCheckRunThread::sem_id = -1;
pthread_t CCheckRunThread::thread_id;
void (*CCheckRunThread::callback)(int) = nullptr;

static bool is_sem_init_ok(int semid)
{
    struct semid_ds s = {};
    union semun arg = { .buf = &s };

    if (semctl(semid, 0, IPC_STAT, arg) == -1) {
        g_logChkRun.AppendText("get sem info error:%s\n", strerror(errno));
        return -1;
    }

    g_logChkRun.AppendText(
        "sem_nsems:%d; sem_otime:%u; sem_ctime:%u;\n",
        s.sem_nsems,
        s.sem_otime,
        s.sem_ctime
    );
    return !s.sem_otime;
}

static int del_sem(int semid)
{
    if (semctl(semid, 0, IPC_RMID) == -1) {
        g_logChkRun.AppendText("Failed to delete semaphore:%s\n", strerror(errno));
        return -1;
    }

    return 0;
}

static int set_sem_all(int semid, int semnum)
{
    unsigned short val[2] = {};
    union semun arg = { .array = val };

    if (semctl(semid, semnum, SETALL, arg) == -1) {
        g_logChkRun.AppendText("Set sem error(%d):%s\n", errno, strerror(errno));
        return -1;
    }

    return 0;
}

// release the semaphore by 1
static int semaphore_v(int semid, unsigned short semnum)
{
    struct sembuf sbuf = { semnum, 1, SEM_UNDO };

    if (semop(semid, &sbuf, 1) == -1) {
        g_logChkRun.AppendText("semaphore_v error(%d):%s\n", errno, strerror(errno));
        return -1;
    }

    return 0;
}

// acquire the semaphore by 1
static int semaphore_p(int semid, unsigned short semnum)
{
    struct sembuf sbuf = { semnum, -1, SEM_UNDO };

    if (semop(semid, &sbuf, 1) == -1) {
        g_logChkRun.AppendText("semaphore_p error(%d):%s\n", errno, strerror(errno));
        return errno == EINTR ? 1 : -1;
    }

    return 0;
}

bool CCheckRunThread::StartThread(int *result, void (*callback)(int))
{
    unsigned sem_and_lock;

    if (started) {
        *result = 8;
        return false;
    }

    if (!callback) {
        *result = 2;
        return false;
    }

    // don't know why the original implementation does not assign this
    // well we assign this
    CCheckRunThread::callback = callback;
    sem_and_lock = create_sem_and_lock();

    if (sem_and_lock) {
        *result = sem_and_lock;
        return false;
    }

    if (pthread_create(&thread_id, nullptr, thread_function, nullptr)) {
        *result = 7;
        del_sem(sem_id);
        return false;
    }

    started = true;
    return true;
}

unsigned CCheckRunThread::StopThread()
{
    void *thread_return = nullptr;

    if (!started)
        return 9;

    started = false;

    if (semaphore_v(sem_id, 0))
        g_logChkRun.AppendText("CCheckRunThread StopThread semaphore_v error\n");

    g_logChkRun.AppendText("CCheckRunThread StopThread pthread_join before\n");

    if (pthread_join(thread_id, &thread_return)) {
        g_logChkRun.AppendText("Thread join failed");
        return 10;
    }

    g_logChkRun.AppendText("CCheckRunThread thread_join\n");
    return 0;
}

unsigned CCheckRunThread::create_sem_and_lock()
{
    unsigned i = 0;
    struct sembuf sops = { 1, -1, IPC_NOWAIT | SEM_UNDO };
    // I copied the code from IDA and simply did a few modification
    // so the result may be hard to understand
    sem_id =
        semget(
            519407,
            2,
            S_IRUSR | S_IWUSR |
            S_IRGRP | S_IWGRP |
            S_IROTH | S_IWOTH |
            IPC_CREAT | IPC_EXCL
        );

    if (sem_id != -1)
        goto create_new;

    if (errno != EEXIST) {
        g_logChkRun.AppendText("Error-create sem:%s\n", strerror(errno));
        return 4;
    }

    sem_id =
        semget(
            519407,
            2,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
        );

    if (sem_id == -1) {
        g_logChkRun.AppendText("Error-open sem:%s\n", strerror(errno));
        return 1;
    }

    i = 0;

    while (is_sem_init_ok(sem_id)) {
        g_logChkRun.AppendText("Sem init no ok m_sem_id=%d\n", sem_id);
        sleep(1);

        if (i++ >= 4) {
            g_logChkRun.AppendText("Creater may be exit with sem no init\n");
            goto error_handle;
        }
    }

    g_logChkRun.AppendText("Sem init ok m_sem_id=%d\n", sem_id);

    if (set_sem_all(sem_id, 2)) {
        g_logChkRun.AppendText("set_sem_all Sem init failed\n");
        goto error_handle;
    }

    if (semaphore_v(sem_id, 0))
        return 1;

    i = 0;

    while (semop(sem_id, &sops, 1)) {
        g_logChkRun.AppendText(
            "sem P error(%d):%s_EACCES=%d,EFAULT=%d,EINTR=%d, EINVAL=%d,E2BIG=%d\n",
            errno,
            strerror(errno),
            EACCES,
            EFAULT,
            EINTR,
            EINVAL,
            E2BIG
        );
        sleep(1);

        if (i++ >= 4) {
            g_logChkRun.AppendText(
                "Sem P failed times(%d)-no response,creater may be exit error\n",
                4
            );
            goto error_handle;
        }
    }

    g_logChkRun.AppendText("Sem P success-Others is running\n");
    return 1;
error_handle:

    if (del_sem(sem_id))
        return 1;

    sem_id =
        semget(
            519407,
            2,
            S_IRUSR | S_IWUSR |
            S_IRGRP | S_IWGRP |
            S_IROTH | S_IWOTH |
            IPC_CREAT | IPC_EXCL
        );

    if (sem_id == -1) {
        g_logChkRun.AppendText("Error-create sem:%s\n", strerror(errno));
        return 1;
    }

create_new:
    g_logChkRun.AppendText("create semid...%d\n", sem_id);

    if (set_sem_all(sem_id, 2)) {
        g_logChkRun.AppendText("Sem init failed\n");
        del_sem(sem_id);
        return 5;
    }

    if (semaphore_v(sem_id, 0) || semaphore_p(sem_id, 0)) {
        del_sem(sem_id);
        return 5;
    }

    return 0;
}

void *CCheckRunThread::thread_function([[maybe_unused]] void *arg)
{
    int spret = 0;

    while (true) {
        g_logChkRun.AppendText(
            "Waiting...m_sem_id=%d,m_bRunning=%d\n",
            sem_id,
            started
        );
        spret = semaphore_p(sem_id, 0);
        g_logChkRun.AppendText(
            "m_sem_id=%d,m_bRunning=%d...res=%d\n",
            sem_id,
            started,
            spret
        );

        if (!started)
            break;

        if (spret == -1) {
            spret = 5;
            break;
        }

        if (spret == 1)
            continue;

        g_logChkRun.AppendText("semaphore_p success\n");
        g_logChkRun.AppendText("Tell others I are running still\n");

        if (semaphore_v(sem_id, 1)) {
            spret = 5;
            break;
        }
    }

    g_logChkRun.AppendText(
        "CCheckRunThread::thread_function del_sem before "
        "m_sem_id=%d,m_bRunning=%d\n",
        sem_id,
        started
    );
    del_sem(sem_id);

    if (started)
        callback(spret);

    g_logChkRun.AppendText("CCheckRunThread::thread_function exit\n");
    return nullptr;
}
