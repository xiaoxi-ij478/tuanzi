#ifndef CUSTOMIZEINFO_H_INCLUDED
#define CUSTOMIZEINFO_H_INCLUDED

#include "xmlparser.h"

class CCustomizeInfo
{
    public:
        CCustomizeInfo();
        ~CCustomizeInfo();

        int Load(const char *str);

        std::string create_time;
        std::string chs;
        std::string eng;
        std::string ip;
        unsigned port;
        std::string main_icon;
        std::string success_icon;
        std::string failed_ico;
        std::string disable_ico;

    private:
        std::string GetAttribute(const XML_PARSER &parser, const char *name) const;
        std::string GetText(const XML_PARSER &parser) const;
        int GetTextInt(const XML_PARSER &parser) const;
        int Load(XML_PARSER &parser);
};

#endif // CUSTOMIZEINFO_H_INCLUDED
