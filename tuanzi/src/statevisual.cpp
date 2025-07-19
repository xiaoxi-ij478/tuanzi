#include "all.h"
#include "statevisual.h"

CStateVisual::CStateVisual() : thread_id(), field_10(), state_data()
{}

CStateVisual::~CStateVisual()
{}

void CStateVisual::Initlize()
{}

void CStateVisual::MoveState()
{}

void CStateVisual::SetStateData(CStateData *new_data)
{
    state_data = new_data;
}
