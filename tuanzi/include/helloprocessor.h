#ifndef HELLOPROCESSOR_H_INCLUDED
#define HELLOPROCESSOR_H_INCLUDED

#include "hellothread.h"

class CHelloProcessor
{
    public:
        CHelloProcessor();
        virtual ~CHelloProcessor();

        void Exit() const;
        void ProcessorRun(
            int msgid,
            unsigned hello_timer_interval,
            unsigned hello_para
        ) const;
        void ProcessorStop() const;

    private:
        CHelloThread *hello_thread;
};

#endif // HELLOPROCESSOR_H_INCLUDED
