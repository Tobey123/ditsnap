#include "stdafx.h"
#include "util.h"
#include "EseDataAccess.h"

namespace EseDataAccess
{
	EseInstance::EseInstance(uint pageSize) : jetInstance_(0), sessionId_(0), pageSize_(pageSize)
	{
		auto instanceName = "ditsnap";
		ThrowOnError(JetSetSystemParameter(&jetInstance_, 0, JET_paramDatabasePageSize, pageSize_, nullptr));
		ThrowOnError(JetCreateInstance(&jetInstance_, instanceName));
		ThrowOnError(JetInit(&jetInstance_));
		ThrowOnError(JetBeginSession(jetInstance_, &sessionId_, nullptr, nullptr));
	}

	EseInstance::~EseInstance()
	{
		if (sessionId_ != 0)
		{
			JetEndSession(sessionId_, 0);
		}

		if (jetInstance_ != 0)
		{
			JetTerm(jetInstance_);
		}
	}

	EseDatabase* EseInstance::OpenDatabase(const wstring dbPath) const
	{
		return new EseDatabase(this, string(CW2A(dbPath.c_str())));
	}
}