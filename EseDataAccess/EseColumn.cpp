#include "stdafx.h"
#include "EseDataAccess.h"

namespace EseDataAccess
{
	class EseColumn::Impl
	{
	public:
		Impl(){}
		uint id_;
		string name_;
		uint type_;
		bool isUnicode_;

		DISALLOW_COPY_AND_ASSIGN(EseColumn::Impl);
	};


	EseColumn::EseColumn(uint id, string name, uint type, bool isUnicode) : pimpl(new Impl())		
	{
		pimpl->id_ = id; 
		pimpl->name_ = name; 
		pimpl->type_ = type; 
		pimpl->isUnicode_ = isUnicode;
	}

	EseColumn::~EseColumn()
	{
	}

	uint EseColumn::GetId() const
	{
		return pimpl->id_;
	}

	string EseColumn::GetName() const
	{
		return pimpl->name_;
	}

	uint EseColumn::GetType() const
	{
		return pimpl->type_;
	}

	bool EseColumn::IsUnicode() const
	{
		return pimpl->isUnicode_;
	}
} 
