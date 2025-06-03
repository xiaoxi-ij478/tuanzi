#ifndef CLIENTCENTERPEERMANAGER_H
#define CLIENTCENTERPEERMANAGER_H

#include "lnxthread.h"

#define START_CONNECT_MTYPE 0x2711
#define STOP_CONNECT_MTYPE 0x2712
#define NOTIFY_UPGRADE_MTYPE 0xDD

struct _START_CENTERCONTROL_START_ {
    _START_CENTERCONTROL_START_() :
        ipv4(),
        ipv6(),
        product(),
        major_ver(),
        minor_ver(),
        domain(),
        port(),
        mac(),
        field_38()
    {}

    in_addr_t ipv4;
    uint32_t ipv6[4];
    unsigned int product;
    unsigned int major_ver;
    unsigned int minor_ver;
    std::string domain;
    unsigned int port;
    char mac[12];
    bool field_38; // field_218 @ CClientCenterPeerManager
};


class CClientCenterPeerManager : public CLnxThread
{
    public:
        unsigned int getUpgradeType();

        static bool Start(key_t thread_key_l);
        static bool StartConnect(struct _START_CENTERCONTROL_START_ * info);
        static void Stop();
        static bool StopConnect();

    protected:
        bool DispathMessage(struct LNXMSG *msg) override;
        void OnTimer(int tflag) const override;
        bool ExitInstance() override;
        bool InitInstance() override;

    private:
        CClientCenterPeerManager();
        ~CClientCenterPeerManager() override;

        void OnStart(
            unsigned long buflen, // struct _START_CENTERCONTROL_START_ *
            void *buf
        );
        void OnStop(unsigned long buflen, void *buf);
        void OnTimer(unsigned long buflen, void *buf);
        int ParseResult(const char *result);
        void ProcessConnect();

        static std::string EnCodeStr(const std::string &str);
        static std::string DeCodeStr(const std::string &str);

        static CClientCenterPeerManager *instance;  // m_instance

        timer_t process_connect_timerid;
        unsigned int process_connect_timer_interval;
        struct _START_CENTERCONTROL_START_ control_center_info;
        key_t thread_key;
        unsigned int upgrade_type;
};

#endif // CLIENTCENTERPEERMANAGER_H
