#ifndef CRITICALSECTION_H_INCLUDED
#define CRITICALSECTION_H_INCLUDED

// the mutex lock
// uses the name from Windows

class CRITICAL_SECTION
{
    public:
        CRITICAL_SECTION();
        ~CRITICAL_SECTION();

        friend unsigned DeleteCriticalSection(CRITICAL_SECTION *lock);
        friend unsigned EnterCriticalSection(CRITICAL_SECTION *lock);
        friend unsigned InitializeCriticalSection(CRITICAL_SECTION *lock);
        friend unsigned LeaveCriticalSection(CRITICAL_SECTION *lock);

    private:
        pthread_mutex_t pthread_mutex;
        bool inited;
        pthread_mutexattr_t pthread_mutexattr;
        unsigned enter();
        unsigned Delete();
        unsigned init();
        unsigned leave();
};

unsigned DeleteCriticalSection(CRITICAL_SECTION *lock);
unsigned EnterCriticalSection(CRITICAL_SECTION *lock);
unsigned InitializeCriticalSection(CRITICAL_SECTION *lock);
unsigned LeaveCriticalSection(CRITICAL_SECTION *lock);

#endif // CRITICALSECTION_H_INCLUDED
