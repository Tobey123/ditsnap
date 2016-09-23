#include "stdafx.h"
#include "EseDataAccess.h"
#include <spdlog/details/spdlog_impl.h>

namespace Ese
{
	string GetJetErrorMessage(JET_ERR err) {
		auto bufsize = 512;
		vector<char> buf(bufsize);
		auto r = JetGetSystemParameter(NULL, JET_sesidNil, JET_paramErrorToString,
		                                   reinterpret_cast<ULONG_PTR *>(&err), buf.data(), bufsize);
		if (r == JET_errSuccess) {
			return string(buf.data());
		}

		return string("Unknown Error.");
	}

	JET_RETINFO InitRetInfo(unsigned long itagSequence) {
		JET_RETINFO retInfo = {0};
		retInfo.cbStruct = sizeof(JET_RETINFO);
		retInfo.itagSequence = itagSequence;
		return retInfo;
	}

	void ThrowOnError(JET_ERR x) {
		if (x != JET_errSuccess)
			throw runtime_error(GetJetErrorMessage(x));
	}

	string wtos(wstring w) {
		return string(CW2A(w.c_str()));
	}

	std::shared_ptr<spdlog::logger> GetLogger(string loggerName) {
		auto logger = spdlog::get(loggerName);
		if (logger != nullptr) {
			return logger;
		}

#ifdef _DEBUG
		spdlog::set_level(spdlog::level::debug);
#endif
		wchar_t fileName[MAX_PATH];
		GetModuleFileName(nullptr, fileName, MAX_PATH);
		auto pos = wstring(fileName).find_last_of(L"\\/");
		auto dirName = wstring(fileName).substr(0, pos);
		auto logFile = dirName + L"\\ditsnap.log";
		return spdlog::basic_logger_mt(loggerName, logFile);
	}
}
