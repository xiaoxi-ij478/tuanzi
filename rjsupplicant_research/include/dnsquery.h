#ifndef DNSQUERY_H_INCLUDED
#define DNSQUERY_H_INCLUDED

#define POST_DNS_QUERY_MTYPE 0x7DA
#define STOP_DNS_QUERY_MTYPE 0x7DB

struct CHostEnt {
    struct hostent hostent_entry;
    struct CHostEnt *hostent_next;
    long last_update_time;
};

struct DNSQueryStruct {
    unsigned long mtype;
    char buf[1024];
};

#define DNSQueryStruct_MSGSZ \
    (sizeof(struct DNSQueryStruct) - offsetof(struct DNSQueryStruct, buf))

class CDNSQuery
{
    public:
        static int PostQueryByName(const char *hostname, struct CHostEnt **dest);
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
        static pthread_t thread_key; // m_thread
        static pthread_mutex_t mutex_list; // m_mutex_list
        static struct CHostEnt *hostent_list_hdr; // m_hostEntListHdr
};

#endif // DNSQUERY_H_INCLUDED
