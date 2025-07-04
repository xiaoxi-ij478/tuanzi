#include "all.h"
#include "global.h"
#include "supf.h"

// some of the  functions were once integrated into wpa_supplicant,
// but we peel them off to maintain the independence of tuanzi and wpa_supplicant
// so some of the function such as wpa_printf() will be replaced with
// more general version like std::cout

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

void *supf_event_callback_recv_fun(void *param)
{
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
}

unsigned generate_network_config(
    int *read_config_pipe,
    const char *ssid,
    const char *identity,
    const char *password
)
{

}

unsigned generate_cmd_arguments(
    int *num,
    char ***arguments,
    const struct SuPlatformParam *param,
    int config_pipe_read
)
{
    char **real_arguments = new char
}

void supf_thread_function()
{

}

unsigned su_platform_cmd(struct SupfCmd* cmd)
{

}

unsigned supf_start(struct StartCmdCtx* start_ctx)
{

}

void supf_msg_handle(const struct SupfMsgData* msg_data)
{

}

void supf_state_handle(enum SupfState state)
{

}

unsigned supf_wlan_scan()
{

}

unsigned supf_write_config_to_pipe(char* config_buf, int* pipe_read)
{

}

unsigned su_platform_init(const struct SuPlatformParam *param)
{
    int wpa_init_ret = 0;

    if (!param)
        return 1;

    if (g_supf_param)
        return 11;

    if ((wpa_init_ret = wpa_global_init(param))) {
        wpa_global_free();
        return wpa_init_ret;
    }

    supf

    wpa_global_free();
    return 5;
}

unsigned su_platform_deinit()
{

}

unsigned wpa_global_init(const struct SuPlatformParam *param)
{
    int pipe_fds[2] = {}; // [rsp+0h] [rbp-28h] BYREF

    if (!(g_supf_param = new struct SuPlatformParam))
        return 3;

    strcpy(g_supf_param->driver_name, param->driver_name);
    strcpy(g_supf_param->ifname, param->ifname);
    g_supf_param->event_callback = param->event_callback;

    if (param->debug_file) {
        if (!(g_supf_param->debug_file = new char[strlen(param->debug_file) + 1]))
            return 3;

        strcpy(g_supf_param->debug_file, param->debug_file);

    } else
        g_supf_param->debug_file = nullptr;

    if (pipe(pipe_fds) == -1) {
        std::cerr << "pipe error:" << strerror(errno);
        return 5;
    }

    g_supf_cmd_read_pipe = pipe_fds[0];
    g_supf_cmd_write_pipe = pipe_fds[1];

    if (pipe(pipe_fds) == -1) {
        std::cerr << "pipe error:" << strerror(errno);
        return 5;
    }

    g_supf_cb_read_pipe = pipe_fds[0];
    g_supf_cb_write_pipe = pipe_fds[1];
    return 0;
}

void wpa_global_free()
{
    if (g_supf_param) {
        delete[] g_supf_param->debug_file;
        delete g_supf_param;
        g_supf_param = nullptr;
    }

    if (g_supf_cmd_read_pipe != -1) {
        close(g_supf_cmd_read_pipe);
        g_supf_cmd_read_pipe = -1;
    }

    if (g_supf_cmd_write_pipe != -1) {
        close(g_supf_cmd_write_pipe);
        g_supf_cmd_write_pipe = -1;
    }

    if (g_supf_cb_read_pipe != -1) {
        close(g_supf_cb_read_pipe);
        g_supf_cb_read_pipe = -1;
    }

    if (g_supf_cb_write_pipe != -1) {
        close(g_supf_cb_write_pipe);
        g_supf_cb_write_pipe = -1;
    }
}
