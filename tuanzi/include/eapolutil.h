#ifndef EAPOLUTIL_H_INCLUDED
#define EAPOLUTIL_H_INCLUDED

#define FIELD_MAGIC 0x1311

#include "stdpkgs.h"
#include "miscdefs.h"

extern struct eapolpkg *ChangeToUChar(
    const struct EAPOLFrame *eapol_frame,
    unsigned *length
);
extern struct EAPOLFrame *ChangeToEAPOLFrame(
    const struct eapolpkg *eapol_pkg,
    unsigned length
);
extern struct eapolpkg *CreateEapolPacket(
    const struct EAPOLFrame *eapol_frame,
    unsigned *length
);
extern void DeleteFrameMemory(struct EAPOLFrame *eapol_frame);
extern void DhcpIpInfoToUChar(char *buf, const struct EAPOLFrame *eapol_frame);
extern void EncapUCharDhcpIpInfo(
    char *buf,
    const struct EAPOLFrame *eapol_frame
);
extern void InitEAPOLFrame(struct EAPOLFrame *eapol_frame);
extern void AppendPrivateProperty(
    char *buf,
    unsigned& len,
    const struct EAPOLFrame *eapol_frame
);
extern void ParsePrivateProperty(
    const char *buf,
    unsigned len,
    struct EAPOLFrame *eapol_frame
);
extern void EncapProgrammName(const std::string& prog_name, char *buf);
extern void EncapUCharVersionNumber(char *buf);

#endif // EAPOLUTIL_H_INCLUDED
