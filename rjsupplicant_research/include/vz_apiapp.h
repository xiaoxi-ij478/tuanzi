#ifndef VZ_APIAPP_H
#define VZ_APIAPP_H

// the first part is the appdata's hash algorithm, and
// the second part is the dlldata's hash algorithm
enum HASH_TYPE {
    MD5_MD5_WHIRLPOOL,
    SHA1_SHA1_WHIRLPOOL,
    RIPEMD128_SHA1_WHIRLPOOL,
    TIGER_RIPEMD128_WHIRLPOOL,
    TIGER_SHA1_WHIRLPOOL
};

class CVz_APIApp
{
    public:
        CVz_APIApp();
        ~CVz_APIApp();

        void V3HeartbeatAPI(
            unsigned char *,
            unsigned,
            unsigned char *,
            unsigned
        );
        void Vz_API(char *, char *, const char *);

    private:
        void *GetAppData(int &size);
        void *GetDllData(int &size);
        void *GetFileData(int &size, const char *filename);
        void PrepareData(
            char *result,
            enum HASH_TYPE hash_type,
            char *md5_challenge,
            [[maybe_unused]] const char *a5
        );
};

#endif // VZ_APIAPP_H
