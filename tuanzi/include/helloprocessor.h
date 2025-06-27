#ifndef HELLOPROCESSOR_H_INCLUDED
#define HELLOPROCESSOR_H_INCLUDED

#include "hellothread.h"

class CHelloProcessor
{
    public:
        CHelloProcessor();
        virtual ~CHelloProcessor();

        void Exit();
        void ProcessorRun(
            int msgid,
            unsigned hello_timer_interval,
            unsigned hello_para
        );
        void ProcessorStop();

    private:
        CHelloThread *hello_thread;
};

#endif // HELLOPROCESSOR_H_INCLUDED
