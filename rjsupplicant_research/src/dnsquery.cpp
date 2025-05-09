#include "timeutil.h"
#include "global.h"
#include "dnsquery.h"

bool CDNSQuery::running = false;
int CDNSQuery::msgid = 0;
int CDNSQuery::timeout = 60000;
pthread_t CDNSQuery::thread_key;
pthread_mutex_t CDNSQuery::mutex_list;
struct CHostEnt *CDNSQuery::hostent_list_hdr = nullptr;

static void copy_hostent(struct hostent *src, struct hostent *dst)
{
    int alias_count = 0, addr_count = 0;
    dst->h_addrtype = src->h_addrtype;
    dst->h_length = src->h_length;

    if (src->h_name) {
        dst->h_name = new char[strlen(src->h_name) + 1];
        strcpy(dst->h_name, src->h_name);
    }

    if (src->h_aliases) {
        for (char **alias = src->h_aliases; *alias; alias++, alias_count++);

        dst->h_aliases = new char *[alias_count + 1];

        for (
            char **salias = src->h_aliases,
            **dalias = dst->h_aliases;
            *salias;
            salias++, dalias++
        )
            strcpy(*dalias = new char[strlen(*salias) + 1], *salias);
    }

    dst->h_aliases[alias_count] = nullptr;

    if (src->h_addr_list) {
        for (char **addr = src->h_addr_list; *addr; addr++, addr_count++);

        dst->h_addr_list = new char *[addr_count + 1];

        for (
            char **saddr = src->h_addr_list,
            **daddr = dst->h_addr_list;
            *saddr;
            saddr++, daddr++
        )
            strcpy(*daddr = new char[strlen(*saddr) + 1], *saddr);
    }

    dst->h_addr_list[addr_count] = nullptr;
}

static void delete_hostent(struct hostent *entry)
{
    delete[] entry->h_name;

    if (entry->h_aliases) {
        for (char **alias = entry->h_aliases; *alias; alias++)
            delete[] *alias;

        delete[] entry->h_aliases;
    }

    if (entry->h_addr_list) {
        for (char **addr = entry->h_addr_list; *addr; addr++)
            delete[] *addr;

        delete[] entry->h_addr_list;
    }
}

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

    query.mtype = POST_DNS_QUERY_MTYPE;
    strcpy(query.buf, buf);

    if (msgsnd(msgid, &query, DNSQueryStruct_MSGSZ, 0) == -1) {
        std::cerr << "msgsnd failed" << std::endl;
        return 2;
    }

    return 1;
}

int CDNSQuery::QueryByName(const char *hostname, struct CHostEnt **dest)
{
    int list_len = 1;
    int ret = 0;
    char buf[1024] = { 0 };
    bool found = false;
    long cur_time = GetTickCount();
    int alias_count = 0;
    int addr_count = 0;
    struct CHostEnt *cur_ent = nullptr;

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

    if (!hostent_list_hdr) {
        pthread_mutex_unlock(&mutex_list);
        g_logFile_dns.AppendText("\t list len:%d; ret val %d", 0, 1);
        return 1;
    }

#define CUR_ENT_HOSTENT cur_ent->hostent_entry

    for (
        cur_ent = hostent_list_hdr;
        cur_ent;
        cur_ent = cur_ent->hostent_next, list_len++, found = false
    ) {
        g_logFile_dns.AppendText(
            "\t item[%d] h_name:%s",
            list_len,
            CUR_ENT_HOSTENT.h_name
        );

        for (char **alias = CUR_ENT_HOSTENT.h_aliases; *alias; alias++) {
            g_logFile_dns.AppendText("\t h_aliases:%s", *alias);

            if (!strcmp(*alias, hostname)) {
                found = true;
                break;
            }
        }

        if (found || !strcmp(CUR_ENT_HOSTENT.h_name, hostname)) {
            found = true;
            break;
        }

        if (cur_time - timeout) {
            g_logFile_dns.AppendText(
                "\t this item is too old now(%u) time(%u) CLOCKS_PER_SEC(%u)",
                cur_time,
                cur_ent->last_update_time,
                1000000
            );
            UpdateHostEntList(cur_ent);
            pthread_mutex_unlock(&mutex_list);
            g_logFile_dns.AppendText("\t list len:%d; ret val %d", list_len, 1);
            return 1;
        }
    }

#undef CUR_ENT_HOSTENT

    if (!found) {
        pthread_mutex_unlock(&mutex_list);
        g_logFile_dns.AppendText("\t list len:%d; ret val %d", list_len, 1);
        return 1;
    }

    g_logFile_dns.AppendText("\t equal");
    *dest = new CHostEnt;
    copy_hostent(&cur_ent->hostent_entry, &(*dest)->hostent_entry);
    (*dest)->hostent_next = nullptr;
    (*dest)->last_update_time = 0;
    pthread_mutex_unlock(&mutex_list);
    g_logFile_dns.AppendText("\t list len:%d; ret val %d", list_len, ret);
    return 1;
}

bool CDNSQuery::StartQueryThread(char *errmsg)
{
    pthread_mutexattr_t mutexattr;
    int lmsgid = 0;
    char exename[1024] = { 0 };
    g_logFile_dns.CreateLogFile_S(g_strAppPath + "log/dns_query.log", 1);

    if (running) {
        if (errmsg)
            strcpy(errmsg, "Thread is running,please exit first");

        return false;
    }

    if (pthread_mutexattr_init(&mutexattr)) {
        if (errmsg)
            strcpy(errmsg, "mutex attr init failed");

        return false;
    }

    if (pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE)) {
        if (errmsg)
            strcpy(errmsg, "mutex attr set type failed");

        return false;
    }

    if (pthread_mutex_init(&mutex_list, &mutexattr)) {
        pthread_mutexattr_destroy(&mutexattr);

        if (errmsg)
            strcpy(errmsg, "Mutex initialization failed");

        return false;
    }

    pthread_mutexattr_destroy(&mutexattr);

    if (readlink("/proc/self/exe", exename, sizeof(exename)) >= 1024) {
        if (errmsg)
            sprintf(errmsg, "readlink with error: %s\n", strerror(errno));

        return false;
    }

    if ((lmsgid = ftok(exename, 0x17)) == -1) {
        if (errmsg)
            sprintf(errmsg, "ftok with error: %s\n", strerror(errno));

        return false;
    }

    if ((msgid = msgget(lmsgid, 0666 | IPC_CREAT)) == -1) {
        if (errmsg)
            sprintf(errmsg, "msgget failed with error: %d\n", errno);

        pthread_mutex_destroy(&mutex_list);
        return false;
    }

    if (pthread_create(&thread_key, nullptr, &thread_function, nullptr)) {
        if (errmsg)
            strcpy(errmsg, "Thread creation failed");

        pthread_mutex_destroy(&mutex_list);
        return false;
    }

    running = true;
    return true;
}

int CDNSQuery::StopQueryThread()
{
    struct DNSQueryStruct msg = { STOP_DNS_QUERY_MTYPE };

    if (!thread_is_running())
        return -1;

    // this message does not contain other information
    // so msgsz == 0
    if (msgsnd(msgid, &msg, 0, 0) == -1) {
        std::cerr << "msgsnd failed" << std::endl;
        return -2;
    }

    if (pthread_join(thread_key, nullptr)) {
        perror("Thread join failed");
        return -3;
    }

    pthread_mutex_lock(&mutex_list);

    for (
        struct CHostEnt *nent = hostent_list_hdr->hostent_next;
        hostent_list_hdr;
        hostent_list_hdr = nent,
        nent = hostent_list_hdr ? nullptr : hostent_list_hdr->hostent_next
    ) {
        delete_hostent(&hostent_list_hdr->hostent_entry);
        delete hostent_list_hdr;
    }

    hostent_list_hdr = nullptr;
    pthread_mutex_unlock(&mutex_list);

    if (pthread_mutex_destroy(&mutex_list))
        perror("thread mutex destroy error\n");

    if (msgctl(msgid, IPC_RMID, nullptr))
        perror("Thread msgid clear error\n");

    running = false;
    return 0;
}

bool CDNSQuery::thread_is_running()
{
    return running;
}

void CDNSQuery::UpdateHostEntList(struct CHostEnt *entry)
{
    struct CHostEnt *to_be_deleted = nullptr;
    pthread_mutex_lock(&mutex_list);

    for (
        to_be_deleted = hostent_list_hdr;
        to_be_deleted != entry;
        to_be_deleted = to_be_deleted->hostent_next
    );

    if (!to_be_deleted) {
        pthread_mutex_unlock(&mutex_list);
        return;
    }

    if (to_be_deleted == hostent_list_hdr)
        hostent_list_hdr = nullptr;

    for (
        struct CHostEnt *nent = to_be_deleted->hostent_next;
        to_be_deleted;
        to_be_deleted = nent,
        nent = to_be_deleted ? nullptr : to_be_deleted->hostent_next
    ) {
        delete_hostent(&to_be_deleted->hostent_entry);
        delete to_be_deleted;
    }

    pthread_mutex_unlock(&mutex_list);
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
    struct hostent *entry = nullptr;
    int alias_count = 0;
    int addr_count = 0;

    if (*hostname && hostname[strlen(hostname) - 1] == '\n')
        hostname[strlen(hostname) - 1] = 0;

    g_logFile_dns.AppendText("In GetHostByName host name:%s\n", hostname);
    *dest = new CHostEnt;

    if (!(entry = gethostbyname(hostname))) {
        g_logFile_dns.AppendText("\t Host not found....");
        (*dest)->hostent_entry.h_name = new char[strlen(hostname) + 1];
        strcpy((*dest)->hostent_entry.h_name, hostname);
        return;
    }

    g_logFile_dns.AppendText("\t get host by name success.\n");
    copy_hostent(entry, &(*dest)->hostent_entry);
}

void *CDNSQuery::thread_function(void *arg)
{
    int rcv_msgid = 0;
    struct DNSQueryStruct msg = { 0 };
    struct CHostEnt *entry = nullptr;
    g_logFile_dns.AppendText("QueryDNSThread start,running...");

    while (rcv_msgid != STOP_DNS_QUERY_MTYPE) {
        if (msgrcv(msgid, &msg, DNSQueryStruct_MSGSZ, 0, 0) == -1) {
            std::cerr << "msgrcv failed with error: " << errno << std::endl;
            exit(1);
        }

        if (msg.mtype != POST_DNS_QUERY_MTYPE)
            continue;

        GetHostByName(msg.buf, &entry);
        AddToHostEntList(entry);
        g_logFile_dns.AppendText("Add dns query result To list");
    }

    g_logFile_dns.AppendText("Quit reason-rev quit msg");
    g_logFile_dns.AppendText("QueryDNSThread quit");
    return nullptr;
}
