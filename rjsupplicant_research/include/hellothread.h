#ifndef HELLOTHREAD_H
#define HELLOTHREAD_H

#include "lnxthread.h"

class CHelloThread : public CLnxThread
{
    public:
        CHelloThread();
        virtual ~CHelloThread() override;

    protected:

    private:
};

#endif // HELLOTHREAD_H
