#pragma once
#include "esent.h"

namespace Ese 
{
	class EseMetaData
	{
	public:
		EseMetaData(JET_SESID sessionId, JET_INSTANCE instance);
		~EseMetaData();
		JET_SESID GetJetSessionId();
		JET_INSTANCE GetJetInstance();
	private:
		JET_SESID sessionId_;
		JET_INSTANCE instance_;
	};
}