#include "stdafx.h"
#include "DebugOutputManager.h"
#include <iostream>

DebugOutputManager* DebugOutputManager::instance = 0;

void DebugOutputManager::DebugLine(const std::string& line, DebugType debugType)
{
	if (!m_DebuggingAllowed) return;

	bool debug = false;

	switch (debugType)
	{
	case DebugType::FSM_STATE:
		break;
	case DebugType::GOAP_PLANNER:
		break;
	case DebugType::SEARCH_ALGORITHM:
		debug = true;
		break;
	}

	if (debug)
	{
		std::cout << line;
	}
}
