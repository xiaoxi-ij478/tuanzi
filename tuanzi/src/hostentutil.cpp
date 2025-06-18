#include "all.h"
#include "hostentutil.h"

void copy_hostent(struct hostent *src, struct hostent *dst)
{
    int alias_count = 0, addr_count = 0;

    if (!src || !dst)
        return;

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
            memcpy(*daddr = new char[src->h_length], *saddr, src->h_length);
    }

    dst->h_addr_list[addr_count] = nullptr;
}

void delete_hostent(struct hostent *entry)
{
    if (!entry)
        return;

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

    delete entry;
}
