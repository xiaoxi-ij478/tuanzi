#ifndef CHANGELANGUAGE_H_INCLUDED
#define CHANGELANGUAGE_H_INCLUDED

enum LANG {
    LANG_INVALID,
    LANG_ENGLISH,
    LANG_CHINESE
};

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
        static CChangeLanguage &Instance();
        const std::string &LoadString(unsigned str_id) const;
        bool SetLanguage(enum LANG lang_id);

    private:
        CChangeLanguage();

        bool lang_inited;
        enum LANG language_id;
        std::vector<tagSectionUnit> trans_strings;
        static std::string translate_filename;
};

#endif // CHANGELANGUAGE_H_INCLUDED
