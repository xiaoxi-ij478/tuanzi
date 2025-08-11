#include "all.h"
#include "xmlparser.h"

XML_PARSER::XML_PARSER() : TiXmlDocument(), currentElement()
{}

bool XML_PARSER::Add_ElementAfter(const char *name)
{
    TiXmlElement el(name);
    TiXmlNode *newNode = nullptr;

    if (!currentElement)
        return true;

    if (!currentElement->Parent())
        return true;

    if (!(newNode = currentElement->Parent()->InsertAfterChild(currentElement, el)))
        return true;

    currentElement = newNode;
    return true;
}

bool XML_PARSER::Add_ElementBefore(const char *name)
{
    TiXmlElement el(name);
    TiXmlNode *newNode = nullptr;

    if (!currentElement)
        return true;

    if (!currentElement->Parent())
        return true;

    if (
        !(
            newNode =
                currentElement->Parent()->InsertBeforeChild(currentElement, el)
        )
    )
        return true;

    currentElement = newNode;
    return true;
}

bool XML_PARSER::Add_FirstChildElement(const char *name)
{
    TiXmlElement el(name);
    TiXmlNode *newNode = nullptr;

    if (!currentElement)
        return true;

    if (currentElement->FirstChildElement()) {
        if (
            !(
                newNode =
                    currentElement->InsertBeforeChild(
                        currentElement->FirstChildElement(),
                        el
                    )
            )
        )
            return true;

    } else if (!(newNode = currentElement->InsertEndChild(el)))
        return true;

    currentElement = newNode;
    return true;
}

bool XML_PARSER::Add_LastChildElement(const char *name)
{
    TiXmlElement el(name);
    TiXmlNode *newNode = nullptr;

    if (!currentElement)
        return true;

    if (!(newNode = currentElement->InsertEndChild(el)))
        return true;

    currentElement = newNode;
    return true;
}

std::string XML_PARSER::GetAttributeValue(const char *name) const
{
    TiXmlElement *el = nullptr;
    const char *attr = nullptr;

    if (!currentElement)
        return std::string();

    if (!(el = currentElement->ToElement()))
        return std::string();

    if (!(attr = el->Attribute(name)))
        return std::string();

    return attr;
}

bool XML_PARSER::GetAttributeValue(const char *name, std::string& result) const
{
    TiXmlElement *el = nullptr;
    const char *attr = nullptr;
    result.clear();

    if (!currentElement)
        return false;

    if (!(el = currentElement->ToElement()))
        return false;

    if (!(attr = el->Attribute(name)))
        return false;

    result = attr;
    return true;
}

int XML_PARSER::GetAttributeValueInt(const char *name) const
{
    TiXmlElement *el = nullptr;
    int attr = 0;

    if (!currentElement)
        return 0;

    if (!(el = currentElement->ToElement()))
        return 0;

    el->QueryIntAttribute(name, &attr);
    return attr;
}

bool XML_PARSER::GetAttributeValueInt(const char *name, int& result) const
{
    TiXmlElement *el = nullptr;
    result = 0;

    if (!currentElement)
        return false;

    if (!(el = currentElement->ToElement()))
        return false;

    return el->QueryIntAttribute(name, &result);
}

const char *XML_PARSER::Get_Text() const
{
    TiXmlElement *el = nullptr;
    const char *attr = nullptr;

    if (!currentElement)
        return nullptr;

    if (!(el = currentElement->ToElement()))
        return nullptr;

    if (!(attr = el->GetText()))
        return nullptr;

    return attr;
}

bool XML_PARSER::Get_XML(std::string& result) const
{
    TiXmlPrinter printer;
    printer.SetIndent(nullptr);
    printer.SetLineBreak(nullptr);

    if (!Accept(&printer))
        return false;

    result = printer.Str();
    return true;
}

bool XML_PARSER::Go_to_Child(const char *name)
{
    TiXmlNode *childNode = nullptr;

    if (!currentElement)
        return false;

    if (!(childNode = currentElement->FirstChildElement(name)))
        return false;

    currentElement = childNode;
    return true;
}

bool XML_PARSER::Go_to_NextSibling(const char *name)
{
    TiXmlNode *siblingNode = nullptr;

    if (!currentElement)
        return false;

    if (!(siblingNode = currentElement->NextSiblingElement(name)))
        return false;

    currentElement = siblingNode;
    return true;
}

bool XML_PARSER::Go_to_Parent(const char *name)
{
    TiXmlNode *parentNode = nullptr;

    if (!(parentNode = currentElement))
        return false;

    do
        parentNode = parentNode->Parent();

    while (parentNode && strcmp(name, parentNode->Value()));

    currentElement = parentNode;
    return true;
}

bool XML_PARSER::Go_to_PrevSibling(const char *name)
{
    TiXmlNode *prevSiblingNode = nullptr;

    if (!currentElement)
        return false;

    do
        prevSiblingNode = currentElement->PreviousSibling(name);

    while (prevSiblingNode && prevSiblingNode->Type() != TINYXML_ELEMENT);

    currentElement = prevSiblingNode;
    return true;
}

TiXmlNode *XML_PARSER::Go_to_Root()
{
    return currentElement = FirstChildElement();
}

bool XML_PARSER::Is_Having_Attribute(const char *name) const
{
    TiXmlElement *el = nullptr;

    if (!currentElement)
        return false;

    if (!(el = currentElement->ToElement()))
        return false;

    return el->Attribute(name);
}

bool XML_PARSER::Load_XML_Document(const char *filename)
{
    bool status = LoadFile(filename);
    currentElement = status ? FirstChildElement() : nullptr;
    return status;
//    LoadFile(filename);
//    return !!(currentElement = FirstChildElement());
}

bool XML_PARSER::Load_XML_String(const char *str)
{
    Parse(str, nullptr, TIXML_DEFAULT_ENCODING);
    return !!(currentElement = FirstChildElement());
}

bool XML_PARSER::RemoveChildElements(const char *name) const
{
    TiXmlNode *childElement = nullptr;

    if (currentElement)
        return false;

    while ((childElement = currentElement->FirstChildElement(name)))
        if (!currentElement->RemoveChild(childElement))
            return false;

    return true;
}

bool XML_PARSER::RemoveFirstChildElement(const char *name) const
{
    TiXmlNode *childElement = nullptr;

    if (!currentElement)
        return false;

    if (!(childElement = currentElement->FirstChildElement(name)))
        return true; /* ??? */

    return currentElement->RemoveChild(childElement);
}

bool XML_PARSER::Remove_Attribute(const char *name) const
{
    TiXmlElement *el = nullptr;

    if (!currentElement)
        return false;

    if (!(el = currentElement->ToElement()))
        return false;

    el->RemoveAttribute(name);
    return true;
}

bool XML_PARSER::Save_XML_Document(const char *filename) const
{
    return SaveFile(filename);
}

bool XML_PARSER::Set_Attribute(const char *name, const char *value) const
{
    TiXmlElement *el = nullptr;

    if (!currentElement)
        return false;

    if (!(el = currentElement->ToElement()))
        return false;

    el->SetAttribute(name, value);
    return true;
}

bool XML_PARSER::Set_Text(const char *str) const
{
    TiXmlText textnode(str);
    TiXmlElement *el = nullptr;

    if (!currentElement)
        return true;

    if (!(el = currentElement->ToElement()))
        return true;

    for (
        TiXmlNode *i = el->FirstChild(), *next_l = i->NextSibling();
        i;
        i = next_l, next_l = i ? nullptr : i->NextSibling()
    ) {
        if (i->Type() != TINYXML_TEXT)
            continue;

        if (!el->RemoveChild(i))
            return true;

        if (!next_l) {
            el->InsertEndChild(textnode);
            return true;
        }
    }

    el->InsertEndChild(textnode);
    return true;
}
