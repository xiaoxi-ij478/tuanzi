#ifndef STATES_H_INCLUDED
#define STATES_H_INCLUDED

#include "statevisual.h"

enum STATES {
    STATE_INVALID,
    STATE_DISCONNECTED,
    STATE_CONNECTING,
    STATE_ACQUIRED,
    STATE_AUTHENTICATING,
    STATE_AUTHENTICATED,
    STATE_HOLD,
    STATE_LOGOFF
};

class CStateDisconnected : public CStateVisual
{
    public:
        void Initlize() const override;
        void MoveState() const override;
};

class CStateConnecting : public CStateVisual
{
    public:
        void Initlize() const override;
        void MoveState() const override;
};

class CStateAcquired : public CStateVisual
{
    public:
        void Initlize() const override;
        void MoveState() const override;
};

class CStateAuthenticating : public CStateVisual
{
    public:
        void Initlize() const override;
        void MoveState() const override;
};

class CStateAuthenticated : public CStateVisual
{
    public:
        void Initlize() const override;
        void MoveState() const override;
};

class CStateHold : public CStateVisual
{
    public:
        void Initlize() const override;
        void MoveState() const override;
};

class CStateLogOff : public CStateVisual
{
    public:
        void Initlize() const override;
        void MoveState() const override;
};

#endif // STATES_H_INCLUDED
