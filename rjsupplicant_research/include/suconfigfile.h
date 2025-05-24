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
        static unsigned GetPrivateProfileInt(
            const char *domain,
            const char *key,
            unsigned defval
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

        static void EnablePrivilege(const char *a1, bool a2);
        static void GetSysUPTime(unsigned &a1, unsigned &a2);
        static void LogToFile(const char *str);
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
