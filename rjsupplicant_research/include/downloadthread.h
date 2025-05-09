#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H

#include "lnxthread.h"

// various helper structs & enums

struct len_and_sockaddr {
    int len;
    struct sockaddr addr;
}

struct ftp_host_info_s {
    char *username;
    char *password;
    struct len_and_sockaddr *addr;
}


class CDownloadThread : public CLnxThread
{
    public:
        CDownloadThread();
        virtual ~CDownloadThread();

    protected:

    private:
};

#endif // DOWNLOADTHREAD_H
