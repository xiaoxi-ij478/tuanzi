#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <string>

#include "tinyxml.h"

class XML_PARSER : public TiXmlDocument
{
    public:
        XML_PARSER();
        virtual ~XML_PARSER();
        bool Add_ElementAfter(const char *name);
        bool Add_ElementBefore(const char *name);
        bool Add_FirstChildElement(const char *name);
        bool Add_LastChildElement(const char *name);
        std::string GetAttributeValue(const char *name);
        bool GetAttributeValue(const char *name, std::string &result);
        int GetAttributeValueInt(const char *name);
        bool GetAttributeValueInt(const char *name, int &result);
        const char *Get_Text();
        bool Get_XML(std::string &result);
        bool Go_to_Child(const char *name);
        bool Go_to_NextSibling(const char *name);
        bool Go_to_Parent(const char *name);
        bool Go_to_PrevSibling(const char *name);
        TiXmlNode *Go_to_Root();
        bool Is_Having_Attribute(const char *name);
        bool Load_XML_Document(const char *filename);
        bool Load_XML_String(const char *str);
        bool RemoveChildElements(const char *name);
        bool RemoveFirstChildElement(const char *name);
        bool Remove_Attribute(const char *name);
        bool Save_XML_Document(const char *filename);
        bool Set_Attribute(const char *name, const char *value);
        bool Set_Text(const char *str);

    protected:

    private:
        TiXmlNode *currentElement;
};

#endif // XMLPARSER_H
