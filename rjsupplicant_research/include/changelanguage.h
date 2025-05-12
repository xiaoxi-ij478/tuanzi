#ifndef CHANGELANGUAGE_H
#define CHANGELANGUAGE_H


enum LANG { LANG_INVALID, LANG_ENGLISH, LANG_CHINESE };

struct tagSectionUnit {
    tagSectionUnit(unsigned id, const std::string &str) :
        id(id), str(str)
    { }
    unsigned int id;
    std::string str;
};

class CChangeLanguage
{
    public:
        void CleanLanguage();
        enum LANG GetLanguage();
        bool InitLanguage();
        static CChangeLanguage &Instance();
        const std::string &LoadString(unsigned int str_id);
        bool SetLanguage(enum LANG lang_id);

    private:
        CChangeLanguage();

        bool lang_inited;
        enum LANG language_id;
        std::vector<tagSectionUnit> trans_strings;
        static std::string translate_filename;
};

#endif // CHANGELANGUAGE_H
