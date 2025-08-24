#ifndef MSGUTIL_H_INCLUDED
#define MSGUTIL_H_INCLUDED

extern void CreateNewMsgFile();
extern void AddMsgItem(unsigned type, const std::string &msg);
extern void DelMsgItem(unsigned type, const std::string &msg);
extern unsigned GetMsgArray(std::vector<struct tagMsgItem> &msgarr);
extern unsigned GetMsgArray_Ex(
    std::vector<struct tagMsgItem> &msgarr,
    bool replace_crlf
);
extern const std::string &GetMessageType(unsigned type);
extern void print_msg_item(const struct tagMsgItem &item);
extern void print_msg_item_header();
extern void ShowLocalMsg(const std::string &content, const std::string &header);

#endif // MSGUTIL_H_INCLUDED
