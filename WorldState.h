#pragma once

#include <vector>
#include "structs.h"
#include <unordered_map>

class StateField
{
public:
	StateField() = default;
	virtual ~StateField() = default;
};

template <class T>
class State final: public StateField
{
public:
	State(T value)
	{
		m_Value = value;
	}

	T GetValue() const { return m_Value; }
	void SetValue(T newValue) { m_Value = newValue; }
private:
	T m_Value;
};

class WorldState
{
public:
	WorldState() = default;

	template <class T>
	void AddState(const std::string& key, T value)
	{
		auto it = m_pStates.find(key);
		if (it == m_pStates.end())
		{
			std::cout << "Adding state: " << key << " \n";
			m_pStates[key] = new State<T>(value);
			return;
		}
		std::cout << "ERROR: State already exists\n";
		return;
	}

	template <class T>
	void SetState(const std::string& key, T newValue)
	{
		auto it = m_pStates.find(key);
		if (it != m_pStates.end())
		{
			State<T>* pState = dynamic_cast<State<T>*>(m_pStates[key]);
			if (pState)
			{
				pState->SetValue(newValue);
			}
		}
	}

	template <class T>
	bool GetState(const std::string& key, T& value) 
	{
		auto it = m_pStates.find(key);
		if (it != m_pStates.end())
		{
			State<T>* pState = dynamic_cast<State<T>*>(m_pStates[key]);
			if (pState)
			{
				value = pState->GetValue();
				return true;
			}
		}

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
	std::unordered_map<std::string, StateField*> m_pStates;
};