#pragma once

#include <vector>
#include "structs.h"
#include <unordered_map>

//class StateField
//{
//public:
//	StateField() = default;
//	virtual ~StateField() = default;
//};
//
//class State final : public StateField
//{
//public:
//	State(bool value)
//	{
//		m_Value = value;
//	}
//
//	bool GetValue() const { return m_Value; }
//	void SetValue(bool newValue) { m_Value = newValue; }
//private:
//	bool m_Value;
//};

class WorldState
{
public:
	WorldState() = default;

	void AddState(const std::string& key, bool value)
	{
		auto it = m_pStates.find(key);
		if (it == m_pStates.end())
		{
			std::cout << "Adding state: " << key << " \n";
			m_pStates[key] = value;
			return;
		}
		std::cout << "ERROR: State already exists\n";
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