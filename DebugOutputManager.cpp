#include "stdafx.h"
#include "DebugOutputManager.h"
#include "windows.h"
#include <iostream>

DebugOutputManager* DebugOutputManager::instance = 0;

void DebugOutputManager::DebugLine(const std::string& line, DebugType debugType)
{
	if (!m_DebuggingAllowed) return;

	bool debug = false;
	SetConsoleTextAttribute(hConsole, int(TextColor::WHITE));

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
		debug = m_DebugGOAPPlanner;
		break;
	case DebugType::GOAP_ACTION:
		debug = m_DebugGOAPAction;
		break;
	case DebugType::SEARCH_ALGORITHM:
		debug = m_DebugSearchAlgorithm;
		break;
	case DebugType::PROBLEM:
		SetConsoleTextAttribute(hConsole, int(TextColor::RED));
		debug = m_DebugProblem;
		break;
	case DebugType::INVENTORY:
		debug = m_DebugInventory;
		break;
	case DebugType::STEERING:
		debug = m_DebugSteering;
		break;
	case DebugType::WORLDSTATE:
		debug = m_DebugWorldState;
		SetConsoleTextAttribute(hConsole, int(TextColor::BLUE));
		break;
	default:
		debug = true;
		break;
	}

	if (debug)
	{
		std::cout << line;
	}
}
