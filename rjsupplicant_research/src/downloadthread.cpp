#include "cmdutil.h"
#include "threadutil.h"
#include "lnxthread.h"
#include "fileutil.h"
#include "global.h"
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

    return true;
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
    char domain[1024] = { 0 };
    char path[1024] = { 0 };
    char username[1024] = { 0 };
    char password[1024] = { 0 };
    unsigned int port = 0;
    assert(dl_para.thread_key);
    dl_para.create_progress_dialog = create_progress_dialog;

    switch (get_url_kind(url)) {
        case URL_HTTP:
            return http_download(url, default_path, suffix, domain, path, &port) != -1;

        case URL_FTP:
            return ftp_download(
                       url, default_path, suffix, domain, path, &port, username, password
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

    strncpy(buf, sizepos + strlen("213"), sizeof(buf));
    http_del_blank(buf);
    *length = strtoul(buf, nullptr, 10);
    return 0;
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
    char *path_begin = nullptr;
    char *domain_begin = nullptr;
    char *password_begin = nullptr;
    char *port_begin = nullptr;
    char tmp_url[2048] = { 0 };
    char tmp_domain[2048] = { 0 };
    *port = 21;
    *password = 0;
    strcpy(username, "anonymous");

    if (strlen(url) <= 6)
        return -1;

    if (strncasecmp(url, "ftp://", 6))
        return -1;

    strcpy(tmp_url, url + 6);

    if ((path_begin = strchr(tmp_url, '/'))) {
        *path_begin++ = 0;
        strcpy(path, path_begin);
    }

    strcpy(tmp_domain, tmp_url);

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
            *port = strtol(port_begin, nullptr, 10);

            if (*port > 65535)
                return -1;
        }

        strcpy(domain, domain_begin);

    } else {
        if ((port_begin = strchr(tmp_domain, ':'))) {
            *port_begin++ = 0;
            *port = strtol(port_begin, nullptr, 10);

            if (*port > 65535)
                return -1;
        }

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
    char reply[264] = { 0 };
    char save_path[2048] = { 0 };
    char dir[1024] = { 0 };
    char filename[1024] = { 0 };
    char *new_save_path = nullptr;
    int socket_fd = 0;
    int file_fd = 0;
    int tmpret = 0;
    enum DOWNLOAD_STATUS ret = DOWNLOAD_OK;
    unsigned long read_byte = 0;
    unsigned long total_read_byte = 0;
    unsigned long file_size = 0;

    if (!server_path ||
            !ftpcmd("PASV", nullptr, socket_file, reply) ||
            ((socket_fd = xconnect_ftpdata(ftp_info, reply) == -1))) {
        ret = DOWNLOAD_ERROR_4;
        goto quit;
    }

    if (ftpcmd("SIZE", server_path, socket_file, reply) != 213 ||
            ftp_get_len(reply, &file_size) == -1 ||
            (tmpret = ftpcmd("RETR", server_path, socket_file, reply)) != 125 &&
            tmpret != 150
       ) {
        ret = DOWNLOAD_ERROR_5;
        goto quit;
    }

    if ((default_path && strlen(default_path) >= 2048) ||
            (suffix && strlen(suffix) >= 2048)) {
        rj_printf_debug("local path or name file is too long\n");
        ret = DOWNLOAD_ERROR_9;
        goto quit;
    }

    if (get_local_path(default_path, dir) == -1 ||
            get_local_filename(server_path, suffix, filename)) {
        ret = DOWNLOAD_ERROR_8;
        goto quit;
    }

    if (strlen(dir) + strlen(filename) >= 2048) {
        rj_printf_debug("path + name is too long\n");
        ret = DOWNLOAD_ERROR_9;
        goto quit;
    }

    strcpy(save_path, dir);
    strcat(save_path, "/");
    strcat(save_path, filename);

    if (cmd_mkdir(dir) || is_exist_dir(dir)) {
        ret = DOWNLOAD_ERROR_8;
        goto quit;
    }

    if ((file_fd = open(save_path, O_WRONLY | O_CREAT | O_TRUNC, 0777) == -1)) {
        std::cerr << "open path error:" << strerror(errno)
                  << std::endl;
        rj_printf_debug("open %s file error\n", save_path);
        ret = DOWNLOAD_ERROR_6;
        goto quit;
    }

    while (true) {
        read_byte = read(socket_fd, reply, 256);

        if (read_byte == -1) {
            ret = DOWNLOAD_ERROR_5;
            goto inner_quit;
        }

        if (!read_byte) {
            ret = DOWNLOAD_OK;
            goto inner_quit;
        }

        if (write(file_fd, reply, read_byte) == -1) {
            ret = DOWNLOAD_ERROR_3;
            goto inner_quit;
        }

        total_read_byte += read_byte;

        if (read_only_once) {
            ret = DOWNLOAD_OK;
            goto inner_quit;
        }
    }

inner_quit:
    close(socket_fd);

    if (update_progress_dialog)
        update_progress_dialog = false;

    if (ftpcmd(nullptr, nullptr, socket_file, reply) == 226) {
        if (total_read_byte != file_size)
            ret = DOWNLOAD_ERROR_2;

        goto quit;

    } else {
        rj_printf_debug("file download is not ok!\n");
        ret = DOWNLOAD_ERROR_2;
        goto quit;
    }

quit:

    if (!dl_para.thread_key) {
        if (ret)
            return -1;

        ftpcmd("QUIT", nullptr, socket_file, reply);

        if (file_fd)
            close(file_fd);

        return 0;
    }

    if (ret) {
        ::PostThreadMessage(dl_para.thread_key, dl_para.mtype, nullptr, ret);
        return -1;
    }

    new_save_path = new char[strlen(save_path) + 1];
    strcpy(new_save_path, save_path);

    if (!::PostThreadMessage(dl_para.thread_key, dl_para.mtype, new_save_path, 0))
        delete[] new_save_path;

    ftpcmd("QUIT", nullptr, socket_file, reply);

    if (file_fd)
        close(file_fd);

    return 0;
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
            result = strtol(recv_buf, nullptr, 10);
            recv_buf[3] = ' ';
            return result;
        }
    }

    return -1;
}

int CDownLoadThread::get_local_filename(
    const char *server_path,
    const char *suffix,
    char *filename
)
{
    const char *tmp = strrchr(server_path, '/');
    char buf[2048] = { 0 };

    if (!tmp) {
        std::cerr << "strrchr server path failed" << std::endl;
        return -1;
    }

    if (!tmp[1]) {
        std::cerr << "sever path end of / -- error" << std::endl;
        return -1;
    }

    if (!suffix) {
        strcpy(filename, &tmp[1]);
        return 0;
    }

    if (get_surfix(server_path, buf) == -1)
        return -1;

    if (!strrchr(suffix, '.')) {
        if (strlen(buf) + strlen(suffix) > 2048)
            return -1;

        strcpy(filename, suffix);
        strcat(filename, buf);
        return 0;
    }

    strcpy(filename, suffix);
    return 0;
}

int CDownLoadThread::get_local_path(
    const char *default_path,
    char *final_path
)
{
    char *tmp = nullptr;
    *final_path = 0;

    if (default_path) {
        strcpy(final_path, default_path);

        if (final_path[strlen(final_path) - 1] == '/')
            final_path[strlen(final_path) - 1] = 0;

        return 0;
    }

    strcpy(final_path, "./download/");
    tmp = strrchr(final_path, '/');

    if (!tmp || final_path == tmp) {
        std::cerr << "strrchr failed or default path is wrong format" << std::endl;
        return -1;
    }

    *tmp = 0;
    return 0;
}

int CDownLoadThread::get_remote_file(
    const char *domain,
    unsigned int port,
    const char *server_path,
    const char *default_path,
    const char *suffix
)
{
    int file_fd = 0;
    int socket_fd = 0;
    char dir[2048] = { 0 };
    char filename[2048] = { 0 };
    char save_path[2048] = { 0 };
    enum DOWNLOAD_STATUS ret = DOWNLOAD_OK;

    if (!*domain || !*server_path)
        return -1;

    if (http_send(socket_fd, domain, port, server_path) == -1) {
        ret = DOWNLOAD_ERROR_4;
        g_log_Wireless.AppendText("http_send failed.");
        file_fd = -1;
        goto error_quit;
    }

    if (default_path && strlen(default_path) > 0x7FF || suffix &&
            strlen(suffix) > 0x7FF) {
        rj_printf_debug("local path or name file is too long\n");
        ret = DOWNLOAD_ERROR_9;
        file_fd = -1;
        goto error_quit;
    }

    if (get_local_path(default_path, dir) == -1
            || get_local_filename(server_path, suffix, filename) == -1) {
        ret = DOWNLOAD_ERROR_8;
        file_fd = -1;
        goto error_quit;
    }

    if (strlen(dir) + strlen(filename) >= 2048) {
        rj_printf_debug("path + name is too long\n");
        ret = DOWNLOAD_ERROR_9;
        file_fd = -1;
        goto error_quit;
    }

    strcpy(save_path, dir);
    strcat(save_path, "/");
    strcat(save_path, filename);

    if (cmd_mkdir(dir) || is_exist_dir(dir)) {
        ret = DOWNLOAD_ERROR_9;
        file_fd = -1;
        goto error_quit;
    }

    if ((file_fd = open(save_path, O_WRONLY | O_CREAT | O_TRUNC, 0777)) == -1) {
        std::cerr << "open path error:" << strerror(errno) << std::endl;
        ret = DOWNLOAD_ERROR_6;
        goto error_quit;
    }

error_quit:

    if (socket_fd != -1)
        shutdown(socket_fd, SHUT_RDWR);

    if (file_fd != -1)
        close(file_fd);

    if (dl_para.thread_key)
        ::PostThreadMessage(dl_para.thread_key, dl_para.mtype, nullptr, ret);

    rj_printf_debug("nDownLoadResult=%d\n", ret);
    return -1;
}

int CDownLoadThread::get_surfix(const char *filename, char *suffix)
{
    const char *tmp = nullptr;
    tmp = strrchr(filename, '.');

    if (tmp)
        strcpy(suffix, tmp);

    return 0;
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
        if (*str++ != ' ')
            *ptr++ = *str;

    while (*str && *str != '\r' && *str != '\n');
}

int CDownLoadThread::http_download(
    const char *url,
    const char *default_path,
    const char *suffix,
    char *domain,
    char *path,
    unsigned int *port
)
{
    if (http_parse_url(url, domain, path, port) == -1) {
        g_log_Wireless.AppendText("http_parse_url failed");
        return -1;

    } else {
        g_log_Wireless.AppendText(
            "http_parse_url %s %s %s %d",
            url, domain, path, *port
        );

        if (get_remote_file(domain, *port, path, default_path, suffix) == -1)
            return -1;
    }

    return 0;
}

int CDownLoadThread::http_parse_response_head(
    const char *header,
    unsigned long *length
)
{
    const char *sizepos = strstr(header, "Content-Length:");
    char buf[25] = { 0 };

    if (!sizepos)
        return -1;

    strncpy(buf, sizepos + strlen("Content-Length:"), sizeof(buf));
    http_del_blank(buf);
    *length = strtoul(buf, nullptr, 10);
    return 0;
}

int CDownLoadThread::http_parse_url(
    const char *url,
    char *domain,
    char *path,
    unsigned int *port
)
{
    char *path_begin = nullptr;
    char *port_begin = nullptr;
    char tmp_url[2048] = { 0 };
    char tmp_domain[2048] = { 0 };
    *port = 80;

    if (strlen(url) <= 7)
        return -1;

    if (strncasecmp(url, "http://", 7))
        return -1;

    strcpy(tmp_url, url + 7);

    if ((path_begin = strchr(tmp_url, '/'))) {
        *path_begin++ = 0;
        strcpy(path, path_begin);
    }

    strcpy(tmp_domain, tmp_url);

    if ((port_begin = strchr(tmp_domain, ':'))) {
        *port_begin++ = 0;
        *port = strtol(port_begin, nullptr, 10);
    }

    strcpy(domain, tmp_domain);
    return 0;
}

int CDownLoadThread::http_send(
    int *fd,
    const char *domain,
    unsigned int port,
    const char *path
)
{
    unsigned char tmp_ipaddr[4] = { 0 };
    char send_buf[2048] = { 0 };
    struct sockaddr_in dest_ipaddr = { 0 };
    struct hostent *domain_resolve = nullptr;
    // maybe from one of their employee's browser xD
#define HTTP_REQUEST \
    "GET %s HTTP/1.1\r\n" \
    "User-Agent: Mozilla/4.0 (compatiable; MSIE 6.0; Windows NT 5.1; SV1; Maxthon)\r\n" \
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" \
    "Accept-Language: zh-cn\r\n" \
    "Accept-Encoding: gzip,deflate\r\n" \
    "If-Range: 8957d2fd1f40cb1:a43\r\n" \
    "Host: %s\r\n" \
    "Connection: Keep-Alive\r\n" \
    "\r\n"

    if (
        sscanf(
            domain,
            "%d.%d.%d.%d",
            &tmp_ipaddr[3],
            &tmp_ipaddr[2],
            &tmp_ipaddr[1],
            &tmp_ipaddr[0]
        ) == 4
//        && tmp_ipaddr[3] < 256
//        && tmp_ipaddr[2] < 256
//        && tmp_ipaddr[1] < 256
//        && tmp_ipaddr[0] < 256
    )
        dest_ipaddr.sin_addr.s_addr = inet_addr(domain);

    else {
        if (!(domain_resolve = gethostbyname(domain)))
            return -1;

        memcpy(&dest_ipaddr.sin_addr, domain_resolve->h_addr_list,
               domain_resolve->h_length);
    }

    dest_ipaddr.sin_port = htons(port);

    if ((*fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return -1;

    if ((connect(
                *fd,
                reinterpret_cast<struct sockaddr *>(&dest_ipaddr),
                sizeof(dest_ipaddr)
            )) == -1)
        return -1;

    if (strlen(path) + strlen(domain) + strlen(HTTP_REQUEST) > 2048)
        return -1;

    sprintf(send_buf, HTTP_REQUEST, path, domain);

    if (write(*fd, send_buf, strlen(send_buf)) == -1)
        return -1;

    return 0;
#undef HTTP_REQUEST
}

int CDownLoadThread::is_exist_dir(const char *dir)
{
    return access(dir, F_OK) == -1 ? -1 : 0;
}

void CDownLoadThread::set_nport(
    struct len_and_sockaddr *addr,
    unsigned int port
)
{
    if (addr->addr.sin_family == AF_INET)
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

    if ((ptmp = strtol(tmp, nullptr, 10)) < 0 || ptmp > 255) {
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

    if ((ptmp = strtol(tmp, nullptr, 10)) < 0 || ptmp > 255) {
        rj_printf_debug("pasv tras num failed\n");
        return -1;
    }

    port |= ptmp;
    set_nport(hostinfo->addr, port);
    return xconnect_stream(hostinfo->addr);
}
