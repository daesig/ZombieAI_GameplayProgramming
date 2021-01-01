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

	if (enemyInSight)
	{
		// Get the angle between the goal vector and the direction towards the enemy
		float angle = atan2(lastSeenEnemyPos->y - agentInfo.Position.y, lastSeenEnemyPos->x - agentInfo.Position.x);
		float angleDeg = angle * 180.f / float(M_PI);

		// Get the vector to the enemy
		Elite::Vector2 v{ *lastSeenEnemyPos - agentInfo.Position };
		// Mirror the toEnemy vector over the agent direction to obtain a dodge velocity
		//const Elite::Vector2 normal = agentToGoalVec.GetNormalized();
		const double perp = 2.0 * v.Dot(normal);
		Elite::Vector2 reflectDir = v - (perp * normal);
		reflectDir.x *= -1.f;
		reflectDir.y *= -1.f;

		//steering.LinearVelocity = agentToGoalVec;
		steering.LinearVelocity = normal * 3.f + reflectDir;
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= agentInfo.MaxLinearSpeed;

		steering.RunMode = true;

		pInterface->Draw_Direction(agentInfo.Position, reflectDir, 5.f, Elite::Vector3{ 0.f,0.f,1.f });
	}
	else
	{
		// Move towards goal location, dodge is not required
		steering.LinearVelocity = agentGoalPosition - agentInfo.Position;
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= agentInfo.MaxLinearSpeed;
		steering.RunMode = false;
	}

	if (distance < 4.f)
	{
		steering.LinearVelocity *= .5f;
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
