#ifndef STATEDATA_H_INCLUDED
#define STATEDATA_H_INCLUDED

#include "states.h"

class CStateData
{
    public:
        CStateData();
        virtual ~CStateData();

        bool IsAuthenticated() const;
        bool IsDisconnected() const;
        bool IsHeld() const;
        bool IsLogOff() const;
        void SetAuthWhile();
        void SetHeldWhile();
        void SetRecvID(char id);
        void SetReqAuth();
        void SetReqID();
        void SetStartWhen();
        void SetUserLogOff();

        bool logoff;
        bool logoff_init;
        bool disconnected;
        char field_B;
        char field_C;
        bool req_id;
        bool req_auth;
        bool eap_failure;
        bool eap_success;
        unsigned auth_attempt_count;
        unsigned recv_id2;
        char recv_id;
        char eap_md5_datalen;
        char eap_md5_data[256];
        unsigned state;
        unsigned prev_state;
        unsigned hold_timeout;
        unsigned connect_timeout;
        unsigned auth_timeout;
        timer_t connect_timer;
        timer_t hold_timer;
        timer_t auth_timer;
        unsigned hold_count;
        CStateAcquired state_acquired;
        CStateAuthenticated state_authenticated;
        CStateAuthenticating state_authenticating;
        CStateConnecting state_connecting;
        CStateDisconnected state_disconnected;
        CStateHold state_hold;
        CStateLogOff state_logoff;
};

#endif // STATEDATA_H_INCLUDED
