#ifndef RGPRIVATEPROC_H_INCLUDED
#define RGPRIVATEPROC_H_INCLUDED

struct SuRadiusPrivate;
struct EAPOLFrame;

class CRGPrivateProc
{
    public:
        static void EncapRGVerdorSeg(char *buf, unsigned &len);
        static void EncapRGVerdorSegForEapHost(
            char *buf,
            unsigned &len,
            const std::string &
        );
        static void EncapRGVerdorSegForPeap(char *buf, unsigned &len, char *);
        static void ReadRGVendorSeg(const char *buf, unsigned len);

    private:
        static void GetAlternateDNS(char *buf, unsigned &len);
        static void GetClientOSBits(char *buf, unsigned &len);
        static void GetClientReleaseVersion(char *buf, unsigned &len);
        static void GetClientVersion(char *buf, unsigned &len);
        static void GetDHCPAuthPhase(char *buf, unsigned &len);
        static void GetDirectCommunicationHighestVersion(char *buf, unsigned &len);
        static void GetHardDiskSN(char *buf, unsigned &len);
        static void GetIPDClientRunResult(char *buf, unsigned &len);
        static void GetIPv4Info(char *buf, unsigned &len);
        static void GetIPv4InfoForPeap(char *buf, unsigned &len);
        static void GetIPv6Info(char *buf, unsigned &len);
        static void GetMACAddr(char *buf, unsigned &len);
        static void GetSecCheckResult(char *buf, unsigned &len);
        static void GetSecDomainName(char *buf, unsigned &len);
        static void GetServiceName(char *buf, unsigned &len);
        static void GetUserPasswd(char *buf, unsigned &len);
        static void GetUserPasswd4Peap(char *buf, unsigned &len);
        static void GetV2SegmentHash(char *buf, unsigned &len);
        static void GetV3SegmentHash(char *buf, unsigned &len);
        static void GetV3SegmentHash4Peap(char *buf, unsigned &len);
        static void ParseHello(
            const struct EAPOLFrame *eapol_frame,
            struct SuRadiusPrivate &private_infobuf
        );
        static void ParseNotification(
            const struct EAPOLFrame *eapol_frame,
            struct SuRadiusPrivate &private_infobuf
        );
        static void ParseProxyAvoid(
            const struct EAPOLFrame *eapol_frame,
            struct SuRadiusPrivate &private_infobuf
        );
        static void ParseRadiusInfo_RuijieNas(
            const struct EAPOLFrame *eapol_frame,
            struct SuRadiusPrivate &private_infobuf
        );
        static void ParseUpGrade(
            const struct EAPOLFrame *eapol_frame,
            struct SuRadiusPrivate &private_infobuf
        );
};

#endif // RGPRIVATEPROC_H_INCLUDED
