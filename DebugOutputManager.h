#pragma once
#include <string>

class DebugOutputManager
{
public:
	enum class DebugType
	{
		FSM_STATE,
		GOAP_PLANNER,
		SEARCH_ALGORITHM
	};

	static DebugOutputManager* GetInstance()
	{
		if (!instance)
		{
			instance = new DebugOutputManager();
		}
		return instance;
	}

	void DebugLine(const std::string& line, DebugType debugType);

	static DebugOutputManager* instance;
private:
	DebugOutputManager() = default;

	bool m_DebuggingAllowed = true;
};

