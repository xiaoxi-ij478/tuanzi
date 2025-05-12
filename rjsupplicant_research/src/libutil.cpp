#include "global.h"
#include "libutil.h"

#ifdef USE_EXTERNAL_LIBRT
static void *librt_handle = nullptr; /* orig name: librt */
#endif // USE_EXTERNAL_LIBRT

// the return value is its FAILURE value
// we use native librt, so make it a no-op
int load_librt()
{
#ifdef USE_EXTERNAL_LIBRT
    std::string try1, try2;

    if (check_glibc_version()) {
        try1 = "librt.so";
        try2 = "librt.so.1";

    } else {
        try1 = g_strAppPath + "lib/librt-2.6.so";
        try2 = g_strAppPath + "lib/librt-2.10.2.so";
    }

    // I just love this style
    if (!(librt_handle = dlopen(try1.c_str(), RTLD_LAZY)) ||
            !(librt_handle = dlopen(try2.c_str(), RTLD_LAZY)))
        return 1;

#define LOAD_SYMBOL(symbol) do { \
        *reinterpret_cast<void **>(&my_##symbol) = dlsym(librt_handle, #symbol); \
        if (dlerror()) { \
            free_librt(); \
            return -1; \
        } \
    } while (0)
    LOAD_SYMBOL(timer_create);
    LOAD_SYMBOL(timer_settime);
    LOAD_SYMBOL(timer_delete);
    LOAD_SYMBOL(timer_gettime);
    LOAD_SYMBOL(timer_getoverrun);
#undef LOAD_SYMBOL
#endif // USE_EXTERNAL_LIBRT
    return 0;
}

void free_librt()
{
#ifdef USE_EXTERNAL_LIBRT

    if (!librt_handle)
        return;

    dlclose(librt_handle);
    dlerror();
    librt_handle = nullptr;
#endif // USE_EXTERNAL_LIBRT
}

bool check_glibc_version()
{
#ifdef __GLIBC__
    const char *libc_version = gnu_get_libc_version();
    unsigned int major = strtol(libc_version, nullptr, 10);
    unsigned int minor = strtol(strchr(libc_version, '.'), nullptr, 10);
    return major < 2 && minor >= 7;
#else
    return false;
#endif // __GLIBC__
}
