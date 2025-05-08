#include "timeutil.h"
#include "global.h"
#include "dnsquery.h"

bool CDNSQuery::running = false;
int CDNSQuery::msgid = 0;
int CDNSQuery::timeout = 60000;
pthread_mutex_t CDNSQuery::mutex_list;
struct CHostEnt *CDNSQuery::hostent_list_hdr = nullptr;

int CDNSQuery::PostQueryByName(const char *hostname, struct CHostEnt **dest)
{
    struct DNSQueryStruct query = { 0 };
    int ret = 0;
    char buf[1024] = { 0 };

    if (strlen(hostname) > 1023)
        return -2;

    strcpy(buf, hostname);

    if (*buf && buf[strlen(buf) - 1] == '\n')
        buf[strlen(buf) - 1] = 0;

    if ((ret = QueryByName(buf, dest)) != 1)
        return ret;

    DNSQueryStruct.mtype = POST_DNS_QUERY_MTYPE;
    strcpy(query.buf, buf);

    if (msgsnd(msgid, &query, sizeof(query), 0) == -1) {
        std::cerr << "msgsnd failed" << std::endl;
        return 2;
    }

    return 1;
}

int CDNSQuery::QueryByName(const char *hostname, struct CHostEnt **dest)
{
    int list_len = 0;
    int ret=0;
    char buf[1024] = { 0 };

    if (strlen(hostname) > 1023)
        return -2;

    strcpy(buf, hostname);

    if (*buf && buf[strlen(buf) - 1] == '\n')
        buf[strlen(buf) - 1] = 0;

    if (!running)
        return -1;

    if (!dest)
        return -2;

    pthread_mutex_lock(&mutex_list);

    if (!hostent_list_hdr)
        {
            ret=1;
            goto done;
        }

    list_len = 1;

    if (timeout < GetTickCount() - hostent_list_hdr->last_update_time) {
        g_logFile_dns.AppendText(
            "\t this item is too old now(%u) time(%u) CLOCKS_PER_SEC(%u)",
            GetTickCount(),
            hostent_list_hdr->last_update_time,
            1000000
        );
        UpdateHostEntList(hostent_list_hdr);
            ret=1;
        goto done;
    }

done:
    pthread_mutex_unlock(&mutex_list);
    g_logFile_dns.AppendText("\t list len:%d; ret val %d", list_len, ret);
    return 1;
}

bool CDNSQuery::StartQueryThread(char *errmsg)
{
}

int CDNSQuery::StopQueryThread()
{
}

bool CDNSQuery::thread_is_running()
{
}

void CDNSQuery::UpdateHostEntList(struct CHostEnt *entry)
{
}

void CDNSQuery::AddToHostEntList(struct CHostEnt *entry)
{
    if (!entry)
        return;

    g_logFile_dns.AppendText(
        "AddToHostEntList h_name:%s",
        entry->hostent_entry.h_name
    );

    for (char **alias = entry->hostent_entry.h_aliases; *alias; alias++)
        g_logFile_dns.AppendText(" name:%s", *alias);

    pthread_mutex_lock(&mutex_list);
    entry->last_update_time = GetTickCount();
    entry->hostent_next = hostent_list_hdr;
    hostent_list_hdr = entry;
    pthread_mutex_unlock(&mutex_list);
}

void CDNSQuery::GetHostByName(char *hostname, struct CHostEnt **dest)
{
#define DEST_HOST_ENTRY (*dest)->hostent_entry
    struct hostent *entry = nullptr;
    int alias_count = 0;
    int addr_count = 0;
    char

    if (*hostname && hostname[strlen(hostname) - 1] == '\n')
        hostname[strlen(hostname) - 1] = 0;

    g_logFile_dns.AppendText("In GetHostByName host name:%s\n", hostname);

    if (!(entry = gethostbyname(hostname))) {
        g_logFile_dns.AppendText("\t Host not found....");
        *dest = new CHostEnt;
        DEST_HOST_ENTRY.h_name = new char[strlen(hostname) + 1];
        strcpy(DEST_HOST_ENTRY.h_name, hostname);
        return;
    }

    *dest = new CHostEnt;
    g_logFile_dns.AppendText("\t get host by name success.\n");
    DEST_HOST_ENTRY.h_addrtype = entry->h_addrtype;
    DEST_HOST_ENTRY.h_length = entry->h_length;

    if (entry->h_name) {
        DEST_HOST_ENTRY.h_name = new char[strlen(entry->h_name) + 1];
        strcpy(DEST_HOST_ENTRY.h_name, entry->h_name);
    }

    if (entry->h_aliases) {
        for (char **alias = entry->h_aliases; *alias; alias++, alias_count++);

        DEST_HOST_ENTRY.h_aliases = new char *[alias_count + 1];

        for (
            char **salias = entry->h_aliases,
            **dalias = DEST_HOST_ENTRY.h_aliases;
            *salias;
            salias++, dalias++
        )
            strcpy((*dalias = new char[strlen(*salias) + 1]), *salias);
    }

    DEST_HOST_ENTRY.h_aliases[alias_count] = nullptr;

    if (entry->h_addr_list) {
        for (char **addr = entry->h_addr_list; *addr; addr++, addr_count++);

        DEST_HOST_ENTRY.h_addr_list = new char *[addr_count + 1];

        for (
            char **saddr = entry->h_addr_list,
            **daddr = DEST_HOST_ENTRY.h_addr_list;
            *saddr;
            saddr++, daddr++
        )
            strcpy((*daddr = new char[strlen(*saddr) + 1]), *saddr);
    }

    DEST_HOST_ENTRY.h_addr_list[addr_count] = nullptr;
#undef DEST_HOST_ENTRY
}

void *CDNSQuery::thread_function(void *arg)
{
}
