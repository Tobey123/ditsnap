#include "EseMetaData.h"

namespace Ese
{
	EseMetaData::EseMetaData(JET_SESID sessionId, JET_INSTANCE instance) : sessionId_(sessionId), instance_(instance)
	{
	}

	EseMetaData::~EseMetaData()
	{
	}

	JET_SESID EseMetaData::GetJetSessionId()
	{
		return sessionId_;
	}

	JET_INSTANCE EseMetaData::GetJetInstance()
	{
		return instance_;
	}
}