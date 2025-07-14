#ifndef PASSWORDMODIFIER_H_INCLUDED
#define PASSWORDMODIFIER_H_INCLUDED

#include "dirtranstags.h"

class CPasswordModifier
{
    public:
        CPasswordModifier();
        ~CPasswordModifier();

    private:
        static struct tagPasSecurityInfo *GetPasswordSecurityInfo();
        static std::string GetPasswordSecurityWarningInfo();
        static const std::string &GetSubmitedNewPassword();
        static void KillForceOfflineWaitTimer();
        static void LogoffForPasswordInsecurity();
        static bool SendModifyPWRequest(
            const std::string &somestr1,
            const std::string &somestr2
        );
        static void SetForceOfflineWaitTimer();
        static void UpdateToNewPassword();
        static void SetPasswordSecurityInfo(
            const struct tagPasSecurityInfo *new_info
        );
        static void SetSubmitedNewPassword(const std::string &new_password);
        static void _OnForceOfflineTimerEntry(union sigval arg);

        static std::string submited_new_password; // m_submitedNewPassword
        static struct tagPasSecurityInfo password_scurity; // m_passwordScurity
        static timer_t timer; //m_timer
};

#endif // PASSWORDMODIFIER_H_INCLUDED
