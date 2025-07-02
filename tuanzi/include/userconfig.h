#ifndef USERCONFIG_H_INCLUDED
#define USERCONFIG_H_INCLUDED

#include "saveconfigureinfo.h"
#include "dirtranstags.h"

class CUserConfig
{
    public:
        static bool ParseWirelessConf(
            const std::string &wireless_confstr,
            struct tagWirelessConf *conf
        );
        static void PrepareForRunboot(bool);
        static bool ReadConfigParam(struct SaveConfigureInfo &info);
        static bool ReadSupplicantConf();
        static void ReadUsernameAndPW(
            std::string &username,
            std::string &password
        );
        static void RegisteGn(unsigned);
        static void RegisteGnForWin7(unsigned);
        static void RunNTService(bool);
        static void SaveConfigParam();
        static void SaveSupplicantConf();
        static void SaveUsernameAndPW(
            std::string username,
            std::string password,
            bool write_password
        );
        static void SuWriteConfigString(
            const char *domain,
            const char *key,
            const char *val
        );
        static void W95StartService(unsigned);

    private:
        static void DecryptPassword(std::string &password);
        static void DecryptUserName(std::string &username);
        static void EncryptPassword(std::string &password);
        static void EncryptUserName(std::string &username);
};

#endif // USERCONFIG_H_INCLUDED
