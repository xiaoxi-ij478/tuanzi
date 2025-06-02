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
        void SetStateData(CStateData *data);

    private:
        unsigned long field_8;
        unsigned long field_10;
        CStateData *data;

};

#endif // STATEVISUAL_H_INCLUDED
