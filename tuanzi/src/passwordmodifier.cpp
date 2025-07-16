#include "all.h"
#include "mtypes.h"
#include "cmdutil.h"
#include "changelanguage.h"
#include "userconfig.h"
#include "passwordmodifier.h"

std::string CPasswordModifier::submited_new_password;
struct tagPasSecurityInfo CPasswordModifier::password_scurity = {};
timer_t CPasswordModifier::timer;

CPasswordModifier::CPasswordModifier()
{}

CPasswordModifier::~CPasswordModifier()
{
    KillForceOfflineWaitTimer();
}

struct tagPasSecurityInfo *CPasswordModifier::GetPasswordSecurityInfo() {
    return &password_scurity;
}

void CPasswordModifier::KillForceOfflineWaitTimer()
{
    if (timer) {
        my_timer_delete(timer);
        timer = 0;
    }
}

void CPasswordModifier::LogoffForPasswordInsecurity()
{
    if (
        !CtrlThread->PostThreadMessage(
            WM_OTHER_WANT_LOGOFF,
            password_scurity.failcode ? 0x82 : 0x81,
            1
        )
    )
        rj_printf_debug("LogoffForPasswordInsecurity postMessage error");
}

bool CPasswordModifier::SendModifyPWRequest(
    const std::string &old_password,
    const std::string &new_password
)
{
    char buf[300] = {};
    unsigned len = 0;
    buf[len++] = 0x01;
    buf[len++] = 0x00;
    buf[len++] = 0x01;
    buf[len++] = 0x2B;
    buf[len++] = 0x02;
    *reinterpret_cast<uint16_t *>(&buf[len]) =
        CtrlThread->configure_info.last_auth_username.length();
    len += 2;
    memcpy(
        &buf[len],
        CtrlThread->configure_info.last_auth_username.c_str(),
        CtrlThread->configure_info.last_auth_username.length()
    );
    len += CtrlThread->configure_info.last_auth_username.length();
    buf[len++] = 0x03;
    *reinterpret_cast<uint16_t *>(&buf[len]) =
        CtrlThread->configure_info.diskid.length();
    len += 2;
    memcpy(
        &buf[len],
        CtrlThread->configure_info.diskid.c_str(),
        CtrlThread->configure_info.diskid.length()
    );
    len += CtrlThread->configure_info.diskid.length();
    buf[len++] = 0xBF;
    *reinterpret_cast<uint16_t *>(&buf[len]) = old_password.length();
    len += 2;
    strcpy(&buf[len], old_password.c_str());
    len += old_password.length();
    buf[len++] = 0xC0;
    *reinterpret_cast<uint16_t *>(&buf[len]) = new_password.length();
    len += 2;
    strcpy(&buf[len], new_password.c_str());
    len += new_password.length();
    assert(len <= 300);
    return CtrlThread->dir_tran_srv ?
           CtrlThread->dir_tran_srv->PostToSmp(buf, len) :
           false;
}

void CPasswordModifier::SetForceOfflineWaitTimer()
{
    struct sigevent timer_info = {};
    struct itimerspec interval = {};

    if (
        password_scurity.result ||
        password_scurity.force_offline != 1 ||
        !password_scurity.offline_wait_time ||
        timer
    )
        return;

    timer_info.sigev_notify = SIGEV_THREAD;
    timer_info.sigev_notify_function = _OnForceOfflineTimerEntry;
    timer_info.sigev_value.sival_int = 1;

    if (my_timer_create(CLOCK_REALTIME, &timer_info, &timer))
        return;

    interval.it_value.tv_sec = (60000 * password_scurity.offline_wait_time) / 1000;
    interval.it_value.tv_nsec = (60000 * password_scurity.offline_wait_time) % 1000;
    interval.it_interval = interval.it_value;
    my_timer_settime(timer, TIMER_ABSTIME, &interval, nullptr);
}

void CPasswordModifier::UpdateToNewPassword()
{
    if (!CtrlThread->configure_info.public_savecheck)
        return;

    CUserConfig::SaveUsernameAndPW(
        CtrlThread->configure_info.last_auth_username,
        GetSubmitedNewPassword(),
        true
    );
}

std::string CPasswordModifier::GetPasswordSecurityWarningInfo()
{
    std::string ret;
    char buf[1024] = {};

    if (!password_scurity.password_modify_message.empty())
        return password_scurity.password_modify_message;

    if (password_scurity.result)
        return ret;

    if (password_scurity.failcode) {
        if (password_scurity.failcode == 1)
            ret = CChangeLanguage::Instance().LoadString(268);

        else {
            snprintf(
                buf,
                sizeof(buf),
                CChangeLanguage::Instance().LoadString(269).c_str(),
                password_scurity.failcode
            );
            ret = buf;
        }

    } else
        ret = CChangeLanguage::Instance().LoadString(267);

    if (password_scurity.force_offline == 1 && password_scurity.offline_wait_time) {
        sprintf(
            buf,
            CChangeLanguage::Instance().LoadString(270).c_str(),
            password_scurity.offline_wait_time
        );
        ret.append(buf);
    }

    return ret;
}

const std::string &CPasswordModifier::GetSubmitedNewPassword()
{
    return submited_new_password;
}

void CPasswordModifier::SetPasswordSecurityInfo(
    const struct tagPasSecurityInfo *new_info
)
{
    if (!new_info)
        return;

    if (new_info != &password_scurity)
        password_scurity = *new_info;

    SetForceOfflineWaitTimer();
    std::string &&warning_info = GetPasswordSecurityWarningInfo();

    if (!warning_info.empty())
        shownotify(warning_info, CChangeLanguage::Instance().LoadString(95), 0);

    if (password_scurity.result)
        KillForceOfflineWaitTimer();
}

void CPasswordModifier::SetSubmitedNewPassword(const std::string &new_password)
{
    submited_new_password = new_password;
}

void CPasswordModifier::_OnForceOfflineTimerEntry(union sigval arg)
{
    if (arg.sival_int == 1)
        return;

    rj_printf_debug("stop OnForceOfflineTimerEntry");
    KillForceOfflineWaitTimer();
    LogoffForPasswordInsecurity(/* "hide_dailog" */);
}
