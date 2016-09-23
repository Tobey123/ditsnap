#include "stdafx.h"
#include "util.h"
#include "EseDataAccess.h"
#include "spdlog/spdlog.h"

namespace Ese
{
	class EseInstance::Impl
	{
	public:
		Impl() {}
		JET_INSTANCE jetInstance_{0};
		JET_SESID sessionId_{0};
		uint pageSize_{0};
		std::shared_ptr<spdlog::logger> log_;

		DISALLOW_COPY_AND_ASSIGN(EseInstance::Impl);
	};

	EseInstance::EseInstance(uint pageSize) : pimpl(new Impl) {
		pimpl->pageSize_ = pageSize;
		pimpl->log_ = GetLogger();
		pimpl->log_->info("Starting ESE instance with page size {}...", pageSize);
		auto instanceName = "ditsnap";
		ThrowOnError(JetSetSystemParameter(&pimpl->jetInstance_, 0, JET_paramDatabasePageSize, pimpl->pageSize_, nullptr));
		ThrowOnError(JetCreateInstance(&pimpl->jetInstance_, instanceName));
		ThrowOnError(JetInit(&pimpl->jetInstance_));
		ThrowOnError(JetBeginSession(pimpl->jetInstance_, &pimpl->sessionId_, nullptr, nullptr));
		pimpl->log_->info("Successfully started ESE instance.");
	}

	EseInstance::~EseInstance() {
		pimpl->log_->info("Stopping ESE instance...");
		if (pimpl->sessionId_ != 0) {
			JetEndSession(pimpl->sessionId_, 0);
		}

		if (pimpl->jetInstance_ != 0) {
			JetTerm(pimpl->jetInstance_);
		}
		pimpl->log_->info("Stopped ESE instance.");
	}

	EseDatabase* EseInstance::OpenDatabase(wstring dbPath) const {
		pimpl->log_->info("Opening database {}...", wtos(dbPath));
		return new EseDatabase(this, string(CW2A(dbPath.c_str())));
	}

	JET_INSTANCE EseInstance::GetJetInstance() const {
		return pimpl->jetInstance_;
	}

	JET_SESID EseInstance::GetSessionId() const {
		return pimpl->sessionId_;
	}
}
