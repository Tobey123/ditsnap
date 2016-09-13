#include "stdafx.h"
#include "EseDataAccess.h"

namespace EseDataAccess
{
	EseColumn::EseColumn(uint id, const string& name, uint type, bool isUnicode) :
		id_(id), name_(name), type_(type), isUnicode_(isUnicode)
	{
	}

	EseColumn::~EseColumn()
	{
	}

	uint EseColumn::GetId() const
	{
		return id_;
	}

	string EseColumn::GetName() const
	{
		return name_;
	}

	uint EseColumn::GetType() const
	{
		return type_;
	}

	bool EseColumn::IsUnicode() const
	{
		return isUnicode_;
	}
} 
