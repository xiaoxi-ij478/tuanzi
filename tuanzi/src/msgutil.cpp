#include "all.h"
#include "changelanguage.h"
#include "criticalsection.h"
#include "global.h"
#include "util.h"
#include "timeutil.h"
#include "cmdutil.h"
#include "logfile.h"
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

void AddMsgItem(unsigned type, const std::string &msg)
{
    std::string msgcopy = msg;
    std::string inifile = g_strAppPath + "systemmsg.ini";
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

    for (auto it = msgarr.cbegin(); it != msgarr.cend(); it++)
        if (type == it->ntype && msg == it->msg) {
            msgarr.erase(it);
            break;
        }

    GetCurDataAndTime(time);
    msgarr.emplace_back(type, time, msg);

    for (unsigned i = 0; i < msgarr.size(); i++) {
        std::string inikey = "msg_" + std::to_string(i);
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

void DelMsgItem(unsigned type, const std::string &msg)
{
    std::string msgcopy = msg;
    std::string inifile = g_strAppPath + "systemmsg.ini";
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

    for (auto it = msgarr.cbegin(); it != msgarr.cend(); it++)
        if (type == it->ntype && msg == it->msg) {
            msgarr.erase(it);
            break;
        }

    for (unsigned i = 0; i < msgarr.size(); i++) {
        std::string inikey = "msg_" + std::to_string(i);
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

unsigned GetMsgArray(std::vector<struct tagMsgItem> &msgarr)
{
    return GetMsgArray_Ex(msgarr, true);
}

unsigned GetMsgArray_Ex(
    std::vector<struct tagMsgItem> &msgarr,
    bool replace_crlf
)
{
    std::string inipath = g_strAppPath + "systemmsg.ini";
    std::string msgtime, msg;
    int ntype;
    int msgcnt = 0;
    dictionary *ini = nullptr;

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
        std::string msgid = "msg_" + std::to_string(msgcnt);

        if (replace_crlf)
            replace_all_distinct(msg, "\r\n", "{$\\r\\n$}");

        ntype = iniparser_getint(ini, std::string(msgid + ":ntype").c_str(), -1);
        msgtime =
            iniparser_getstring(ini, std::string(msgid + ":msgtime").c_str(), "");
        msg = iniparser_getstring(ini, std::string(msgid + ":msg").c_str(), "");

        if (ntype == -1 || msgtime == "empty" || msg == "empty")
            break;

        msgarr.emplace_back(ntype, msgtime, msg);
    }

    CreateNewMsgFile();
    return 0;
}

const std::string &GetMessageType(unsigned type)
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

void print_msg_item(const struct tagMsgItem &item)
{
    std::string s(item.msg);
    s.append("\t").append(GetMessageType(item.ntype));
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

void ShowLocalMsg(const std::string &content, const std::string &header)
{
    char cur_date[64] = {};
    GetCurDataAndTime(cur_date);
    message_info(std::string(cur_date).append(header));
    format_tc_string(
        get_tc_width(),
        20,
        std::string("\n").append(content).append("\n")
    );
    CLogFile::LogToFile(
        (header + ' ' + content).c_str(),
        g_runLogFile.c_str(),
        true,
        true
    );
}
