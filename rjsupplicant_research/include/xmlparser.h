#ifndef XMLPARSER_H_INCLUDED
#define XMLPARSER_H_INCLUDED

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
        std::string GetAttributeValue(const char *name) const;
        bool GetAttributeValue(const char *name, std::string &result) const;
        int GetAttributeValueInt(const char *name) const;
        bool GetAttributeValueInt(const char *name, int &result) const;
        const char *Get_Text() const;
        bool Get_XML(std::string &result) const;
        bool Go_to_Child(const char *name);
        bool Go_to_NextSibling(const char *name);
        bool Go_to_Parent(const char *name);
        bool Go_to_PrevSibling(const char *name);
        TiXmlNode *Go_to_Root();
        bool Is_Having_Attribute(const char *name) const;
        bool Load_XML_Document(const char *filename);
        bool Load_XML_String(const char *str);
        bool RemoveChildElements(const char *name) const;
        bool RemoveFirstChildElement(const char *name) const;
        bool Remove_Attribute(const char *name) const;
        bool Save_XML_Document(const char *filename) const;
        bool Set_Attribute(const char *name, const char *value) const;
        bool Set_Text(const char *str) const;

    protected:

    private:
        TiXmlNode *currentElement;
};

#endif // XMLPARSER_H_INCLUDED
