#ifndef ALL_H_INCLUDED
#define ALL_H_INCLUDED

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <arpa/inet.h>
#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <features.h>
#include <getopt.h>
#include <iconv.h>
#include <ifaddrs.h>
#include <linux/ethtool.h>
#include <linux/hdreg.h>
#include <linux/sockios.h>
#include <linux/wireless.h>
//#include <net/if.h>
#include <net/route.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <scsi/scsi.h>
#include <scsi/sg.h>
#include <semaphore.h>
#include <sys/ioctl.h>
#include <sys/msg.h>
#include <sys/select.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#ifdef __GLIBC__
    #include <gnu/libc-version.h>
#endif // __GLIBC__

#include "mmd5.h"
#include "tinyxml.h"
#include "iniparser.h"
#include "crc16.h"
#include "sha1.h"
#include "tiger.h"
#include "whirlpool.h"
#include "ripemd128.h"
#include "rc4.h"
#include "pcap.h"

#endif // ALL_H_INCLUDED
