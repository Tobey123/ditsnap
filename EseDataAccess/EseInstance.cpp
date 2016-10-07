#include "EseDataAccess.h"
#include "EseMetaData.h"
#include "util.h"

namespace Ese
{
	class EseInstance::Impl
	{
	public:
		Impl() {}
		JET_INSTANCE jetInstance_{0};
		JET_SESID sessionId_{0};
		unsigned int pageSize_{0};

		DISALLOW_COPY_AND_ASSIGN(EseInstance::Impl);
	};

	EseInstance::EseInstance(unsigned int pageSize) : pimpl(new Impl) {
		pimpl->pageSize_ = pageSize;
		auto instanceName = "ditsnap";
		ThrowOnError(JetSetSystemParameter(&pimpl->jetInstance_, 0, JET_paramDatabasePageSize, pimpl->pageSize_, nullptr));
		ThrowOnError(JetCreateInstance(&pimpl->jetInstance_, instanceName));
		ThrowOnError(JetInit(&pimpl->jetInstance_));
		ThrowOnError(JetBeginSession(pimpl->jetInstance_, &pimpl->sessionId_, nullptr, nullptr));
	}

	EseInstance::~EseInstance() {
		if (pimpl->sessionId_ != 0) {
			JetEndSession(pimpl->sessionId_, 0);
		}

		if (pimpl->jetInstance_ != 0) {
			JetTerm(pimpl->jetInstance_);
		}
	}

	EseDatabase* EseInstance::OpenDatabase(wstring dbPath) const {
		return new EseDatabase(*this, string(CW2A(dbPath.c_str())));
	}

	EseMetaData* EseInstance::GetMetaData() const
	{
		return new EseMetaData(pimpl->sessionId_, pimpl->jetInstance_);
	}
}
