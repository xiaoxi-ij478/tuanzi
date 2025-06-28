#include "all.h"
#include "changelanguage.h"
#include "logfile.h"
#include "cmdutil.h"
#include "timeutil.h"
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
