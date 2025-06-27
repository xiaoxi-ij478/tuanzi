#ifndef BACKOFFREAUTHENTICATIONMANAGER_H_INCLUDED
#define BACKOFFREAUTHENTICATIONMANAGER_H_INCLUDED

class CBackoffReAuthenticationManager
{
    public:
        unsigned GetReAuthenticationTimerElapse() const;
        bool IsNeedReAuthentication() const;
        void Reset();

        static CBackoffReAuthenticationManager &Instance();

    private:
        CBackoffReAuthenticationManager();

        unsigned reauth_count;
        timer_t reauth_timer;
};

#endif // BACKOFFREAUTHENTICATIONMANAGER_H_INCLUDED
