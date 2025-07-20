#ifndef STATEMACHINETHREAD_H_INCLUDED
#define STATEMACHINETHREAD_H_INCLUDED

#include "lnxthread.h"
#include "statedata.h"
#include "miscdefs.h"

class CStateMachineThread : public CLnxThread
{
        friend class CStateDisconnected;
        friend class CStateConnecting;
        friend class CStateAcquired;
        friend class CStateAuthenticating;
        friend class CStateAuthenticated;
        friend class CStateHold;
        friend class CStateLogOff;

    public:
        CStateMachineThread();

        DECLARE_DISPATH_MESSAGE_HANDLER(OnStateMove);
        void txRspID() const;
        void txLogOff(char a1) const;
        void txRspAuth() const;
        void txRspAuthPAP() const;
        void txSetLogOff_CompThird();
        void txStart() const;

    protected:
        void DispathMessage(struct LNXMSG *msg) override;
        void OnTimer(int tflag) override;

    private:
        CStateVisual *CreateState(enum STATES state);
        struct EAPOLFrame *EncapsulateFrame(
            enum IEEE8021X_PACKET_TYPE packet_type,
            enum EAP_TYPES eap_type,
            unsigned short buflen,
            const char *buf
        ) const;
        void FailNotification(struct EAPOLFrame *eapol_frame);
        void InitState() const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnPacketNotify);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnSayHello) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnSendLogoff) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnStartMachine);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnTimer);
        void SendNAKFrame() const;
        void SendNotificationFrame() const;
        int parseFrame(struct EAPOLFrame *eapol_frame);

        char field_1D0;
        char field_1D1[3007];
        CStateVisual *state_visual;
        CStateData state_data;
};

#endif // STATEMACHINETHREAD_H_INCLUDED
