#ifndef DISKUTIL_H_INCLUDED
#define DISKUTIL_H_INCLUDED

int get_sata_serial(int fd, char *dst);
int getdiskid(char *buf, int buflen);

#endif // DISKUTIL_H_INCLUDED
