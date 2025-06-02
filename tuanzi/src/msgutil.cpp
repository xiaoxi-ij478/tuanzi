#include "changelanguage.h"
#include "criticalsection.h"
#include "global.h"
#include "util.h"
#include "timeutil.h"
#include "cmdutil.h"
#include "msgutil.h"

static CRITICAL_SECTION msg_write_lock;

void CreateNewMsgFile()
{
    std::string filename = g_strAppPath + "systemmsg.ini";
    std::ofstream ofs(filename, std::ios::trunc);

    if (!ofs)
        return;

    ofs << "[system]" << std::endl
        << "nmsg=0" << std::endl
        << "version=1.0";
    ofs.close();
}

void AddMsgItem(int type, const std::string &msg)
{
    std::string msgcopy = msg;
    std::string inifile = g_strAppPath + "systemmsg.ini";
    std::string inikey;
    std::vector<struct tagMsgItem> msgarr;
    dictionary *ini = nullptr;
    char time[512] = {};

    if (msg.empty())
        return;

    EnterCriticalSection(&msg_write_lock);
    replace_all_distinct(msgcopy, "\n", "\r\n");
    replace_all_distinct(msgcopy, "{$\\r\\n$}", "\n");
    GetMsgArray_Ex(msgarr, false);
    CreateNewMsgFile();

    if (!(ini = iniparser_load(inifile.c_str()))) {
        LeaveCriticalSection(&msg_write_lock);
        return;
    }

    for (auto it = msgarr.begin(); it != msgarr.end(); it++) {
        if (type != it->ntype || msg != it->msg)
            continue;

        msgarr.erase(it);
        break;
    }

    GetCurDataAndTime(time);
    msgarr.emplace_back(type, time, msg);

    for (int i = 0; i < msgarr.size(); i++) {
        inikey = "msg_" + std::to_string(i);
        iniparser_set(
            ini, (inikey + ":ntype").c_str(),
            std::to_string(msgarr[i].ntype).c_str()
        );
        iniparser_set(
            ini, (inikey + ":msgtime").c_str(),
            msgarr[i].msgtime.c_str()
        );
        iniparser_set(
            ini, (inikey + ":msg").c_str(),
            msgarr[i].msg.c_str()
        );
    }

    iniparser_set(ini, "system:nmsg", std::to_string(msgarr.size()).c_str());
    LeaveCriticalSection(&msg_write_lock);
}

void DelMsgItem(int type, const std::string &msg)
{
    std::string msgcopy = msg;
    std::string inifile = g_strAppPath + "systemmsg.ini";
    std::string inikey;
    std::vector<struct tagMsgItem> msgarr;
    dictionary *ini = nullptr;

    if (msg.empty())
        return;

    EnterCriticalSection(&msg_write_lock);
    replace_all_distinct(msgcopy, "\n", "\r\n");
    replace_all_distinct(msgcopy, "{$\\r\\n$}", "\n");
    GetMsgArray_Ex(msgarr, false);
    CreateNewMsgFile();

    if (!(ini = iniparser_load(inifile.c_str()))) {
        LeaveCriticalSection(&msg_write_lock);
        return;
    }

    for (auto it = msgarr.begin(); it != msgarr.end(); it++) {
        if (type != it->ntype || msg != it->msg)
            continue;

        msgarr.erase(it);
        break;
    }

    for (int i = 0; i < msgarr.size(); i++) {
        inikey = "msg_" + std::to_string(i);
        iniparser_set(
            ini,
            (inikey + ":ntype").c_str(),
            std::to_string(msgarr[i].ntype).c_str()
        );
        iniparser_set(
            ini,
            (inikey + ":msgtime").c_str(),
            msgarr[i].msgtime.c_str()
        );
        iniparser_set(
            ini,
            (inikey + ":msg").c_str(),
            msgarr[i].msg.c_str()
        );
    }

    iniparser_set(ini, "system:nmsg", std::to_string(msgarr.size()).c_str());
    LeaveCriticalSection(&msg_write_lock);
}

int GetMsgArray(std::vector<struct tagMsgItem> &msgarr)
{
    return GetMsgArray_Ex(msgarr, true);
}

int GetMsgArray_Ex(std::vector<struct tagMsgItem> &msgarr, bool replace_crlf)
{
    std::string inipath = g_strAppPath + "systemmsg.ini";
    std::string msgid;
    int ntype;
    std::string msgtime, msg;
    dictionary *ini = nullptr;
    int msgcnt = 0;

    if (!msgarr.empty())
        msgarr.clear();

    if (!(ini = iniparser_load(inipath.c_str()))) {
        CreateNewMsgFile();
        return 0;
    }

    if ((msgcnt = iniparser_getint(ini, "system:nmsg", 0)) <= 0) {
        iniparser_freedict(ini);
        return 1;
    }

    while (msgcnt--) {
        msgid = "msg_" + std::to_string(msgcnt);

        if (replace_crlf)
            replace_all_distinct(msg, "\r\n", "{$\\r\\n$}");

        ntype = iniparser_getint(ini, std::string(msgid + ":ntype").c_str(), -1);
        msgtime = iniparser_getstring(ini, std::string(msgid + ":msgtime").c_str(), "");
        msg = iniparser_getstring(ini, std::string(msgid + ":msg").c_str(), "");

        if (ntype == -1 || msgtime == "empty" || msg == "empty")
            break;

        msgarr.emplace_back(ntype, msgtime, msg);
    }

    CreateNewMsgFile();
    return 0;
}

const std::string &GetMessageType(int type)
{
    static std::string none;

    switch (type) {
        case 0:
            return CChangeLanguage::Instance().LoadString(77);

        case 1:
            return CChangeLanguage::Instance().LoadString(78);

        case 5:
            return CChangeLanguage::Instance().LoadString(92);

        default:
            return none;
    }
}

void print_msg_item(tagMsgItem *item)
{
    std::string s;
    s.append(item->msg).append("\t").append(GetMessageType(item->ntype));
    format_tc_string(get_tc_width(), 40, s);
    std::cout << std::endl;
}

void print_msg_item_header()
{
    std::cout << '\t' << CChangeLanguage::Instance().LoadString(2033)
              << "\t\t" << CChangeLanguage::Instance().LoadString(2034)
              << "\t\t" << CChangeLanguage::Instance().LoadString(2035)
              << std::endl;
}
