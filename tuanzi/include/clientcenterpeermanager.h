#ifndef CLIENTCENTERPEERMANAGER_H_INCLUDED
#define CLIENTCENTERPEERMANAGER_H_INCLUDED

#include "lnxthread.h"

class CClientCenterPeerManager : public CLnxThread
{
    public:
        unsigned getUpgradeType() const;

        static bool Start(key_t thread_key_l);
        static bool StartConnect(struct _START_CENTERCONTROL_START_ * info);
        static void Stop();
        static bool StopConnect();

    protected:
        void DispathMessage(struct LNXMSG *msg) override;
        void OnTimer(int tflag) override;
        bool ExitInstance() override;
        bool InitInstance() override;

    private:
        CClientCenterPeerManager();
        ~CClientCenterPeerManager() override;

        DECLARE_DISPATH_MESSAGE_HANDLER(OnStart);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnStop);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnTimer);
        int ParseResult(const char *result);
        void ProcessConnect();

        static std::string EnCodeStr(const std::string &str);
        static std::string DeCodeStr(const std::string &str);

        static CClientCenterPeerManager *instance;  // m_instance

        timer_t process_connect_timerid;
        unsigned process_connect_timer_interval;
        struct _START_CENTERCONTROL_START_ control_center_info;
        key_t thread_key;
        unsigned upgrade_type;
};

#endif // CLIENTCENTERPEERMANAGER_H_INCLUDED
