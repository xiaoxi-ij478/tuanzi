#include "all.h"
#include "miscdefs.h"
#include "saveconfigureinfo.h"
#include "suconfigfile.h"
#include "global.h"
#include "util.h"

std::string GetParentPath(std::string strInstallPath)
{
    if (strInstallPath.back() == '/')
        strInstallPath.pop_back();

    return strInstallPath.substr(0, strInstallPath.find_last_of("/"));
}

int CleanFile(const std::string &strPath)
{
    if (strPath == "/")
        return -1;

    std::string strCmd;
    strCmd = "rm -fr " + strPath;
    system(strCmd.c_str());
    return 0;
}

int DispatchFile(
    const std::string &strInstallPath,
    const std::string &strCurPath
)
{
    std::string strCmd;
    std::string strCurPathFile;
    std::string strIParentPath;
    strIParentPath = GetParentPath(strInstallPath);
    strCurPathFile = GetParentPath(strCurPath);

    if (strCurPathFile == "/")
        return -1;

    strCurPathFile += "*";
    strCmd =  "cp -rf " + strCurPathFile +  " " + strIParentPath;
    system(strCmd.c_str());
    return 0;
}

int ModifyConfFile(
    const std::string &strInstallPath,
    const std::string &strCurPath
)
{
    typedef struct _tagModifyInfo {
        std::string strAdapterName;
        bool bSavePassword;
        std::string strLanguage;
        unsigned int dwSvrNum;
        std::vector<std::string> strArraySvrName;
        std::string strSelSvr;
        std::string strSelWireless;
        bool bAutoRun;
        bool bAutoRunHint;
        std::string strAutoRunHintMsg;
        bool bRunConnect;
        bool bBeforeOsloginAuth;
        bool bDomainAuth;
        std::string strAuthMode;
        std::string strAuthMediaType;
        bool bEnableSecDomain;
        std::vector<std::string> strArraySecDomain;
        std::string strSelSecDomain;
        int nDhcpAuth;
        int nDaemon;
    } MODIFYINFO;
    MODIFYINFO modifyinfo = {};
    unsigned dwSupportAuthMode = 0;
    char strConfName[128] = {};
    std::string strKey;
    std::string strSecDomain;
    std::string strSvrname;
    std::string strPath = strInstallPath + "SuConfig.dat";
    CSuConfigFile oldConfigfile;
    CSuConfigFile newConfigfile;
    oldConfigfile.Lock();

    if (!oldConfigfile.Open(strPath.c_str())) {
        oldConfigfile.Unlock();
        g_logSystem.AppendText(
            "升级程序：读取旧配置失败 ［%s］",
            strPath.c_str()
        );
        return -1;
    }

#define GET_INT(name, domain, key, defval) \
    modifyinfo.name = oldConfigfile.GetPrivateProfileInt((domain), (key), (defval))
#define GET_STRING(name, domain, key, defval) \
    oldConfigfile.GetPrivateProfileString((domain), (key), (defval), modifyinfo.name)
    GET_STRING(strAdapterName, "PUBLIC", "Adapter", "");
    GET_INT(bSavePassword, "PUBLIC", "SaveCheck", 0);
    GET_STRING(strLanguage, "PUBLIC", "Language", "");
    GET_INT(dwSvrNum, "SERVER", "Number", 0);

    for (unsigned i = 0; i < modifyinfo.dwSvrNum; i++) {
        memset(strConfName, 0, sizeof(strConfName));
        sprintf(strConfName, "%s%d", "Name", i);
        oldConfigfile.GetPrivateProfileString("SERVER", strConfName, "", strSvrname);
        modifyinfo.strArraySvrName.push_back(strSvrname);
    }

    GET_STRING(strSelSvr, "PUBLIC", "Service", "");
    GET_STRING(strSelWireless, "PUBLIC", "WirelessConf", "");
    GET_INT(nDhcpAuth, "PUBLIC", "DHCP", -1);
    GET_INT(nDaemon, "PUBLIC", "Daemon", 0);
    GET_INT(bAutoRun, "PUBLIC", "RunBoot", 0);
    GET_INT(bAutoRunHint, "PUBLIC", "Prompt", 0);
    GET_STRING(strAutoRunHintMsg, "PUBLIC", "PromptMsg", "");
    GET_INT(bRunConnect, "PUBLIC", "RunConnect", 0);
    GET_INT(bBeforeOsloginAuth, "PUBLIC", "ServiceRun", 0);
    GET_INT(bDomainAuth, "DOMAIN", "Value", 0);
    GET_STRING(strAuthMode, "PUBLIC", "AuthMode", "");
    GET_INT(bEnableSecDomain, "SECDOMAIN", "SecDomainEnable", 0);

    if (modifyinfo.bEnableSecDomain) {
        for (
            unsigned i = 0,
            dwCount =
                oldConfigfile.GetPrivateProfileInt("SECDOMAIN", "SecDomainCount", 0);
            i < dwCount;
            i++
        ) {
            strKey = "SecDomain" + DWordToString(i);
            oldConfigfile.GetPrivateProfileString(
                "SECDOMAIN",
                strKey.c_str(),
                "",
                strSecDomain
            );

            if (!strSecDomain.empty())
                modifyinfo.strArraySecDomain.push_back(strSecDomain);
        }

        GET_STRING(strSelSecDomain, "PUBLIC", "SecDomain", "");
    }

    oldConfigfile.Close();
    oldConfigfile.Unlock();
#undef GET_INT
#undef GET_STRING
    newConfigfile.Lock();
    strPath = strCurPath + "SuConfig.dat";

    if (!oldConfigfile.Open(strPath.c_str())) {
        oldConfigfile.Unlock();
        g_logSystem.AppendText(
            "升级程序：写入新配置失败 ［%s］",
            strPath.c_str()
        );
        return -2;
    }

#define WRITE_DIRECT_INT(domain, key, val) \
    newConfigfile.WritePrivateProfileString((domain), (key), DWordToString(val).c_str())
#define WRITE_DIRECT_FIELD(domain, key, val) \
    newConfigfile.WritePrivateProfileString((domain), (key), (val))
#define WRITE_INT(domain, key, name) \
    WRITE_DIRECT_INT(domain, key, modifyinfo.name)
#define WRITE_FIELD(domain, key, name) \
    WRITE_DIRECT_FIELD(domain, key, modifyinfo.name)
    WRITE_FIELD("PUBLIC", "Adapter", strAdapterName.c_str());
    WRITE_INT("PUBLIC", "SaveCheck", bSavePassword);
    WRITE_FIELD("PUBLIC", "Language", strLanguage.c_str());

    if (modifyinfo.nDhcpAuth != -1)
        WRITE_INT("PUBLIC", "DHCP", nDhcpAuth);

    WRITE_INT("PUBLIC", "Daemon", nDaemon);
    WRITE_INT("SERVER", "Number", dwSvrNum);

    for (unsigned i = 0; i < modifyinfo.dwSvrNum ; i++) {
        strKey = "Name" + DWordToString(i);
        newConfigfile.WritePrivateProfileString(
            "SERVER",
            strKey.c_str(),
            modifyinfo.strArraySvrName[i].c_str()
        );
    }

    WRITE_FIELD("PUBLIC", "Service", strSelSvr.c_str());
    WRITE_FIELD("PUBLIC", "WirelessConf", strSelWireless.c_str());

    if (
        newConfigfile.GetPrivateProfileInt("DOMAIN", "SupportAutoRun", 0)
        &&
        !newConfigfile.GetPrivateProfileInt("PUBLIC", "RunBoot", 0)
    ) {
        WRITE_INT("PUBLIC", "RunBoot", bAutoRun);
        WRITE_INT("PUBLIC", "Prompt", bAutoRunHint);
        WRITE_FIELD("PUBLIC", "PromptMsg", strAutoRunHintMsg.c_str());
    }

    if (newConfigfile.GetPrivateProfileInt("DOMAIN", "SupportSoftrunAuth", 0))
        WRITE_INT("PUBLIC", "RunConnect", bRunConnect);

    if (newConfigfile.GetPrivateProfileInt("DOMAIN", "SupportOsloginAuth", 0))
        WRITE_INT("PUBLIC", "ServiceRun", bBeforeOsloginAuth);

    dwSupportAuthMode =
        newConfigfile.GetPrivateProfileInt("AUTHMODE", "Support", 0);

    if (
        (modifyinfo.strAuthMode == "EAPMD5" && dwSupportAuthMode & 1) ||
        (modifyinfo.strAuthMode == "EAPTLS" && dwSupportAuthMode & 2) ||
        (modifyinfo.strAuthMode == "EAPWIRELESS" && dwSupportAuthMode & 4)
    )
        WRITE_FIELD("PUBLIC", "AuthMode", strAuthMode.c_str());

    if (
        !newConfigfile.GetPrivateProfileInt("SECDOMAIN", "SecDomainEnable", 0) &&
        modifyinfo.bEnableSecDomain
    ) {
        WRITE_DIRECT_INT("SECDOMAIN", "SecDomainEnable", 1);
        WRITE_INT("SECDOMAIN", "SecDomainCount", strArraySecDomain.size());

        for (unsigned i = 0; i < modifyinfo.strArraySecDomain.size(); i++) {
            strKey = "SecDomain" + DWordToString(i);
            newConfigfile.WritePrivateProfileString(
                "SECDOMAIN",
                strKey.c_str(),
                modifyinfo.strArraySecDomain[i].c_str()
            );
        }

        WRITE_FIELD("PUBLIC", "SecDomain", strSelSecDomain.c_str());
    }

#undef WRITE_DIRECT_INT
#undef WRITE_DIRECT_FIELD
#undef WRITE_INT
#undef WRITE_FIELD
    newConfigfile.Close();
    newConfigfile.Unlock();
    return modifyinfo.bSavePassword;
}

int do_update(const std::string &strInstallPath, const std::string &strCurPath)
{
    int nRet = 0;
    struct UserInfo userinfo = {};
    ReadRegUserInfo(userinfo, strInstallPath + "fileReg.ini");
    WriteRegUserInfo(userinfo, strCurPath + "fileReg.ini");

    if ((nRet = ModifyConfFile(strInstallPath, strCurPath)) < 0) {
        g_logSystem.AppendText(
            "升级程序：更新配置文件信息失败 old[%s],new[%s],error=%d",
            strInstallPath.c_str(),
            strCurPath.c_str(),
            nRet
        );
        return -1;
    }

    if ((nRet = DispatchFile(strInstallPath, strCurPath)) < 0) {
        g_logSystem.AppendText(
            "升级程序：复制文件失败 src[%s],dst[%s],error=%d",
            strInstallPath.c_str(),
            strCurPath.c_str(),
            nRet
        );
        return -1;
    }

    if ((nRet = CleanFile(GetParentPath(strCurPath))) < 0) {
        g_logSystem.AppendText(
            "升级程序：删除临时文件[%s]失败 error=%d",
            GetParentPath(strCurPath).c_str(),
            nRet
        );
        return -1;
    }

    return nRet;
}

const char *opt_string = "h?p:e";

int main(int argc, char **argv)
{
    int opt = 0;
    int english = 0;
    int nResult = 0;
    std::string strExe;
    std::string strCurPath;
    std::string strInstallPath;

    while ((opt = getopt(argc, argv, opt_string)) == -1) {
        switch (opt) {
            case 'e':
                english = 1;
                break;

            case 'p':
                strInstallPath = optarg;
                break;
        }
    }

    if (strInstallPath.empty())
        return -1;

    TakeAppPath(g_strAppPath);

    if (g_strAppPath.empty())
        return -1;

    g_logSystem.CreateLogFile_S(strInstallPath, 3);
    strCurPath = g_strAppPath;

    if ((nResult = do_update(strInstallPath, strCurPath)) < 0) {
        // *INDENT-OFF*
        printf(english ? "rjsupplicant update failed.\n" : "客户端升级失败。\n");
        // *INDENT-ON*
        return -1;
    }

    // *INDENT-OFF*
    printf(english ? "rjsupplicant update success.\n" : "客户端升级成功。\n");
    // *INDENT-ON*
    if (nResult == 1) {
        strExe = strInstallPath + "rjsupplicant.sh";
        execl(strExe.c_str(), nullptr);
    }

    return 0;
}
