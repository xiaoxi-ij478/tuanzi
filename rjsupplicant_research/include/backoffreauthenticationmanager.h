#ifndef BACKOFFREAUTHENTICATIONMANAGER_H
#define BACKOFFREAUTHENTICATIONMANAGER_H

class CBackoffReAuthenticationManager
{
    public:
        static CBackoffReAuthenticationManager &Instance();
        unsigned GetReAuthenticationTimerElapse() const;
        bool IsNeedReAuthentication() const;
        void Reset();

        unsigned reauth_count;
        timer_t reauth_timer;

    private:
        CBackoffReAuthenticationManager();
};

#endif // BACKOFFREAUTHENTICATIONMANAGER_H
