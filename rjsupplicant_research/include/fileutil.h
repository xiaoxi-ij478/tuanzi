#ifndef FILEUTIL_H_INCLUDED
#define FILEUTIL_H_INCLUDED

unsigned long get_file_size(const char *filename);
void DeleteFile(const char *filename);
int cmd_mkdir(const char *dirname);
void removeFileOrDir(const char *filename);
int get_sh_name(const char *src, const char *dst);
bool decompressFile(const char *cfilename, const char *cdestdir);
bool isFileExist(const char *filename);

#endif // FILEUTIL_H_INCLUDED
