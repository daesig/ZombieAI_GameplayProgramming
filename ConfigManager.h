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

	bool m_DebugHouseScoutVectors = true;
	bool m_DebugHouseCornerLocations = true;
	bool m_DebugLastEnemyLocation = false;
	bool m_DebugSteering = false;
	bool m_DebugGoalPosition = false;
	bool m_DebugDistantGoalPosition = false;
};

