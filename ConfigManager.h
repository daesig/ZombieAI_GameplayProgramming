#pragma once
class ConfigManager
{
public:
	static ConfigManager* GetInstance()
	{
		if (!instance)
		{
			instance = new ConfigManager();
		}
		return instance;
	}

	static ConfigManager* instance;

	bool GetDebugHouseScoutVectors() const;
	bool GetDebugHouseCornerLocations() const;
	bool GetDebugLastEnemyLocation() const;
	bool GetDebugSteering() const;
	bool GetDebugGoalPosition() const;
	bool GetDebugDistantGoalPosition() const;
private:
	ConfigManager() = default;

	bool m_DebugHouseScoutVectors = false;
	bool m_DebugHouseCornerLocations = true;
	bool m_DebugLastEnemyLocation = true;
	bool m_DebugSteering = false;
	bool m_DebugGoalPosition = true;
	bool m_DebugDistantGoalPosition = true;
};

