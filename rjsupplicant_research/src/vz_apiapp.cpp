#include "global.h"
#include "cmdutil.h"
#include "md5forvz.h"
#include "vz_apiapp.h"

CVz_APIApp::CVz_APIApp()
{
    int ret = pthread_rwlock_init(&g_fileLock, nullptr);

    if (ret)
        rj_printf_debug("pthread_rwlock_init error %d\n", ret);
}

CVz_APIApp::~CVz_APIApp()
{
    int ret = pthread_rwlock_destroy(&g_fileLock);

    if (ret)
        rj_printf_debug("pthread_rwlock_destroy error %d\n", ret);
}
void CVz_APIApp::V3HeartbeatAPI(
    unsigned char *,
    unsigned,
    unsigned char *,
    unsigned
)
{
}

void CVz_APIApp::Vz_API(char *, char *, const char *)
{
}

void *CVz_APIApp::GetAppData(int &size)
{
    return
        memcpy(
            new unsigned char[sizeof(g_pAppData)],
            g_pAppData,
            size = sizeof(g_pAppData)
        );
}

void *CVz_APIApp::GetDllData(int &size)
{
    return
        memcpy(
            new unsigned char[sizeof(g_pDllData)],
            g_pDllData,
            size = sizeof(g_pDllData)
        );
}

void *CVz_APIApp::GetFileData(int &size, const char *filename)
{
    std::ifstream ifs;
    unsigned char *ret = nullptr;

    if (!pthread_rwlock_tryrdlock(&g_fileLock))
        return nullptr;

    ifs.open(filename);

    if (!ifs)
        return nullptr;

    ifs.seekg(0, std::ios::end);
    ret = new unsigned char[size = ifs.tellg()];
    ifs.seekg(0, std::ios::beg);
    ifs.read(reinterpret_cast<char *>(ret), size);
    ifs.close();
    pthread_rwlock_unlock(&g_fileLock);
    return ret;
}

void CVz_APIApp::PrepareData(
    char *result,
    enum HASH_TYPE hash_type,
    char *md5_challenge,
    [[maybe_unused]] const char *a5
)
{
    int appdata_len = 0;
    int dlldata_len = 0;
    unsigned char *appdata = nullptr;
    unsigned char *dlldata = nullptr;
    char *temp_appdata_md5 = nullptr;
    char *temp_dlldata_md5 = nullptr;
    char appdata_md5[300] = {};
    char dlldata_md5[300] = {};
    unsigned char appdata_sha1[20] = {};
    unsigned char dlldata_sha1[20] = {};
    unsigned char appdata_ripemd128[16] = {};
    unsigned char dlldata_ripemd128[16] = {};
    unsigned char appdata_tiger[24] = {};
    unsigned char whirlpool_tmpbuf1[96] = {};
    unsigned char whirlpool_tmpbuf2[56] = {};
    unsigned char whirlpool_tmpbuf3[52] = {};
    unsigned char whirlpool_tmpbuf4[60] = {};
    unsigned char whirlpool_dstbuf[64] = {};
    unsigned char dstbuf[600] = {};
    struct whirlpool_ctx whirlpool_context = {};
    struct sha1_ctx sha1_context = {};
    struct ampheck_ripemd128 ripemd128_context = {};
    struct tiger_ctx tiger_context = {};

    if (!(appdata = static_cast<unsigned char *>(GetAppData(appdata_len))))
        return;

    if (!(dlldata = static_cast<unsigned char *>(GetDllData(dlldata_len))))
        return;

    switch (hash_type) {
        case MD5_MD5_WHIRLPOOL:
            temp_appdata_md5 = CMD5ForVz::GetMD5(appdata, appdata_len);
            temp_dlldata_md5 = CMD5ForVz::GetMD5(dlldata, dlldata_len);
            strcpy(appdata_md5, temp_appdata_md5);
            strcpy(dlldata_md5, temp_dlldata_md5);
            delete[] temp_appdata_md5;
            delete[] temp_dlldata_md5;

            // alternatively insert md5_challenge into
            // appdata_md5 and dlldata_md5
            for (int i = 0, a = 0, b = 0; i < 16; i++)
                sprintf(
                    (i & 1 ? dlldata_md5 : appdata_md5) + i * 2 + 32,
                    "%02x",
                    md5_challenge[i]
                );

            memcpy(dstbuf, appdata_md5, strlen(appdata_md5));
            memcpy(dstbuf + strlen(appdata_md5), dlldata_md5, strlen(dlldata_md5));
            memcpy(whirlpool_tmpbuf1, dstbuf, sizeof(whirlpool_tmpbuf1));
            rhash_whirlpool_init_Vz(&whirlpool_context);
            rhash_whirlpool_update_Vz(
                &whirlpool_context,
                whirlpool_tmpbuf1,
                32 * 2 + 16 * 2
            );
            rhash_whirlpool_final_Vz(&whirlpool_context, whirlpool_dstbuf);
            break;

        case SHA1_SHA1_WHIRLPOOL:
            rhash_sha1_init_Vz(&sha1_context);
            rhash_sha1_update_Vz(&sha1_context, appdata, appdata_len);
            rhash_sha1_final_Vz(&sha1_context, appdata_sha1);
            rhash_sha1_init_Vz(&sha1_context);
            rhash_sha1_update_Vz(&sha1_context, dlldata, dlldata_len);
            rhash_sha1_final_Vz(&sha1_context, dlldata_sha1);
            // first add all dlldata_sha1
            // then 6 bytes of md5_challenge
            // then add all appdata_sha1
            // finally add 10 bytes of md5_challenge
            memcpy(whirlpool_tmpbuf2, dlldata_sha1, sizeof(dlldata_sha1));
            memcpy(
                whirlpool_tmpbuf2 + sizeof(dlldata_sha1),
                md5_challenge,
                6
            );
            memcpy(
                whirlpool_tmpbuf2 + sizeof(dlldata_sha1) + 6,
                appdata_sha1,
                sizeof(appdata_sha1)
            );
            memcpy(
                whirlpool_tmpbuf2 + sizeof(dlldata_sha1) + 6 + sizeof(appdata_sha1),
                md5_challenge + 6,
                10
            );
            rhash_whirlpool_init_Vz(&whirlpool_context);
            rhash_whirlpool_update_Vz(
                &whirlpool_context,
                whirlpool_tmpbuf2,
                sizeof(appdata_sha1) + sizeof(dlldata_sha1) + 16
            );
            rhash_whirlpool_final_Vz(&whirlpool_context, whirlpool_dstbuf);
            break;

        case RIPEMD128_SHA1_WHIRLPOOL:
            ampheck_ripemd128_init_Vz(&ripemd128_context);
            ampheck_ripemd128_update_Vz(&ripemd128_context, appdata, appdata_len);
            ampheck_ripemd128_final_Vz(&ripemd128_context, appdata_ripemd128);
            rhash_sha1_init_Vz(&sha1_context);
            rhash_sha1_update_Vz(&sha1_context, dlldata, dlldata_len);
            rhash_sha1_final_Vz(&sha1_context, dlldata_sha1);
            // first add all dlldata_sha1
            // then 6 bytes of md5_challenge
            // then add all appdata_ripemd128
            // finally add 10 bytes of md5_challenge
            memcpy(whirlpool_tmpbuf3, dlldata_sha1, sizeof(dlldata_sha1));
            memcpy(
                whirlpool_tmpbuf3 + sizeof(dlldata_sha1),
                md5_challenge,
                6
            );
            memcpy(
                whirlpool_tmpbuf3 + sizeof(dlldata_sha1) + 6,
                appdata_ripemd128,
                sizeof(appdata_ripemd128)
            );
            memcpy(
                whirlpool_tmpbuf3 + sizeof(dlldata_sha1) + 6 +
                sizeof(appdata_ripemd128),
                md5_challenge + 6,
                10
            );
            rhash_whirlpool_init_Vz(&whirlpool_context);
            rhash_whirlpool_update_Vz(
                &whirlpool_context,
                whirlpool_tmpbuf3,
                sizeof(dlldata_sha1) + sizeof(appdata_ripemd128) + 16
            );
            rhash_whirlpool_final_Vz(&whirlpool_context, whirlpool_dstbuf);
            break;

        case TIGER_RIPEMD128_WHIRLPOOL:
            rhash_tiger_init_Vz(&tiger_context);
            rhash_tiger_update_Vz(&tiger_context, appdata, appdata_len);
            rhash_tiger_final_Vz(&tiger_context, appdata_tiger);
            ampheck_ripemd128_init_Vz(&ripemd128_context);
            ampheck_ripemd128_update_Vz(&ripemd128_context, dlldata, dlldata_len);
            ampheck_ripemd128_final_Vz(&ripemd128_context, dlldata_ripemd128);
            // first add all appdata_tiger
            // then add 10 bytes of md5_challenge !!!!!
            // then add all dlldata_ripemd128
            // finally add 6 bytes of md5_challenge
            memcpy(whirlpool_tmpbuf2, appdata_tiger, sizeof(appdata_tiger));
            memcpy(
                whirlpool_tmpbuf2 + sizeof(appdata_tiger),
                md5_challenge,
                10
            );
            memcpy(
                whirlpool_tmpbuf2 + sizeof(appdata_tiger) + 10,
                dlldata_ripemd128,
                sizeof(dlldata_ripemd128)
            );
            memcpy(
                whirlpool_tmpbuf2 + sizeof(appdata_tiger) + 10 +
                sizeof(dlldata_ripemd128),
                md5_challenge + 10,
                6
            );
            rhash_whirlpool_init_Vz(&whirlpool_context);
            rhash_whirlpool_update_Vz(
                &whirlpool_context,
                whirlpool_tmpbuf2,
                sizeof(appdata_tiger) + sizeof(dlldata_ripemd128) + 16
            );
            rhash_whirlpool_final_Vz(&whirlpool_context, whirlpool_dstbuf);
            break;

        case TIGER_SHA1_WHIRLPOOL:
            rhash_tiger_init_Vz(&tiger_context);
            rhash_tiger_update_Vz(&tiger_context, appdata, appdata_len);
            rhash_tiger_final_Vz(&tiger_context, appdata_tiger);
            rhash_sha1_init_Vz(&sha1_context);
            rhash_sha1_update_Vz(&sha1_context, dlldata, dlldata_len);
            rhash_sha1_final_Vz(&sha1_context, dlldata_sha1);
            // first add all appdata_tiger
            // then add 8 bytes of md5_challenge !!!!!
            // then add all dlldata_sha1
            // finally add 8 bytes of md5_challenge
            memcpy(whirlpool_tmpbuf4, appdata_tiger, sizeof(appdata_tiger));
            memcpy(
                whirlpool_tmpbuf4 + sizeof(appdata_tiger),
                md5_challenge,
                8
            );
            memcpy(
                whirlpool_tmpbuf4 + sizeof(appdata_tiger) + 8,
                dlldata_sha1,
                sizeof(dlldata_sha1)
            );
            memcpy(
                whirlpool_tmpbuf4 + sizeof(appdata_tiger) + 8 +
                sizeof(dlldata_sha1),
                md5_challenge + 8,
                8
            );
            rhash_whirlpool_init_Vz(&whirlpool_context);
            rhash_whirlpool_update_Vz(
                &whirlpool_context,
                whirlpool_tmpbuf4,
                sizeof(appdata_tiger) + sizeof(dlldata_sha1) + 16
            );
            rhash_whirlpool_final_Vz(&whirlpool_context, whirlpool_dstbuf);
            break;

        default:
            delete[] appdata;
            delete[] dlldata;
            return;
    }

    memset(dstbuf, 0, 128);

    for (int i = 0; i < 64; i++)
        sprintf(
            reinterpret_cast<char *>(dstbuf + 2 * i),
            "%02x",
            whirlpool_dstbuf[i]
        );

    memcpy(result, dstbuf, 128);
    delete[] appdata;
    delete[] dlldata;
}
