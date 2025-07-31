#include "all.h"
#include "global.h"
#include "criticalsection.h"

CRITICAL_SECTION::CRITICAL_SECTION() :
    pthread_mutex(),
    inited(),
    pthread_mutexattr()
{}

CRITICAL_SECTION::~CRITICAL_SECTION()
{
    int ret = 0;

    if (!inited)
        return;

    pthread_mutexattr_destroy(&pthread_mutexattr);
    ret = pthread_mutex_destroy(&pthread_mutex);

    if (ret)
        g_logSystem.AppendText("pthread_mutex_destroy return %d", ret);
}

bool CRITICAL_SECTION::enter()
{
    int ret = pthread_mutex_lock(&pthread_mutex);

    if (ret)
        g_logSystem.AppendText("pthread_mutex_lock return %d", ret);

    return !ret;
}

bool CRITICAL_SECTION::Delete()
{
    int ret = 0;

    if (!inited)
        return true;

    ret = pthread_mutex_destroy(&pthread_mutex);
    pthread_mutexattr_destroy(&pthread_mutexattr);

    if (ret)
        g_logSystem.AppendText("pthread_mutex_destroy return %d", ret);

    inited = false;
    return true; // !ret;
}

bool CRITICAL_SECTION::init()
{
    if (pthread_mutexattr_init(&pthread_mutexattr))
        return false;

    if (pthread_mutexattr_settype(&pthread_mutexattr, PTHREAD_MUTEX_RECURSIVE)) {
        g_logSystem.AppendText("pthread_mutex_settype return %s", strerror(errno));
        pthread_mutexattr_destroy(&pthread_mutexattr);
        return false;
    }

    if (pthread_mutex_init(&pthread_mutex, &pthread_mutexattr)) {
        pthread_mutexattr_destroy(&pthread_mutexattr);
        return false;
    }

    inited = true;
    return true;
}

bool CRITICAL_SECTION::leave()
{
    int ret = pthread_mutex_unlock(&pthread_mutex);

    if (ret)
        g_logSystem.AppendText("pthread_mutex_unlock return %d", ret);

    return !ret;
}

bool DeleteCriticalSection(CRITICAL_SECTION *lock)
{
    return lock->Delete();
}

bool EnterCriticalSection(CRITICAL_SECTION *lock)
{
    return lock->enter();
}

bool InitializeCriticalSection(CRITICAL_SECTION *lock)
{
    return lock->init();
}

bool LeaveCriticalSection(CRITICAL_SECTION *lock)
{
    return lock->leave();
}
