#ifndef MSGUTIL_H_INCLUDED
#define MSGUTIL_H_INCLUDED

struct tagMsgItem {
    tagMsgItem(
        int ntype = 0,
        const std::string &msgtime = "",
        const std::string &msg = ""
    ) : ntype(ntype), msgtime(msgtime), msg(msg)
    {}
    int ntype;
    std::string msgtime;
    std::string msg;
};

void CreateNewMsgFile();
void AddMsgItem(int type, const std::string &msg);
void DelMsgItem(int type, const std::string &msg);
int GetMsgArray(std::vector<struct tagMsgItem> &msgarr);
int GetMsgArray_Ex(std::vector<struct tagMsgItem> &msgarr, bool replace_crlf);
const std::string &GetMessageType(int type);
void print_msg_item(tagMsgItem *item);
void print_msg_item_header();

#endif // MSGUTIL_H_INCLUDED
