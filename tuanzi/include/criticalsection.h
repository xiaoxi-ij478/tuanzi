#ifndef CRITICALSECTION_H_INCLUDED
#define CRITICALSECTION_H_INCLUDED

// the mutex lock
// uses the name from Windows

class CRITICAL_SECTION
{
    public:
        CRITICAL_SECTION();
        ~CRITICAL_SECTION();

        friend bool DeleteCriticalSection(CRITICAL_SECTION *lock);
        friend bool EnterCriticalSection(CRITICAL_SECTION *lock);
        friend bool InitializeCriticalSection(CRITICAL_SECTION *lock);
        friend bool LeaveCriticalSection(CRITICAL_SECTION *lock);

    private:
        pthread_mutex_t pthread_mutex;
        bool inited;
        pthread_mutexattr_t pthread_mutexattr;
        bool enter();
        bool Delete();
        bool init();
        bool leave();
};

bool DeleteCriticalSection(CRITICAL_SECTION *lock);
bool EnterCriticalSection(CRITICAL_SECTION *lock);
bool InitializeCriticalSection(CRITICAL_SECTION *lock);
bool LeaveCriticalSection(CRITICAL_SECTION *lock);

#endif // CRITICALSECTION_H_INCLUDED
