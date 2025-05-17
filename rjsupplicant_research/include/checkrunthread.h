#ifndef CHECKRUNTHREAD_H
#define CHECKRUNTHREAD_H

class CCheckRunThread
{
    public:
        static bool StartThread(int *result, void (*callback)(int));
        static unsigned int StopThread();

    private:
        static unsigned int create_sem_and_lock();
        static void *thread_function([[maybe_unused]] void *arg);

        static bool started;
        static int sem_id; // original name: m_sem_id
        static pthread_t thread_key;
        static void (*callback)(int); // original name: m_callback
};

#endif // CHECKRUNTHREAD_H
