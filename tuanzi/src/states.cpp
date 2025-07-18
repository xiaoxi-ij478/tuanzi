#include "all.h"
#include "global.h"
#include "mtypes.h"
#include "timeutil.h"
#include "states.h"

void CStateDisconnected::Initlize()
{
    state_data->prev_state = state_data->state;
    state_data->eap_success = false;
    state_data->state = STATE_DISCONNECTED;
    state_data->eap_failure = false;
    state_data->auth_attempt_count = 0;
    state_data->logoff_init = false;
    state_data->recv_id2 = 256;
    state_data->disconnected = false;
}

void CStateDisconnected::MoveState()
{
    if (CtrlThread->machine_thread)
        CtrlThread->machine_thread->OnStateMove(STATE_CONNECTING, 0);
}

void CStateConnecting::Initlize()
{
    state_data->prev_state = state_data->state;
    state_data->state = STATE_CONNECTING;
    state_data->connect_timeout = CtrlThread->configure_info.authparam_starttimeout;

    if (state_data->auth_timer) {
        CtrlThread->machine_thread->KillTimer(state_data->auth_timer);
        state_data->auth_timer = 0;
    }

    if (state_data->hold_timer) {
        CtrlThread->machine_thread->KillTimer(state_data->hold_timer);
        state_data->hold_timer = 0;
    }

    if (state_data->connect_timer) {
        CtrlThread->machine_thread->KillTimer(state_data->connect_timer);
        state_data->connect_timer = 0;
    }

    state_data->connect_timer =
        CtrlThread->machine_thread->SetTimer(
            nullptr,
            START_WHILE_MTYPE,
            1000 * state_data->connect_timeout,
            nullptr
        );
    state_data->auth_attempt_count++;
    state_data->req_id = 0;
    Sleep(100);
    CtrlThread->machine_thread->txStart();
}

void CStateConnecting::MoveState()
{
    if (state_data->req_id && CtrlThread->machine_thread)
        CtrlThread->machine_thread->OnStateMove(STATE_ACQUIRED, 0);

    else if (
        !state_data->connect_timeout &&
        state_data->auth_attempt_count <
        CtrlThread->configure_info.authparam_startnumber &&
        CtrlThread->machine_thread
    )
        CtrlThread->machine_thread->OnStateMove(STATE_CONNECTING, 0);

    else {
        CtrlThread->reconnect_fail = true;

        if (CtrlThread->machine_thread)
            CtrlThread->machine_thread->OnStateMove(STATE_AUTHENTICATED, 0);
    }
}
}

void CStateAcquired::Initlize()
{
    if (
        !CtrlThread->IsRuijieNas() &&
        state_data->prev_state == STATE_AUTHENTICATED
    ) {
        state_data->req_id = false;
        state_data->req_auth = false;
        CtrlThread->txRspID();
        state_data->recv_id2 = state_data->recv_id;
        return;
    }

    state_data->auth_timeout = 45;
    state_data->prev_state = state_data->state;
    state_data->state = STATE_ACQUIRED;

    if (state_data->hold_timer) {
        CtrlThread->machine_thread->KillTimer(state_data->hold_timer);
        state_data->hold_timer = 0;
    }

    if (state_data->connect_timer) {
        CtrlThread->machine_thread->KillTimer(state_data->connect_timer);
        state_data->connect_timer = 0;
    }

    if (state_data->auth_timer) {
        CtrlThread->machine_thread->KillTimer(state_data->auth_timer);
        state_data->auth_timer = 0;
    }

    state_data->auth_timer =
        CtrlThread->machine_thread->SetTimer(
            nullptr,
            AUTH_WHILE_MTYPE,
            1000 * state_data->auth_timeout,
            nullptr
        );
    state_data->auth_attempt_count = 0;
    state_data->req_id = false;
    state_data->req_auth = false;
    CtrlThread->txRspID();
    state_data->recv_id2 = state_data->recv_id;
}

void CStateAcquired::MoveState()
{
    if (state_data->req_auth && CtrlThread->machine_thread) {
        CtrlThread->machine_thread->OnStateMove(STATE_AUTHENTICATING, 0);
        return;
    }

    if (state_data->req_id && CtrlThread->machine_thread) {
        CtrlThread->machine_thread->OnStateMove(STATE_ACQUIRED, 0);
        return;
    }

    if (state_data->auth_timeout)
        return;

    if (CtrlThread->has_auth_success && !CtrlThread->IsRuijieNas())
        return;

    state_data->hold_count = 3;
    CtrlThread->machine_thread->OnStateMove(STATE_HOLD, 0);
}

void CStateAuthenticating::Initlize()
{
    state_data->prev_state = state_data->state;
    state_data->state = STATE_AUTHENTICATING;
    state_data->auth_timeout = v2->configure_info.authparam_authtimeout;

    if (state_data->auth_timer)
        CtrlThread->machine_thread->KillTimer(state_data->auth_timer);

    state_data->auth_timer =
        CtrlThread->machine_thread->SetTimer(
            nullptr,
            AUTH_WHILE_MTYPE,
            1000 * state_data->auth_timeout,
            nullptr
        );
    state_data->req_auth = false;

    if (state_data->eap_md5_datalen)
        CStateMachineThread::txRspAuth(CtrlThread->machine_thread);

    else
        CStateMachineThread::txRspAuthPAP(CtrlThread->machine_thread);

    state_data->recv_id2 = state_data->recv_id;
}

void CStateAuthenticating::MoveState()
{
    if (state_data->req_auth && CtrlThread->machine_thread)
        CtrlThread->machine_thread->OnStateMove(STATE_AUTHENTICATING, 0);

    else if (state_data->req_id && CtrlThread->machine_thread)
        CtrlThread->machine_thread->OnStateMove(STATE_ACQUIRED, 0);

    else if (!state_data->auth_timeout && !CtrlThread->has_auth_success) {
        state_data->hold_count = 3;

        if (CtrlThread->machine_thread)
            CtrlThread->machine_thread->OnStateMove(STATE_HOLD, 0);
    }
}

void CStateAuthenticated::Initlize()
{
    state_data->eap_success = false;
    state_data->prev_state = state_data->state;
    state_data->state = STATE_AUTHENTICATED;
    state_data->eap_failure = false;

    if (state_data->connect_timer) {
        CtrlThread->machine_thread->KillTimer(state_data->connect_timer);
        state_data->connect_timer = 0;
    }

    if (state_data->auth_timer) {
        CtrlThread->machine_thread->KillTimer(state_data->auth_timer);
        state_data->auth_timer = 0;
    }
}

void CStateAuthenticated::MoveState()
{
    if (state_data->req_id && CtrlThread->machine_thread)
        CtrlThread->machine_thread->OnStateMove(machine_thread, STATE_ACQUIRED, 0);
}

void CStateHold::Initlize()
{
    state_data->prev_state = state_data->state;
    state_data->state = STATE_HOLD;

    if (state == STATE_AUTHENTICATED)
        state_data->hold_count = 99;

    state_data->hold_timeout = CtrlThread->configure_info.authparam_heldtimeout;

    if (state_data->auth_timer) {
        CtrlThread->machine_thread->KillTimer(state_data->auth_timer);
        state_data->auth_timer = 0;
    }

    if (state_data->hold_timer) {
        CtrlThread->machine_thread->KillTimer(state_data->hold_timer);
        state_data->hold_timer = 0;
    }

    if (state_data->connect_timer) {
        CtrlThread->machine_thread->KillTimer(state_data->connect_timer);
        state_data->connect_timer = 0;
    }

    state_data->hold_timer =
        CtrlThread->machine_thread->SetTimer(
            nullptr,
            HOLD_WHILE_MTYPE,
            1000 * state_data->hold_timeout,
            nullptr
        );
    state_data->hold_count++;
    state_data->eap_failure = false;
    state_data->eap_success = false;
}

void CStateHold::MoveState()
{
    if (CtrlThread->configure_info.public_sutype == 1 && !CtrlThread->field_538)
        PostThreadMessage(theApp.field_0, HOLD_MTYPE, STATE_ACQUIRED, 0);

    if (state_data->hold_count == 100) {
        g_log_Wireless.AppendText("CStateHold::MoveState eap failure");
        state_data->SetUserLogOff();
        CtrlThread->PostThreadMessage(
            CtrlThread->IsRuijieNas() ? FORCE_OFFLINE_MTYPE : COMPATIBLE_LOGOFF_MTYPE,
            CtrlThread->IsRuijieNas() ? nullptr : CtrlThread->logoff_message.c_str(),
            CtrlThread->IsRuijieNas() ? 0 : CtrlThread->logoff_message.length()
        );
        return;
    }

    if (hold_count > 2) {
        state_data->SetUserLogOff();
        CtrlThread->connect_fail = true;
    }

    if (state_data->hold_timeout) {
        if (state_data->req_id && CtrlThread->machine_thread)
            CtrlThread->machine_thread->OnStateMove(STATE_ACQUIRED, 0);

    } else if (CtrlThread->machine_thread)
        CtrlThread->machine_thread->OnStateMove(STATE_CONNECTING, 0);
}

void CStateLogOff::Initlize()
{
    state_data->prev_state = state_data->state;
    state_data->state = STATE_LOGOFF;

    if (state_data->auth_timer) {
        CtrlThread->machine_thread->KillTimer(state_data->auth_timer);
        state_data->auth_timer = 0;
    }

    if (state_data->hold_timer) {
        CtrlThread->machine_thread->KillTimer(state_data->hold_timer);
        state_data->hold_timer = 0;
    }

    if (state_data->connect_timer) {
        CtrlThread->machine_thread->KillTimer(state_data->connect_timer);
        state_data->connect_timer = 0;
    }

    if (CtrlThread->machine_thread)
        CtrlThread->machine_thread->txLogOff(1);

    state_data->logoff_init = true;
}

void CStateLogOff::MoveState()
{
    if (!state_data->logoff && CtrlThread->machine_thread)
        CtrlThread->machine_thread->OnStateMove(STATE_DISCONNECTED, 0);
}
