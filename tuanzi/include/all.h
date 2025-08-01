#ifndef ALL_H_INCLUDED
#define ALL_H_INCLUDED

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <csignal>
#include <cstdarg>
#include <cstdbool>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <numeric>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <byteswap.h>
#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <features.h>
#include <getopt.h>
#include <iconv.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <linux/ethtool.h>
#include <linux/hdreg.h>
#include <linux/sockios.h>
#include <linux/wireless.h>
#include <net/route.h>
#include <netinet/ether.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <scsi/scsi.h>
#include <scsi/sg.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/select.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifdef __GLIBC__
#include <gnu/libc-version.h>
#endif // __GLIBC__

#include "mmd5.h"
#include "d3des.h"
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
