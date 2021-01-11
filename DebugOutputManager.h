#pragma once
#include <string>

class DebugOutputManager
{
public:
	enum class DebugType
	{
		CONSTRUCTION,
		DESTRUCTION,
		FSM_STATE,
		GOAP_PLANNER,
		GOAP_ACTION,
		SEARCH_ALGORITHM,
		INVENTORY,
		STEERING,
		PROBLEM
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
	enum class TextColor
	{
		GREEN = 2,
		BLUE=3,
		RED=4,
		WHITE=7
	};

	DebugOutputManager() = default;

	// Handle for console color
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// Debug bools
	bool m_DebuggingAllowed = true;
	bool m_DebugFSMState = true;
	bool m_DebugGOAPPlanner = false;
	bool m_DebugGOAPAction = true;
	bool m_DebugSearchAlgorithm = false;
	bool m_DebugInventory = false;
	bool m_DebugSteering = false;
	bool m_DebugProblem = true;
};

