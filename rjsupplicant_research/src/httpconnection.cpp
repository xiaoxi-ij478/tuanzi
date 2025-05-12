#include "httpconnection.h"

CHttpConnection::CHttpConnection() :
    error_num(0),
    connect_timeout(0),
    socket_fd(-1),
    content_length(-1),
    http_request_version("HTTP/1.0"),
    http_request_header(
        "Accept: text/*\r\n"
        "User-Agent: HttpConnection\r\n"
        "Accept-Language: en-us\r\n"
    )
{ }

CHttpConnection::~CHttpConnection()
{
    httpClose();
}

void CHttpConnection::addRequestHeader(const char *header)
{
    if (!header)
        return;

    http_request_header = header;
}

int CHttpConnection::getErrorCode()
{
    return error_num;
}

const std::string &CHttpConnection::getErrorText()
{
    return error_msg;
}

int CHttpConnection::getLength()
{
    return content_length;
}

void CHttpConnection::setTimeout(int timeout)
{
    connect_timeout = timeout;
}

void CHttpConnection::httpClose()
{
    if (socket_fd == -1)
        return;

    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
    socket_fd = -1;
}

int CHttpConnection::httpConnect(const char *url)
{
    std::string request;
    std::string domain;
    int port;
    std::string path;

    if (!parseUrl(url, domain, port, path))
        return -1;

    request
    .append("GET ").append(path).append(" ").append(http_request_version)
    .append("\r\n")
    .append(http_request_header).append("\r\n")
    .append("Host: ").append(domain).append("\r\n")
    .append("Cache-Control: no-cache\r\n\r\n");
    return sendRequest(domain.c_str(), port, request.c_str());
}

int CHttpConnection::httpRead(void *buf, int buflen)
{
}

int CHttpConnection::getHttpContentLength(const char *header)
{
    const char *version_begin = strstr(header, "HTTP/");
    const char *size_begin = nullptr;
    int status_code = 0;
    int size = 0;

    if (!version_begin)
        return -1;

    while (*version_begin++ != ' ');

    if (
        sscanf(version_begin, "%d", &status_code) != 1 ||
        status_code != 200
    )
        return -1;

    size_begin = strstr(header, "Content-Length:")
                 ? : strstr(header, "Content-length:");

    if (!size_begin)
        return -1;

    if (sscanf(size_begin + strlen("Content-Length:"), "%d", &size) != 1)
        return -1;

    return size;
}

int CHttpConnection::makeSocket(const char *addr, unsigned short port)
{
    struct sockaddr_in inaddr = { 0 };
    struct hostent *entry = nullptr;
    int fd = 0;

    if (!(entry = gethostbyname(addr))) {
        setError("makeSocket:gethostbyname: %s", hstrerror(h_errno));
        return -1;
    }

    memcpy(&inaddr.sin_addr, *entry->h_addr_list, entry->h_length);
    inaddr.sin_port = htons(port);
    inaddr.sin_family = entry->h_addrtype;

    if ((fd = socket(inaddr.sin_family, SOCK_STREAM, 0)) == -1) {
        setErrorCode(errno, "makeSocket:socket");
        return -1;
    }

    if (connect(
                fd,
                reinterpret_cast<struct sockaddr *>(&inaddr),
                sizeof(inaddr)
            ) == -1) {
        setErrorCode(errno, "makeSocket:connect");
        close(fd);
        return -1;
    }

    return fd;
}

bool CHttpConnection::parseUrl(
    const char *url,
    std::string &domain,
    int &port,
    std::string &path
)
{
    char *url_copy = nullptr;
    char *domain_begin = nullptr;
    char *path_begin = nullptr;
    char *port_begin = nullptr;

    if (!url)
        return false;

    url_copy = new char[strlen(url) + 1];
    strcpy(url_copy, url);

    if ((domain_begin = strstr(url_copy, "://"))) {
        *domain_begin = 0;
        domain_begin += 3;

    } else
        domain_begin = url_copy;

    if ((path_begin = strchr(domain_begin, '/'))) {
        *path_begin++ = 0;
        path = path_begin;

    } else
        path = "/";

    if ((port_begin = strchr(domain_begin, ':'))) {
        *port_begin++ = 0;
        port = strtol(port_begin, nullptr, 10);

    } else
        port = 80;

    domain = domain_begin;

    if (port < 1 || port > 65535) {
        setError("The url port is error.");
        return false;
    }

    delete[] url_copy;
    return true;
}

bool CHttpConnection::readHttpHeader(int fd)
{
    fd_set listen_fd;
struct timeval timeout = { connect_timeout,0 };
while (true)
{
    FD_ZERO(&listen_fd);
    FS_SET(fd,&listen_fd);
    timeout={connect_timeout,0};
    switch (
            select(
                   fd+1,&listen_fd,nullptr,nullptr,connect_timeout?&timeout:nullptr
                   )
            )
            {
            case 0:
                   setError( "readHttpHeader: time out.");

            }
}
}

int CHttpConnection::sendRequest(
    const char *addr,
    int port,
    const char *request
)
{
    int fd = 0;
    long written = 0;
    long remain = 0;

    if (!addr || !request)
        return -1;

    if ((fd = makeSocket(addr, port)) == -1)
        return -1;

    remain = strlen(request);

    while (remain) {
        written = write(fd, request, remain);

        switch (written)
        {
        case -1:
            setErrorCode(errno);
        case 0:
            close(fd);return -1;
        }
        remain -= written;
        request += written;
    }

    if (!readHttpHeader(fd)) {
        close(fd);
        return -1;
    }

    if ((content_length = getHttpContentLength(reply_header)) == -1) {
        setError("Failed to get http content length.");
        close(fd);
        return -1;
    }

    socket_fd = fd;
    return 0;
}

void CHttpConnection::setError(const char *format, ...)
{
    char buf[1024] = { 0 };
    va_list va;
    va_start(va, format);
    vsnprintf(buf, sizeof(buf), format, va);
    va_end(va);
    error_msg.append(buf).append("\n");
}

void CHttpConnection::setErrorCode(int error)
{
    error_num = error;

    if (!strerror(errno))
        return;

    error_msg.append(strerror(errno)).append("\n");
}

void CHttpConnection::setErrorCode(int error, const char *prefix)
{
    error_num = error;

    if (!strerror(errno))
        return;

    if (prefix)
        error_msg.append(prefix).append(": ");

    if (!strerror(errno))
        return;

    error_msg.append(strerror(errno)).append("\n");
}
