#include "all.h"
#include "util.h"
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
    return mkdir(dirname, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
}

void removeFileOrDir(const char *filename)
{
    // the original implementation uses shell
    // "rm -rf $filename"
    remove(filename);
}

int get_sh_name(const char *src, char *dst)
{
    char buf[1024] = {};
    char *tmp = nullptr;
    char *tmp2 = nullptr;

    if (strlen(src) >= sizeof(buf))
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

bool decompressFile(const char *filename, const char *destdir)
{
    if (!filename || !destdir)
        return false;

#define COMPARE_EQUAL(suffix) \
    (!strcmp(strchr(filename, 0) - strlen(suffix), suffix))

    if (COMPARE_EQUAL(".tar.gz"))
        system(
            std::string("tar -C ")
            .append(destdir)
            .append(" -zxf ")
            .append(filename)
//               .append(" 2>&-")
            .c_str()
        );

    else if (COMPARE_EQUAL(".tar"))
        system(
            std::string("tar -C ")
            .append(destdir)
            .append(" -xf ")
            .append(filename)
//               .append(" 2>&-")
            .c_str()
        );

    else if (COMPARE_EQUAL(".zip"))
        system(
            std::string("unzip -d ")
            .append(destdir)
            .append(" -q -o ")
            .append(filename)
//               .append(" 2>&-")
            .c_str()
        );

#undef COMPARE_EQUAL
    return true;
}

bool isFileExist(const char *filename)
{
    return !access(filename, F_OK);
}

bool SuCreateDirectory(const std::string &dirname)
{
    // "mkdir -p -m 666 $dirname"
    std::vector<std::string> pathnames;
    std::string tmp;

    if (
        mkdir(
            dirname.c_str(),
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
        ) != -1
    )
        return true;

    if (errno != ENOENT)
        return true;

    ParseString(dirname, '/', pathnames);

    for (const std::string &path : pathnames) {
        if (!path.empty())
            tmp.append("/");

        tmp.append(path);

        if (
            mkdir(
                tmp.c_str(),
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
            ) == -1
        )
            return true;
    }

    return true;
}
