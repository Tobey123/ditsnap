#pragma once
#include "stdafx.h"
#include "spdlog/spdlog.h"

namespace EseDataAccess
{
	string GetJetErrorMessage(JET_ERR err);
	JET_RETINFO InitRetInfo(unsigned long itagSequence = 1);
	void ThrowOnError(JET_ERR x);
	string w_to_s(wstring w);
	std::shared_ptr<spdlog::logger> GetLogger(string loggerName = "EseDataAccess");
}
