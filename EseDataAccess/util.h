#pragma once
#include "stdafx.h"
#include "spdlog/spdlog.h"

namespace EseDataAccess
{
	string GetJetErrorMessage(JET_ERR err);

	JET_RETINFO InitRetInfo(unsigned long itagSequence = 1);

	inline void ThrowOnError(JET_ERR x)
	{
		if (x != JET_errSuccess)
			throw runtime_error(GetJetErrorMessage(x));
	}

	string w_to_s(wstring w);

	std::shared_ptr<spdlog::logger> GetLogger(string loggerName);
}
