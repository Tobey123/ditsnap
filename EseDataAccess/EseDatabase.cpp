#include "stdafx.h"
#include "util.h"
#include "EseDataAccess.h"

namespace EseDataAccess
{
	EseDatabase::EseDatabase(const EseInstance* const eseInstance, const string& dbPath)
		: eseInstance_(eseInstance), sessionId_(eseInstance->GetSessionId()),
		dbId_(0), dbPath_(dbPath), tableCount_(-1)
	{
		ThrowOnError(JetAttachDatabase(sessionId_, dbPath_.c_str(), JET_bitDbReadOnly));
		ThrowOnError(JetOpenDatabase(sessionId_, dbPath_.c_str(), nullptr, &dbId_, JET_bitDbReadOnly));
	}

	EseDatabase::~EseDatabase(void)
	{
		if (dbId_ != 0)
		{
			JetCloseDatabase(sessionId_, dbId_, 0);
		}

		JetDetachDatabase(sessionId_, dbPath_.c_str());
	}

	EseTable* EseDatabase::OpenTable(const wstring tableName) const
	{
		return new EseTable(this, string(CW2A(tableName.c_str())));
	}

	vector<wstring> EseDatabase::GetTableNames()
	{
		//Get a temporary table which contains all table names.
		JET_OBJECTLIST tableList{0};
		ThrowOnError(JetGetObjectInfo(sessionId_, dbId_, JET_objtypTable,
			nullptr, nullptr, &tableList, sizeof(JET_OBJECTLIST), JET_ObjInfoList));
		tableCount_ = tableList.cRecord;
		vector<wstring> tableNames;
		for (auto i = 0; i < tableList.cRecord; ++i)
		{
			unsigned long actualSize = 0;
			auto retInfo = InitRetInfo();
			vector<char> tableNameBuffer(JET_cbNameMost + 1);
			try
			{
				ThrowOnError(JetRetrieveColumn(sessionId_, tableList.tableid, tableList.columnidobjectname, 
					tableNameBuffer.data(), JET_cbNameMost, &actualSize, 0, &retInfo));
				tableNameBuffer[actualSize] = NULL;
				auto tableName = wstring(tableNameBuffer.begin(), tableNameBuffer.end());
				tableNames.push_back(tableName);
				auto r = JetMove(sessionId_, tableList.tableid, JET_MoveNext, 0);
				if (r == JET_errNoCurrentRecord)
					break;
				ThrowOnError(r);
			}
			catch (runtime_error&)
			{
				JetCloseTable(sessionId_, tableList.tableid);
				throw;
			}
		}

		JetCloseTable(sessionId_, tableList.tableid);
		return tableNames;
	}
}