#include "all.h"
#include "cmdutil.h"
#include "util.h"
#include "diskutil.h"

int get_sata_serial(int fd, char *dst)
{
    static unsigned char args[0x200] = {};
    unsigned short pos = 0;
    struct sg_io_hdr hdr = {};
    unsigned char cmd_blk[] = {
        0x85, 0x08, 0x2E, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xEC, 0x00
    };
    unsigned char sense_blk[32] = {};
    hdr.interface_id = 'S';
    hdr.cmd_len = sizeof(cmd_blk);
    hdr.mx_sb_len = sizeof(sense_blk);
    hdr.dxfer_direction = SG_DXFER_FROM_DEV;
    hdr.dxfer_len = sizeof(args);
    hdr.dxferp = args;
    hdr.cmdp = cmd_blk;
    hdr.sbp = sense_blk;
    hdr.timeout = 10000;

    if (ioctl(fd, SG_IO, &hdr) == -1) {
        std::cerr << "SG_IO ioctl not supported" << std::endl;
        return -1;
    }

    if (
        hdr.host_status != 0x00 /* DID_OK */ ||
        hdr.driver_status != 0x08 /* DRIVER_SENSE */ || (
            hdr.status != 0x00 /* GOOD */ &&
            hdr.status != 0x02 /* CONDITION_GOOD */
        )
    ) {
        errno = EIO;
        return -2;
    }

    if (sense_blk[0] != 0x72 || sense_blk[7] <= 13) {
        errno = EIO;
        return -3;
    }

    if (sense_blk[8] != 0x09 || sense_blk[9] <= 11) {
        errno = EIO;
        return -4;
    }

    if (sense_blk[21] & 1) {
        errno = EIO;
        return -5;
    }

    // the original function is named as print_ascii, but we inline it here
    pos = 19;

    while (args[++pos] == ' ' && pos < 40);

    pos -= pos & 1;

    while (40 - pos) {
        *dst++ = args[pos + 1];
        *dst++ = args[pos];
        pos += 2;
    }

    return 0;
//    if (hdr.host_status == 0x00/* DID_OK */ &&
//            hdr.driver_status == 0x08/* DRIVER_SENSE */ && (
//                hdr.status == 0x00 /* GOOD */ ||
//                hdr.status == 0x02/* CONDITION_GOOD */
//            )
//       ) {
//        if (sense_blk[0] == 0x72 && sense_blk[7] > 13) {
//            if (sense_blk[8] == 0x09 && sense_blk[9] > 11) {
//                if (!(sense_blk[21] & 1)) {
//                    print_ascii(&args[20], 10, dst);
//                    return 0;
//                }
//
//                errno = EIO;
//                return -5;
//            }
//
//            errno = EIO;
//            return -4;
//        }
//
//        errno = EIO;
//        return -3;
//    }
//
//    errno = EIO;
//    return -2;
}

int getdiskid(char *buf, int buflen)
{
    std::ifstream ifs("/etc/mtab");
    std::string line;
    std::vector<std::string> val;
    struct hd_driveid serialid = {};
    int fd = 0;
    unsigned pos = 0;
    memset(buf, 0, buflen);

    if (!ifs) {
        rj_printf_debug("%s", "No /etc/mtab file.\n");
        return -1;
    }

    while (std::getline(ifs, line)) {
        ParseString(line, ' ', val);

        if (val[1] == "/")
            break;
    }

    ifs.close();

    while (val[0].back() >= '0' && val[0].back() <= '9')
        val[0].pop_back();

    if ((fd = open(val[0].c_str(), O_RDONLY)) == -1) {
        rj_printf_debug("%s", "open hard disk device failed\n");
        return -1;
    }

    if (!get_sata_serial(fd, buf)) {
        close(fd);
        return 0;
    }

    if (ioctl(fd, HDIO_GET_IDENTITY, &serialid) < 0) {
        close(fd);
        return -1;
    }

    pos = 0;

    while (
        serialid.serial_no[++pos] == ' ' &&
        pos < strlen(reinterpret_cast<char *>(serialid.serial_no))
    );

    pos -= pos & 1;

    while (40 - pos) {
        *buf++ = serialid.serial_no[pos + 1];
        *buf++ = serialid.serial_no[pos];
        pos += 2;
    }

    return 1;
}
