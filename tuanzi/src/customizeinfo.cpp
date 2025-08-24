#include "all.h"
#include "encodeutil.h"
#include "xmlparser.h"
#include "customizeinfo.h"

CCustomizeInfo::CCustomizeInfo() :
    create_time(),
    chs(),
    eng(),
    ip(),
    port(80),
    main_icon(),
    success_icon(),
    failed_ico(),
    disable_ico()
{}

CCustomizeInfo::~CCustomizeInfo()
{}

int CCustomizeInfo::Load(const char *str)
{
    XML_PARSER parser;
    return parser.Load_XML_String(str) ? Load(parser) : 4;
}

std::string CCustomizeInfo::GetAttribute(
    const XML_PARSER &parser,
    const char *name
) const
{
    std::string obuf;
    std::string attr;
    parser.GetAttributeValue(name, attr);
    ConvertUtf8ToGBK(attr.c_str(), attr.length(), obuf);
    return obuf;
}

std::string CCustomizeInfo::GetText(const XML_PARSER &parser) const
{
    std::string obuf;
    const char *attr = parser.Get_Text();
    ConvertUtf8ToGBK(attr, strlen(attr), obuf);
    return obuf;
}

int CCustomizeInfo::GetTextInt(const XML_PARSER &parser) const
{
    return std::stoi(GetText(parser), nullptr, 10);
}

int CCustomizeInfo::Load(XML_PARSER &parser)
{
    if (!parser.Go_to_Child("custom_info"))
        return 5;

    create_time = GetAttribute(parser, "create_time");

    if (!parser.Go_to_Child("soft_caption"))
        return 6;

    chs = GetAttribute(parser, "chs");
    eng = GetAttribute(parser, "eng");
    parser.Go_to_Parent("custom_info");

    if (parser.Go_to_Child("icon")) {
        main_icon = GetAttribute(parser, "main_icon");
        success_icon = GetAttribute(parser, "success_icon");
        failed_ico = GetAttribute(parser, "failed_ico");
        disable_ico = GetAttribute(parser, "disable_ico");
        parser.Go_to_Parent("custom_info");
    }

    if (parser.Go_to_Child("manager_center")) {
        ip = GetAttribute(parser, "ip");
        port = parser.GetAttributeValueInt("port");
        parser.Go_to_Parent("custom_info");
        return 0;
    }

    return 0;
}
