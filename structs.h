#pragma once
#include <string>

struct GOAPProperty
{
	std::string propertyKey;

	union PropertyValue
	{
		PropertyValue(bool value) { bValue = value; }
		PropertyValue(int value) { iValue = value; }
		PropertyValue(float value) { fValue = value; }
		PropertyValue(Elite::Vector2 value) { position = value; }

		bool bValue;
		int iValue;
		float fValue;
		Elite::Vector2 position;
	} value;
};