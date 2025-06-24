#ifndef MSGUTIL_H_INCLUDED
#define MSGUTIL_H_INCLUDED

struct tagMsgItem {
    tagMsgItem() = default;
    tagMsgItem(
        int ntype,
        const std::string &msgtime,
        const std::string &msg
    ) : ntype(ntype), msgtime(msgtime), msg(msg)
    {}
    int ntype;
    std::string msgtime;
    std::string msg;
};

extern void CreateNewMsgFile();
extern void AddMsgItem(int type, const std::string &msg);
extern void DelMsgItem(int type, const std::string &msg);
extern int GetMsgArray(std::vector<struct tagMsgItem> &msgarr);
extern int GetMsgArray_Ex(
    std::vector<struct tagMsgItem> &msgarr,
    bool replace_crlf
);
extern const std::string &GetMessageType(int type);
extern void print_msg_item(tagMsgItem *item);
extern void print_msg_item_header();

#endif // MSGUTIL_H_INCLUDED
