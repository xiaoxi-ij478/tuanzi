#ifndef CRITICALSECTION_H_INCLUDED
#define CRITICALSECTION_H_INCLUDED

// the mutex lock
// uses the name from Windows

class CRITICAL_SECTION
{
    public:
        CRITICAL_SECTION();
        ~CRITICAL_SECTION();

        friend unsigned DeleteCriticalSection(CRITICAL_SECTION *lock) {
            return lock->Delete();
        }

        friend unsigned EnterCriticalSection(CRITICAL_SECTION *lock) {
            return lock->enter();
        }

        friend unsigned InitializeCriticalSection(CRITICAL_SECTION *lock) {
            return lock->init();
        }

        friend unsigned LeaveCriticalSection(CRITICAL_SECTION *lock) {
            return lock->leave();
        }

    private:
        pthread_mutex_t pthread_mutex;
        bool inited;
        pthread_mutexattr_t pthread_mutexattr;
        unsigned enter();
        unsigned Delete();
        unsigned init();
        unsigned leave();
};


#endif // CRITICALSECTION_H_INCLUDED
