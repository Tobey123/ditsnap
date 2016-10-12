#pragma once
#include "esent.h"

namespace Ese
{
	wstring to_w(string s);
	wstring to_w(const char* s);
	string GetJetErrorMessage(JET_ERR err);
	JET_RETINFO InitRetInfo(unsigned long itagSequence = 1);
	void ThrowOnError(JET_ERR x);
	string wtos(wstring w);
}
