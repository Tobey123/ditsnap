#include "EseDataAccess.h"
#include "EseMetaData.h"

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
}
