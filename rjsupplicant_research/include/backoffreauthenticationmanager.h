#ifndef BACKOFFREAUTHENTICATIONMANAGER_H
#define BACKOFFREAUTHENTICATIONMANAGER_H

#include <ctime>

class CBackoffReAuthenticationManager
{
    public:
        CBackoffReAuthenticationManager();
        CBackoffReAuthenticationManager &Instance();
        unsigned int GetReAuthenticationTimerElapse();
        bool IsNeedReAuthentication();
        void Reset();

        unsigned int reauth_count;
        timer_t reauth_timer;
};

#endif // BACKOFFREAUTHENTICATIONMANAGER_H
