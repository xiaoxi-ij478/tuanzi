#ifndef STATEVISUAL_H_INCLUDED
#define STATEVISUAL_H_INCLUDED

#include "statedata.h"

class CStateVisual
{
    public:
        CStateVisual();
        virtual ~CStateVisual();

        virtual void Initlize();
        virtual void MoveState();
        virtual void SetStateData(CStateData *data);

    protected:

    private:

};

#endif // STATEVISUAL_H_INCLUDED
