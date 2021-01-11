#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "Blackboard.h"
#include "Agent.h"
#include "IExamInterface.h"
#include "utils.h"

//SEEK (base>ISteeringBehavior)
SteeringPlugin_Output Seek::CalculateSteering(IExamInterface* pInterface, float deltaT, AgentInfo& agentInfo, Blackboard* pBlackboard, bool changeGoal)
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

SeekAndDodge::SeekAndDodge() :
	ISteeringBehavior()
{}

// Dodge the enemy, keeping in mind the goal location
SteeringPlugin_Output SeekAndDodge::CalculateSteering(IExamInterface* pInterface, float deltaT, AgentInfo& agentInfo, Blackboard* pBlackboard, bool changeGoal)
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
	float orientationAngle = utils::GetCorrectedOrientationAngleInDeg(agentInfo.Orientation);
	orientationAngle = fmod(orientationAngle, 360.f);
	float orientationAngleRad = orientationAngle * float(M_PI) / 180.f;

	if (enemyInSight)
	{
		int* enemyCount = nullptr;
		pBlackboard->GetData("EnemyCount", enemyCount);
		float dodgeRange{ 10.f };
		if (*enemyCount > 1)
		{
			dodgeRange = 40.f;
		}

		// Get the angle towards the enemy
		float angleToEnemy = atan2(lastSeenEnemyPos->y - agentInfo.Position.y, lastSeenEnemyPos->x - agentInfo.Position.x);
		float angleToEnemyDeg = angleToEnemy * 180.f / float(M_PI);
		angleToEnemyDeg = angleToEnemyDeg - orientationAngle;
		// Map angle to enemy to a values between -180 and 180
		if (angleToEnemyDeg > 180.f)
			angleToEnemyDeg -= 360.f;
		if (angleToEnemyDeg < -180.f)
			angleToEnemyDeg += 360.f;

		m_AngleToLastEnemy = angleToEnemyDeg;
		m_LastOrientationAngle = orientationAngle;

		// If the enemy is in right front of us
		if (abs(angleToEnemyDeg) < 20.f)
		{
			float sign = angleToEnemyDeg / abs(angleToEnemyDeg);
			angleToEnemyDeg += 15.f * sign;
		}

		// Convert angle back to radians
		angleToEnemy = angleToEnemyDeg * float(M_PI) / 180.f;

		if (changeGoal)
		{
			// Set a new goal position that dodges the enemy
			float halfPi = float(M_PI) / 2.f;
			Elite::Vector2 newGoal = agentInfo.Position + Elite::Vector2{ cos(orientationAngleRad + -angleToEnemy) * dodgeRange, sin(orientationAngleRad + -angleToEnemy) * dodgeRange };
			pAgent->SetGoalPosition(pInterface->NavMesh_GetClosestPathPoint(newGoal));
			steering.RunMode = true;
		}
	}
	else
		steering.RunMode = false;

	// Move towards goal location, dodge is not required
	steering.LinearVelocity = pAgent->GetGoalPosition() - agentInfo.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;

	if (pAgent->WasBitten())
	{
		std::cout << "BITTEN\n";
		steering.RunMode = true;
	}

	// Slow down when we get close to the goal and face the target
	if (distance < 3.f)
	{
		const Elite::Vector2& goalPos{ pAgent->GetGoalPosition() };
		float angleToGoal = atan2(goalPos.y - agentInfo.Position.y, goalPos.x - agentInfo.Position.x);

		//steering.LinearVelocity *= .5f;
		steering.AutoOrient = false;
		//steering.AngularVelocity = 1.f;

		// Determine the angle from our orientation towards the goal
		float angleFromAgentToGoal = angleToGoal - orientationAngleRad;
		if (angleFromAgentToGoal > float(M_PI))
			angleFromAgentToGoal -= float(M_PI) * 2.f;
		if (angleFromAgentToGoal < -float(M_PI))
			angleFromAgentToGoal += float(M_PI) * 2.f;

		if (abs(angleFromAgentToGoal) < .1f)
		{
			steering.AutoOrient = false;
			steering.AngularVelocity = 0.f;
		}
		else
		{
			float sign = angleFromAgentToGoal / abs(angleFromAgentToGoal);
			steering.AngularVelocity = agentInfo.MaxAngularSpeed * sign;
		}



		//if (angleFromAgentToGoal > .1f)
		//{
		//	steering.AngularVelocity = agentInfo.MaxAngularSpeed;
		//}
		//else if (angleFromAgentToGoal < -.1f)
		//{
		//	steering.AngularVelocity = -agentInfo.MaxAngularSpeed;
		//}
		//else
		//{
		//	//steering.AutoOrient = true;
		//}
	}
	else
	{
		// Use stamina when we have enough
		if (pInterface->Agent_GetInfo().Stamina > 9.f)
			steering.RunMode = true;
	}

	bool isInPurgeZone{ false };
	if (pBlackboard->GetData("AgentInPurgeZone", isInPurgeZone))
	{
		if (isInPurgeZone)
			steering.RunMode = true;
	}

	// Debug agent velocity
	pInterface->Draw_Direction(agentInfo.Position, agentToGoalVec, 5.f, Elite::Vector3{ 0.f,1.f,0.f });
	return steering;
}

KillBehavior::KillBehavior() :
	SeekAndDodge()
{}
SteeringPlugin_Output KillBehavior::CalculateSteering(IExamInterface* pInterface, float deltaT, AgentInfo& agentInfo, Blackboard* pBlackboard, bool changeGoal)
{
	SteeringPlugin_Output steering = SeekAndDodge::CalculateSteering(pInterface, deltaT, agentInfo, pBlackboard, false);
	steering.AutoOrient = false;

	//Elite::Vector2 closestEnemyPos{};
	//bool isValid = pBlackboard->GetData("LastEnemyPos", closestEnemyPos);
	//if (!isValid)
	//	return steering;

	steering.AngularVelocity = agentInfo.MaxAngularSpeed;

	float goalAngle{ 1.f };
	//float goalAngleRad = goalAngle * float(M_PI) / 180.f;

	WorldState* pWorldState = nullptr;
	if (pBlackboard->GetData("WorldState", pWorldState))
	{
		Elite::Vector2* lastSeenEnemyPos{};
		pBlackboard->GetData("LastEnemyPos", lastSeenEnemyPos);
		pInterface->Draw_Circle(*lastSeenEnemyPos, 1.5f, { 1.f,1.f,1.f });

		steering.AngularVelocity = agentInfo.MaxAngularSpeed;
		if (pWorldState->IsStateMet("EnemyInSight", true))
		{
			std::cout << "angle: " << m_AngleToLastEnemy << "\n";

			if (m_AngleToLastEnemy > goalAngle)
			{
				steering.AngularVelocity = agentInfo.MaxAngularSpeed;
			}
			else if (m_AngleToLastEnemy < -goalAngle)
			{
				steering.AngularVelocity = -agentInfo.MaxAngularSpeed;
			}
			else
			{
				std::cout << "SHOOT\n";
				// Stop turning
				steering.AngularVelocity = 0.f;

				Agent* pAgent = nullptr;
				bool isValid = pBlackboard->GetData("Agent", pAgent);
				if (pAgent)
				{
					std::cout << "SHOOT GUN\n";
					pAgent->Shoot();
				}
			}
		}
	}

	//auto vEntitiesInFOV = utils::GetEntitiesInFOV(pInterface);
	//for (EntityInfo& entityInfo : vEntitiesInFOV)
	//{
	//	if (entityInfo.Type == eEntityType::ENEMY)
	//	{
	//		if(entityInfo.Location)
	//	}
	//}

	return steering;
}

//WANDER (base>ISteeringBehavior)
Wander::Wander() :
	m_DistanceFromActor{ 5.f },
	m_WanderRadius{ 5.f },
	m_RenewDistance{ 2.f }
{}
SteeringPlugin_Output Wander::CalculateSteering(IExamInterface* pInterface, float deltaT, AgentInfo& agentInfo, Blackboard* pBlackboard, bool changeGoal)
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
