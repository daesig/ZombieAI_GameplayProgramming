#pragma once
#include <string>

struct GOAPProperty
{
	std::string propertyKey;

	union PropertyValue
	{
		bool bValue;
		int iValue;
		float fValue;
	};

	PropertyValue value;
};