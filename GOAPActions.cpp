#include "stdafx.h"
#include "GOAPActions.h"
#include "GOAPPlanner.h"
#include "IExamInterface.h"
#include "Blackboard.h"

bool GOAPAction::RequiresMovement() const
{
	return m_RequiresMovement;
}
