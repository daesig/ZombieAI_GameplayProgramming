#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"

//WANDER (base> SEEK)
Wander::Wander() :
	m_DistanceFromActor{ 5.f },
	m_WanderRadius{ 5.f },
	m_RenewDistance{ 2.f }
{}

SteeringPlugin_Output Wander::CalculateSteering(float deltaT, AgentInfo& pAgent)
{
	SteeringPlugin_Output steering{};

	// Check where the agent is looking
	float rotation = pAgent.Orientation;

	// Determine wander radius in front of actor
	// Scale the wander distance with the target movement speed for better wander behavior
	float finalDistanceFromActor = m_DistanceFromActor + pAgent.AgentSize + (pAgent.CurrentLinearSpeed / 2.f);
	const Elite::Vector2 wanderCenter
	{
		pAgent.Position +
		Elite::Vector2(finalDistanceFromActor * (float)cos(rotation - M_PI / 2.f),
						finalDistanceFromActor * (float)sin(rotation - M_PI / 2.f))
	};


	// Scale the wander radius with the target movement speed for better wander behavior
	float finalWanderRadius = m_WanderRadius + pAgent.AgentSize + pAgent.MaxLinearSpeed / 2.f;
	// Agent is close enough to wanderTarget or the target has left the wander radius
	if ((pAgent.Position.Distance(m_WanderTarget.Position) < m_RenewDistance + pAgent.AgentSize) ||
		(wanderCenter.Distance(m_WanderTarget.Position) > (finalWanderRadius)))
	{
		// Create an offset from the center of the wanderradius
		int randomAngle{ Elite::randomInt(360) };
		Elite::Vector2 randomOffset{
			Elite::randomFloat(finalWanderRadius) * (float)sin(randomAngle),
			Elite::randomFloat(finalWanderRadius) * (float)cos(randomAngle)
		};

		// Set target position with the offset
		m_WanderTarget.Position = wanderCenter + randomOffset;
	}

	// Seek
	steering.LinearVelocity = m_WanderTarget.Position - pAgent.Position; //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= pAgent.MaxLinearSpeed; //Rescale to Max Speed=

	return steering;
}
void Wander::SetTarget(const TargetData& target)
{
	// Intentionally left blank so that no manual target can be set.
	return;
}

