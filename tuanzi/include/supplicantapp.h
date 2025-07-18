#ifndef SUPPLICANTAPP_H_INCLUDED
#define SUPPLICANTAPP_H_INCLUDED

#include "criticalsection.h"
#include "miscdefs.h"

class CSupplicantApp
{
    public:
        CSupplicantApp();
        ~CSupplicantApp();

        void GUI_QuitMainLoop(const std::string &msg) const;
        void GUI_ShowMainWindow(const std::string &msg) const;
        void GUI_update_LOGOFF(
            enum LOGOFF_REASON reason,
            enum STATES new_state
        ) const;
        void GUI_update_connect_states_and_text(
            enum STATES new_state,
            const std::string &msg
        ) const;
        void GUI_update_connect_text(const std::string &msg) const;
        void GUI_update_connectdlg_by_states(enum STATES new_state) const;
        bool IsOnline() const;

        key_t thread_key;
        std::string version;
        CRITICAL_SECTION su_config_file_lock;

    private:
        unsigned field_8;
        CRITICAL_SECTION field_48;
        enum STATES state;
        unsigned field_7C;
        unsigned long success_time;
};

#endif // SUPPLICANTAPP_H_INCLUDED
