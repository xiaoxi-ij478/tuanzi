#ifndef HELLOTHREAD_H_INCLUDED
#define HELLOTHREAD_H_INCLUDED

#include "lnxthread.h"

struct [[gnu::packed]] HelloPacket_somewhat {
    uint32_t field_0;
    uint32_t field_4;
    uint16_t field_8;
};

struct [[gnu::packed]] HelloPacket {
    struct ether_header ether_header;
    uint8_t code;
    uint8_t id;
    uint16_t length;
    uint8_t type;
    uint8_t data;
    struct HelloPacket_somewhat field_14;
    struct HelloPacket_somewhat field_1E;
    uint32_t field_28;
    uint8_t field_2C;
};

class CHelloThread : public CLnxThread
{
    public:
        CHelloThread();

        int msgid;
        unsigned hello_timer_interval;
        unsigned hello_para;

    protected:
        void DispathMessage(struct LNXMSG *msg) override;
        bool InitInstance() override;
        void OnTimer(int tflag) const override;

    private:
        struct HelloPacket *CreateHelloPacket(unsigned &packet_len);
        void CreateHelloTimer(unsigned interval);
        unsigned GetCurCRCID();
        DECLARE_DISPATH_MESSAGE_HANDLER(OnChangeHelloPara);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnHelloTimer);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnSetHelloTimerPlease);

        timer_t hello_timerid;
        unsigned field_1E8;
        unsigned hello_id_offset;
        unsigned field_1F0;
};

#endif // HELLOTHREAD_H_INCLUDED
