#pragma once
#include "stdafx.h"

namespace EseDataAccess
{
	string GetJetErrorMessage(JET_ERR err);

	JET_RETINFO InitRetInfo(unsigned long itagSequence = 1);

	inline void ThrowOnError(JET_ERR x)
	{
		if (x != JET_errSuccess)
			throw runtime_error(GetJetErrorMessage(x));
	}
}