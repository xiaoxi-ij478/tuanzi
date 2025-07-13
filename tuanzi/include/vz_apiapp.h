#ifndef VZ_APIAPP_H_INCLUDED
#define VZ_APIAPP_H_INCLUDED

// the first part is the appdata/upper_data's hash algorithm, and
// the second part is the dlldata/lower_data's hash algorithm
enum HASH_TYPE {
    HASH_MD5_MD5,
    HASH_SHA1_SHA1,
    HASH_RIPEMD128_SHA1,
    HASH_TIGER_RIPEMD128,
    HASH_TIGER_SHA1,
    HASH_MAX
};

class CVz_APIApp
{
    public:
        CVz_APIApp();
        ~CVz_APIApp();

        void Vz_API(char *result, const char *md5_challenge, const char *a4) const;
        static void V3HeartbeatAPI(
            const char *data,
            unsigned datalen,
            char *result,
            enum HASH_TYPE hash_type
        );

    private:
        char *GetAppData(unsigned &size) const;
        char *GetDllData(unsigned &size) const;
        char *GetFileData(unsigned &size, const char *filename) const;
        void PrepareData(
            char *result,
            enum HASH_TYPE hash_type,
            const char *md5_challenge,
            const char *
        ) const;
};

#endif // VZ_APIAPP_H_INCLUDED
