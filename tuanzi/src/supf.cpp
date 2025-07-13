#include "all.h"
#include "global.h"
#include "mtypes.h"
#include "changelanguage.h"
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

void *supf_event_callback_recv_fun(void *)
{
    struct SupfPipeStateMsgData *read_data =
            reinterpret_cast<struct SupfPipeStateMsgData *>
        (new char[sizeof(enum SUPF_EVENT_TYPE) + sizeof(enum SupfState)]);
    struct SupfPipeStateMsgData *new_read_data = nullptr;
    struct SupfMsgData msg_data = {};

    while (!g_supf_exited) {
        read(
            g_supf_cb_read_pipe,
            read_data,
            sizeof(enum SUPF_EVENT_TYPE) + sizeof(enum SupfState)
        );

        switch (read_data->type) {
            case SUPF_STATE:
                supf_state_handle(read_data->state);
                break;

            case SUPF_MSG:
                new_read_data =
                    reinterpret_cast<struct SupfPipeStateMsgData *>
                    (new char[sizeof(struct SupfPipeStateMsgData) + read_data->len]);
                *new_read_data = *read_data;
                read(g_supf_cb_read_pipe, new_read_data->data, new_read_data->len);
                delete[] read_data;
                read_data = nullptr;
                msg_data.msg = new_read_data->msg;
                msg_data.buf = new_read_data->data;
                msg_data.len = new_read_data->len;
                supf_msg_handle(&msg_data);
                delete[] reinterpret_cast<char *>(new_read_data);
                new_read_data = nullptr;
                read_data =
                    reinterpret_cast<struct SupfPipeStateMsgData *>
                    (new char[sizeof(enum SUPF_EVENT_TYPE) + sizeof(enum SupfState)]);
                break;
        }
    }

    return nullptr;
}

unsigned generate_network_config(
    int *read_config_pipe,
    const char *ssid,
    const char *identity,
    const char *password
)
{
    std::ostringstream oss;
    oss << "update_config=1" << std::endl
        << "eapol_version=1" << std::endl
        << "ap_scan=1" << std::endl
        << "fast_reauth=1" << std::endl;

    if (ssid && identity && password)
        oss << "network={" << std::endl
            << "     ssid=\"%s\"" << std::endl
            << "     scan_ssid=1" << std::endl
            << "     key_mgmt=WPA-EAP" << std::endl
            << "     eap=PEAP" << std::endl
            << "     identity=\"%s\"" << std::endl
            << "     password=\"%s\"" << std::endl
            << "     phase2=\"auth=MSCHAPV2\"" << std::endl
            << "     priority=10" << std::endl
            << "}";

    if (supf_write_config_to_pipe(oss.str().c_str(), read_config_pipe) == -1) {
        std::cout << "write config to pipe failed" << std::endl;
        return 4;
    }

    g_conf_pipe_read = *read_config_pipe;
    return 0;
}


unsigned generate_cmd_arguments(
    int *num,
    char ***arguments,
    const struct SuPlatformParam *param,
    int config_pipe_read
)
{
    int argc = param->debug_file ? 10 : 8;
    unsigned next_argc = 0;
    char **real_arguments = nullptr;

    if (!param->driver_name[0] || !param->ifname[0])
        return 1;

    if (!(real_arguments = new char *[argc + 1]))
        return 3;

    real_arguments[argc] = nullptr;

    for (unsigned i = 0; i < argc; i++) {
        if ((real_arguments[i] = new char[512]))
            continue;

        while (--i)
            delete[] real_arguments[i];

        delete[] real_arguments;
        return 3;
    }

#define COPY_NEXT_ARG(format, ...) \
    sprintf(real_arguments[next_argc++], (format), ##__VA_ARGS__)
    COPY_NEXT_ARG("-wpa_supplicant");
    COPY_NEXT_ARG("%d", g_supf_cmd_read_pipe);
    COPY_NEXT_ARG("%d", g_supf_cb_write_pipe);
    COPY_NEXT_ARG("-D%s", param->driver_name);
    COPY_NEXT_ARG("-i%s", param->ifname);
    COPY_NEXT_ARG("-a%d", config_pipe_read);
    COPY_NEXT_ARG("-t");
    COPY_NEXT_ARG("-K");

    if (param->debug_file) {
        COPY_NEXT_ARG("-f%s", param->debug_file);
        COPY_NEXT_ARG("-dd");
    }

#undef COPY_NEXT_ARG
    *num = argc;
    *arguments = real_arguments;
    return 0;
}

void *supf_thread_function(void *)
{
    char **argv = nullptr;
    int read_conf_pipe = -1;
    int argc = 0;

    if (
        generate_network_config(&read_conf_pipe, nullptr, nullptr, nullptr) ||
        generate_cmd_arguments(&argc, &argv, g_supf_param, read_conf_pipe)
    )
        return nullptr;

    switch (fork()) {
        case 0:
            // we should adjust to a more suitable path name after releasing software
            execv("../../../wpa_supplicant/out/sbin/wpa_supplicant", argv);
            [[fallthrough]];

        case -1:
            return nullptr;
    }

    wait(nullptr);
    return nullptr;
}

unsigned su_platform_cmd(struct SupfCmd *cmd)
{
    struct SupfPipeCmdMsgData data = { SUPF_PIPE_STOP_CMD };
    int exact_write = 0;

    switch (cmd->cmd_type) {
        case SUPF_SCAN_CMD:
            return g_supf_param ? supf_wlan_scan() : 9;

        case SUPF_START_CMD:
            return g_supf_param ? cmd->cmd_ctx ? supf_start(cmd->cmd_ctx) : 1 : 9;

        case SUPF_STOP_CMD:
            if (g_supf_cmd_write_pipe == -1)
                return 6;

            std::cout << "SUPF: Recv stop cmd" << std::endl;

            if (
                (exact_write =
                     write(g_supf_cmd_write_pipe, &data, sizeof(enum SupfPipeCmdType))
                ) != sizeof(enum SupfPipeCmdType)
            ) {
                std::cerr << "Write cmd error - bytes=" << exact_write
                          << " data len=" << sizeof(enum SupfPipeCmdType) << std::endl;
                return 5;
            }

            return 0;

        default:
            return 7;
    }
}

unsigned supf_start(struct StartCmdCtx *start_ctx)
{
    unsigned gen_network_cfg_ret = 0;
    int exact_write = 0;
    struct SupfPipeCmdMsgData *msg_data = nullptr;
    int conf_pipe_read[2] = {};

    if (!g_supf_param)
        return 9;

    if (g_supf_cmd_write_pipe == -1)
        return 6;

    if (
        (gen_network_cfg_ret =
             generate_network_config(
                 conf_pipe_read,
                 start_ctx->ssid,
                 start_ctx->identity,
                 start_ctx->password))
    )
        return gen_network_cfg_ret;

    msg_data = reinterpret_cast<struct SupfPipeCmdMsgData *>
               (new char[sizeof(struct SupfPipeCmdMsgData) + start_ctx->private_len]);

    if (!msg_data)
        return 3;

    msg_data->cmd = SUPF_PIPE_START_CMD;
    msg_data->data_len = start_ctx->private_len;
    memcpy(msg_data->private_data, start_ctx->private_data, start_ctx->private_len);

    if (
        (exact_write =
             write(
                 g_supf_cmd_write_pipe,
                 msg_data,
                 sizeof(struct SupfPipeCmdMsgData) + start_ctx->private_len
             )
        ) != sizeof(struct SupfPipeCmdMsgData) + start_ctx->private_len
    ) {
        std::cerr << "Write cmd error - bytes=" << exact_write
                  << " data len=" << sizeof(enum SupfPipeCmdType) << std::endl;
        return 5;
    }

    return 0;
}

void supf_msg_handle(const struct SupfMsgData *msg_data)
{
    CRGPrivateProc priproc;
//    char *private_buf = nullptr;

    if (!msg_data)
        return;

    // we do not allocate an extra buffer for private data
//    if (msg_data->buf) {
//        if ((private_buf = new char[msg_data->len]))
//            memcpy(private_buf, msg_data->buf, msg_data->len);
//
//        else
//            g_WlanStateLog.AppendText("MSG: malloc failed");
//    }
    g_WlanStateLog.AppendText("RECV: %s", getSupfMsgText(msg_data->msg));

    switch (msg_data->msg) {
        case SUPF_MSG_SCAN_RES:
            CtrlThread->WlanScanComplete(
                static_cast<const struct ScanCmdCtx *>(msg_data->buf)
            );
            break;

        case SUPF_MSG_EAP_ERR:
            if (msg_data->buf && msg_data->len) {
                priproc.ReadRGVendorSeg(msg_data->buf, msg_data->len);
                CtrlThread->logoff_message = CtrlThread->private_properties.field_50;

            } else
                CtrlThread->logoff_message.clear();

            CtrlThread->PostThreadMessage(HANDSHAKE_TO_SAM_MTYPE, 15, 0);
            break;

        case SUPF_MSG_EAP_SUC:
            if (msg_data->buf && msg_data->len)
                priproc.ReadRGVendorSeg(msg_data->buf, msg_data->len);

            break;
    }

//    delete[] private_buf;
}

void supf_state_handle(enum SupfState state)
{
    static enum SupfState last_state = SUPF_STOP;
    static unsigned wlan_no_found_times = 0;
    static unsigned timeoutNum = 0;
    static unsigned disassocTimes = 0;
    static bool roaming = false;
    g_WlanStateLog.AppendText(
        "WLAN STATE: %s -> %s",
        getSupfStateText(last_state),
        getSupfStateText(state)
    );

    if (last_state == SUPF_STOP && state != SUPF_START) {
        g_WlanStateLog.AppendText(
            "WLAN STATE: it is stoped - skip the current state."
        );
        return;
    }

    if (
        (state == SUPF_WLAN_NOFOUND && ++wlan_no_found_times >= 3) ||
        (state == SUPF_AUTH_TIMEOUT && ++timeoutNum >= 3) ||
        (state == SUPF_DISASSOC && ++disassocTimes >= 3)
    ) {
        g_WlanStateLog.AppendText(
            "WLAN STATE: end auth when condition( (wlan_no_found_times(%d) >= 3)"
            "  or(timeoutNum(%d) >= 3) or (disassocTimes(%d) >= 3) ) is true.",
            wlan_no_found_times,
            timeoutNum,
            disassocTimes
        );
        CtrlThread->logoff_message = CChangeLanguage::Instance().LoadString(260);
        CtrlThread->PostThreadMessage(HANDSHAKE_TO_SAM_MTYPE, 15, 0);
        last_state = SUPF_STOP;
    }

    if (state == last_state) {
        g_WlanStateLog.AppendText("WLAN STATE: same as last time");
        return;
    }

    // preprocessing
    switch (state) {
        case SUPF_DISASSOC:
            if (
                last_state == SUPF_STOP ||
                last_state == SUPF_START ||
                last_state == SUPF_WLAN_NOFOUND ||
                last_state == SUPF_DISASSOC
            ) {
                g_WlanStateLog.AppendText(
                    "WLAN STATE: it isn't associating or associated - "
                    "so skip the disassociated sate."
                );
                return;
            }

            CtrlThread->PostThreadMessage(HANDSHAKE_TO_SAM_MTYPE, 13, 0);
            break;

        case SUPF_START:
            wlan_no_found_times = 0;
            disassocTimes = 0;
            timeoutNum = 0;
            CtrlThread->PostThreadMessage(HANDSHAKE_TO_SAM_MTYPE, 12, 0);
            break;

        case SUPF_CONNECTING:
            CtrlThread->PostThreadMessage(HANDSHAKE_TO_SAM_MTYPE, 2, 0);
            break;

        case SUPF_AUTHENTICATING:
            CtrlThread->PostThreadMessage(HANDSHAKE_TO_SAM_MTYPE, 4, 0);
            break;

        case SUPF_COMPLETE_SUCCESS:
            wlan_no_found_times = 0;
            disassocTimes = 0;
            timeoutNum = 0;

            if (roaming)
                CtrlThread->PostThreadMessage(HANDSHAKE_TO_SAM_MTYPE, 14, 0);

            else
                CtrlThread->PostThreadMessage(HANDSHAKE_TO_SAM_MTYPE, 5, 0);

            break;

        default:
            return;
    }

    switch (state) {
        case SUPF_4WAY_HANDSHAKE:
            if (last_state != SUPF_AUTHENTICATING) {
                g_WlanStateLog.AppendText("WLAN STATE: Roaming start...");
                roaming = true;
            }

            [[fallthrough]];

        default:
            if (roaming)
                g_WlanStateLog.AppendText("WLAN STATE: Roaming...");

            break;

        case SUPF_GROUP_HANDSHAKE:
            roaming = false;
            break;
    }

    last_state = state;
}

unsigned supf_wlan_scan()
{
    struct SupfPipeCmdMsgData data = { SUPF_PIPE_SCAN_CMD };
    int exact_write = 0;

    if (g_supf_cmd_write_pipe == -1)
        return 6;

    if (
        (exact_write =
             write(g_supf_cmd_write_pipe, &data, sizeof(enum SupfPipeCmdType))
        ) != sizeof(enum SupfPipeCmdType)
    ) {
        std::cerr << "Write cmd error - bytes=" << exact_write
                  << " data len=" << sizeof(enum SupfPipeCmdType) << std::endl;
        return 5;
    }

    return 0;
}

int supf_write_config_to_pipe(const char *config_buf, int *pipe_read)
{
    int file_pipes[2] = {};
    int this_written = 0, written = 0, left_to_write = 0;

    if (pipe(file_pipes) == -1) {
        std::cout << "pipe error:" << strerror(errno) << std::endl;
        return -1;
    }

    if (!strlen(config_buf)) {
        close(file_pipes[1]);
        *pipe_read = file_pipes[0];
        return 0;
    }

    left_to_write = strlen(config_buf) + 1;

    while (left_to_write) {
        if (
            (this_written =
                 write(
                     file_pipes[1],
                     config_buf + written,
                     left_to_write
                 )) == -1
        ) {
            std::cerr << "Write error on pipe" << std::endl;
            close(file_pipes[0]);
            close(file_pipes[1]);
            return -1;
        }

        left_to_write -= written;
        written += this_written;
    }

    close(file_pipes[1]);
    *pipe_read = file_pipes[0];
    return 0;
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

    g_supf_exited = false;

    if (
        !pthread_create(&g_supf_thread, nullptr, supf_thread_function, nullptr) &&
        !pthread_create(
            &g_supf_callback_thread,
            nullptr,
            supf_event_callback_recv_fun,
            nullptr
        )
    )
        return 0;

    g_supf_exited = true;
    std::cerr << "Create thread error:" << strerror(errno) << std::endl;
    wpa_global_free();
    return 5;
}

unsigned su_platform_deinit()
{
    struct SupfPipeCmdMsgData data = { SUPF_PIPE_EXIT_CMD };
    int exact_write = 0;

    if (g_supf_cmd_write_pipe == -1)
        return 0;

    if (
        (exact_write =
             write(g_supf_cmd_write_pipe, &data, sizeof(enum SupfPipeCmdType))
        ) != sizeof(enum SupfPipeCmdType)
    ) {
        std::cerr << "Write cmd error - bytes=" << exact_write
                  << " data len=" << sizeof(enum SupfPipeCmdType) << std::endl;
        return 0;
    }

    g_supf_exited = true;

    if (
        !pthread_join(g_supf_thread, nullptr) &&
        !pthread_join(g_supf_callback_thread, nullptr)
    )
        return 0;

    g_supf_exited = false;
    perror("Thread join failed");
    return 5;
}

unsigned wpa_global_init(const struct SuPlatformParam *param)
{
    int pipe_fds[2] = {};

    if (!(g_supf_param = new struct SuPlatformParam))
        return 3;

    strcpy(g_supf_param->driver_name, param->driver_name);
    strcpy(g_supf_param->ifname, param->ifname);
//    g_supf_param->event_callback = param->event_callback;

    if (param->debug_file) {
        if (!(g_supf_param->debug_file = new char[strlen(param->debug_file) + 1]))
            return 3;

        strcpy(g_supf_param->debug_file, param->debug_file);

    } else
        g_supf_param->debug_file = nullptr;

    if (pipe(pipe_fds) == -1) {
        std::cerr << "pipe error:" << strerror(errno) << std::endl;
        return 5;
    }

    g_supf_cmd_read_pipe = pipe_fds[0];
    g_supf_cmd_write_pipe = pipe_fds[1];

    if (pipe(pipe_fds) == -1) {
        std::cerr << "pipe error:" << strerror(errno) << std::endl;
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
