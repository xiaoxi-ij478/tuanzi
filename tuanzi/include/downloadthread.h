#ifndef DOWNLOADTHREAD_H_INCLUDED
#define DOWNLOADTHREAD_H_INCLUDED

#include "lnxthread.h"
#include "miscdefs.h"
#include "cmdutil.h"

struct len_and_sockaddr {
    int len;
    struct sockaddr_in addr;
};

struct ftp_host_info_s {
    char *username;
    char *password;
    struct len_and_sockaddr *addr;
};

enum URL_KIND {
    URL_INVALID = -1,
    URL_OTHER,
    URL_HTTP,
    URL_FTP
};

enum DOWNLOAD_STATUS {
    DOWNLOAD_OK,
    DOWNLOAD_ERROR_1,
    DOWNLOAD_ERROR_2,
    DOWNLOAD_ERROR_3,
    DOWNLOAD_ERROR_4,
    DOWNLOAD_ERROR_5,
    DOWNLOAD_ERROR_6,
    DOWNLOAD_ERROR_7,
    DOWNLOAD_ERROR_8,
    DOWNLOAD_ERROR_9,
};

class CDownLoadThread : public CLnxThread
{
    public:
        CDownLoadThread();
        ~CDownLoadThread() override;
        void SetDlPara(struct tagDownLoadPara& para);

    protected:
        void DispathMessage(struct LNXMSG *msg) override;

    private:
        DECLARE_DISPATH_MESSAGE_HANDLER(OnStartThread);
        bool download(
            const char *url,
            const char *save_path,
            const char *save_filename,
            bool create_progress_dialog
        );
        int ftp_download(
            const char *url,
            const char *save_path,
            const char *save_filename,
            char *domain,
            char *path,
            unsigned *port,
            char *username,
            char *password
        );
        int ftp_get_len(char *reply, unsigned long *length) const;
        FILE *ftp_login(struct ftp_host_info_s *hostinfo) const;
        int ftp_parse_url(
            const char *url,
            char *domain,
            char *path,
            char *username,
            char *password,
            unsigned *port
        ) const;
        int ftp_receive(
            struct ftp_host_info_s *ftp_info,
            FILE *socket_file,
            const char *save_path,
            const char *server_path,
            const char *save_filename
        );
        int ftpcmd(
            const char *cmd,
            const char *arg,
            FILE *socket_file,
            char *recv_buf
        ) const;
        int get_local_filename(
            const char *server_path,
            const char *save_filename,
            char *filename
        ) const;
        int get_local_path(const char *save_path, char *final_path) const;
        int get_remote_file(
            const char *domain,
            unsigned port,
            const char *server_path,
            const char *save_path,
            const char *save_filename
        );
        int get_surfix(const char *filename, char *suffix) const;
        enum URL_KIND get_url_kind(const char *url) const;
        void http_del_blank(char *str) const;
        int http_download(
            const char *url,
            const char *save_path,
            const char *save_filename,
            char *domain,
            char *path,
            unsigned *port
        );
        int http_parse_response_head(
            const char *header,
            unsigned long *length
        ) const;
        int http_parse_url(
            const char *url,
            char *domain,
            char *path,
            unsigned *port
        ) const;
        int http_send(
            int *fd,
            const char *domain,
            unsigned port,
            const char *path
        ) const;
        int is_exist_dir(const char *dir) const;
        void set_nport(struct len_and_sockaddr *addr, unsigned port) const;
        struct len_and_sockaddr *str2sockaddr(
            const char *straddr,
            int port,
            int flag
        ) const;
        int xconnect(int fd, const sockaddr *addr, socklen_t addrlen) const;
        int xconnect_ftpdata(
            struct ftp_host_info_s *hostinfo,
            char *pasv_reply
        ) const;
        int xconnect_stream(struct len_and_sockaddr *addr) const;

        struct tagDownLoadPara dl_para;
        bool create_progress_dialog;
        bool down_para_set;
        bool update_progress_dialog;
        bool read_only_once; // for test?
};

#endif // DOWNLOADTHREAD_H_INCLUDED
