#include "fileutil.h"

unsigned long get_file_size(const char *filename)
{
    std::ifstream ifs(filename);
    unsigned long ret = 0;

    if (!ifs)
        return 0;

    ifs.seekg(0, std::ios::end);
    ret = ifs.tellg();
    ifs.close();
    return ret;
}

void DeleteFile(const char *filename)
{
    // the original implementation uses shell
    // "rm -rf $filename"
    remove(filename);
}

int cmd_mkdir(const char *dirname)
{
    return mkdir(dirname, 0755);
}

void removeFileOrDir(const char *filename)
{
    // the original implementation uses shell
    // "rm -rf $filename"
    remove(filename);
}

int get_sh_name(const char *src, char *dst)
{
    char buf[1024] = { 0 };
    char *tmp = nullptr;
    char *tmp2 = nullptr;

    if (strlen(src) >= 1024)
        return -1;

    strcpy(buf, src);

    if (!(tmp = strrchr(buf, '/')))
        return -1;

    if (!(tmp2 = strrchr(tmp, '.')))
        return -1;

    *tmp2 = 0;
    strcpy(dst, tmp);
    strcat(dst, ".sh");
    return 0;
}
