#ifndef DNSQUERY_H_INCLUDED
#define DNSQUERY_H_INCLUDED

struct CHostEnt;

struct DNSQueryStruct {
    unsigned long mtype;
    char buf[1024];
};

#define DNSQueryStruct_MSGSZ \
    (sizeof(struct DNSQueryStruct) - offsetof(struct DNSQueryStruct, buf))

class CDNSQuery
{
    public:
        static int PostQueryByName(
            const char *hostname,
            struct CHostEnt **dest
        );
        static int QueryByName(const char *hostname, struct CHostEnt **dest);
        static bool StartQueryThread(char *errmsg);
        static int StopQueryThread();
        static bool thread_is_running();

    private:
        static void UpdateHostEntList(struct CHostEnt *entry);
        static void AddToHostEntList(struct CHostEnt *entry);
        static void GetHostByName(char *hostname, struct CHostEnt **dest);
        static void *thread_function(void *arg);

        static bool running; // m_bRunning
        static int msgid; // m_msgid
        static int timeout; // m_timeOut; seems to like TTL
        static pthread_t thread_id; // m_thread
        static pthread_mutex_t mutex_list; // m_mutex_list
        static struct CHostEnt *hostent_list_hdr; // m_hostEntListHdr
};

#endif // DNSQUERY_H_INCLUDED
