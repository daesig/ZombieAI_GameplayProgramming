#pragma once

#include <vector>
#include "structs.h"
#include <unordered_map>

class WorldState
{
public:
	WorldState() = default;

	void AddState(const std::string& key, bool value)
	{
		auto it = m_pStates.find(key);
		if (it == m_pStates.end())
		{
			DebugOutputManager::GetInstance()->DebugLine("Adding state: " + key + " \n",
				DebugOutputManager::DebugType::WORLDSTATE);
			m_pStates[key] = value;
			return;
		}
		DebugOutputManager::GetInstance()->DebugLine("ERROR: State " + key + " already exists\n",
			DebugOutputManager::DebugType::PROBLEM);
		return;
	}

	void SetState(const std::string& key, bool newValue)
	{
		auto it = m_pStates.find(key);
		if (it != m_pStates.end())
		{
			m_pStates[key] = newValue;
		}
	}

	// Returns true if the state was found, puts the value into the reference
	bool GetState(const std::string& key, bool& value)
	{
		auto it = m_pStates.find(key);
		if (it != m_pStates.end())
		{
			value = m_pStates[key];
			return true;
		}
		return false;
	}

	bool IsStateMet(const std::string& key, const bool value)
	{
		if (DoesStateExist(key))
			return m_pStates[key] == value;
		return false;
	}

	bool DoesStateExist(const std::string& key) const
	{
		auto it = m_pStates.find(key);
		if (it != m_pStates.end())
		{
			return true;
		}
		return false;
	}
private:
	std::unordered_map<std::string, bool> m_pStates{};
};