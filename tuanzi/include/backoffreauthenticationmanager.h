#ifndef BACKOFFREAUTHENTICATIONMANAGER_H_INCLUDED
#define BACKOFFREAUTHENTICATIONMANAGER_H_INCLUDED

class CBackoffReAuthenticationManager
{
    public:
        unsigned GetReAuthenticationTimerElapse() const;
        bool IsNeedReAuthentication() const;
        void Reset();

        static CBackoffReAuthenticationManager &Instance();

        unsigned reauth_count;
        timer_t reauth_timer;

    private:
        CBackoffReAuthenticationManager();
};

#endif // BACKOFFREAUTHENTICATIONMANAGER_H_INCLUDED
