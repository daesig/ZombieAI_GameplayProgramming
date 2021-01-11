#include "stdafx.h"
#include "DebugOutputManager.h"
#include "windows.h"
#include <iostream>

DebugOutputManager* DebugOutputManager::instance = 0;

void DebugOutputManager::DebugLine(const std::string& line, DebugType debugType)
{
	if (!m_DebuggingAllowed) return;

	bool debug = false;

	switch (debugType)
	{
	case DebugType::FSM_STATE:
		debug = m_DebugFSMState;
		break;
	case DebugType::GOAP_PLANNER:
		SetConsoleTextAttribute(hConsole, int(TextColor::WHITE));
		debug = m_DebugGOAPPlanner;
		break;
	case DebugType::SEARCH_ALGORITHM:
		SetConsoleTextAttribute(hConsole, int(TextColor::WHITE));
		debug = m_DebugSearchAlgorithm;
		break;
	case DebugType::PROBLEM:
		SetConsoleTextAttribute(hConsole, int(TextColor::RED));
		debug = m_DebugProblem;
		break;
	default:
		SetConsoleTextAttribute(hConsole, int(TextColor::WHITE));
		debug = true;
		break;
	}

	if (debug)
	{
		std::cout << line;
	}
}
