#ifndef FILEUTIL_H_INCLUDED
#define FILEUTIL_H_INCLUDED

extern unsigned long get_file_size(const char *filename);
extern void DeleteFile(const char *filename);
extern int cmd_mkdir(const char *dirname);
extern void removeFileOrDir(const char *filename);
extern int get_sh_name(const char *src, const char *dst);
extern bool decompressFile(const char *filename, const char *destdir);
extern bool isFileExist(const char *filename);

#endif // FILEUTIL_H_INCLUDED
