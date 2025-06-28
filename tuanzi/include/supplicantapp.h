#ifndef SUPPLICANTAPP_H_INCLUDED
#define SUPPLICANTAPP_H_INCLUDED

#include "criticalsection.h"

enum APPSTATUS {
    APPSTATUS_INVALID,
    APPSTATUS_1 = 1,
    APPSTATUS_STARTING = 2,
    APPSTATUS_3 = 3,
    APPSTATUS_4 = 4,
    APPSTATUS_SUCCESS = 5,
    APPSTATUS_FAILED = 7
};

enum LOGOFF_REASON {
    LOGOFF_REASON_UNKNOWN_REASON = 0,
    LOGOFF_REASON_NORMAL_LOGOFF = 1,
    LOGOFF_REASON_MULTIPLE_NIC = 102,
    LOGOFF_REASON_PROXY_DETECTED = 104,
    LOGOFF_REASON_NIC_NOT_FOUND = 106,
    LOGOFF_REASON_AUTH_FAIL = 108,
    LOGOFF_REASON_FORCE_OFFLINE_3 = 117,
    LOGOFF_REASON_FORCE_OFFLINE_2 = 120,
    LOGOFF_REASON_COULD_NOT_COMM_WITH_SERVER = 125,
    LOGOFF_REASON_OTHERS_FAKING_MAC = 126,
    LOGOFF_REASON_HTTP_PROXY = 127,
    LOGOFF_REASON_SOCKS_PROXY = 128,
    LOGOFF_REASON_NIC_NOT_CONNECTED = 1001,
    LOGOFF_REASON_NIC_DISABLED = 1002,
    LOGOFF_REASON_COMM_FAIL_NO_RESPONSE = 1003,
    LOGOFF_REASON_COMM_FAIL_TIMEOUT = 1004,
    LOGOFF_REASON_FAIL_GETTING_DYNAMIC_IP = 1005,
    LOGOFF_REASON_MULTIPLE_IP = 1006,
    LOGOFF_REASON_IP_CHANGED = 1007,
    LOGOFF_REASON_MAC_CHANGED = 1008,
};

class CSupplicantApp
{
    public:
        CSupplicantApp();
        ~CSupplicantApp();

        void GUI_QuitMainLoop(const std::string &msg) const;
        void GUI_ShowMainWindow(const std::string &msg) const;
        void GUI_update_LOGOFF(
            enum LOGOFF_REASON reason,
            enum APPSTATUS new_status
        ) const;
        void GUI_update_connect_states_and_text(
            enum APPSTATUS new_status,
            const std::string &msg
        ) const;
        void GUI_update_connect_text(const std::string &msg) const;
        void GUI_update_connectdlg_by_states(enum APPSTATUS new_status) const;
        bool IsOnline() const;

    private:
        unsigned long field_0;
        unsigned field_8;
        std::string version;
        CRITICAL_SECTION field_18;
        CRITICAL_SECTION field_48;
        enum APPSTATUS status;
        unsigned field_7C;
        unsigned long success_time;
};

#endif // SUPPLICANTAPP_H_INCLUDED
