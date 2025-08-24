#include "all.h"
#include "changelanguage.h"
#include "logfile.h"
#include "cmdutil.h"
#include "timeutil.h"
#include "msgutil.h"
#include "global.h"
#include "contextcontrolthread.h"
#include "supplicantapp.h"

CSupplicantApp::CSupplicantApp() :
    thread_key(),
    field_8(1),
    version("RG-SU For Linux V1.30"),
    state(),
    field_7C(),
    success_time()
{
    InitializeCriticalSection(&su_config_file_lock);
    InitializeCriticalSection(&self_lock);
}

CSupplicantApp::~CSupplicantApp()
{
    DeleteCriticalSection(&su_config_file_lock);
    DeleteCriticalSection(&self_lock);
}

void CSupplicantApp::GUI_QuitMainLoop(const std::string &msg) const
{
    ShowLocalMsg(msg + '\n', CChangeLanguage::Instance().LoadString(96));
    CLogFile::LogToFile(msg.c_str(), g_runLogFile.c_str(), true, true);
    CLogFile::LogToFile(
        CChangeLanguage::Instance().LoadString(2051).c_str(),
        g_runLogFile.c_str(),
        true,
        true
    );
}

void CSupplicantApp::GUI_ShowMainWindow(const std::string &msg) const
{
    GUI_QuitMainLoop(msg);
}

void CSupplicantApp::GUI_update_LOGOFF(
    // *INDENT-OFF*
    enum LOGOFF_REASON reason,
    enum STATES new_state
    // *INDENT-ON*
)
{
    std::string notify_msg;
    bool should_not_exit = false;
    char cur_date[256] = {};
    CChangeLanguage &cinstance = CChangeLanguage::Instance();
#define SET_MSG_AND_EXIT_FLAG(reason, msg_id, flag) \
case (reason): \
    notify_msg = cinstance.LoadString(msg_id); \
    should_not_exit = (flag); \
    break
#define SET_CUSTOM_MSG_AND_EXIT_FLAG(reason, msg, flag) \
case (reason): \
    notify_msg = (msg); \
    should_not_exit = (flag); \
    break

    switch (reason) {
            SET_MSG_AND_EXIT_FLAG(LOGOFF_REASON_UNKNOWN_REASON, 91, true);
            SET_CUSTOM_MSG_AND_EXIT_FLAG(LOGOFF_REASON_NORMAL_LOGOFF, "", true);
            SET_MSG_AND_EXIT_FLAG(LOGOFF_REASON_MULTIPLE_NIC, 98, true);
            SET_MSG_AND_EXIT_FLAG(LOGOFF_REASON_PROXY_DETECTED, 99, true);
            SET_MSG_AND_EXIT_FLAG(LOGOFF_REASON_NIC_NOT_FOUND, 1, true);
            SET_CUSTOM_MSG_AND_EXIT_FLAG(
                LOGOFF_REASON_AUTH_FAIL,
                CtrlThread->logoff_message.empty() ?
                cinstance.LoadString(91) :
                CtrlThread->logoff_message,
                true
            );
            SET_CUSTOM_MSG_AND_EXIT_FLAG(
                LOGOFF_REASON_FORCE_OFFLINE_3,
                CtrlThread->logoff_message.empty() ?
                "" :
                cinstance.LoadString(29) +
                ": " + CtrlThread->logoff_message,
                false
            );
            SET_CUSTOM_MSG_AND_EXIT_FLAG(
                LOGOFF_REASON_FORCE_OFFLINE_2,
                CtrlThread->logoff_message.empty() ?
                cinstance.LoadString(91) :
                CtrlThread->logoff_message,
                true
            );
            SET_MSG_AND_EXIT_FLAG(LOGOFF_REASON_COULD_NOT_COMM_WITH_SERVER, 250, true);
            // we'll use message id 152 instead
//            SET_CUSTOM_MSG_AND_EXIT_FLAG(LOGOFF_REASON_OTHERS_FAKING_MAC, "", true);
            SET_MSG_AND_EXIT_FLAG(LOGOFF_REASON_OTHERS_FAKING_MAC, 152, true);
            SET_CUSTOM_MSG_AND_EXIT_FLAG(
                LOGOFF_REASON_HTTP_PROXY,
                CtrlThread->logoff_message.empty() ?
                cinstance.LoadString(99) :
                CtrlThread->logoff_message,
                true
            );
            SET_CUSTOM_MSG_AND_EXIT_FLAG(
                LOGOFF_REASON_SOCKS_PROXY,
                CtrlThread->logoff_message.empty() ?
                cinstance.LoadString(99) :
                CtrlThread->logoff_message,
                true
            );
            SET_MSG_AND_EXIT_FLAG(LOGOFF_REASON_NIC_NOT_CONNECTED, 251, false);
            SET_MSG_AND_EXIT_FLAG(LOGOFF_REASON_NIC_DISABLED, 31, false);
            SET_MSG_AND_EXIT_FLAG(LOGOFF_REASON_COMM_FAIL_NO_RESPONSE, 254, false);
            SET_MSG_AND_EXIT_FLAG(LOGOFF_REASON_COMM_FAIL_TIMEOUT, 253, false);
            SET_MSG_AND_EXIT_FLAG(LOGOFF_REASON_FAIL_GETTING_DYNAMIC_IP, 101, false);
            SET_MSG_AND_EXIT_FLAG(LOGOFF_REASON_MULTIPLE_IP, 256, true);
            SET_MSG_AND_EXIT_FLAG(LOGOFF_REASON_IP_CHANGED, 60, true);
            SET_MSG_AND_EXIT_FLAG(LOGOFF_REASON_MAC_CHANGED, 62, true);
#undef SET_MSG_AND_EXIT_FLAG
#undef SET_CUSTOM_MSG_AND_EXIT_FLAG

        default:
            break;
    }

    AddMsgItem(5, notify_msg);
    GUI_update_connectdlg_by_states(new_state);

    if (!should_not_exit) {
        GetCurDataAndTime(cur_date);
        std::string final_msg = std::string(cur_date).append(notify_msg);
        GUI_update_connect_text(final_msg);
        CLogFile::LogToFile(final_msg.c_str(), g_runLogFile.c_str(), true, true);

    } else
        GUI_ShowMainWindow(notify_msg);

    g_uilog.AppendText(
        "CSupplicantApp::GUI_update_LOGOFF (STATE_LOGOFF) strLogoffInfo=%s",
        notify_msg.c_str()
    );
}

void CSupplicantApp::GUI_update_connect_states_and_text(
    enum STATES new_state,
    const std::string &msg
)
{
    GUI_update_connect_text(msg);
    GUI_update_connectdlg_by_states(new_state);
}

void CSupplicantApp::GUI_update_connect_text(const std::string &msg) const
{
    message_info(msg + '\n');
}

void CSupplicantApp::GUI_update_connectdlg_by_states(
    // *INDENT-OFF*
    enum STATES new_state
    // *INDENT-ON*
)
{
    char date_buf[64] = {};

    switch (state = new_state) {
        case STATE_AUTHENTICATED:
            rj_printf_debug(
                "CMiniWindow::update_connectdlg_by_states connect success\n"
            );
            success_time = GetTickCount();
            GetCurDataAndTime(date_buf);
            GUI_update_connect_text(
                std::string(date_buf)
                .append(CChangeLanguage::Instance().LoadString(172))
            );
            CLogFile::LogToFile(
                CChangeLanguage::Instance().LoadString(172).c_str(),
                g_runLogFile.c_str(),
                true,
                true
            );
            break;

        case STATE_DISCONNECTED:
        case STATE_CONNECTING:
        case STATE_ACQUIRED:
        case STATE_AUTHENTICATING:
            success_time = 0;
            break;

        default:
            break;
    }
}

bool CSupplicantApp::IsOnline() const
{
    return state == STATE_AUTHENTICATED;
}
