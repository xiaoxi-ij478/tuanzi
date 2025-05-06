#include "backoffreauthenticationmanager.h"

CBackoffReAuthenticationManager::CBackoffReAuthenticationManager()
{
    Reset();
}

CBackoffReAuthenticationManager &CBackoffReAuthenticationManager::Instance()
{
    static CBackoffReAuthenticationManager thisObj;
    return thisObj;
}

unsigned int CBackoffReAuthenticationManager::GetReAuthenticationTimerElapse()
{
    if (reauth_count < 4) {
        srand(time(nullptr));
        return rand() % 5 + 1;
    }

    return 0;
}

bool CBackoffReAuthenticationManager::IsNeedReAuthentication()
{
    return reauth_count < 3;
}

void CBackoffReAuthenticationManager::Reset()
{
    reauth_count = 0;
    reauth_timer = 0;
}
