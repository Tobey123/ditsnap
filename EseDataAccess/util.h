#pragma once
#include "esent.h"

namespace Ese
{
	string GetJetErrorMessage(JET_ERR err);
	JET_RETINFO InitRetInfo(unsigned long itagSequence = 1);
	void ThrowOnError(JET_ERR x);
	string wtos(wstring w);
}
