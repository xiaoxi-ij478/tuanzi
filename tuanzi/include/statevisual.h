#ifndef STATEVISUAL_H_INCLUDED
#define STATEVISUAL_H_INCLUDED

class CStateData;

class CStateVisual
{
    public:
        CStateVisual();
        virtual ~CStateVisual();

        virtual void Initlize();
        virtual void MoveState();

        void SetStateData(const CStateData *new_data);

        pthread_t thread_id;
        unsigned long field_10;
        CStateData *state_data;
};

#endif // STATEVISUAL_H_INCLUDED
