#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "Blackboard.h"
#include "Agent.h"
#include "IExamInterface.h"

//SEEK (base>ISteeringBehavior)
SteeringPlugin_Output Seek::CalculateSteering(IExamInterface* pInterface, float deltaT, AgentInfo& agentInfo, Blackboard* pBlackboard)
{
	SteeringPlugin_Output steering{};

	steering.LinearVelocity = m_Target - agentInfo.Position; //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed

	return steering;
}
void Seek::SetTarget(const Elite::Vector2& target)
{
	ISteeringBehavior::SetTarget(target);
}
Elite::Vector2 Seek::GetTarget() const
{
	return m_Target;
}

// Dodge the enemy, keeping in mind the goal location
SteeringPlugin_Output SeekAndDodge::CalculateSteering(IExamInterface* pInterface, float deltaT, AgentInfo& agentInfo, Blackboard* pBlackboard)
{
	SteeringPlugin_Output steering{};

	// Data 
	Agent* pAgent = nullptr;
	WorldState* pWorldState = nullptr;
	Elite::Vector2* lastSeenEnemyPos{};

	// Check if agent data is valid
	bool dataValid = pBlackboard->GetData("Agent", pAgent)
		&& pBlackboard->GetData("LastEnemyPos", lastSeenEnemyPos)
		&& pBlackboard->GetData("WorldState", pWorldState);
	if (!dataValid) return steering;

	// Recalculate goal pos due to all the navmesh bugs
	if (m_NavMeshRefreshTimer > m_NavMeshRefreshTime)
	{
		std::cout << "Asking new route towards goal...\n";
		pAgent->SetGoalPosition(pInterface->NavMesh_GetClosestPathPoint(pAgent->GetDistantGoalPosition()));
		m_NavMeshRefreshTimer = 0.f;
	}
	m_NavMeshRefreshTimer += deltaT;

	bool enemyInSight = false;
	if (!pWorldState->GetState("EnemyInSight", enemyInSight))
	{
		std::cout << "Failed to get worldstate\n";
	}

	// Get the position that the agent wants to go in
	const Elite::Vector2& agentGoalPosition = pAgent->GetGoalPosition();
	const Elite::Vector2 agentToGoalVec{ agentGoalPosition - agentInfo.Position };
	Elite::Vector2 normal = agentToGoalVec;
	float distance = normal.Normalize();

	// Map orientation to a normal angle...
	float orientationAngle = agentInfo.Orientation * 180.f / float(M_PI);
	// If positive
	if (orientationAngle >= 0.f)
		orientationAngle -= 90.f;
	else
	{
		// If [-180, -90]
		if (orientationAngle <= -90.f)
		{
			orientationAngle = abs(orientationAngle);
			float temp = abs(orientationAngle - 180.f);
			orientationAngle = 90.f + temp;
		}
		// If ]-90, 0[
		else
		{
			orientationAngle = abs(orientationAngle);
			float temp = abs(orientationAngle - 90.f);
			orientationAngle = temp - 180.f;
		}
	}

	if (enemyInSight)
	{
		float* enemyCount = nullptr;
		pBlackboard->GetData("EnemyCount", enemyCount);
		float dodgeRange{ 10.f };
		if (*enemyCount > 1)
		{
			dodgeRange = 40.f;
		}

		// Get the angle towards the enemy
		float angleToEnemy = atan2(lastSeenEnemyPos->y - agentInfo.Position.y, lastSeenEnemyPos->x - agentInfo.Position.x);
		float angleToEnemyDeg = angleToEnemy * 180.f / float(M_PI);
		// If the enemy is in right front of us
		angleToEnemy = angleToEnemyDeg - orientationAngle;
		// Map angle to enemy to a values between -180 and 180
		if (angleToEnemyDeg > 180.f)
			angleToEnemyDeg -= 360.f;
		if (angleToEnemyDeg < -180.f)
			angleToEnemyDeg += 360.f;

		if (abs(angleToEnemy) < 20.f)
		{
			float sign = angleToEnemy / abs(angleToEnemy);
			angleToEnemy += 15.f * sign;
		}

		// Convert angle back to radians
		angleToEnemy = angleToEnemy * float(M_PI) / 180.f;
		orientationAngle = orientationAngle * float(M_PI) / 180.f;

		// Set a new goal position that dodges the enemy
		float halfPi = float(M_PI) / 2.f;
		Elite::Vector2 newGoal = agentInfo.Position + Elite::Vector2{ cos(orientationAngle + -angleToEnemy) * dodgeRange, sin(orientationAngle + -angleToEnemy) * dodgeRange };
		pAgent->SetGoalPosition(pInterface->NavMesh_GetClosestPathPoint(newGoal));
		steering.RunMode = true;
	}
	else
		steering.RunMode = false;

	// Move towards goal location, dodge is not required
	steering.LinearVelocity = pAgent->GetGoalPosition() - agentInfo.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;

	// Slow down when we get close to the goal
	if (distance < 3.f)
	{
		steering.LinearVelocity *= .5f;

		const Elite::Vector2& goalPos{ pAgent->GetGoalPosition() };
		float angleToGoal = atan2(goalPos.y - agentInfo.Position.y, goalPos.x - agentInfo.Position.x);
		float angleFromAgentToGoal = angleToGoal - orientationAngle;
		if (angleFromAgentToGoal > float(M_PI) / 4.f)
		{
			//std::cout << "SLOW DOWN! Angle: " << angleFromAgentToGoal << " \n";
			steering.LinearVelocity.Normalize();
			steering.LinearVelocity *= agentInfo.MaxLinearSpeed;
			steering.AngularVelocity = agentInfo.MaxAngularSpeed * (angleFromAgentToGoal / abs(angleFromAgentToGoal));
		}
	}
	else
	{
		// Use stamina when we have enough
		if (pInterface->Agent_GetInfo().Stamina > 9.f)
			steering.RunMode = true;
	}

	// Debug agent velocity
	pInterface->Draw_Direction(agentInfo.Position, agentToGoalVec, 5.f, Elite::Vector3{ 0.f,1.f,0.f });

	return steering;
}

//WANDER (base>ISteeringBehavior)
Wander::Wander() :
	m_DistanceFromActor{ 5.f },
	m_WanderRadius{ 5.f },
	m_RenewDistance{ 2.f }
{}
SteeringPlugin_Output Wander::CalculateSteering(IExamInterface* pInterface, float deltaT, AgentInfo& agentInfo, Blackboard* pBlackboard)
{
	SteeringPlugin_Output steering{};

	// Check where the agent is looking
	float rotation = agentInfo.Orientation;

	// Determine wander radius in front of actor
	// Scale the wander distance with the target movement speed for better wander behavior
	float finalDistanceFromActor = m_DistanceFromActor + agentInfo.AgentSize + (agentInfo.CurrentLinearSpeed / 2.f);
	const Elite::Vector2 wanderCenter
	{
		agentInfo.Position +
		Elite::Vector2(finalDistanceFromActor * (float)cos(rotation - M_PI / 2.f),
						finalDistanceFromActor * (float)sin(rotation - M_PI / 2.f))
	};


	// Scale the wander radius with the target movement speed for better wander behavior
	float finalWanderRadius = m_WanderRadius + agentInfo.AgentSize + agentInfo.MaxLinearSpeed / 2.f;
	// Agent is close enough to wanderTarget or the target has left the wander radius
	if ((agentInfo.Position.Distance(m_Target) < m_RenewDistance + agentInfo.AgentSize) ||
		(wanderCenter.Distance(m_Target) > (finalWanderRadius)))
	{
		// Create an offset from the center of the wanderradius
		int randomAngle{ Elite::randomInt(360) };
		Elite::Vector2 randomOffset{
			Elite::randomFloat(finalWanderRadius) * (float)sin(randomAngle),
			Elite::randomFloat(finalWanderRadius) * (float)cos(randomAngle)
		};

		// Set target position with the offset
		m_Target = wanderCenter + randomOffset;
	}

	// Seek
	steering.LinearVelocity = m_Target - agentInfo.Position; //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed=

	return steering;
}

SeekItem::SeekItem() :
	SeekAndDodge()
{}
SteeringPlugin_Output SeekItem::CalculateSteering(IExamInterface* pInterface, float deltaT, AgentInfo& agentInfo, Blackboard* pBlackboard)
{
	SteeringPlugin_Output steering{};

	// Data 
	Agent* pAgent = nullptr;
	WorldState* pWorldState = nullptr;
	Elite::Vector2* lastSeenEnemyPos{};

	// Check if agent data is valid
	bool dataValid = pBlackboard->GetData("Agent", pAgent)
		&& pBlackboard->GetData("WorldState", pWorldState);
	if (!dataValid) return steering;

	// Determine position to go to
	// Safely seek towards the desired position
	steering = SeekAndDodge::CalculateSteering(pInterface, deltaT, agentInfo, pBlackboard);
	// Check if we can pick up the item
		// Tell the action...

	return steering;
}
void SeekItem::SetItemToSeek(const eItemType& itemType)
{
	m_ItemTypeToSeek = itemType;
}
