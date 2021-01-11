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
	case DebugType::CONSTRUCTION:
		SetConsoleTextAttribute(hConsole, int(TextColor::GREEN));
		debug = true;
		break;
	case DebugType::DESTRUCTION:
		SetConsoleTextAttribute(hConsole, int(TextColor::GREEN));
		debug = true;
		break;
	case DebugType::FSM_STATE:
		debug = m_DebugFSMState;
		break;
	case DebugType::GOAP_PLANNER:
		SetConsoleTextAttribute(hConsole, int(TextColor::WHITE));
		debug = m_DebugGOAPPlanner;
		break;
	case DebugType::GOAP_ACTION:
		debug = m_DebugGOAPAction;
		SetConsoleTextAttribute(hConsole, int(TextColor::BLUE));
		break;
	case DebugType::SEARCH_ALGORITHM:
		SetConsoleTextAttribute(hConsole, int(TextColor::WHITE));
		debug = m_DebugSearchAlgorithm;
		break;
	case DebugType::PROBLEM:
		SetConsoleTextAttribute(hConsole, int(TextColor::RED));
		debug = m_DebugProblem;
		break;
	case DebugType::INVENTORY:
		debug = m_DebugInventory;
		SetConsoleTextAttribute(hConsole, int(TextColor::WHITE));
		break;
	case DebugType::STEERING:
		debug = m_DebugSteering;
		SetConsoleTextAttribute(hConsole, int(TextColor::WHITE));
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
