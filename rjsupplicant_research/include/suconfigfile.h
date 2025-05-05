#ifndef SUCONFIGFILE_H
#define SUCONFIGFILE_H

#include <string>

#include "criticalsection.h"

class CSuConfigFile
{
    public:
        CSuConfigFile();
        virtual ~CSuConfigFile();
        static void Lock();
        static void Unlock();
        void Close();
        bool Open();
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
        bool WritePrivateProfileString(
            const char *domain,
            const char *key,
            const char *val
        );

    protected:

    private:
        [[maybe_unused]] static void EnablePrivilege(const char *a1, bool a2);
        [[maybe_unused]] static void GetSysUPTime(unsigned int &a1, unsigned int &a2);
        [[maybe_unused]] static void LogToFile(const char *str);
        bool Open(const char *filename);
        bool UpdateConfig();
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
