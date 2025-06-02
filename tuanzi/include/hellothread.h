#ifndef HELLOTHREAD_H_INCLUDED
#define HELLOTHREAD_H_INCLUDED

#include "lnxthread.h"

class CHelloThread : public CLnxThread
{
    public:
        CHelloThread();
        ~CHelloThread() override;

    protected:

    private:
};

#endif // HELLOTHREAD_H_INCLUDED
