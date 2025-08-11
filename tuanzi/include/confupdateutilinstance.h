#ifndef CONFUPDATEUTILINSTANCE_H_INCLUDED
#define CONFUPDATEUTILINSTANCE_H_INCLUDED

class CCustomizeInfo;
class CSuConfigFile;

#define UPDATED 0x2000

class IConfUpdateEventReceiver
{
    public:
        virtual void callback(unsigned arg1, unsigned arg2) = 0;
};

class CConfUpdateUtilInstance
{
    public:
        void Attach(IConfUpdateEventReceiver *event_receiver);
        void Detach(IConfUpdateEventReceiver *event_receiver);

        unsigned UpdateConfigure(const char *xml);
        void UpdateManagerCenterConf(
            CCustomizeInfo *custom_info,
            CSuConfigFile& conffile
        );

        static std::string AppendNum(const char *str, int num);
        static CConfUpdateUtilInstance& Instance();
        static std::string IntToStr(int i);
        static std::string MemoryToHexString(const char *buf, unsigned buflen);

    private:
        std::vector<IConfUpdateEventReceiver *> event_receivers;
        unsigned someflag;
        unsigned someflag2;
};

#endif // CONFUPDATEUTILINSTANCE_H_INCLUDED
