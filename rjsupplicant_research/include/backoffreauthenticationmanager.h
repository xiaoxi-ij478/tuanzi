#ifndef BACKOFFREAUTHENTICATIONMANAGER_H_INCLUDED
#define BACKOFFREAUTHENTICATIONMANAGER_H_INCLUDED

class CBackoffReAuthenticationManager
{
    public:
        static CBackoffReAuthenticationManager &Instance();
        unsigned GetReAuthenticationTimerElapse() const;
        bool IsNeedReAuthentication() const;
        void Reset();

    private:
        CBackoffReAuthenticationManager();

        unsigned reauth_count;
        timer_t reauth_timer;
};

#endif // BACKOFFREAUTHENTICATIONMANAGER_H_INCLUDED
