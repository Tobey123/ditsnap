#include "util.h"
#include "EseDataAccess.h"
#include "EseMetaData.h"

namespace Ese
{
	class EseDatabase::Impl
	{
	public:
		explicit Impl(const EseInstance& eseInstance) : eseInstance_(eseInstance) {}
		const EseInstance& eseInstance_;
		JET_SESID sessionId_{0};
		JET_DBID dbId_{0};
		string dbPath_;
		int tableCount_{0};

		DISALLOW_COPY_AND_ASSIGN(EseDatabase::Impl);
	};

	EseDatabase::EseDatabase(const EseInstance& eseInstance, const string dbPath) : pimpl(new Impl(eseInstance)) {
		unique_ptr<EseMetaData> metadata(eseInstance.GetMetaData());
		pimpl->sessionId_ = metadata->GetJetSessionId();
		pimpl->dbId_ = 0;
		pimpl->dbPath_ = dbPath;
		pimpl->tableCount_ = -1;
		ThrowOnError(JetAttachDatabase(pimpl->sessionId_, pimpl->dbPath_.c_str(), JET_bitDbReadOnly));
		ThrowOnError(JetOpenDatabase(pimpl->sessionId_, pimpl->dbPath_.c_str(), nullptr, &pimpl->dbId_, JET_bitDbReadOnly));
	}

	EseDatabase::~EseDatabase(void) {
		if (pimpl->dbId_ != 0) {
			JetCloseDatabase(pimpl->sessionId_, pimpl->dbId_, 0);
		}

		JetDetachDatabase(pimpl->sessionId_, 0);
	}

	EseTable* EseDatabase::OpenTable(wstring tableName) const {
		return new EseTable(this, string(CW2A(tableName.c_str())));
	}

	vector<wstring> EseDatabase::GetTableNames() const {
		//Get a temporary table which contains all table names.
		JET_OBJECTLIST tableList{0};
		ThrowOnError(JetGetObjectInfo(pimpl->sessionId_, pimpl->dbId_, JET_objtypTable,
		                              nullptr, nullptr, &tableList, sizeof(JET_OBJECTLIST), JET_ObjInfoList));
		pimpl->tableCount_ = tableList.cRecord;
		vector<wstring> tableNames;
		for (unsigned long i = 0; i < tableList.cRecord; ++i) {
			unsigned long actualSize = 0;
			auto retInfo = InitRetInfo();
			vector<char> tableNameBuffer(JET_cbNameMost + 1);
			try {
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
			catch (runtime_error&) {
				JetCloseTable(pimpl->sessionId_, tableList.tableid);
				throw;
			}
		}

		JetCloseTable(pimpl->sessionId_, tableList.tableid);
		return tableNames;
	}

	const EseInstance& EseDatabase::GetEseInstance() const {
		return pimpl->eseInstance_;
	}

	JET_DBID EseDatabase::GetDbId() const {
		return pimpl->dbId_;
	}
}
