#ifndef CRITICAL_SECTION_H
#define CRITICAL_SECTION_H

// the mutex lock
// uses the name from Windows

class CRITICAL_SECTION
{
    public:
        CRITICAL_SECTION();
        ~CRITICAL_SECTION();

        friend unsigned int DeleteCriticalSection(CRITICAL_SECTION *lock) {
            return lock->Delete();
        }

        friend unsigned int EnterCriticalSection(CRITICAL_SECTION *lock) {
            return lock->enter();
        }

        friend unsigned int InitializeCriticalSection(CRITICAL_SECTION *lock) {
            return lock->init();
        }

        friend unsigned int LeaveCriticalSection(CRITICAL_SECTION *lock) {
            return lock->leave();
        }

    private:
        pthread_mutex_t pthread_mutex;
        bool inited;
        pthread_mutexattr_t pthread_mutexattr;
        unsigned int enter();
        unsigned int Delete();
        unsigned int init();
        unsigned int leave();
};


#endif // CRITICAL_SECTION_H
