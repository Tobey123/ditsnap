#include "EseDataAccess.h"

namespace Ese
{
	class EseColumn::Impl
	{
	public:
		Impl() {}
		unsigned int id_{0};
		string name_;
		unsigned int type_{0};
		bool isUnicode_{false};

		DISALLOW_COPY_AND_ASSIGN(EseColumn::Impl);
	};

	EseColumn::EseColumn(unsigned int id, string name, unsigned int type, bool isUnicode) : pimpl(new Impl) {
		pimpl->id_ = id;
		pimpl->name_ = name;
		pimpl->type_ = type;
		pimpl->isUnicode_ = isUnicode;
	}

	EseColumn::~EseColumn() { }

	unsigned int EseColumn::GetId() const {
		return pimpl->id_;
	}

	string EseColumn::GetName() const {
		return pimpl->name_;
	}

	unsigned int EseColumn::GetType() const {
		return pimpl->type_;
	}

	bool EseColumn::IsUnicode() const {
		return pimpl->isUnicode_;
	}
}
