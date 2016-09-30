#pragma once
#include "esent.h"
#include "spdlog/spdlog.h"

namespace Ese
{
	string GetJetErrorMessage(JET_ERR err);
	JET_RETINFO InitRetInfo(unsigned long itagSequence = 1);
	void ThrowOnError(JET_ERR x);
	string wtos(wstring w);
	std::shared_ptr<spdlog::logger> GetLogger(string loggerName = "Ese");
}
