#include "all.h"
#include "backoffreauthenticationmanager.h"

CBackoffReAuthenticationManager::CBackoffReAuthenticationManager() :
    reauth_count(), reauth_timer()
{}

CBackoffReAuthenticationManager &CBackoffReAuthenticationManager::Instance()
{
    static CBackoffReAuthenticationManager obj; // thisObj
    return obj;
}

unsigned CBackoffReAuthenticationManager::GetReAuthenticationTimerElapse() const
{
    if (reauth_count < 4) {
        srand(time(nullptr));
        return rand() % 5 + 1;
    }

    return 0;
}

bool CBackoffReAuthenticationManager::IsNeedReAuthentication() const
{
    return reauth_count < 3;
}

void CBackoffReAuthenticationManager::Reset()
{
    reauth_count = 0;
    reauth_timer = nullptr;
}
