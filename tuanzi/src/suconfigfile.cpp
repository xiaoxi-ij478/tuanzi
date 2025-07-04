#include "all.h"
#include "global.h"
#include "util.h"
#include "compressor.h"
#include "suconfigfile.h"

CRITICAL_SECTION CSuConfigFile::CSuConfigFileLock;

CSuConfigFile::CSuConfigFile() : config_dirty(), is_open(), cfg_filename()
{}

CSuConfigFile::~CSuConfigFile()
{
    Unlock();
}

void CSuConfigFile::Lock()
{
    EnterCriticalSection(&CSuConfigFileLock);
}

void CSuConfigFile::Unlock()
{
    LeaveCriticalSection(&CSuConfigFileLock);
}

void CSuConfigFile::Close()
{
    if (!is_open)
        return;

    if (config_dirty && !UpdateConfig())
        return;

    DeleteTempConfig();
    is_open = false;
}

bool CSuConfigFile::Open()
{
    std::string apppath;
    TakeAppPath(apppath);
    cfg_filename = apppath.append("SuConfig.dat");
    return Open(cfg_filename.c_str());
}

void CSuConfigFile::AppendComma(std::string &str)
{
    assert(str.length());
    unsigned long slen = str.length();
    const char *sym = nullptr;

    if (slen <= 1)
        return;

    if (str[0] == '\'' && str[slen - 1] == '\'')
        sym = "'";

    else if (str[0] == '"' && str[slen - 1] == '"')
        sym = "\"";

    else
        return;

    str.insert(0, sym).append(sym);
}

void CSuConfigFile::DeleteFile(const std::string &filename)
{
    // the original implementation used shell
    // "rm -rf $filename"
    // but it is used only to delete file
    remove(filename.c_str());
}

void CSuConfigFile::DeleteTempConfig()
{
    std::string path;
    TakeAppPath(path);
    path.append("SuTempConfig.dat");
    DeleteFile(path);
}

void CSuConfigFile::EnablePrivilege(const char *, bool)
{}

unsigned CSuConfigFile::GetPrivateProfileInt(
    const char *domain,
    const char *key,
    unsigned defval
)
{
    unsigned ret = 0;
    dictionary *ini = nullptr;
    std::string cfgpath;
    std::string dkey(domain);
    dkey.append(":").append(key);
    TakeAppPath(cfgpath);
    cfgpath.append("SuTempConfig.dat");

    if (!(ini = iniparser_load(cfgpath.c_str()))) {
        g_logSystem.AppendText("ini create[path=%s]failed", cfgpath.c_str());
        return defval;
    }

    ret = iniparser_getint(ini, dkey.c_str(), defval);
    iniparser_freedict(ini);
    return ret;
}

void CSuConfigFile::GetPrivateProfileString(
    const char *domain,
    const char *key,
    const char *defval,
    std::string &dst
)
{
    dictionary *ini = nullptr;
    std::string cfgpath;
    std::string dkey(domain);
    dkey.append(":").append(key);
    TakeAppPath(cfgpath);
    cfgpath.append("SuTempConfig.dat");

    if (!(ini = iniparser_load(cfgpath.c_str()))) {
        g_logSystem.AppendText("ini create[path=%s]failed", cfgpath.c_str());
        dst = defval;
        return;
    }

    dst = iniparser_getstring(ini, dkey.c_str(), defval);
    iniparser_freedict(ini);
    ProfileStringToString(dst);
}

void CSuConfigFile::GetSysUPTime(unsigned &, unsigned &)
{}

void CSuConfigFile::LogToFile([[maybe_unused]] const char *str)
{}

bool CSuConfigFile::Open(const char *rfilename)
{
    std::ifstream ifs(rfilename);
    std::string filename;
    TakeAppPath(filename);
    filename.append("SuTempConfig.dat");
    std::ofstream ofs(filename);
    char *ibuf = nullptr;
    char *obuf = nullptr;
    unsigned long orig_size = 0;
    unsigned long decompress_size = 0;
    cfg_filename = rfilename;

    if (!ifs) {
        g_logSystem.AppendText("ERROR: Open file %s failed.\n", rfilename);
        return false;
    }

    ofs.open(filename);

    if (!ofs) {
        g_logSystem.AppendText("ERROR: Open file %s failed.\n", filename.c_str());
        delete[] obuf;
        obuf = nullptr;
        return false;
    }

    ifs.seekg(0, std::ios::end);
    orig_size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    ibuf = new char[orig_size + 1];

    for (unsigned i = 0; !ifs.eof(); i++)
        ibuf[i] = ~ifs.get();

    ifs.close();
    decompress_size = Decompress(ibuf, obuf, orig_size, 0);
    obuf = new char[decompress_size];
    Decompress(ibuf, obuf, orig_size, decompress_size);

    if (!ofs.write(obuf, decompress_size)) {
        g_logSystem.AppendText("ERROR: write file %s failed.\n", filename.c_str());
        delete[] obuf;
        obuf = nullptr;
        return false;
    }

    is_open = true;
    delete[] ibuf;
    delete[] obuf;
    ibuf = obuf = nullptr;
    return true;
}

static void ConvertEnternewlineToOnetwo(std::string &str)
{
    replace_all_distinct(str, "\x01\x02", "\r\n");
}

void CSuConfigFile::ProfileStringToString(std::string &str)
{
    replace_all_distinct(str, "\x01\x02", "\r\n");
}

void CSuConfigFile::StringToProfileString(std::string &str)
{
    ConvertEnternewlineToOnetwo(str);
    AppendComma(str);
}

bool CSuConfigFile::UpdateConfig()
{
    std::string filename;
    TakeAppPath(filename);
    filename.append("SuTempConfig.dat");
    std::ifstream ifs(filename);
    std::ofstream ofs(cfg_filename);
    unsigned long orig_size = 0;
    unsigned long comp_size = 0;
    char *ibuf = nullptr;
    char *obuf = nullptr;

    if (!ifs) {
        g_logSystem.AppendText("ERROR: Open file %s failed.\n", filename.c_str());
        return false;
    }

    if (!ofs) {
        g_logSystem.AppendText("ERROR: Open file %s failed.\n", cfg_filename.c_str());
        return false;
    }

    ifs.seekg(0, std::ios::end);
    orig_size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    ibuf = new char[orig_size + 1];
    ifs.read(ibuf, orig_size);
    ifs.close();
    comp_size = Compress(ibuf, obuf, orig_size, 0);
    obuf = new char[comp_size];
    Compress(ibuf, obuf, orig_size, comp_size);
    // *INDENT-OFF*
    std::for_each(obuf, obuf + comp_size + 1, [](char &i) { i = ~i; });
    // *INDENT-ON*

    if (!ofs.write(obuf, comp_size)) {
        g_logSystem.AppendText("ERROR: write file %s failed.\n", cfg_filename.c_str());
        delete[] obuf;
        obuf = nullptr;
        return false;
    }

    delete[] ibuf;
    delete[] obuf;
    ibuf = obuf = nullptr;
    config_dirty = false;
    return true;
}

bool CSuConfigFile::WritePrivateProfileString(
    const char *domain,
    const char *key,
    const char *val
) const
{
    assert(is_open);
    dictionary *ini = nullptr;
    std::string cfgpath;
    std::string dkey(domain);
    std::string tmpval(val);
    FILE *fp = nullptr;
    dkey.append(":").append(key);
    TakeAppPath(cfgpath);
    cfgpath.append("SuTempConfig.dat");

    if (!(ini = iniparser_load(cfgpath.c_str()))) {
        g_logSystem.AppendText("ini create[path=%s]failed", cfgpath.c_str());
        return false;
    }

    StringToProfileString(tmpval);

    if (iniparser_set(ini, dkey.c_str(), tmpval.c_str()) == -1)
        return false;

    if ((fp = fopen(cfgpath.c_str(), "w"))) {
        iniparser_dump_ini(ini, fp);
        fclose(fp);
    }

    iniparser_freedict(ini);
    return true;
}
