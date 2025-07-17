#include "all.h"
#include "global.h"
#include "confupdateutilinstance.h"

CConfUpdateUtilInstance::CConfUpdateUtilInstance() :
    event_receivers(),
    someflag(),
    someflag2()
{}

CConfUpdateUtilInstance::~CConfUpdateUtilInstance()
{}

void CConfUpdateUtilInstance::Attach(IConfUpdateEventReceiver *event_receiver)
{
    event_receivers.push_back(event_receiver);
}

void CConfUpdateUtilInstance::Detach(IConfUpdateEventReceiver *event_receiver)
{
    auto p = std::find(
                 event_receivers.cbegin(),
                 event_receivers.cend(),
                 event_receiver
             );

    if (p != event_receivers.cend())
        event_receivers.erase(p);
}

unsigned CConfUpdateUtilInstance::UpdateConfigure(const char *xml)
{
    CCustomizeInfo custom_info;
    CSuConfigFile conffile;
    unsigned ret = 0;

    if ((ret = custom_info.Load(xml))) {
        g_logContextControl.AppendText(
            "Failed to load customize content, error=%d .",
            ret
        );
        return 3;
    }

    if (!conffile.Open())
        return 4;

    someflag = 0;
    UpdateManagerCenterConf(&custom_info, conffile);
    conffile.close();

    if (someflag)
        for (IConfUpdateEventReceiver *event_receiver : event_receivers)
            event_receiver->callback(someflag, 0);

    return 0;
}

void CConfUpdateUtilInstance::UpdateManagerCenterConf(
    CCustomizeInfo *custom_info,
    CSuConfigFile &conffile
)
{
    if (
        (
            custom_info->ip.empty() ||
            CtrlThread->configure_info.client_manager_center_serveraddr ==
            custom_info->ip
        ) &&
        CtrlThread->configure_info.client_manager_center_serverlistenport ==
        custom_info->port
    )
        return;

    CtrlThread->configure_info.client_manager_center_serveraddr = custom_info->ip;
    CtrlThread->configure_info.client_manager_center_serverlistenport =
        custom_info->port;
    conffile.WritePrivateProfileString(
        "CLIENT_MANAGER_CENTER",
        "IsSupport",
        "1"
    );
    conffile.WritePrivateProfileString(
        "CLIENT_MANAGER_CENTER",
        "ServerAddr",
        custom_info->ip.c_str()
    );
    conffile.WritePrivateProfileString(
        "CLIENT_MANAGER_CENTER",
        "ServerListenPort",
        IntToStr(custom_info->port).c_str()
    );
    someflag |= UPDATED;
}

std::string CConfUpdateUtilInstance::AppendNum(const char *str, int num)
{
    return std::string(str).append(std::to_string(num));
}

CConfUpdateUtilInstance &CConfUpdateUtilInstance::Instance()
{
    static CConfUpdateUtilInstance obj; // thisObj
    return obj;
}

std::string CConfUpdateUtilInstance::IntToStr(int i)
{
    return std::to_string(i);
}

std::string CConfUpdateUtilInstance::MemoryToHexString(
    const char *buf,
    unsigned buflen
)
{
    std::ostringstream oss;
    oss << std::hex << std::setw(2) << std::setfill('0');

    while (buflen--)
        oss << static_cast<unsigned>(*buf++);

    return oss.str();
}
