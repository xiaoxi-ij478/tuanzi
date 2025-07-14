#ifndef STATEMACHINETHREAD_H_INCLUDED
#define STATEMACHINETHREAD_H_INCLUDED

#include "lnxthread.h"
#include "states.h"
#include "eapolutil.h"

class CStateMachineThread : public CLnxThread
{
    public:
        CStateMachineThread();
        ~CStateMachineThread() override;

    protected:
        bool InitInstance() override;
        void DispathMessage(struct LNXMSG *msg) override;
        void OnTimer(int tflag) const override;
        bool ExitInstance() override;

    private:
        CStateVisual *CreateState(enum STATES state) const;
        struct EAPOLFrame *EncapsulateFrame(
            enum IEEE8021X_PACKET_TYPE packet_type,
            enum EAP_TYPES eap_type,
            unsigned short buflen,
            char *buf
        ) const;
        void FailNotification(struct EAPOLFrame *eapol_frame) const;
        void InitState() const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnPacketNotify) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnSayHello) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnSendLogoff) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnStartMachine) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnStateMove) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnTimer) const;
        void SendNAKFrame() const;
        void SendNotificationFrame() const;
        int parseFrame(struct EAPOLFrame *eapol_frame) const;
        void txLogOff(char a1) const;
        void txRspAuth() const;
        void txRspAuthPAP() const;
        void txRspID() const;
        void txSetLogOff_CompThird() const;
        void txStart() const;

        char field_1D0;
        char field_1D1[3007];
        CStateVisual *state_visual;
        CStateData state_data;
};

#endif // STATEMACHINETHREAD_H_INCLUDED
