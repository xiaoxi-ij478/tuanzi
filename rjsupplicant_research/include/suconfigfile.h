#ifndef SUCONFIGFILE_H
#define SUCONFIGFILE_H

#include "criticalsection.h"

class CSuConfigFile
{
    public:
        CSuConfigFile();
        virtual ~CSuConfigFile();
        void Close();
        bool Open();
        bool WritePrivateProfileString(
            const char *domain,
            const char *key,
            const char *val
        ) const;

        static void Lock();
        static void Unlock();
        static unsigned int GetPrivateProfileInt(
            const char *domain,
            const char *key,
            unsigned int defval
        );
        static void GetPrivateProfileString(
            const char *domain,
            const char *key,
            const char *defval,
            std::string &dst
        );

    private:
        bool Open(const char *filename);
        bool UpdateConfig();

        [[maybe_unused]] static void EnablePrivilege(
            [[maybe_unused]] const char *a1,
            [[maybe_unused]] bool a2
        );
        [[maybe_unused]] static void GetSysUPTime(
            [[maybe_unused]] unsigned int &a1,
            [[maybe_unused]] unsigned int &a2
        );
        [[maybe_unused]] static void LogToFile(
            [[maybe_unused]] const char *str
        );
        static void DeleteFile(const std::string &filename);
        static void DeleteTempConfig();
        static void ProfileStringToString(std::string &str);
        static void StringToProfileString(std::string &str);
        static void AppendComma(std::string &str);

        bool config_dirty;
        bool is_open;
        std::string cfg_filename;
        static CRITICAL_SECTION CSuConfigFileLock;
};

#endif // SUCONFIGFILE_H
