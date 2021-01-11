#pragma once
#include <string>

class DebugOutputManager
{
public:
	enum class DebugType
	{
		FSM_STATE,
		GOAP_PLANNER,
		SEARCH_ALGORITHM,
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
	bool m_DebugGOAPPlanner = true;
	bool m_DebugSearchAlgorithm = false;
	bool m_DebugProblem = true;
};

