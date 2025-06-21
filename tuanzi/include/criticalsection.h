#ifndef CRITICALSECTION_H_INCLUDED
#define CRITICALSECTION_H_INCLUDED

// the mutex lock
// uses the name from Windows

class CRITICAL_SECTION
{
    public:
        CRITICAL_SECTION();
        ~CRITICAL_SECTION();

        bool enter();
        bool Delete();
        bool init();
        bool leave();

    private:
        pthread_mutex_t pthread_mutex;
        bool inited;
        pthread_mutexattr_t pthread_mutexattr;
};

bool DeleteCriticalSection(CRITICAL_SECTION *lock);
bool EnterCriticalSection(CRITICAL_SECTION *lock);
bool InitializeCriticalSection(CRITICAL_SECTION *lock);
bool LeaveCriticalSection(CRITICAL_SECTION *lock);

#endif // CRITICALSECTION_H_INCLUDED
