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
        virtual void SetStateData(CStateData *data) const;

    protected:

    private:

};

#endif // STATEVISUAL_H_INCLUDED
