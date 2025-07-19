#ifndef CHANGELANGUAGE_H_INCLUDED
#define CHANGELANGUAGE_H_INCLUDED

#include "miscdefs.h"

struct tagSectionUnit {
    tagSectionUnit(unsigned id, const std::string &str) : id(id), str(str)
    {}
    unsigned id;
    std::string str;
};

class CChangeLanguage
{
    public:
        void CleanLanguage();
        enum LANG GetLanguage() const;
        bool InitLanguage();
        const std::string &LoadString(unsigned str_id) const;
        bool SetLanguage(enum LANG lang_id);

        static CChangeLanguage &Instance();

    private:
        CChangeLanguage();

        bool lang_inited;
        std::string translate_filename;
        enum LANG language_id;
        std::vector<tagSectionUnit> trans_strings;
};

#endif // CHANGELANGUAGE_H_INCLUDED
