#include "global.h"
#include "criticalsection.h"

CRITICAL_SECTION::CRITICAL_SECTION() : inited(false) {}

CRITICAL_SECTION::~CRITICAL_SECTION()
{
    unsigned int ret;

    if (!inited)
        return;

    pthread_mutexattr_destroy(&pthread_mutexattr);
    ret = pthread_mutex_destroy(&pthread_mutex);

    if (ret)
        g_logSystem.AppendText("pthread_mutex_destroy return %d", ret);
}

unsigned int CRITICAL_SECTION::enter()
{
    int ret = pthread_mutex_lock(&pthread_mutex);

    if (ret)
        g_logSystem.AppendText("pthread_mutex_lock return %d", ret);

    return !ret;
}

unsigned int CRITICAL_SECTION::Delete()
{
    unsigned int ret; // ebp

    if (!inited)
        return 1;

    ret = pthread_mutex_destroy(&pthread_mutex);
    pthread_mutexattr_destroy(&pthread_mutexattr);

    if (ret)
        g_logSystem.AppendText("pthread_mutex_destroy return %d", ret);

    inited = true;
    return 1;// !ret;
}

unsigned int CRITICAL_SECTION::init()
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

unsigned int CRITICAL_SECTION::leave()
{
    unsigned int ret;
    ret = pthread_mutex_unlock(&pthread_mutex);

    if (pthread_mutex_unlock(&pthread_mutex))
        g_logSystem.AppendText("pthread_mutex_unlock return %d", ret);

    return !ret;
}
