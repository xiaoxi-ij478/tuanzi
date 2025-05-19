#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

class CHttpConnection
{
    public:
        CHttpConnection();
        ~CHttpConnection();

        void addRequestHeader(const char *header);
        int getErrorCode() const;
        const std::string &getErrorText() const;
        int getLength() const;
        void setTimeout(int timeout);
        void httpClose();
        int httpConnect(const char *url);
        int httpRead(unsigned char *buf, int buflen);

    private:
        int getHttpContentLength(const char *header) const;
        int makeSocket(const char *addr, unsigned short port);
        bool parseUrl(
            const char *url,
            std::string &domain,
            int &port,
            std::string &path
        );
        bool readHttpHeader(int fd);
        int sendRequest(const char *addr, int port, const char *request);
        [[gnu::format(printf, 2, 3)]]
        void setError(const char *format, ...);
        void setErrorCode(int error);
        void setErrorCode(int error, const char *prefix);

        int error_num; // errno
        std::string error_msg;
        int connect_timeout;
        int socket_fd;
        char reply_header[1024];
        int content_length;
        std::string http_request_version;
        std::string http_request_header;
};

#endif // HTTPCONNECTION_H
