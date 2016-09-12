#include "stdafx.h"
#include "EseDataAccess.h"

namespace EseDataAccess
{
	EseColumn::EseColumn(uint id, const string& name, uint type, bool isUnicode) :
		id_(id), name_(name), type_(type), isUnicode_(isUnicode)
	{
	}
} 
