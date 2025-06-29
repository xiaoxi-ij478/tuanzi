#include "all.h"
#include "changelanguage.h"
#include "logfile.h"
#include "cmdutil.h"
#include "timeutil.h"
#include "msgutil.h"
#include "global.h"
#include "supplicantapp.h"

CSupplicantApp::CSupplicantApp() :
    field_0(),
    field_8(1),
    version("RG-SU For Linux V1.30"),
    status(),
    field_7C(),
    success_time()
{
    InitializeCriticalSection(&field_18);
    InitializeCriticalSection(&field_48);
}

CSupplicantApp::~CSupplicantApp()
{
    DeleteCriticalSection(&field_18);
    DeleteCriticalSection(&field_48);
}

void CSupplicantApp::GUI_QuitMainLoop(const std::string &msg) const
{
    ShowLocalMsg(msg + '\n', CChangeLanguage::Instance().LoadString(96));
    CLogFile::LogToFile(msg.c_str(), g_runLogFile, true, true);
    CLogFile::LogToFile(
        CChangeLanguage::Instance().LoadString(2051),
        g_runLogFile,
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
    enum APPSTATUS new_status
    // *INDENT-ON*
) const
{
    std::string notify_msg;
    bool should_not_exit = false;
#define SET_MSG_AND_EXIT_FLAG(reason, msg_id, flag) \
case (reason): \
    notify_msg = CChangeLanguage::Instance().LoadString(msg_id); \
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
                CChangeLanguage::Instance().LoadString(91) :
                CtrlThread->logoff_message,
                true
            );
            SET_CUSTOM_MSG_AND_EXIT_FLAG(
                LOGOFF_REASON_FORCE_OFFLINE_3,
                CtrlThread->logoff_message.empty() ?
                "" :
                CChangeLanguage::Instance().LoadString(29) +
                ": " + CtrlThread->logoff_message,
                false
            );
            SET_CUSTOM_MSG_AND_EXIT_FLAG(
                LOGOFF_REASON_FORCE_OFFLINE_2,
                CtrlThread->logoff_message.empty() ?
                CChangeLanguage::Instance().LoadString(91) :
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
                CChangeLanguage::Instance().LoadString(99) :
                CtrlThread->logoff_message,
                true
            );
            SET_CUSTOM_MSG_AND_EXIT_FLAG(
                LOGOFF_REASON_SOCKS_PROXY,
                CtrlThread->logoff_message.empty() ?
                CChangeLanguage::Instance().LoadString(99) :
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
    }
#undef SET_MSG_AND_EXIT_FLAG
#undef SET_CUSTOM_MSG_AND_EXIT_FLAG
AddMsgItem(5,notify_msg);GUI_update_connectdlg_by_states(new_status);
}

void CSupplicantApp::GUI_update_connect_states_and_text(
    enum APPSTATUS new_status,
    const std::string &msg
) const
{
    GUI_update_connect_text(msg);
    GUI_update_connectdlg_by_states(new_status);
}

void CSupplicantApp::GUI_update_connect_text(const std::string &msg) const
{
    message_info(msg + '\n');
}

void CSupplicantApp::GUI_update_connectdlg_by_states(
    // *INDENT-OFF*
    enum APPSTATUS new_status
    // *INDENT-ON*
) const
{
    char date_buf[64] = {};

    switch (status = new_status) {
        case APPSTATUS_SUCCESS:
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
                CChangeLanguage::Instance().LoadString(172),
                g_runLogFile,
                true,
                true
            );
            break;

        case APPSTATUS_1:
        case APPSTATUS_STARTING:
        case APPSTATUS_3:
        case APPSTATUS_4:
            success_time = 0;
            break;
    }
}

bool CSupplicantApp::IsOnline() const
{
    return status == APPSTATUS_SUCCESS;
}
