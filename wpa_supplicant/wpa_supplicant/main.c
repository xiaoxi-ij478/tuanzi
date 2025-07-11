/*
 * WPA Supplicant / main() function for UNIX like OSes and MinGW
 * Copyright (c) 2003-2007, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"
#ifdef __linux__
#include <fcntl.h>
#endif /* __linux__ */

#include "common.h"
#include "wpa_supplicant_i.h"
#include "driver_i.h"
// ADDED BY xiaoxi-ij478 for tuanzi
#include "scan.h"
#include "utils/eloop.h"
// ADDED BY xiaoxi-ij478 for tuanzi END

extern struct wpa_driver_ops *wpa_drivers[];

// ADDED BY xiaoxi-ij478 for tuanzi
int g_supf_cmd_read_pipe = -1;
int g_supf_cb_write_pipe = -1;
int g_conf_pipe_read = -1;
// ADDED BY xiaoxi-ij478 for tuanzi END

static void usage(void)
{
	int i;
	printf("%s\n\n%s\n"
		   "usage:\n"
		   "  wpa_supplicant [-BddhKLqqstuvW] [-P<pid file>] "
		   "[-g<global ctrl>] \\\n"
		   "        -i<ifname> -c<config file> [-C<ctrl>] [-D<driver>] "
		   "[-p<driver_param>] \\\n"
		   "        [-b<br_ifname>] [-f<debug file>] \\\n"
		   "        [-o<override driver>] [-O<override ctrl>] \\\n"
		   "        [-N -i<ifname> -c<conf> [-C<ctrl>] "
		   "[-D<driver>] \\\n"
		   "        [-p<driver_param>] [-b<br_ifname>] ...]\n"
		   "\n"
		   "drivers:\n",
		   wpa_supplicant_version, wpa_supplicant_license);

	for (i = 0; wpa_drivers[i]; i++) {
		printf("  %s = %s\n",
			   wpa_drivers[i]->name,
			   wpa_drivers[i]->desc);
	}

#ifndef CONFIG_NO_STDOUT_DEBUG
	printf("options:\n"
		   "  -b = optional bridge interface name\n"
		   "  -B = run daemon in the background\n"
		   "  -c = Configuration file\n"
		   "  -C = ctrl_interface parameter (only used if -c is not)\n"
		   "  -i = interface name\n"
		   "  -d = increase debugging verbosity (-dd even more)\n"
		   "  -D = driver name (can be multiple drivers: nl80211,wext)\n");
#ifdef CONFIG_DEBUG_FILE
	printf("  -f = log output to debug file instead of stdout\n");
#endif /* CONFIG_DEBUG_FILE */
	printf("  -g = global ctrl_interface\n"
		   "  -K = include keys (passwords, etc.) in debug output\n");
#ifdef CONFIG_DEBUG_SYSLOG
	printf("  -s = log output to syslog instead of stdout\n");
#endif /* CONFIG_DEBUG_SYSLOG */
	printf("  -t = include timestamp in debug messages\n"
		   "  -h = show this help text\n"
		   "  -L = show license (GPL and BSD)\n"
		   "  -o = override driver parameter for new interfaces\n"
		   "  -O = override ctrl_interface parameter for new interfaces\n"
		   "  -p = driver parameters\n"
		   "  -P = PID file\n"
		   "  -q = decrease debugging verbosity (-qq even less)\n");
#ifdef CONFIG_DBUS
	printf("  -u = enable DBus control interface\n");
#endif /* CONFIG_DBUS */
	printf("  -v = show version\n"
		   "  -W = wait for a control interface monitor before starting\n"
		   "  -N = start describing new interface\n");

	printf("example:\n"
		   "  wpa_supplicant -D%s -iwlan0 -c/etc/wpa_supplicant.conf\n",
		   wpa_drivers[i] ? wpa_drivers[i]->name : "wext");
#endif /* CONFIG_NO_STDOUT_DEBUG */
}


static void license(void)
{
#ifndef CONFIG_NO_STDOUT_DEBUG
	printf("%s\n\n%s%s%s%s%s\n",
		   wpa_supplicant_version,
		   wpa_supplicant_full_license1,
		   wpa_supplicant_full_license2,
		   wpa_supplicant_full_license3,
		   wpa_supplicant_full_license4,
		   wpa_supplicant_full_license5);
#endif /* CONFIG_NO_STDOUT_DEBUG */
}


static void wpa_supplicant_fd_workaround(void)
{
#ifdef __linux__
	int s, i;
	/* When started from pcmcia-cs scripts, wpa_supplicant might start with
	 * fd 0, 1, and 2 closed. This will cause some issues because many
	 * places in wpa_supplicant are still printing out to stdout. As a
	 * workaround, make sure that fd's 0, 1, and 2 are not used for other
	 * sockets. */
	for (i = 0; i < 3; i++) {
		s = open("/dev/null", O_RDWR);
		if (s > 2) {
			close(s);
			break;
		}
	}
#endif /* __linux__ */
}

// ADDED BY xiaoxi-ij478 for tuanzi
enum SupfState {
	SUPF_STOP,
	SUPF_START,
	SUPF_WLAN_NOFOUND,
	SUPF_DISASSOC,
	SUPF_ASSOCIATING,
	SUPF_ASSOCIATED,
	SUPF_CONNECTING,
	SUPF_AUTHENTICATING,
	SUPF_4WAY_HANDSHAKE,
	SUPF_GROUP_HANDSHAKE,
	SUPF_COMPLETE_SUCCESS,
	SUPF_AUTH_TIMEOUT
};

enum SupfPipeCmdType : u8 {
	SUPF_PIPE_STOP_CMD,
	SUPF_PIPE_START_CMD,
	SUPF_PIPE_SCAN_CMD,
	SUPF_PIPE_EXIT_CMD
};

enum SupfMsg {
	SUPF_MSG_SCAN_RES,
	SUPF_MSG_EAP_ERR,
	SUPF_MSG_EAP_SUC
};

struct SupfMsgData {
	enum SupfMsg msg;
	const void *buf;
	unsigned len;
};

struct SupfPipeStateMsgData {
	enum SUPF_EVENT_TYPE type;
	union {
		enum SupfState state; // type == SUPF_STATE
		struct { // type == SUPF_MSG
			enum SupfMsg msg;
			unsigned len; // data's length
			u8 data[]; // exact data
		};
	};
};

struct SupfPipeCmdMsgData {
	enum SupfPipeCmdType cmd;
	struct { // used only when cmd == SUPF_PIPE_START_CMD
		unsigned data_len;
		u8 private_data[];
	};
};

static void wpa_supplicant_stop_auth(struct wpa_supplicant *wpa_s)
{
	enum SupfState newState;

	if (wpa_s->event_callback) {
		newState = SUPF_STOP;
		wpa_s->event_callback(SUPF_STATE, &newState);
	}
	wpa_s->reassociate = 0;
	wpa_s->disconnected = 1;
	wpa_supplicant_deauthenticate(wpa_s, 3);
}

static void wpa_supplicant_start_auth(
		struct wpa_supplicant *wpa_s,
		u8 *private_data,
		int data_len)
{
	enum SupfState newState;

	free(wpa_s->private_upload_data);
	wpa_s->private_upload_data = private_data;
	wpa_s->private_upload_data_len = data_len;
	wpa_s->disconnected = 0;
	wpa_s->reassociate = 1;
	wpa_s->conf_pipe_read = g_conf_pipe_read;
	if (wpa_supplicant_reload_configuration(wpa_s)) {
		wpa_printf(MSG_ERROR, "wpa_supplicant_reload_configuration error.");
	} else if (wpa_s->event_callback) {
		newState = SUPF_START;
		wpa_s->event_callback(SUPF_STATE, &newState);
	}
}

static void wpa_supplicant_manual_req_scan(
	struct wpa_supplicant *wpa_s,
	int sec,
	int usec)
{
	wpa_s->scan_req = 2;
	wpa_supplicant_req_scan(wpa_s, sec, usec);
}

static void wpa_supplicant_ctrl_handler(
		int sock,
		void *eloop_ctx,
		void *sock_ctx)
{
	int readlen;
	// allocate 128 bytes first
	struct SupfPipeCmdMsgData *buffer =
		malloc(sizeof(struct SupfPipeCmdMsgData) + 128);

	readlen = read(sock, (u8 *)buffer, sizeof(buffer));
	if ( readlen <= 0 ) {
		wpa_printf(MSG_ERROR, "%s read num=%d\n", __func__, readlen);
		return;
	}

	if (!eloop_ctx) {
		wpa_printf(MSG_ERROR, "wpa_s is null");
		return;
	}
	for (unsigned i = 0; i < readlen;) {
		wpa_printf(
		  MSG_DEBUG,
			"%s read(%d/%d):%d\n",
			__func__,
			i, readlen, ((u8 *)buffer)[i]);
		switch (buffer->cmd) {
			case SUPF_PIPE_STOP_CMD:
				i++;
				wpa_printf(MSG_DEBUG, "Recv WPA_STOP CMD");
				wpa_supplicant_stop_auth((struct wpa_supplicant *)eloop_ctx);
				wpa_printf(MSG_DEBUG, "Recv WPA_STOP CMD END");
				break;
			case SUPF_PIPE_START_CMD:
				wpa_printf(MSG_DEBUG, "Recv WPA_START CMD");
				if (readlen - i <= 1) {
					i++;
					wpa_printf(MSG_ERROR, "%s START ERROR PARA\n", __func__);
					break;
				}
				if(buffer->data_len > 128) {
					// buffer too small, reallocate
					buffer = realloc(buffer, buffer->data_len + sizeof(struct SupfPipeCmdMsgData));
					read(
						sock,
						 ((u8 *)buffer) + readlen,
						buffer->data_len + sizeof(struct SupfPipeCmdMsgData) - readlen
					);
				}
				wpa_printf(MSG_DEBUG, "%s data_len=%d", __func__, buffer->data_len);
				wpa_supplicant_start_auth(
					(struct wpa_supplicant *)eloop_ctx,
					buffer->private_data,
					buffer->data_len);
				break;
			case SUPF_PIPE_SCAN_CMD:
				wpa_printf(MSG_DEBUG, "Recv WPA_SCAN CMD");
				wpa_supplicant_manual_req_scan((struct wpa_supplicant *)eloop_ctx, 0, 0);
				break;
			case SUPF_PIPE_EXIT_CMD:
				wpa_printf(MSG_DEBUG, "Recv WPA_EXIT CMD");
				eloop_terminate();
				break;
			default:
			  wpa_printf(
				MSG_DEBUG,
				"%s read(%d):%d,undefine command\n",
				__func__,
				readlen,
				((u8 *)buffer)[i]);
				break;
		}
	}
}

static void supf_event_cb(
	enum SUPF_EVENT_TYPE event_type,
	const void *msg_data
)
{
	struct SupfPipeStateMsgData *write_data =
		malloc(sizeof(enum SUPF_EVENT_TYPE) + sizeof(enum SupfState));

	switch (event_type) {
		case SUPF_STATE:
			write_data->state = *(enum SupfState *)msg_data;
			write(
				g_supf_cb_write_pipe,
				write_data,
				sizeof(enum SUPF_EVENT_TYPE) + sizeof(enum SupfState)
			);
			break;

		case SUPF_MSG:
			write_data = realloc(
				write_data,
				sizeof(struct SupfPipeStateMsgData) + ((struct SupfMsgData *)msg_data)->len
			);
			write_data->msg = ((struct SupfMsgData *)msg_data)->msg;
			write_data->len = ((struct SupfMsgData *)msg_data)->len;
			memcpy(
				write_data->data,
				((struct SupfMsgData *)msg_data)->buf,
				((struct SupfMsgData *)msg_data)->len
			);

			write(
				g_supf_cb_write_pipe,
				write_data,
				sizeof(struct SupfPipeStateMsgData) + write_data->len
			);
			break;
	}
	free(write_data);
}
// ADDED BY xiaoxi-ij478 for tuanzi END

int main(int argc, char *argv[])
{
	int c, i;
	struct wpa_interface *ifaces, *iface;
	int iface_count, exitcode = -1;
	struct wpa_params params;
	struct wpa_global *global;

	if (os_program_init())
		return -1;

	os_memset(&params, 0, sizeof(params));
	params.wpa_debug_level = MSG_INFO;

	iface = ifaces = os_zalloc(sizeof(struct wpa_interface));
	if (ifaces == NULL)
		return -1;
	iface_count = 1;

	wpa_supplicant_fd_workaround();

	// ADDED BY xiaoxi-ij478 for tuanzi
	// as a special case, if the supplicant was started with argv[0][0] == '-'
	// then the first two arguments are:
	// - command read pipe fd
	// - callback write pipe fd
	// I know I should use argument switch to cleanly implement this

	if (argv[0][0] == '-') {
		g_supf_cmd_read_pipe = strtol(argv[1], NULL, 10);
		g_supf_cb_write_pipe = strtol(argv[2], NULL, 10);
		argv += 2;
		argc -= 2;
	}
	// ADDED BY xiaoxi-ij478 for tuanzi END

	for (;;) {
		// ADDED BY xiaoxi-ij478 for tuanzi
		c = getopt(argc, argv, "a:b:Bc:C:D:df:g:hi:KLNo:O:p:P:qstuvW");
		if (c < 0)
			break;
		switch (c) {
		case 'a':
			g_conf_pipe_read = strtol(optarg, NULL, 10);
			break;
		// ADDED BY xiaoxi-ij478 for tuanzi END
		case 'b':
			iface->bridge_ifname = optarg;
			break;
		case 'B':
			params.daemonize++;
			break;
		case 'c':
			iface->confname = optarg;
			break;
		case 'C':
			iface->ctrl_interface = optarg;
			break;
		case 'D':
			iface->driver = optarg;
			break;
		case 'd':
#ifdef CONFIG_NO_STDOUT_DEBUG
			printf("Debugging disabled with "
				   "CONFIG_NO_STDOUT_DEBUG=y build time "
				   "option.\n");
			goto out;
#else /* CONFIG_NO_STDOUT_DEBUG */
			params.wpa_debug_level--;
			break;
#endif /* CONFIG_NO_STDOUT_DEBUG */
#ifdef CONFIG_DEBUG_FILE
		case 'f':
			params.wpa_debug_file_path = optarg;
			break;
#endif /* CONFIG_DEBUG_FILE */
		case 'g':
			params.ctrl_interface = optarg;
			break;
		case 'h':
			usage();
			exitcode = 0;
			goto out;
		case 'i':
			iface->ifname = optarg;
			break;
		case 'K':
			params.wpa_debug_show_keys++;
			break;
		case 'L':
			license();
			exitcode = 0;
			goto out;
		case 'o':
			params.override_driver = optarg;
			break;
		case 'O':
			params.override_ctrl_interface = optarg;
			break;
		case 'p':
			iface->driver_param = optarg;
			break;
		case 'P':
			os_free(params.pid_file);
			params.pid_file = os_rel2abs_path(optarg);
			break;
		case 'q':
			params.wpa_debug_level++;
			break;
#ifdef CONFIG_DEBUG_SYSLOG
		case 's':
			params.wpa_debug_syslog++;
			break;
#endif /* CONFIG_DEBUG_SYSLOG */
		case 't':
			params.wpa_debug_timestamp++;
			break;
#ifdef CONFIG_DBUS
		case 'u':
			params.dbus_ctrl_interface = 1;
			break;
#endif /* CONFIG_DBUS */
		case 'v':
			printf("%s\n", wpa_supplicant_version);
			exitcode = 0;
			goto out;
		case 'W':
			params.wait_for_monitor++;
			break;
		case 'N':
			iface_count++;
			iface = os_realloc(ifaces, iface_count *
					   sizeof(struct wpa_interface));
			if (iface == NULL)
				goto out;
			ifaces = iface;
			iface = &ifaces[iface_count - 1];
			os_memset(iface, 0, sizeof(*iface));
			break;
		default:
			usage();
			exitcode = 0;
			goto out;
		}
	}

	exitcode = 0;
	global = wpa_supplicant_init(&params);
	if (global == NULL) {
		wpa_printf(MSG_ERROR, "Failed to initialize wpa_supplicant");
		exitcode = -1;
		goto out;
	}

	// ADDED BY xiaoxi-ij478 for tuanzi
	wpa_printf(
		MSG_INFO,
		"++++++++++++++++++++++++++++++"
		"wpa_supplicant_init success"
		"+++++++++++++++++++++++++++++++++++++++");
	for (i = 0; exitcode == 0 && i < iface_count; i++) {
		wpa_printf(
			MSG_INFO,
			"confname=%s; ctrl_interface=%s; conf_pipe_read=%d; ifname=%s\n",
			ifaces[i].confname ?: "NULL",
			ifaces[i].ctrl_interface ?: "NULL",
			ifaces[i].conf_pipe_read,
			ifaces[i].ifname ?: "NULL");
		if ((ifaces[i].confname == NULL &&
			 ifaces[i].ctrl_interface == NULL) ||
			ifaces[i].ifname == NULL) {
			wpa_printf(
				MSG_INFO,
				"iface_count=%d; params.ctrl_interface=%s; params.dbus_ctrl_interface=%s",
				i,
				params.ctrl_interface ? "NO NULL" : "NULL",
				params.dbus_ctrl_interface ? "NO NULL" : "NULL");
			if (iface_count == 1 && (params.ctrl_interface ||
						 params.dbus_ctrl_interface))
				break;
			usage();
			exitcode = -1;
			break;
		}
		if (wpa_supplicant_add_iface(global, &ifaces[i]) == NULL)
			exitcode = -1;
	}

	global->ifaces->event_callback = supf_event_cb;
	eloop_register_read_sock(
		g_supf_cmd_read_pipe,
		wpa_supplicant_ctrl_handler,
		global->ifaces,
		NULL
	);
	if (exitcode == 0)
		exitcode = wpa_supplicant_run(global);
	wpa_printf(MSG_DEBUG, "wpa_supplicant_run - end.");
	eloop_unregister_read_sock(g_supf_cmd_read_pipe);
	wpa_printf(MSG_DEBUG, "wpa_supplicant_deinit.");
	wpa_supplicant_deinit(global);
	// ADDED BY xiaoxi-ij478 for tuanzi END

out:
	os_free(ifaces);
	os_free(params.pid_file);

	os_program_deinit();
	// ADDED BY xiaoxi-ij478 for tuanzi
	close(g_supf_cmd_read_pipe);
	close(g_supf_cb_write_pipe);
	// ADDED BY xiaoxi-ij478 for tuanzi END

	return exitcode;
}
