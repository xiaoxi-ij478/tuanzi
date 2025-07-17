#include "all.h"
#include "global.h"
#include "cmdutil.h"
#include "md5forvz.h"
#include "vz_apiapp.h"

// helper macro for copying 4-parted data (see below for usage)
// that is,
// <data1>
// <data2[:first_len]>
// <data3>
// <data2[first_len:first_len + second_len]>
// They must be all arrays to make sizeof work
#define COPY_4_PARTED_DATA(to, data1, data2, data3, data2_first_len, data2_second_len) \
    do { \
        memcpy((to), (data1), sizeof(data1)); \
        memcpy((to) + sizeof(data1), (data2), (data2_first_len)); \
        memcpy((to) + sizeof(data1) + (data2_first_len), (data3), sizeof(data3)); \
        memcpy( \
                (to) + sizeof(data1) + (data2_first_len) + sizeof(data3), \
                (data2) + (data2_first_len), \
                (data2_second_len) \
              ); \
    } while (0)

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

// most of the codes are derived from CVz_APIApp::PrepareData
void CVz_APIApp::V3HeartbeatAPI(
    const char *data,
    unsigned datalen,
    char *result,
    // otherwise astyle treats it as enumerate declaration
    // *INDENT-OFF*
    enum HASH_TYPE hash_type
    // *INDENT-ON*
)
{
    const char md5_challenge[16] = {
        0x17, 0xFE, 0x8D, 0x44, 0x1A, 0x19, 0x09, 0xC0,
        0xA2, 0xB5, 0x31, 0xD7, 0xA8, 0x0D, 0xC9, 0x91
    };
    unsigned upper_data_len = datalen / 2;
    unsigned lower_data_len = datalen - datalen / 2;
    const char *upper_data = data;
    const char *lower_data = data + upper_data_len;
    char *temp_upper_data_md5 = nullptr;
    char *temp_lower_data_md5 = nullptr;
    char upper_data_md5[300] = {};
    char lower_data_md5[300] = {};
    char upper_data_sha1[20] = {};
    char lower_data_sha1[20] = {};
    char upper_data_ripemd128[16] = {};
    char lower_data_ripemd128[16] = {};
    char upper_data_tiger[24] = {};
    char whirlpool_tmpbuf1[96] = {};
    char whirlpool_tmpbuf2[56] = {};
    char whirlpool_tmpbuf3[52] = {};
    char whirlpool_tmpbuf4[60] = {};
    char whirlpool_dstbuf[64] = {};
    struct whirlpool_ctx whirlpool_context = {};
    struct sha1_ctx sha1_context = {};
    struct ampheck_ripemd128 ripemd128_context = {};
    struct tiger_ctx tiger_context = {};

    switch (hash_type % HASH_MAX) {
        case HASH_MD5_MD5:
            temp_upper_data_md5 = CMD5ForVz::GetMD5(upper_data, upper_data_len);
            temp_lower_data_md5 = CMD5ForVz::GetMD5(lower_data, lower_data_len);
            strcpy(upper_data_md5, temp_upper_data_md5);
            strcpy(lower_data_md5, temp_lower_data_md5);
            delete[] temp_upper_data_md5;
            delete[] temp_lower_data_md5;

            // alternatively insert md5_challenge into
            // upper_data_md5 and lower_data_md5
            for (unsigned i = 0; i < 16; i++)
                sprintf(
                    (i & 1 ? lower_data_md5 : upper_data_md5) + i * 2 + 32,
                    "%02x",
                    md5_challenge[i]
                );

//            memcpy(whirlpool_tmpbuf1, upper_data_md5, strlen(upper_data_md5));
//            memcpy(
//                whirlpool_tmpbuf1 + strlen(upper_data_md5),
//                lower_data_md5,
//                strlen(lower_data_md5)
//            );
            strcpy(whirlpool_tmpbuf1, upper_data_md5);
            strcpy(whirlpool_tmpbuf1 + strlen(upper_data_md5), lower_data_md5);
            rhash_whirlpool_init_Vz(&whirlpool_context);
            rhash_whirlpool_update_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_tmpbuf1),
                strlen(upper_data_md5) + strlen(lower_data_md5) + 16 * 2
            );
            rhash_whirlpool_final_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_dstbuf)
            );
            break;

        case HASH_SHA1_SHA1:
            rhash_sha1_init_Vz(&sha1_context);
            rhash_sha1_update_Vz(
                &sha1_context,
                reinterpret_cast<const unsigned char *>(lower_data),
                lower_data_len
            );
            rhash_sha1_final_Vz(
                &sha1_context,
                reinterpret_cast<unsigned char *>(lower_data_sha1)
            );
            rhash_sha1_init_Vz(&sha1_context);
            rhash_sha1_update_Vz(
                &sha1_context,
                reinterpret_cast<const unsigned char *>(upper_data),
                upper_data_len
            );
            rhash_sha1_final_Vz(
                &sha1_context,
                reinterpret_cast<unsigned char *>(upper_data_sha1)
            );
            // first add all lower_data_sha1
            // then 6 bytes of md5_challenge
            // then add all upper_data_sha1
            // finally add 10 bytes of md5_challenge
            COPY_4_PARTED_DATA(
                whirlpool_tmpbuf2,
                lower_data_sha1,
                md5_challenge,
                upper_data_sha1,
                6,
                10
            );
            rhash_whirlpool_init_Vz(&whirlpool_context);
            rhash_whirlpool_update_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_tmpbuf2),
                sizeof(lower_data_sha1) + sizeof(upper_data_sha1) + 16
            );
            rhash_whirlpool_final_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_dstbuf)
            );
            break;

        case HASH_RIPEMD128_SHA1:
            ampheck_ripemd128_init_Vz(&ripemd128_context);
            ampheck_ripemd128_update_Vz(
                &ripemd128_context,
                reinterpret_cast<const unsigned char *>(upper_data),
                upper_data_len
            );
            ampheck_ripemd128_finish_Vz(
                &ripemd128_context,
                reinterpret_cast<unsigned char *>(upper_data_ripemd128)
            );
            rhash_sha1_init_Vz(&sha1_context);
            rhash_sha1_update_Vz(
                &sha1_context,
                reinterpret_cast<const unsigned char *>(lower_data),
                lower_data_len
            );
            rhash_sha1_final_Vz(
                &sha1_context,
                reinterpret_cast<unsigned char *>(lower_data_sha1)
            );
            // first add all lower_data_sha1
            // then 6 bytes of md5_challenge
            // then add all upper_data_ripemd128
            // finally add 10 bytes of md5_challenge
            COPY_4_PARTED_DATA(
                whirlpool_tmpbuf3,
                lower_data_sha1,
                md5_challenge,
                upper_data_ripemd128,
                6,
                10
            );
            rhash_whirlpool_init_Vz(&whirlpool_context);
            rhash_whirlpool_update_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_tmpbuf3),
                sizeof(lower_data_sha1) + sizeof(upper_data_ripemd128) + 16
            );
            rhash_whirlpool_final_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_dstbuf)
            );
            break;

        case HASH_TIGER_RIPEMD128:
            rhash_tiger_init_Vz(&tiger_context);
            rhash_tiger_update_Vz(
                &tiger_context,
                reinterpret_cast<const unsigned char *>(upper_data),
                upper_data_len
            );
            rhash_tiger_final_Vz(
                &tiger_context,
                reinterpret_cast<unsigned char *>(upper_data_tiger)
            );
            ampheck_ripemd128_init_Vz(&ripemd128_context);
            ampheck_ripemd128_update_Vz(
                &ripemd128_context,
                reinterpret_cast<const unsigned char *>(lower_data),
                lower_data_len
            );
            ampheck_ripemd128_finish_Vz(
                &ripemd128_context,
                reinterpret_cast<unsigned char *>(lower_data_ripemd128)
            );
            // first add all upper_data_tiger
            // then add 10 bytes of md5_challenge !!!!!
            // then add all lower_data_ripemd128
            // finally add 6 bytes of md5_challenge
            COPY_4_PARTED_DATA(
                whirlpool_tmpbuf2,
                upper_data_tiger,
                md5_challenge,
                lower_data_ripemd128,
                10,
                6
            );
            rhash_whirlpool_init_Vz(&whirlpool_context);
            rhash_whirlpool_update_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_tmpbuf2),
                sizeof(upper_data_tiger) + sizeof(lower_data_ripemd128) + 16
            );
            rhash_whirlpool_final_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_dstbuf)
            );
            break;

        case HASH_TIGER_SHA1:
            rhash_tiger_init_Vz(&tiger_context);
            rhash_tiger_update_Vz(
                &tiger_context,
                reinterpret_cast<const unsigned char *>(upper_data),
                upper_data_len
            );
            rhash_tiger_final_Vz(
                &tiger_context,
                reinterpret_cast<unsigned char *>(upper_data_tiger)
            );
            rhash_sha1_init_Vz(&sha1_context);
            rhash_sha1_update_Vz(
                &sha1_context,
                reinterpret_cast<const unsigned char *>(lower_data),
                lower_data_len
            );
            rhash_sha1_final_Vz(
                &sha1_context,
                reinterpret_cast<unsigned char *>(lower_data_sha1)
            );
            // first add all upper_data_tiger
            // then add 8 bytes of md5_challenge !!!!!
            // then add all lower_data_sha1
            // finally add 8 bytes of md5_challenge
            COPY_4_PARTED_DATA(
                whirlpool_tmpbuf4,
                upper_data_tiger,
                md5_challenge,
                lower_data_sha1,
                8,
                8
            );
            rhash_whirlpool_init_Vz(&whirlpool_context);
            rhash_whirlpool_update_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_tmpbuf4),
                sizeof(upper_data_tiger) + sizeof(lower_data_sha1) + 16
            );
            rhash_whirlpool_final_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_dstbuf)
            );
            break;
    }

    for (unsigned i = 0; i < 64; i++)
        sprintf(result + 2 * i, "%02x", whirlpool_dstbuf[i]);
}

void CVz_APIApp::Vz_API(
    char *result,
    const char *md5_challenge,
    const char *a4
) const
{
    char random_staff[256] = {};
    srand(time(nullptr));

    // I don't know its usage...
    for (int i = 0; i < 128; i += 4)
        sprintf(random_staff + i, "%04x", rand());

    strcat(
        random_staff,
        "fd8e40a61e70c67ba4dd65c0b6424b77"
        "20de104fcb4c620ab474be6136d62b92"
        "c995edef2c149236c126f78a35fd371d"
        "b6a4d4c4c7f7e600467a51074e425bdd"
    );
    PrepareData(
        result,
        static_cast<enum HASH_TYPE>((md5_challenge[3] + md5_challenge[0]) % 5),
        md5_challenge,
        a4
    );
}

char *CVz_APIApp::GetAppData(unsigned &size) const
{
    return
        static_cast<char *>(
            memcpy(
                new char[sizeof(g_pAppData)],
                g_pAppData,
                size = sizeof(g_pAppData)
            )
        );
}

char *CVz_APIApp::GetDllData(unsigned &size) const
{
    return
        static_cast<char *>(
            memcpy(
                new char[sizeof(g_pDllData)],
                g_pDllData,
                size = sizeof(g_pDllData)
            )
        );
}

char *CVz_APIApp::GetFileData(unsigned &size, const char *filename) const
{
    std::ifstream ifs;
    char *ret = nullptr;

    if (!pthread_rwlock_tryrdlock(&g_fileLock))
        return nullptr;

    ifs.open(filename);

    if (!ifs)
        return nullptr;

    ifs.seekg(0, std::ios::end);
    ret = new char[size = ifs.tellg()];
    ifs.seekg(0, std::ios::beg);
    ifs.read(ret, size);
    ifs.close();
    pthread_rwlock_unlock(&g_fileLock);
    return ret;
}

void CVz_APIApp::PrepareData(
    char *result,
    enum HASH_TYPE hash_type,
    const char *md5_challenge,
    const char *
) const
{
    unsigned appdata_len = 0;
    unsigned dlldata_len = 0;
    char *appdata = nullptr;
    char *dlldata = nullptr;
    char *temp_appdata_md5 = nullptr;
    char *temp_dlldata_md5 = nullptr;
    char appdata_md5[300] = {};
    char dlldata_md5[300] = {};
    char appdata_sha1[20] = {};
    char dlldata_sha1[20] = {};
    char appdata_ripemd128[16] = {};
    char dlldata_ripemd128[16] = {};
    char appdata_tiger[24] = {};
    char whirlpool_tmpbuf1[96] = {};
    char whirlpool_tmpbuf2[56] = {};
    char whirlpool_tmpbuf3[52] = {};
    char whirlpool_tmpbuf4[60] = {};
    char whirlpool_dstbuf[64] = {};
//    char dstbuf[600] = {};
    struct whirlpool_ctx whirlpool_context = {};
    struct sha1_ctx sha1_context = {};
    struct ampheck_ripemd128 ripemd128_context = {};
    struct tiger_ctx tiger_context = {};

    if (!(appdata = GetAppData(appdata_len)))
        return;

    if (!(dlldata = GetDllData(dlldata_len)))
        return;

    switch (hash_type) {
        case HASH_MD5_MD5:
            temp_appdata_md5 = CMD5ForVz::GetMD5(appdata, appdata_len);
            temp_dlldata_md5 = CMD5ForVz::GetMD5(dlldata, dlldata_len);
            strcpy(appdata_md5, temp_appdata_md5);
            strcpy(dlldata_md5, temp_dlldata_md5);
            delete[] temp_appdata_md5;
            delete[] temp_dlldata_md5;

            // alternatively insert md5_challenge into
            // appdata_md5 and dlldata_md5
            for (unsigned i = 0; i < 16; i++)
                sprintf(
                    (i & 1 ? dlldata_md5 : appdata_md5) + i * 2 + 32,
                    "%02x",
                    md5_challenge[i]
                );

//            memcpy(dstbuf, appdata_md5, strlen(appdata_md5));
//            memcpy(dstbuf + strlen(appdata_md5), dlldata_md5, strlen(dlldata_md5));
//            memcpy(whirlpool_tmpbuf1, dstbuf, sizeof(whirlpool_tmpbuf1));
//            memcpy(whirlpool_tmpbuf1, appdata_md5, strlen(appdata_md5));
//            memcpy(
//                whirlpool_tmpbuf1 + strlen(appdata_md5),
//                dlldata_md5,
//                strlen(dlldata_md5)
//            );
            strcpy(whirlpool_tmpbuf1, appdata_md5);
            strcpy(whirlpool_tmpbuf1 + strlen(appdata_md5), dlldata_md5);
            rhash_whirlpool_init_Vz(&whirlpool_context);
            rhash_whirlpool_update_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_tmpbuf1),
                strlen(appdata_md5) + strlen(dlldata_md5) + 16 * 2
            );
            rhash_whirlpool_final_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_dstbuf)
            );
            break;

        case HASH_SHA1_SHA1:
            rhash_sha1_init_Vz(&sha1_context);
            rhash_sha1_update_Vz(
                &sha1_context,
                reinterpret_cast<unsigned char *>(appdata),
                appdata_len
            );
            rhash_sha1_final_Vz(
                &sha1_context,
                reinterpret_cast<unsigned char *>(appdata_sha1)
            );
            rhash_sha1_init_Vz(&sha1_context);
            rhash_sha1_update_Vz(
                &sha1_context,
                reinterpret_cast<unsigned char *>(dlldata),
                dlldata_len
            );
            rhash_sha1_final_Vz(
                &sha1_context,
                reinterpret_cast<unsigned char *>(dlldata_sha1)
            );
            // first add all dlldata_sha1
            // then 6 bytes of md5_challenge
            // then add all appdata_sha1
            // finally add 10 bytes of md5_challenge
            COPY_4_PARTED_DATA(
                whirlpool_tmpbuf2,
                dlldata_sha1,
                md5_challenge,
                appdata_sha1,
                6,
                10
            );
            rhash_whirlpool_init_Vz(&whirlpool_context);
            rhash_whirlpool_update_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_tmpbuf2),
                sizeof(appdata_sha1) + sizeof(dlldata_sha1) + 16
            );
            rhash_whirlpool_final_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_dstbuf)
            );
            break;

        case HASH_RIPEMD128_SHA1:
            ampheck_ripemd128_init_Vz(&ripemd128_context);
            ampheck_ripemd128_update_Vz(
                &ripemd128_context,
                reinterpret_cast<unsigned char *>(appdata),
                appdata_len
            );
            ampheck_ripemd128_finish_Vz(
                &ripemd128_context,
                reinterpret_cast<unsigned char *>(appdata_ripemd128)
            );
            rhash_sha1_init_Vz(&sha1_context);
            rhash_sha1_update_Vz(
                &sha1_context,
                reinterpret_cast<unsigned char *>(dlldata),
                dlldata_len
            );
            rhash_sha1_final_Vz(
                &sha1_context,
                reinterpret_cast<unsigned char *>(dlldata_sha1)
            );
            // first add all dlldata_sha1
            // then 6 bytes of md5_challenge
            // then add all appdata_ripemd128
            // finally add 10 bytes of md5_challenge
            COPY_4_PARTED_DATA(
                whirlpool_tmpbuf3,
                dlldata_sha1,
                md5_challenge,
                appdata_ripemd128,
                6,
                10
            );
            rhash_whirlpool_init_Vz(&whirlpool_context);
            rhash_whirlpool_update_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_tmpbuf3),
                sizeof(dlldata_sha1) + sizeof(appdata_ripemd128) + 16
            );
            rhash_whirlpool_final_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_dstbuf)
            );
            break;

        case HASH_TIGER_RIPEMD128:
            rhash_tiger_init_Vz(&tiger_context);
            rhash_tiger_update_Vz(
                &tiger_context,
                reinterpret_cast<unsigned char *>(appdata),
                appdata_len
            );
            rhash_tiger_final_Vz(
                &tiger_context,
                reinterpret_cast<unsigned char *>(appdata_tiger)
            );
            ampheck_ripemd128_init_Vz(&ripemd128_context);
            ampheck_ripemd128_update_Vz(
                &ripemd128_context,
                reinterpret_cast<unsigned char *>(dlldata),
                dlldata_len
            );
            ampheck_ripemd128_finish_Vz(
                &ripemd128_context,
                reinterpret_cast<unsigned char *>(dlldata_ripemd128)
            );
            // first add all appdata_tiger
            // then add 10 bytes of md5_challenge !!!!!
            // then add all dlldata_ripemd128
            // finally add 6 bytes of md5_challenge
            COPY_4_PARTED_DATA(
                whirlpool_tmpbuf2,
                appdata_tiger,
                md5_challenge,
                dlldata_ripemd128,
                10,
                6
            );
            rhash_whirlpool_init_Vz(&whirlpool_context);
            rhash_whirlpool_update_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_tmpbuf2),
                sizeof(appdata_tiger) + sizeof(dlldata_ripemd128) + 16
            );
            rhash_whirlpool_final_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_dstbuf)
            );
            break;

        case HASH_TIGER_SHA1:
            rhash_tiger_init_Vz(&tiger_context);
            rhash_tiger_update_Vz(
                &tiger_context,
                reinterpret_cast<unsigned char *>(appdata),
                appdata_len
            );
            rhash_tiger_final_Vz(
                &tiger_context,
                reinterpret_cast<unsigned char *>(appdata_tiger)
            );
            rhash_sha1_init_Vz(&sha1_context);
            rhash_sha1_update_Vz(
                &sha1_context,
                reinterpret_cast<unsigned char *>(dlldata),
                dlldata_len
            );
            rhash_sha1_final_Vz(
                &sha1_context,
                reinterpret_cast<unsigned char *>(dlldata_sha1)
            );
            // first add all appdata_tiger
            // then add 8 bytes of md5_challenge !!!!!
            // then add all dlldata_sha1
            // finally add 8 bytes of md5_challenge
            COPY_4_PARTED_DATA(
                whirlpool_tmpbuf4,
                appdata_tiger,
                md5_challenge,
                dlldata_sha1,
                8,
                8
            );
            rhash_whirlpool_init_Vz(&whirlpool_context);
            rhash_whirlpool_update_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_tmpbuf4),
                sizeof(appdata_tiger) + sizeof(dlldata_sha1) + 16
            );
            rhash_whirlpool_final_Vz(
                &whirlpool_context,
                reinterpret_cast<unsigned char *>(whirlpool_dstbuf)
            );
            break;

        default:
            delete[] appdata;
            delete[] dlldata;
            return;
    }

//    memset(dstbuf, 0, 128);

    for (unsigned i = 0; i < 64; i++)
        sprintf(result + 2 * i, "%02x", whirlpool_dstbuf[i]);

//    memcpy(result, dstbuf, 128);
    delete[] appdata;
    delete[] dlldata;
}
