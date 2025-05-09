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

bool decompressFile(const char *cfilename, const char *cdestdir)
{
    std::string filename;
    std::string destdir;

    if (!cfilename || !cdestdir)
        return false;

    filename = cfilename;
    destdir = cdestdir;

    if (!filename.compare(filename.length() - 8, 7, ".tar.gz"))
        system(
            std::string("tar -C ")
            .append(destdir)
            .append(" -zxf ")
            .append(filename)
//               .append(" 2>&-")
            .c_str()
        );

    else if (!filename.compare(filename.length() - 5, 4, ".tar"))
        system(
            std::string("tar -C ")
            .append(destdir)
            .append(" -xf ")
            .append(filename)
//               .append(" 2>&-")
            .c_str()
        );

    else if (!filename.compare(filename.length() - 5, 4, ".zip"))
        system(
            std::string("unzip -d ")
            .append(destdir)
            .append(" -q -o ")
            .append(filename)
//               .append(" 2>&-")
            .c_str()
        );

    return true;
}

bool isFileExist(const char *filename)
{
    return !access(filename, F_OK);
}
