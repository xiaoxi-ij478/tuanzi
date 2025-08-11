#include "all.h"
#include "global.h"
#include "util.h"
#include "changelanguage.h"

CChangeLanguage::CChangeLanguage() :
    lang_inited(),
    translate_filename(),
    language_id(LANG_INVALID),
    trans_strings()
{}

CChangeLanguage& CChangeLanguage::Instance()
{
    static CChangeLanguage lang; // sLang
    return lang;
}

void CChangeLanguage::CleanLanguage()
{
    trans_strings.clear();
    lang_inited = false;
}

enum LANG CChangeLanguage::GetLanguage() const
{
    return language_id;
}

bool CChangeLanguage::InitLanguage()
{
    std::ifstream translate_file(translate_filename);
    std::string line;
    std::vector<std::string> val;
//    size_t equal_pos;

    if (!translate_file && !lang_inited)
        return false;

    CleanLanguage();
    lang_inited = true;

    while (std::getline(translate_file, line)) {
        if (line.empty() || line.front() == '#')
            continue; // skip empty or comment line

        // use our enhanced version of split
//        equal_pos = s.find('=');
//
//        if (equal_pos == std::string::npos)
//            continue; // skip invalid line
//
//        trans_id_str = s.substr(0, equal_pos);
//        trans_str = s.substr(equal_pos + 1);
        ParseString(line, '=', val, 1);

        if (val.size() == 1)
            continue;

        std::string& trans_str = val[1];
        TrimAll(trans_str, " \r\n");
        replace_all_distinct(trans_str, "\\r", "\r");
        replace_all_distinct(trans_str, "\\n", "\n");
        trans_strings[std::stoi(val[0])] = trans_str;
    }

    translate_file.close();
    return true;
}

const std::string& CChangeLanguage::LoadString(unsigned str_id) const
{
    static std::string none;

    if (!lang_inited)
        return none;

    return trans_strings.at(str_id);
}

bool CChangeLanguage::SetLanguage(enum LANG lang_id)
{
    bool result = false;

    if (language_id == lang_id && lang_inited)
        return true;

    translate_filename =
        g_strAppPath + (
            lang_id == LANG_CHINESE ? "uiCHS.ini" : "uiENG.ini"
        );

    if ((result = InitLanguage()))
        language_id = lang_id;

    return result;
}
