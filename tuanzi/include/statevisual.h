#ifndef STATEVISUAL_H_INCLUDED
#define STATEVISUAL_H_INCLUDED

#include "statedata.h"

class CStateVisual
{
    public:
        CStateVisual();
        virtual ~CStateVisual();

        virtual void Initlize() const;
        virtual void MoveState() const;

        void SetStateData(const CStateData *data);

    private:
        pthread_t thread_id;
        unsigned long field_10;
        CStateData *data;
};

#endif // STATEVISUAL_H_INCLUDED
