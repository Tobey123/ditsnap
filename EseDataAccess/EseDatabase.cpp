#include "stdafx.h"
#include "util.h"
#include "EseDataAccess.h"

namespace EseDataAccess
{
	class EseDatabase::Impl
	{
	public:
		Impl(): eseInstance_(nullptr), sessionId_(0){}
		const EseInstance* eseInstance_;
		JET_SESID sessionId_;
		JET_DBID dbId_;
		string dbPath_;
		int tableCount_;
		std::shared_ptr<spdlog::logger> log_;

		DISALLOW_COPY_AND_ASSIGN(EseDatabase::Impl);
	};


	EseDatabase::EseDatabase(const EseInstance* const eseInstance, const string dbPath) 
		:pimpl(new Impl())
	{		
		pimpl->eseInstance_ = eseInstance;
		pimpl->sessionId_ = eseInstance->GetSessionId();
		pimpl->dbId_ = 0;
		pimpl->dbPath_ = dbPath;
		pimpl->tableCount_ = -1;
		pimpl->log_ = GetLogger();
		pimpl->log_->info("Opening ESE Database {}...", dbPath);
		ThrowOnError(JetAttachDatabase(pimpl->sessionId_, pimpl->dbPath_.c_str(), JET_bitDbReadOnly));
		ThrowOnError(JetOpenDatabase(pimpl->sessionId_, pimpl->dbPath_.c_str(), nullptr, &pimpl->dbId_, JET_bitDbReadOnly));
		pimpl->log_->info("Opened ESE Database {}.", dbPath);
	}

	EseDatabase::~EseDatabase(void)
	{
		pimpl->log_->info("Closing ESE Database {}...", pimpl->dbPath_);
		if (pimpl->dbId_ != 0)
		{
			JetCloseDatabase(pimpl->sessionId_, pimpl->dbId_, 0);
		}

		JetDetachDatabase(pimpl->sessionId_, pimpl->dbPath_.c_str());
		pimpl->log_->info("Closed ESE Database {}.", pimpl->dbPath_);
	}

	EseTable* EseDatabase::OpenTable(wstring tableName) const
	{
		return new EseTable(this, string(CW2A(tableName.c_str())));
	}

	vector<wstring> EseDatabase::GetTableNames() const
	{
		pimpl->log_->info("Listing tables...");
		//Get a temporary table which contains all table names.
		JET_OBJECTLIST tableList{0};
		ThrowOnError(JetGetObjectInfo(pimpl->sessionId_, pimpl->dbId_, JET_objtypTable,
			nullptr, nullptr, &tableList, sizeof(JET_OBJECTLIST), JET_ObjInfoList));
		pimpl->tableCount_ = tableList.cRecord;
		vector<wstring> tableNames;
		for (auto i = 0; i < tableList.cRecord; ++i)
		{
			unsigned long actualSize = 0;
			auto retInfo = InitRetInfo();
			vector<char> tableNameBuffer(JET_cbNameMost + 1);
			try
			{
				ThrowOnError(JetRetrieveColumn(pimpl->sessionId_, tableList.tableid, tableList.columnidobjectname, 
					tableNameBuffer.data(), JET_cbNameMost, &actualSize, 0, &retInfo));
				tableNameBuffer[actualSize] = NULL;
				auto tableName = wstring(tableNameBuffer.begin(), tableNameBuffer.end());
				tableNames.push_back(tableName);
				auto r = JetMove(pimpl->sessionId_, tableList.tableid, JET_MoveNext, 0);
				if (r == JET_errNoCurrentRecord)
					break;
				ThrowOnError(r);
			}
			catch (runtime_error&)
			{
				JetCloseTable(pimpl->sessionId_, tableList.tableid);
				throw;
			}
		}

		JetCloseTable(pimpl->sessionId_, tableList.tableid);
		pimpl->log_->info("Successfully listed tables.");
		return tableNames;
	}

	const EseInstance* EseDatabase::GetEseInstance() const
	{
		return pimpl->eseInstance_;
	}

	JET_DBID EseDatabase::GetDbId() const
	{
		return pimpl->dbId_;
	}
}
