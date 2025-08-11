#ifndef SUCONFIGFILE_H_INCLUDED
#define SUCONFIGFILE_H_INCLUDED

class CSuConfigFile
{
    public:
        CSuConfigFile();
        virtual ~CSuConfigFile();

        void Close();
        bool Open();
        bool Open(const char *filename);
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
            std::string& dst
        );

    private:
        bool UpdateConfig();

        static void EnablePrivilege(const char *lpszPrivilegeName, bool bEnable);
        static void GetSysUPTime(unsigned& highDataTime, unsigned& lowDataTime);
        static void LogToFile(const char *str);
        static void DeleteFile(const std::string& filename);
        static void DeleteTempConfig();
        static void ProfileStringToString(std::string& str);
        static void StringToProfileString(std::string& str);
        static void AppendComma(std::string& str);

        bool config_dirty;
        bool is_open;
        std::string cfg_filename;
};

#endif // SUCONFIGFILE_H_INCLUDED
