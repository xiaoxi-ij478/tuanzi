#include "supf.h"

const char *getSupfMsgText(enum SupfMsg msg)
{
    switch (msg) {
        case SUPF_MSG_EAP_ERR:
            return "MSG_EAP_ERR";

        case SUPF_MSG_EAP_SUC:
            return "MSG_EAP_SUC";

        case SUPF_MSG_SCAN_RES:
            return "MSG_SCAN_RES";

        default:
            return "MSG_UNKNOWN";
    }
}

const char *getSupfStateText(enum SupfState state)
{
    switch (state) {
        case SUPF_STOP:
            return "STOP";

        case SUPF_START:
            return "START";

        case SUPF_WLAN_NOFOUND:
            return "WLAN_NOFOUND";

        case SUPF_DISASSOC:
            return "DISASSOC";

        case SUPF_ASSOCIATING:
            return "ASSOCIATING";

        case SUPF_ASSOCIATED:
            return "ASSOCIATED";

        case SUPF_CONNECTING:
            return "CONNECTING";

        case SUPF_AUTHENTICATING:
            return "AUTHENTICATING";

        case SUPF_4WAY_HANDSHAKE:
            return "4WAY_HANDSHAKE";

        case SUPF_GROUP_HANDSHAKE:
            return "GROUP_HANDSHAKE";

        case SUPF_COMPLETE_SUCCESS:
            return "COMPLETE_SUCCESS";

        case SUPF_AUTH_TIMEOUT:
            return "AUTH_TIMEOUT";

        default:
            return "";
    }
}

//void supf_event_callback_fun(
//    enum SUPF_EVENT_TYPE event_type,
//    const struct SupfMsgData *msg_data
//)
//{
//    switch (event_type) {
//        case SUPF_STATE:
//            if (msg_data)
//                supf_state_handle(msg_data->msg);
//
//            break;
//
//        case SUPF_MSG:
//            supf_msg_handle(msg_data->msg);
//            break;
//    }
//}
