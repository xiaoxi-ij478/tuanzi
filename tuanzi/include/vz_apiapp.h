#ifndef VZ_APIAPP_H_INCLUDED
#define VZ_APIAPP_H_INCLUDED

// the first part is the appdata/upper_data's hash algorithm, and
// the second part is the dlldata/lower_data's hash algorithm
enum HASH_TYPE {
    HASH_MD5_MD5,
    HASH_SHA1_SHA1,
    HASH_RIPEMD128_SHA1,
    HASH_TIGER_RIPEMD128,
    HASH_TIGER_SHA1
};

class CVz_APIApp
{
    public:
        CVz_APIApp();
        ~CVz_APIApp();

        void Vz_API(char *result, const char *md5_challenge, const char *a4) const;
        static void V3HeartbeatAPI(
            const unsigned char *data,
            unsigned datalen,
            unsigned char *result,
            enum HASH_TYPE hash_type
        );

    private:
        void *GetAppData(unsigned &size) const;
        void *GetDllData(unsigned &size) const;
        void *GetFileData(unsigned &size, const char *filename) const;
        void PrepareData(
            char *result,
            enum HASH_TYPE hash_type,
            const char *md5_challenge,
            const char *a5
        ) const;
};

#endif // VZ_APIAPP_H_INCLUDED
