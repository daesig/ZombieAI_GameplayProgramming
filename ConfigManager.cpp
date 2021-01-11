#include "stdafx.h"
#include "ConfigManager.h"

ConfigManager* ConfigManager::instance = 0;

bool ConfigManager::GetDebugHouseScoutVectors() const
{
	return m_DebugHouseScoutVectors;
}

bool ConfigManager::GetDebugHouseCornerLocations() const
{
	return m_DebugHouseCornerLocations;
}

bool ConfigManager::GetDebugLastEnemyLocation() const
{
	return m_DebugLastEnemyLocation;
}

bool ConfigManager::GetDebugSteering() const
{
	return m_DebugSteering;
}

bool ConfigManager::GetDebugGoalPosition() const
{
	return m_DebugGoalPosition;
}

bool ConfigManager::GetDebugDistantGoalPosition() const
{
	return m_DebugDistantGoalPosition;
}
