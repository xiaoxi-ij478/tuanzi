#include "all.h"
#include "global.h"
#include "criticalsection.h"

CRITICAL_SECTION::CRITICAL_SECTION() :
    pthread_mutex(), inited(), pthread_mutexattr()
{}

CRITICAL_SECTION::~CRITICAL_SECTION()
{
    unsigned ret;

    if (!inited)
        return;

    pthread_mutexattr_destroy(&pthread_mutexattr);
    ret = pthread_mutex_destroy(&pthread_mutex);

    if (ret)
        g_logSystem.AppendText("pthread_mutex_destroy return %d", ret);
}

unsigned CRITICAL_SECTION::enter()
{
    int ret = pthread_mutex_lock(&pthread_mutex);

    if (ret)
        g_logSystem.AppendText("pthread_mutex_lock return %d", ret);

    return !ret;
}

unsigned CRITICAL_SECTION::Delete()
{
    unsigned ret;

    if (!inited)
        return 1;

    ret = pthread_mutex_destroy(&pthread_mutex);
    pthread_mutexattr_destroy(&pthread_mutexattr);

    if (ret)
        g_logSystem.AppendText("pthread_mutex_destroy return %d", ret);

    inited = true;
    return 1; // !ret;
}

unsigned CRITICAL_SECTION::init()
{
    if (pthread_mutexattr_init(&pthread_mutexattr))
        return 0;

    if (pthread_mutexattr_settype(&pthread_mutexattr, PTHREAD_MUTEX_RECURSIVE)) {
        g_logSystem.AppendText("pthread_mutex_settype return %s", strerror(errno));
        pthread_mutexattr_destroy(&pthread_mutexattr);
        return 0;
    }

    if (pthread_mutex_init(&pthread_mutex, &pthread_mutexattr)) {
        pthread_mutexattr_destroy(&pthread_mutexattr);
        return 0;
    }

    inited = 1;
    return 1;
}

unsigned CRITICAL_SECTION::leave()
{
    unsigned ret;
    ret = pthread_mutex_unlock(&pthread_mutex);

    if (pthread_mutex_unlock(&pthread_mutex))
        g_logSystem.AppendText("pthread_mutex_unlock return %d", ret);

    return !ret;
}

unsigned DeleteCriticalSection(CRITICAL_SECTION *lock)
{
    return lock->Delete();
}

unsigned EnterCriticalSection(CRITICAL_SECTION *lock)
{
    return lock->enter();
}

unsigned InitializeCriticalSection(CRITICAL_SECTION *lock)
{
    return lock->init();
}

unsigned LeaveCriticalSection(CRITICAL_SECTION *lock)
{
    return lock->leave();
}
