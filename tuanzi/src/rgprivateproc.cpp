#include "all.h"
#include "global.h"
#include "netutil.h"
#include "sysutil.h"
#include "cmdutil.h"
#include "util.h"
#include "encodeutil.h"
#include "vz_apiapp.h"
#include "changelanguage.h"
#include "rgprivateproc.h"

void CRGPrivateProc::EncapRGVerdorSeg(char *buf, unsigned &len)
{
    unsigned field_len = 0;
    len = 0;
#define GET_FIELD(func_name) \
    do { \
        (func_name)(&buf[len], field_len); \
        len += field_len; \
    } while (0)
    GET_FIELD(GetClientVersion);
    GET_FIELD(GetIPv4Info);
    GET_FIELD(GetMACAddr);
    GET_FIELD(GetDHCPAuthPhase);
    GET_FIELD(GetIPv6Info);
    GET_FIELD(GetServiceName);
    GET_FIELD(GetUserPasswd);
    GET_FIELD(GetAlternateDNS);
    GET_FIELD(GetV3SegmentHash);
    GET_FIELD(GetHardDiskSN);
    GET_FIELD(GetSecDomainName);
    GET_FIELD(GetSecCheckResult);
    GET_FIELD(GetIPDClientRunResult);
    GET_FIELD(GetClientOSBits);
    GET_FIELD(GetClientReleaseVersion);
    GET_FIELD(GetDirectCommunicationHighestVersion);
#undef GET_FIELD
    g_dhcpDug.AppendText("Su获取到的本地私有属性，准备上传");
    g_dhcpDug.HexPrinter(buf, len);
}

void CRGPrivateProc::EncapRGVerdorSegForEapHost(
    char *buf,
    unsigned &len,
    std::string
)
{
    unsigned field_len = 0;
    struct DHCPIPInfo dhcp_ipinfo = {};
    len = 0;
    GetDHCPIPInfo(dhcp_ipinfo, false);
    g_log_Wireless.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForEapHost GetDHCPIPInfo "
        "nDhcpEnable:%d IP:%d.%d.%d.%d",
        dhcp_ipinfo.dhcp_enabled,
        dhcp_ipinfo.ip4_ipaddr >> 24,
        dhcp_ipinfo.ip4_ipaddr >> 16 & 0xff,
        dhcp_ipinfo.ip4_ipaddr >> 8 & 0xff,
        dhcp_ipinfo.ip4_ipaddr & 0xff
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetClientVersion"
    );
    GetClientVersion(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetClientVersion"
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetIPv4Info"
    );

    if (dhcp_ipinfo.dhcp_enabled) {
        GetIPv4InfoForPeap(&buf[len], field_len);
        len += field_len;
    }

    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetIPv4Info"
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetMACAddr"
    );
    GetMACAddr(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetMACAddr"
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetDHCPAuthPhase"
    );
    GetDHCPAuthPhase(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetDHCPAuthPhase"
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetIPv6Info"
    );
    GetIPv6Info(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetIPv6Info"
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetServiceName"
    );
    GetServiceName(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetServiceName"
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetUserPasswd4Peap"
    );
    GetUserPasswd4Peap(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetUserPasswd4Peap"
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetV3SegmentHash4Peap"
    );
    GetV3SegmentHash4Peap(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetV3SegmentHash4Peap"
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetHardDiskSN"
    );
    GetHardDiskSN(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetHardDiskSN"
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetSecDomainName"
    );
    GetSecDomainName(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetSecDomainName"
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetSecCheckResult"
    );
    GetSecCheckResult(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetSecCheckResult"
    );
    GetIPDClientRunResult(&buf[len], field_len);
    len += field_len;
    GetClientOSBits(&buf[len], field_len);
    len += field_len;
    GetClientReleaseVersion(&buf[len], field_len);
    len += field_len;
    GetDirectCommunicationHighestVersion(&buf[len], field_len);
    len += field_len;
    // don't know why it breaks this line
    // *INDENT-OFF*
    g_dhcpDug.AppendText("Su获取到的本地私有属性，准备上传, 长度为%u", len);
    // *INDENT-ON*
    g_dhcpDug.HexPrinter(buf, len);
}

void CRGPrivateProc::EncapRGVerdorSegForPeap(char *buf, unsigned &len, char *)
{
    unsigned field_len = 0;
    len = 0;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetClientVersion"
    );
    GetClientVersion(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetClientVersion unAttrLen=%d",
        field_len
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetIPv4Info"
    );
    GetIPv4InfoForPeap(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetIPv4Info unAttrLen=%d",
        field_len
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetMACAddr"
    );
    GetMACAddr(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetMACAddr unAttrLen=%d",
        field_len
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetDHCPAuthPhase"
    );
    GetDHCPAuthPhase(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetDHCPAuthPhase unAttrLen=%d",
        field_len
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetIPv6Info"
    );
    GetIPv6Info(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetIPv6Info unAttrLen=%d",
        field_len
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetServiceName"
    );
    GetServiceName(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetServiceName unAttrLen=%d",
        field_len
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetAlternateDNS"
    );
    GetAlternateDNS(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetAlternateDNS unAttrLen=%d",
        field_len
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetUserPasswd4Peap"
    );
    GetUserPasswd4Peap(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetUserPasswd4Peap unAttrLen=%d",
        field_len
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetV3SegmentHash4Peap"
    );
    GetV3SegmentHash4Peap(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetV3SegmentHash4Peap unAttrLen=%d",
        field_len
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetHardDiskSN"
    );
    GetHardDiskSN(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetHardDiskSN unAttrLen=%d",
        field_len
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetSecDomainName"
    );
    GetSecDomainName(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetSecDomainName unAttrLen=%d",
        field_len
    );
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, before GetSecCheckResult"
    );
    GetSecCheckResult(&buf[len], field_len);
    g_dhcpDug.AppendText(
        "CRGPrivateProc::EncapRGVerdorSegForPeap, after GetSecCheckResult unAttrLen=%d",
        field_len
    );
    len += field_len;
    GetIPDClientRunResult(&buf[len], field_len);
    len += field_len;
    GetDirectCommunicationHighestVersion(&buf[len], field_len);
    len += field_len;
    g_dhcpDug.AppendText("Su获取到的本地私有属性，准备上传");
    g_dhcpDug.HexPrinter(buf, len);
}

void CRGPrivateProc::GetAlternateDNS(char *buf, unsigned &len)
{
    char tmpbuf[253] = {};
    unsigned tmpbuflen = sizeof(tmpbuf);
    len = 0;
    get_alternate_dns(tmpbuf, tmpbuflen);
    g_log_Wireless.AppendText(
        "CRGPrivateProc::GetAlternateDNS length:%d 内容:%s",
        tmpbuflen,
        tmpbuf
    );
    buf[len++] = 0x76;
    buf[len++] = tmpbuflen;
    memcpy(&buf[len], tmpbuf, tmpbuflen);
    len += tmpbuflen;
}

void CRGPrivateProc::GetClientOSBits(char *buf, unsigned &len)
{
    len = 0;
    buf[len++] = 0x70;
    buf[len++] = 0x01;
    buf[len++] = Is64BIT() ? 64 : 32;
}

void CRGPrivateProc::GetClientReleaseVersion(char *buf, unsigned &len)
{
    len = 0;
    buf[len++] = 0x6F;
    buf[len++] = theApp.version.length() + 1;
    strcpy(&buf[len], theApp.version.c_str());
    buf[len += theApp.version.length()] = 0;
    len++;
}

void CRGPrivateProc::GetClientVersion(char *buf, unsigned &len)
{
    len = 0;
    buf[len++] = 0x11;
    buf[len++] = 0x24;
    EncapProgrammName("8021x.exe", &buf[len]);
    EncapUCharVersionNumber(&buf[len += 32]);
    len += 4;
}

void CRGPrivateProc::GetDHCPAuthPhase(char *buf, unsigned &len)
{
    g_log_Wireless.AppendText("CRGPrivateProc::GetDHCPAuthPhase");
    len = 0;

    if (!CtrlThread->GetDHCPAuthStep())
        return;

    buf[len++] = 0x63;
    buf[len++] = 0x01;
    buf[len++] = CtrlThread->GetDHCPAuthStep();
}

void CRGPrivateProc::GetDirectCommunicationHighestVersion(
    char *buf,
    unsigned &len
)
{
    len = 0;
    buf[len++] = 0x79;
    buf[len++] = 0x01;
    buf[len++] = 0x02;
}

void CRGPrivateProc::GetHardDiskSN(char *buf, unsigned &len)
{
    len = 0;
    buf[len++] = 0x54;
    g_log_Wireless.AppendText(
        "CRGPrivateProc::GetHardDiskSN 硬盘序列号 length:%d 内容:\r\n%s",
        CtrlThread->diskid.length(),
        CtrlThread->diskid.c_str()
    );
    buf[len++] = std::max(CtrlThread->diskid.length(), 64);
    memcpy(&buf[len], CtrlThread->diskid.c_str(), buf[1]);
    len += buf[1];
}

void CRGPrivateProc::GetIPDClientRunResult(
    [[maybe_unused]] char *buf,
    unsigned &len
)
{
    len = 0;
}

void CRGPrivateProc::GetIPv4Info(char *buf, unsigned &len)
{
    struct DHCPIPInfo dhcp_ipinfo = {};
    len = 0;
    CtrlThread->GetDHCPInfoParam(dhcp_ipinfo);
    buf[len++] = 0x50;
    buf[len++] = 0x11;
    buf[len++] = dhcp_ipinfo.dhcp_enabled ? 0 : dhcp_ipinfo.ip4_ipaddr >> 24;
    buf[len++] = dhcp_ipinfo.dhcp_enabled ? 0 : dhcp_ipinfo.ip4_ipaddr >> 16 & 0xff;
    buf[len++] = dhcp_ipinfo.dhcp_enabled ? 0 : dhcp_ipinfo.ip4_ipaddr >> 8 & 0xff;
    buf[len++] = dhcp_ipinfo.dhcp_enabled ? 0 : dhcp_ipinfo.ip4_ipaddr & 0xff;
    buf[len++] = dhcp_ipinfo.ip4_netmask >> 24;
    buf[len++] = dhcp_ipinfo.ip4_netmask >> 16 & 0xff;
    buf[len++] = dhcp_ipinfo.ip4_netmask >> 8 & 0xff;
    buf[len++] = dhcp_ipinfo.ip4_netmask & 0xff;
    buf[len++] = dhcp_ipinfo.gateway >> 24;
    buf[len++] = dhcp_ipinfo.gateway >> 16 & 0xff;
    buf[len++] = dhcp_ipinfo.gateway >> 8 & 0xff;
    buf[len++] = dhcp_ipinfo.gateway & 0xff;
    buf[len++] = dhcp_ipinfo.dns >> 24;
    buf[len++] = dhcp_ipinfo.dns >> 16 & 0xff;
    buf[len++] = dhcp_ipinfo.dns >> 8 & 0xff;
    buf[len++] = dhcp_ipinfo.dns & 0xff;
    buf[len++] = dhcp_ipinfo.dhcp_enabled;
}

void CRGPrivateProc::GetIPv4InfoForPeap(char *buf, unsigned &len)
{
    struct DHCPIPInfo dhcp_ipinfo = {};
    len = 0;
    CtrlThread->GetDHCPInfoParam(dhcp_ipinfo);
    buf[len++] = 0x50;
    buf[len++] = 0x11;
    buf[len++] = dhcp_ipinfo.dhcp_enabled ? 0 : dhcp_ipinfo.ip4_ipaddr >> 24;
    buf[len++] = dhcp_ipinfo.dhcp_enabled ? 0 : dhcp_ipinfo.ip4_ipaddr >> 16 & 0xff;
    buf[len++] = dhcp_ipinfo.dhcp_enabled ? 0 : dhcp_ipinfo.ip4_ipaddr >> 8 & 0xff;
    buf[len++] = dhcp_ipinfo.dhcp_enabled ? 0 : dhcp_ipinfo.ip4_ipaddr & 0xff;
    buf[len++] = dhcp_ipinfo.ip4_netmask >> 24;
    buf[len++] = dhcp_ipinfo.ip4_netmask >> 16 & 0xff;
    buf[len++] = dhcp_ipinfo.ip4_netmask >> 8 & 0xff;
    buf[len++] = dhcp_ipinfo.ip4_netmask & 0xff;
    buf[len++] = dhcp_ipinfo.gateway >> 24;
    buf[len++] = dhcp_ipinfo.gateway >> 16 & 0xff;
    buf[len++] = dhcp_ipinfo.gateway >> 8 & 0xff;
    buf[len++] = dhcp_ipinfo.gateway & 0xff;
    buf[len++] = dhcp_ipinfo.dns >> 24;
    buf[len++] = dhcp_ipinfo.dns >> 16 & 0xff;
    buf[len++] = dhcp_ipinfo.dns >> 8 & 0xff;
    buf[len++] = dhcp_ipinfo.dns & 0xff;
    buf[len++] = dhcp_ipinfo.dhcp_enabled;
}

void CRGPrivateProc::GetIPv6Info(char *buf, unsigned &len)
{
    struct DHCPIPInfo dhcp_ipinfo = {};
    CtrlThread->GetDHCPInfoParam(dhcp_ipinfo);
    len = 0;

    if (!dhcp_ipinfo.ipaddr6_count)
        return;

    buf[len++] = 0x35;
    buf[len++] = 0x01;
    buf[len++] = dhcp_ipinfo.ipaddr6_count;

    if (
        dhcp_ipinfo.ip6_link_local_ipaddr.s6_addr[0] &&
        dhcp_ipinfo.ip6_link_local_ipaddr.s6_addr[1]
    ) {
        g_dhcpDug.AppendText("上传的本地链路单播地址:");
        g_dhcpDug.HexPrinter(
            reinterpret_cast<char *>(dhcp_ipinfo.ip6_link_local_ipaddr.s6_addr),
            sizeof(dhcp_ipinfo.ip6_link_local_ipaddr)
        );
        buf[len++] = 0x38;
        buf[len++] = 0x10;
        memcpy(
            &buf[len],
            reinterpret_cast<char *>(dhcp_ipinfo.ip6_link_local_ipaddr.s6_addr),
            sizeof(dhcp_ipinfo.ip6_link_local_ipaddr)
        );
        len += 16;
    }

    if (dhcp_ipinfo.ip6_ipaddr.s6_addr[0] && dhcp_ipinfo.ip6_ipaddr.s6_addr[1]) {
        g_dhcpDug.AppendText("上传的可集聚全球单播地址:");
        g_dhcpDug.HexPrinter(
            reinterpret_cast<char *>(dhcp_ipinfo.ip6_ipaddr.s6_addr),
            sizeof(dhcp_ipinfo.ip6_ipaddr)
        );
        buf[len++] = 0x4E;
        buf[len++] = 0x10;
        memcpy(
            &buf[len],
            dhcp_ipinfo.ip6_ipaddr.s6_addr,
            sizeof(dhcp_ipinfo.ip6_ipaddr)
        );
        len += 16;
    }

    buf[len++] = 0x36;
    buf[len++] = 0x10;
    memcpy(
        &buf[len],
        dhcp_ipinfo.ip6_my_ipaddr.s6_addr,
        sizeof(dhcp_ipinfo.ip6_my_ipaddr)
    );
    len += 16;
}

void CRGPrivateProc::GetMACAddr(char *buf, unsigned &len)
{
    len = 0;
    buf[len++] = 0x2D;
    buf[len++] = 0x06;
    CtrlThread->GetAdapterMac(&buf[len]);
    len += 6;
}

void CRGPrivateProc::GetSecCheckResult(char *buf, unsigned &len)
{
    len = 0;
    buf[len++] = 0x62;
    buf[len++] = 0x01;
    buf[len++] = GetHIRusultByLocal();
}

void CRGPrivateProc::GetSecDomainName(char *buf, unsigned &len)
{
    len = 0;

    if (CtrlThread->sec_domain_name.empty())
        return;

    buf[len++] = 0x55;
    buf[len++] = CtrlThread->sec_domain_name.length();
    strcpy(&buf[len], CtrlThread->sec_domain_name.c_str());
}

void CRGPrivateProc::GetServiceName(char *buf, unsigned &len)
{
    char service_name[128] = {};
    len = 0;

    if (CtrlThread->configure_info.public_service.empty())
        return;

    ConvertUtf8ToGBK(
        service_name,
        sizeof(service_name),
        CtrlThread->configure_info.public_service.c_str(),
        CtrlThread->configure_info.public_service.length()
    );
    buf[len++] = 0x39;
    buf[len++] = 0x20;
    memcpy(&buf[len], service_name, strlen(service_name));
    len += 32;
}

void CRGPrivateProc::GetUserPasswd(char *buf, unsigned &len)
{
    char password[128] = {};
    unsigned password_len = 0;
    len = 0;
    ConvertUtf8ToGBK(
        password,
        sizeof(password),
        CtrlThread->configure_info.last_auth_password.c_str(),
        CtrlThread->configure_info.last_auth_password.length()
    );
    password_len = strlen(password);
    password_len = ((password_len >> 4) + !!(password_len & 15)) << 4;
    buf[len++] = 0x2F;
    buf[len++] = password_len;
    RadiusEncrpytPwd(
        e_pMd5Chanllenge,
        sizeof(e_pMd5Chanllenge),
        password,
        password_len,
        &buf[len]
    );
    len += password_len;
}

void CRGPrivateProc::GetUserPasswd4Peap(char *buf, unsigned &len)
{
    char password[512] = {};
    unsigned password_len = 0;
    len = 0;
    ConvertUtf8ToGBK(
        password,
        sizeof(password),
        CtrlThread->configure_info.last_auth_password.c_str(),
        CtrlThread->configure_info.last_auth_password.length()
    );
    password_len = strlen(password);
    password_len = ((password_len >> 4) + !!(password_len & 15)) << 4;
    buf[len++] = 0x5F;
    buf[len++] = password_len;
    memcpy(e_pMd5Chanllenge, "!jierui9002pmsus", 16);
    RadiusEncrpytPwd(
        e_pMd5Chanllenge,
        sizeof(e_pMd5Chanllenge),
        password,
        password_len,
        &buf[len]
    );
    len += password_len;
}

void CRGPrivateProc::GetV2SegmentHash(char *buf, unsigned &len)
{
    len = 0;
    buf[len++] = 0x17;
    buf[len++] = 0x22;
    // this function is not used anyway, so I won't understand why the heck
    // these are
    sprintf(
        &buf[len],
        "%X%X%X%X%X%X9884773d9f46acafd7839eb38789088ac9534",
        buf[0],
        buf[0],
        buf[0],
        buf[0],
        buf[0],
        buf[0]
    );
    len += 32;
}

void CRGPrivateProc::GetV3SegmentHash(char *buf, unsigned &len)
{
    CVz_APIApp vz_apiapp;
    len = 0;
    buf[len++] = 0x4D;
    buf[len++] = 0x80;
    vz_apiapp.Vz_API(
        &buf[len],
        e_pMd5Chanllenge,
        CtrlThread->configure_info.public_title.c_str()
    );
    len += 128;
}

void CRGPrivateProc::GetV3SegmentHash4Peap(char *buf, unsigned &len)
{
    CVz_APIApp vz_apiapp;
    len = 0;
    buf[len++] = 0x60;
    buf[len++] = 0x80;
    vz_apiapp.Vz_API(
        &buf[len],
        e_pMd5Chanllenge,
        CtrlThread->configure_info.public_title.c_str()
    );
    len += 128;
}

void CRGPrivateProc::ParseHello(
    const struct EAPOLFrame *eapol_frame,
    struct SuRadiusPrivate &private_infobuf
)
{
    if (
        eapol_frame->parse_hello_magic != FIELD_MAGIC ||
        eapol_frame->parse_hello_val != 10
    ) {
        g_log_Wireless.AppendText("ParseHello 0");
        private_infobuf.parse_hello = 0;
        return;
    }

    if (eapol_frame->parse_hello_val2) {
        private_infobuf.parse_hello = 1;
        private_infobuf.parse_hello_inv = eapol_frame->parse_hello_inv;
        private_infobuf.parse_hello_id = eapol_frame->parse_hello_id;
        g_log_Wireless.AppendText(
            "ParseHello enable inv %u; id %u",
            eapol_frame->parse_hello_inv,
            eapol_frame->parse_hello_id
        );

    } else {
        g_log_Wireless.AppendText("ParseHello disable");
        private_infobuf.parse_hello = -1;
    }
}

void CRGPrivateProc::ParseNotification(
    const struct EAPOLFrame *eapol_frame,
    struct SuRadiusPrivate &private_infobuf
)
{
    if (eapol_frame->fail_reason_magic != FIELD_MAGIC) {
        g_log_Wireless.AppendText("ParseNotification null");
        return;
    }

    private_infobuf.broadcast_str =
        !eapol_frame->fail_reason[0] &&
        CtrlThread->configure_info.other_authhintenable ?
        CChangeLanguage::Instance().LoadString(172) :
        eapol_frame->fail_reason;
    rj_printf_debug(
        "radiusInfo.m_strBroadCast= %s\n",
        private_infobuf.broadcast_str.c_str()
    );
}

void CRGPrivateProc::ParseProxyAvoid(
    const struct EAPOLFrame *eapol_frame,
    struct SuRadiusPrivate &private_infobuf
)
{
    if (
        eapol_frame->proxy_avoid_magic != FIELD_MAGIC ||
        eapol_frame->proxy_avoid_val != 1 ||
        eapol_frame->proxy_avoid_val2 > 1
    ) {
        g_log_Wireless.AppendText("ParseProxyAvoid null");
        private_infobuf.proxy_avoid = 0;
        return;
    }

    if (eapol_frame->proxy_avoid_val2 == 1) {
        g_log_Wireless.AppendText("ParseProxyAvoid 1");
        private_infobuf.proxy_avoid = 1;

    } else {
        g_log_Wireless.AppendText("ParseProxyAvoid disable");
        private_infobuf.proxy_avoid = 0;
    }
}

void CRGPrivateProc::ParseRadiusInfo_RuijieNas(
    const struct EAPOLFrame *eapol_frame,
    struct SuRadiusPrivate &private_infobuf
)
{
    ParseNotification(eapol_frame, private_infobuf);
    ParseUpGrade(eapol_frame, private_infobuf);
    ParseProxyAvoid(eapol_frame, private_infobuf);
    ParseHello(eapol_frame, private_infobuf);
}

void CRGPrivateProc::ParseUpGrade(
    const struct EAPOLFrame *eapol_frame,
    struct SuRadiusPrivate &private_infobuf
)
{
    private_infobuf.su_upgrade_url.clear();

    if (
        eapol_frame->upgrade_info.magic != FIELD_MAGIC ||
        eapol_frame->upgrade_info.type != 0x37
    )
        return;

    private_infobuf.su_newest_ver = eapol_frame->upgrade_info.su_newest_ver;
    private_infobuf.su_upgrade_url = eapol_frame->upgrade_info.su_upgrade_url;
}

void CRGPrivateProc::ReadRGVendorSeg(const char *buf, unsigned len)
{
    g_dhcpDug.AppendText("CRGPrivateProc::ReadRGVendorSeg(BEGIN)");

    if (!buf || !len || len > 1400)
        return;

    CtrlThread->private_properties.radius_type = 5;
    CtrlThread->private_properties.su_newest_ver = 0;
    CtrlThread->private_properties.account_info.clear();
    CtrlThread->private_properties.persional_info.clear();
    CtrlThread->private_properties.broadcast_str.clear();
    CtrlThread->private_properties.fail_reason.clear();
    CtrlThread->private_properties.su_upgrade_url.clear();
    CtrlThread->private_properties.su_reauth_interv = 0;
    CtrlThread->private_properties.radius_type = 0;
    CtrlThread->private_properties.proxy_avoid = 0;
    CtrlThread->private_properties.dialup_avoid = 0;
    CtrlThread->private_properties.indicate_serv_ip = 0;
    CtrlThread->private_properties.indicate_port = 0;
    CtrlThread->private_properties.field_A4 = 0;
    CtrlThread->private_properties.proxy_dectect_kinds = 0;
    CtrlThread->private_properties.hello_interv = 0;
    memset(
        CtrlThread->private_properties.encrypt_key,
        0,
        sizeof(CtrlThread->private_properties.encrypt_key)
    );
    memset(
        CtrlThread->private_properties.encrypt_iv,
        0,
        sizeof(CtrlThread->private_properties.encrypt_iv)
    );
    CtrlThread->private_properties.server_utc_time = 0;
    CtrlThread->private_properties.svr_switch_result.clear();
    CtrlThread->private_properties.user_login_url.clear();
    CtrlThread->private_properties.msg_client_port = 80;
    CtrlThread->private_properties.utrust_url.clear();
    CtrlThread->private_properties.is_show_utrust_url = 1;
    CtrlThread->private_properties.delay_second_show_utrust_url = 0;
    CtrlThread->private_properties.parse_hello = 0;
    CtrlThread->private_properties.direct_communication_highest_version_supported =
        true;
    CtrlThread->private_properties.direct_comm_heartbeat_flags = 0;
    // Lol :D
    g_dhcpDug.AppendText(
        "CRGPrivateProc::ReadRGVendorSeg()----->m_suRadiusPrivates.init，"
        "以下所打印的信息是在调试时使用的，请在发行时将关键信息注释掉!!!"
    );
    g_dhcpDug.HexPrinter(buf, len);
    g_log_Wireless.AppendText("ReadRGVendorSeg set Radius type:%d", 9);
    CtrlThread->SetRadiusServer(9);
    unsigned cur_pos = 2;
    bool should_exit = false;
    char cur_type = buf[0], cur_datalen = buf[1];
    std::string service_str;

    for (
        ;
        !should_exit && cur_pos <= len && cur_type && cur_datalen != 0xff;
        cur_pos += cur_datalen,
        cur_type = buf[cur_pos],
        cur_datalen = buf[cur_pos + 1],
        cur_pos += 2
    )
        switch (cur_type) {
            case 0x05:
                if (cur_datalen != 4) {
                    should_exit = true;
                    break;
                }

                CtrlThread->private_properties.su_newest_ver =
                    ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos]));
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()----->SU_NEWEST_VER = 0x%X",
                    CtrlThread->private_properties.su_newest_ver
                );
                break;

            case 0x14:
                if (cur_datalen != 4) {
                    should_exit = true;
                    break;
                }

                CtrlThread->private_properties.proxy_avoid =
                    ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos]));
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()----->PROXY_AVOID = %d",
                    CtrlThread->private_properties.proxy_avoid
                );
                break;

            case 0x15:
                if (cur_datalen != 4) {
                    should_exit = true;
                    break;
                }

                CtrlThread->private_properties.dialup_avoid =
                    ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos]));
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()----->DIALUP_AVOID = %d",
                    CtrlThread->private_properties.dialup_avoid
                );
                break;

            case 0x37:
                if (cur_datalen != 247) {
                    should_exit = true;
                    break;
                }

                ConvertGBKToUtf8(
                    CtrlThread->private_properties.su_upgrade_url,
                    &buf[cur_pos],
                    strlen(&buf[cur_pos])
                );
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()----->SU_UPGRADE_URL"
                );
                g_dhcpDug.WriteString(
                    CtrlThread->private_properties.su_upgrade_url.c_str()
                );
                break;

            case 0x3C:
                ConvertGBKToUtf8(
                    CtrlThread->private_properties.account_info,
                    &buf[cur_pos],
                    strlen(&buf[cur_pos])
                );
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()----->ACCOUNT_INFO"
                );
                g_dhcpDug.WriteString(
                    CtrlThread->private_properties.account_info.c_str()
                );
                break;

            case 0x3D:
                ConvertGBKToUtf8(
                    CtrlThread->private_properties.persional_info,
                    &buf[cur_pos],
                    strlen(&buf[cur_pos])
                );
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()----->PERSIONAL_INFO"
                );
                g_dhcpDug.WriteString(
                    CtrlThread->private_properties.persional_info.c_str()
                );
                break;

            case 0x52:
                ConvertGBKToUtf8(
                    CtrlThread->private_properties.broadcast_str,
                    &buf[cur_pos],
                    strlen(&buf[cur_pos])
                );
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()----->BROADCASE_INFO"
                );
                g_dhcpDug.WriteString(
                    CtrlThread->private_properties.broadcast_str.c_str()
                );
                break;

            case 0x53:
                ConvertGBKToUtf8(
                    CtrlThread->private_properties.fail_reason,
                    &buf[cur_pos],
                    strlen(&buf[cur_pos])
                );
                g_dhcpDug.WriteString(
                    CtrlThread->private_properties.fail_reason.c_str()
                );
                break;

            case 0x56:
                if (cur_datalen != 4) {
                    should_exit = true;
                    break;
                }

                CtrlThread->private_properties.su_reauth_interv =
                    ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos]));
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()----->SU_REAUTH_INTERV = %d",
                    CtrlThread->private_properties.su_reauth_interv
                );
                break;

            case 0x59:
                if (cur_datalen != 4) {
                    should_exit = true;
                    break;
                }

                CtrlThread->private_properties.hello_interv =
                    ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos]));
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()----->HELLO_INTERV = %d",
                    CtrlThread->private_properties.hello_interv
                );
                break;

            case 0x5A:
                if (cur_datalen != 4) {
                    should_exit = true;
                    break;
                }

                CtrlThread->private_properties.indicate_port =
                    ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos]));
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()----->INDICATE_PORT = %d",
                    CtrlThread->private_properties.indicate_port
                );
                break;

            case 0x5B:
                if (cur_datalen != 4) {
                    should_exit = true;
                    break;
                }

                CtrlThread->private_properties.indicate_serv_ip =
                    ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos]));
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()----->INDICATE_SERV_IP = 0x%0.8x",
                    CtrlThread->private_properties.indicate_serv_ip
                );
                break;

            case 0x5C:
                if (cur_datalen != 8) {
                    should_exit = true;
                    break;
                }

                memcpy(
                    CtrlThread->private_properties.encrypt_key,
                    &buf[cur_pos],
                    sizeof(CtrlThread->private_properties.encrypt_key)
                );
                RC4(
                    CtrlThread->private_properties.encrypt_key,
                    "com.ruijie.www",
                    sizeof(CtrlThread->private_properties.encrypt_key)
                );
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()----->ENCRYPT_KEY "
                );
                g_dhcpDug.HexPrinter(
                    CtrlThread->private_properties.encrypt_key,
                    sizeof(CtrlThread->private_properties.encrypt_key)
                );
                break;

            case 0x5D:
                if (cur_datalen != 8) {
                    should_exit = true;
                    break;
                }

                memcpy(
                    CtrlThread->private_properties.encrypt_iv,
                    &buf[cur_pos],
                    sizeof(CtrlThread->private_properties.encrypt_iv)
                );
                RC4(
                    CtrlThread->private_properties.encrypt_iv,
                    "com.ruijie.www",
                    sizeof(CtrlThread->private_properties.encrypt_iv)
                );
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()----->ENCRYPT_IV "
                );
                g_dhcpDug.HexPrinter(
                    CtrlThread->private_properties.encrypt_iv,
                    sizeof(CtrlThread->private_properties.encrypt_iv)
                );
                break;

            case 0x5E:
                if (cur_datalen != 8) {
                    should_exit = true;
                    break;
                }

                memcpy(
                    reinterpret_cast<char *>
                    (&CtrlThread->private_properties.server_utc_time),
                    &buf[cur_pos],
                    sizeof(CtrlThread->private_properties.server_utc_time)
                );
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()----->SERVER_UTC_TIME "
                );
                g_dhcpDug.HexPrinter(
                    reinterpret_cast<char *>
                    (&CtrlThread->private_properties.server_utc_time),
                    sizeof(CtrlThread->private_properties.server_utc_time)
                );
                break;

            case 0x61:
                if (cur_datalen != 1) {
                    should_exit = true;
                    break;
                }

                CtrlThread->private_properties.radius_type = buf[cur_pos];
                CtrlThread->SetRadiusServer(buf[cur_pos]);
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()----->RADIUS_SERV_TYPE = %d",
                    CtrlThread->GetRadiusServer()
                );
                CtrlThread->field_1149 = 0;
                logFile.AppendText("radius server is new version");
                break;

            case 0x66:
                logFile.AppendText("ReadRGVendorSeg---------更新服务列表");
                ConvertGBKToUtf8(service_str, &buf[cur_pos], cur_datalen);
                CtrlThread->private_properties.services.clear();
                ParseString(
                    service_str,
                    '@',
                    CtrlThread->private_properties.services
                );
                RcvSvrList(CtrlThread->private_properties.services);
                break;

            case 0x68:
                CtrlThread->private_properties.user_login_url =
                    AsciiToStr(&buf[cur_pos], cur_pos);
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()---->USER_LOGIN_URL=%s",
                    CtrlThread->private_properties.user_login_url.c_str()
                );
                break;

            case 0x6E:
                if (cur_datalen != 4) {
                    should_exit = true;
                    break;
                }

                CtrlThread->private_properties.msg_client_port =
                    ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos]));
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()----->MSG_CLIENT_PORT = %d",
                    CtrlThread->private_properties.msg_client_port
                );
                break;

            case 0x72:
                CtrlThread->private_properties.utrust_url =
                    AsciiToStr(&buf[cur_pos], cur_pos);
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()----->UTRUST_URL = %s",
                    CtrlThread->private_properties.utrust_url.c_str()
                );
                break;

            case 0x74:
                if (cur_datalen != 4) {
                    g_dhcpDug.AppendText(
                        "%s PROXY_DECTECT_KINDS len=%d error",
                        "ReadRGVendorSeg",
                        cur_datalen
                    );
                    should_exit = true;
                    break;
                }

                CtrlThread->private_properties.proxy_dectect_kinds =
                    ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos]));
                g_dhcpDug.AppendText(
                    "%s proxy type:%08x",
                    "ReadRGVendorSeg",
                    CtrlThread->private_properties.proxy_dectect_kinds
                );
                break;

            case 0x77:
                if (len < cur_pos + cur_datalen) {
                    should_exit = true;
                    break;
                }

                if (cur_datalen != 3)
                    continue;

                CtrlThread->private_properties.is_show_utrust_url = buf[cur_pos];
                CtrlThread->private_properties.delay_second_show_utrust_url =
                    ntohs(*reinterpret_cast<const uint16_t *>(&buf[cur_pos + 1]));
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()----->IS_SHOW_UTRUST_URL = %d,"
                    "delaySecondShowUtrustUrl=%d",
                    CtrlThread->private_properties.is_show_utrust_url,
                    CtrlThread->private_properties.delay_second_show_utrust_url
                );
                break;

            case 0x79:
                if (cur_datalen != 1) {
                    g_dhcpDug.AppendText(
                        "Invalid length in the property"
                        "(direct-communication-highest-version-supported)."
                    );
                    should_exit = true;
                    break;
                }

                if (len < cur_pos + 1) {
                    g_dhcpDug.AppendText(
                        "When parsing the property"
                        "(direct-communication-highest-version-supported), "
                        "the private property packet length is too short."
                    );
                    should_exit = true;
                    break;
                }

                CtrlThread->private_properties.direct_communication_highest_version_supported =
                    buf[cur_pos];
                g_dhcpDug.AppendText(
                    "direct-communication-highest-version-supported is %d.",
                    CtrlThread->private_properties.direct_communication_highest_version_supported
                );
                break;

            case 0x80:
                if (cur_datalen != 4) {
                    g_dhcpDug.AppendText(
                        "Invalid length in the property"
                        "(direct-communication-heartbeat-flags)."
                    );
                    should_exit = true;
                    break;
                }

                if (len < cur_pos + 4) {
                    g_dhcpDug.AppendText(
                        "When parsing the property"
                        "(direct-communication-heartbeat-flags), "
                        "the private property packet length is too short.");
                    should_exit = true;
                    break;
                }

                CtrlThread->private_properties.direct_comm_heartbeat_flags =
                    ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos]));
                g_dhcpDug.AppendText(
                    "direct-communication-heartbeat-flags is %u.",
                    CtrlThread->private_properties.direct_comm_heartbeat_flags
                );
                break;

            default:
                g_dhcpDug.AppendText(
                    "CRGPrivateProc::ReadRGVendorSeg()---->出现无法识别的属性，跳过处理"
                );
                break;
        }

    // check if it's error break
    if (!should_exit) {
        if (!cur_type) {
            g_dhcpDug.AppendText(
                "CRGPrivateProc::ReadRGVendorSeg()----->byteType为0，不合要求!!"
            );

        } else if (cur_datalen == 0xff) {
            g_dhcpDug.AppendText(
                "CRGPrivateProc::ReadRGVendorSeg()----->byteDataLen不符合要求!!"
            );
        }
    }

    // normal break or manual break
    g_dhcpDug.AppendText("CRGPrivateProc::ReadRGVendorSeg(END)");
}
