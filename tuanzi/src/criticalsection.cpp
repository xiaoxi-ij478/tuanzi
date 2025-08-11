#include "all.h"
#include "global.h"
#include "criticalsection.h"

CRITICAL_SECTION::CRITICAL_SECTION()
{}

CRITICAL_SECTION::~CRITICAL_SECTION()
{}

bool CRITICAL_SECTION::enter()
{
    mutex.lock();
}

bool CRITICAL_SECTION::Delete()
{}

bool CRITICAL_SECTION::init()
{}

bool CRITICAL_SECTION::leave()
{
    mutex.unlock();
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
