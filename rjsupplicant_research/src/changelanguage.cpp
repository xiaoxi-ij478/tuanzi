#include "global.h"
#include "util.h"
#include "changelanguage.h"

std::string CChangeLanguage::translate_filename;

CChangeLanguage::CChangeLanguage() :
    lang_inited(), language_id(LANG_INVALID), trans_strings()
{}

CChangeLanguage &CChangeLanguage::Instance()
{
    static CChangeLanguage sLang;
    return sLang;
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
    std::string trans_id_str, trans_str;
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
        ParseString(line, '=', val);
        trans_id_str = std::move(val[0]);
        trans_str = std::move(val[1]);

        while (trans_str.front() == ' ' || trans_str.front() == '\n' ||
                trans_str.front() == '\r')
            trans_str.erase(trans_str.begin());

        while (trans_str.back() == ' ' || trans_str.back() == '\n' ||
                trans_str.back() == '\r')
            trans_str.pop_back();

        replace_all_distinct(trans_str, "\\r", "\r");
        replace_all_distinct(trans_str, "\\n", "\n");
//        tagSectionUnit tsu;
//        tsu.id = std::stoi(trans_id_str);
//        tsu.str = trans_str;
//        trans_strings.push_back(tsu);
        trans_strings.emplace_back(std::stoi(trans_id_str), trans_str);
    }

    translate_file.close();

    if (!translate_file)
        return false;

    return true;
}

const std::string &CChangeLanguage::LoadString(unsigned str_id) const
{
    static std::string none;

    if (!lang_inited)
        return none;

    for (const tagSectionUnit &trans_str : trans_strings)
        if (trans_str.id == str_id)
            return trans_str.str;

    return none;
}

bool CChangeLanguage::SetLanguage(enum LANG lang_id)
{
    bool result;

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
