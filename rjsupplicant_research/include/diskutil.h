#ifndef DISKUTIL_H_INCLUDED
#define DISKUTIL_H_INCLUDED

extern int get_sata_serial(int fd, char *dst);
extern int getdiskid(char *buf, int buflen);

#endif // DISKUTIL_H_INCLUDED
