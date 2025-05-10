#include "cmdutil.h"
#include "lnxthread.h"
#include "downloadthread.h"

CDownLoadThread::CDownLoadThread() :
    create_progress_dialog(true),
    down_para_set(false),
    update_progress_dialog(false),
    read_only_once(false)
{}

CDownLoadThread::~CDownLoadThread()
{}

void CDownLoadThread::SetDlPara(struct tagDownLoadPara &para)
{
    if (dl_para.thread_key == para.thread_key) {
        down_para_set = true;
        return;
    }

    dl_para.thread_key = para.thread_key;
    dl_para.mtype = para.mtype;
    dl_para.url = para.url;
    dl_para.default_path = para.default_path;
    dl_para.suffix = para.suffix;
    dl_para.create_progress_dialog = para.create_progress_dialog;
    down_para_set = true;
}

bool CDownLoadThread::DispathMessage(struct LNXMSG *msg)
{
    rj_printf_debug("CDownLoadThread getmsg id=%d\n", msg->mtype);

    if (msg->mtype == START_THREAD_MTYPE)
        OnStartThread(msg->buf, msg->buflen);
}

bool CDownLoadThread::OnStartThread(void *, unsigned long)
{
    if (!down_para_set)
        rj_printf_debug("error ,no para has been set!");

    return download(
               dl_para.url.c_str(),
               dl_para.default_path.c_str(),
               dl_para.suffix.c_str(),
               create_progress_dialog
           );
}

bool CDownLoadThread::download(
    const char *url,
    const char *default_path,
    const char *suffix,
    bool create_progress_dialog
)
{
    assert(dl_para.thread_key);
    dl_para.create_progress_dialog = create_progress_dialog;

    switch (get_url_kind(url)) {
        case URL_HTTP:
            return http_download(url, default_path, suffix, domain, path, port) != -1;

        case URL_FTP:
            return ftp_download(
                       url, default_path, suffix, domain, path, port, username, password
                   ) != -1;

        default:
            return false;
    }
}

int CDownLoadThread::ftp_download(
    const char *url,
    const char *default_path,
    const char *suffix,
    char *domain,
    char *path,
    unsigned int *port,
    char *username,
    char *password
)
{
    struct ftp_host_info_s *hostinfo = nullptr;
    FILE *socket_file = nullptr;
    int ret = 0;

    if (ftp_parse_url(url, domain, path, username, password, port) == -1)
        return -1;

    if (!*domain || ! *path || ! *username)
        return -1;

    hostinfo = new struct ftp_host_info_s;
    hostinfo->username = username;
    hostinfo->password = password;
    hostinfo->addr = str2sockaddr(domain, *port, AI_CANONNAME);

    if (hostinfo->addr && (socket_file = ftp_login(hostinfo)))
        ret = ftp_receive(hostinfo, socket_file, default_path, password, suffix);

    else
        ret = -1;

    delete hostinfo->addr;
    delete hostinfo;
    return ret == -1 ? -1 : 0;
}

int CDownLoadThread::ftp_get_len(char *reply, unsigned long *length)
{
    char *sizepos = strstr(reply, "213");
    char buf[32] = { 0 };

    if (!sizepos)
        return -1;

    strncpy(buf, sizepos + 4, sizeof(buf));
    http_del_blank(buf);
    return atoi(buf);
}

FILE *CDownLoadThread::ftp_login(struct ftp_host_info_s *hostinfo)
{
    char recv_buf[256] = { 0 };
    FILE *socket_file = fdopen(xconnect_stream(hostinfo->addr), "r+");

    if (!socket_file)
        return nullptr;

    if (ftpcmd(nullptr, nullptr, socket_file, recv_buf) != 220)
        return nullptr;

    if (ftpcmd("USER", hostinfo->username, socket_file, recv_buf) != 331)
        return nullptr;

    if (ftpcmd("PASS", hostinfo->password, socket_file, recv_buf) != 230)
        return nullptr;

    if (ftpcmd("TYPE", "I", socket_file, recv_buf) != 200)
        rj_printf_debug("type_send is wrong!\n");

    return socket_file;
}

int CDownLoadThread::ftp_parse_url(
    const char *url,
    char *domain,
    char *path,
    char *username,
    char *password,
    unsigned int *port
)
{
    const char *path_begin = nullptr;
    char *domain_begin = nullptr;
    char *password_begin = nullptr;
    char *port_begin = nullptr;
    char tmp_domain[2048] = { 0 };

    if (strlen(url) <= 6)
        return -1;

    if (strncasecmp(url, "ftp://", 6))
        return -1;

    if ((path_begin = strchr(url + 6, '/'))) {
        strncpy(tmp_domain, url + 6, path_begin - url - 6);
        strcpy(path, path_begin);

    } else
        strcpy(tmp_domain, url + 6);

    if ((domain_begin = strchr(tmp_domain, '@'))) {
        *domain_begin++ = 0;

        if ((password_begin = strchr(tmp_domain, ':'))) {
            *password_begin++ = 0;
            strcpy(password, password_begin);

        } else
            *password = 0;

        strcpy(username, tmp_domain);

        if ((port_begin = strchr(domain_begin, ':'))) {
            *port_begin++ = 0;
            *port = atoi(port_begin);

            if (*port > 65535)
                return -1;

        } else
            *port = 21;

        strcpy(domain, domain_begin);

    } else {
        if ((port_begin = strchr(tmp_domain, ':'))) {
            *port_begin++ = 0;
            *port = atoi(port_begin);

            if (*port > 65535)
                return -1;

        } else
            *port = 21;

        *username = *password = 0;
        strcpy(domain, tmp_domain);
    }

    return 0;
}

int CDownLoadThread::ftp_receive(
    struct ftp_host_info_s *ftp_info,
    FILE *socket_file,
    const char *default_path,
    const char *server_path,
    const char *suffix
)
{
    char pasv_reply[2048] = { 0 };

    if (!server_path) {
        if (!dl_para.thread_key)
            return -1;

        ::PostThreadMessage(
            dl_para.thread_key
            dl_para.mtype,
            nullptr,
            DOWNLOAD_ERROR_4
        );
        return -1;
    }
}

int CDownLoadThread::ftpcmd(
    const char *cmd,
    const char *arg,
    FILE *socket_file,
    char *recv_buf
)
{
    char *crlf = nullptr;
    int result = 0;

    if (cmd)
        if (arg)
            fprintf(socket_file, "%s %s\r\n", cmd, arg);

        else
            fprintf(socket_file, "%s\r\n", cmd);

    while (fgets(recv_buf, 256, socket_file)) {
        if ((crlf = strstr(recv_buf, "\r\n")))
            * crlf = 0;

        if (*recv_buf >= '0' && *recv_buf <= '9' && recv_buf[3] == ' ') {
            recv_buf[3] = 0;
            result = atoi(recv_buf);
            recv_buf[3] = ' ';
            return result;
        }
    }

    return -1;
}

int CDownLoadThread::get_local_filename(
    char *server_path,
    char *suffix,
    char *filename
)
{
}

int CDownLoadThread::get_local_path(const char *default_path, char *final_path)
{
}

int CDownLoadThread::get_remote_file(
    char *domain,
    unsigned int port,
    char *server_path,
    char *default_path,
    char *suffix
)
{
}

int CDownLoadThread::get_surfix(char *filename, char *suffix)
{
}


enum URL_KIND CDownLoadThread::get_url_kind(const char *url)
{
    if (!url || strlen(url) > 2048)
        return URL_INVALID;

    if (!strncasecmp(url, "http://", 7))
        return URL_HTTP;

    if (!strncasecmp(url, "ftp://", 6))
        return URL_FTP;

    return URL_OTHER;
}

void CDownLoadThread::http_del_blank(char *str)
{
    char *ptr = str;

    do
        if (*str != ' ')
            *ptr++ = *str;

    while (*str && *str != '\r' && *str++ != '\n');
}

int CDownLoadThread::http_download(
    char *url,
    char *default_path,
    char *suffix,
    char *domain,
    char *path,
    unsigned int *port
)
{
}

int CDownLoadThread::http_parse_response_head(
    char *header,
    unsigned long *length
)
{
}

int CDownLoadThread::http_parse_url(
    char *url,
    char *domain,
    char *path,
    unsigned int *port
)
{
}

int CDownLoadThread::http_send(
    int *fd,
    char *domain,
    unsigned int port,
    char *path
)
{
}

int CDownLoadThread::is_exist_dir(char *dir)
{
}

void CDownLoadThread::set_nport(
    struct len_and_sockaddr *addr,
    unsigned int port
)
{
    if (addr->addr.sin_family = AF_INET)
        addr->addr.sin_port = port;
}

struct len_and_sockaddr *CDownLoadThread::str2sockaddr(
    const char *straddr,
    int port,
    int flag
)
{
    struct addrinfo hint = { flag & ~AI_CANONNAME, AF_INET, SOCK_STREAM };
    struct addrinfo *res = nullptr;
    struct len_and_sockaddr *ret = nullptr;

    if (getaddrinfo(straddr, nullptr, &hint, &res) || !res) {
        freeaddrinfo(res);
        return nullptr;
    }

    if (!(ret = new struct len_and_sockaddr))
        return nullptr;

    ret->len = res->ai_addrlen;
    ret->addr = *reinterpret_cast<struct sockaddr_in *>(res->ai_addr);
    set_nport(ret, htons(port));
    return ret;
}

int CDownLoadThread::xconnect_ftpdata(
    struct ftp_host_info_s *hostinfo,
    char *pasv_reply
)
{
    unsigned int port = 0;
    int ptmp = 0;
    char *tmp = nullptr;

    if ((tmp = strrchr(pasv_reply, ')')))
        * tmp = 0;

    if ((tmp = strrchr(pasv_reply, ',')))
        * tmp++ = 0;

    else {
        rj_printf_debug("pasv parse error\n");
        return -1;
    }

    if ((ptmp = atoi(tmp)) < 0 || ptmp > 255) {
        rj_printf_debug("pasv tras num failed\n");
        return -1;
    }

    port = ptmp << 8;

    if ((tmp = strrchr(pasv_reply, ',')))
        * tmp++ = 0;

    else {
        rj_printf_debug("pasv parse error\n");
        return -1;
    }

    if ((ptmp = atoi(tmp)) < 0 || ptmp > 255) {
        rj_printf_debug("pasv tras num failed\n");
        return -1;
    }

    port |= ptmp;
    set_nport(hostinfo->addr, port);
    return xconnect_stream(hostinfo->addr);
}
