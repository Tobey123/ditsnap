#include "util.h"
#include "EseDataAccess.h"

namespace Ese
{
	class EseTable::Impl
	{
	public:
		Impl() {}
		const EseDatabase* eseDatabase_{nullptr};
		JET_SESID sessionId_{0};
		JET_DBID dbId_{0};
		JET_TABLEID tableId_{0};
		string tableName_;
		vector<EseColumn*> columns_;
		string GetColumnName(const JET_COLUMNLIST& columnList) const;
		JET_COLTYP GetColumnType(const JET_COLUMNLIST& columnList) const;
		JET_COLUMNID GetColumnId(const JET_COLUMNLIST& columnList) const;
		ushort GetCodePage(const JET_COLUMNLIST& columnList) const;
		EseColumn* GetColumnDefinition(const JET_COLUMNLIST& columnList) const;
		vector<byte> GetColumnData(uint columnIndex, uint itagSequence);
		int CountColumnValue(uint columnIndex) const;
		std::shared_ptr<spdlog::logger> log_;

		DISALLOW_COPY_AND_ASSIGN(EseTable::Impl);
	};

	EseTable::EseTable(const EseDatabase* eseDatabase, string tableName) : pimpl(new Impl) {
		pimpl->eseDatabase_ = eseDatabase;
		pimpl->sessionId_ = eseDatabase->GetEseInstance().GetSessionId();
		pimpl->dbId_ = eseDatabase->GetDbId();
		pimpl->tableName_ = tableName;
		pimpl->log_ = GetLogger();
		pimpl->log_->info("Initializing ESE Table {} in DB ID {}...", tableName, eseDatabase->GetDbId());
		JET_COLUMNLIST columnList{0};
		try {
			ThrowOnError(JetOpenTable(pimpl->sessionId_, pimpl->dbId_, pimpl->tableName_.c_str(), nullptr, 0,
			                          JET_bitTableReadOnly, &pimpl->tableId_));
			// Open a temporary table that contains column definitions
			ThrowOnError(JetGetTableColumnInfo(pimpl->sessionId_, pimpl->tableId_, nullptr,
			                                   &columnList, sizeof(JET_COLUMNLIST), JET_ColInfoList));
			pimpl->columns_.reserve(columnList.cRecord);
			ThrowOnError(JetMove(pimpl->sessionId_, columnList.tableid, JET_MoveFirst, 0));
			JET_ERR ret;
			do {
				pimpl->columns_.push_back(pimpl->GetColumnDefinition(columnList));
			}
			while (JET_errSuccess == (ret = JetMove(pimpl->sessionId_, columnList.tableid, JET_MoveNext, 0)));

			//if cursor don't reach to the end of records, throw exception
			if (ret != JET_errNoCurrentRecord) {
				throw runtime_error(GetJetErrorMessage(ret));
			}

			// close the temporary table 
			JetCloseTable(pimpl->sessionId_, columnList.tableid);
			pimpl->log_->info("Successfully initialized ESE Table {}.", tableName);
		}
		catch (runtime_error&) {
			if (0 != columnList.tableid) {
				JetCloseTable(pimpl->sessionId_, columnList.tableid);
			}
			throw;
		}
	}

	EseTable::~EseTable(void) {
		pimpl->log_->info("Closing ESE table {}...", pimpl->tableName_);
		for (auto& column : pimpl->columns_) {
			delete column;
		}

		if (0 != pimpl->tableId_) {
			JetCloseTable(pimpl->sessionId_, pimpl->tableId_);
		}
		pimpl->log_->info("Closed ESE table {}.", pimpl->tableName_);
	}

	string EseTable::Impl::GetColumnName(const JET_COLUMNLIST& columnList) const {
		vector<char> columnName(JET_cbColumnMost);
		ulong actualSize = 0;
		auto retInfo = InitRetInfo();
		ThrowOnError(JetRetrieveColumn(sessionId_, columnList.tableid,
		                               columnList.columnidBaseColumnName, columnName.data(),
		                               static_cast<ulong>(columnName.size()), &actualSize, 0, &retInfo));
		return string(columnName.data());
	}

	JET_COLTYP EseTable::Impl::GetColumnType(const JET_COLUMNLIST& columnList) const {
		JET_COLTYP colType = 0;
		ulong actualSize = 0;
		auto retInfo = InitRetInfo();
		ThrowOnError(JetRetrieveColumn(sessionId_, columnList.tableid,
		                               columnList.columnidcoltyp, &colType,
		                               JET_coltypLong, &actualSize, 0, &retInfo));
		return colType;
	}

	JET_COLUMNID EseTable::Impl::GetColumnId(const JET_COLUMNLIST& columnList) const {
		JET_COLUMNID columnId = 0;
		ulong actualSize = 0;
		auto retInfo = InitRetInfo();
		ThrowOnError(JetRetrieveColumn(sessionId_, columnList.tableid,
		                               columnList.columnidcolumnid, &columnId,
		                               JET_coltypLong, &actualSize, 0, &retInfo));
		return columnId;
	}

	ushort EseTable::Impl::GetCodePage(const JET_COLUMNLIST& columnList) const {
		ushort codePage = 0;
		ulong actualSize = 0;
		auto retInfo = InitRetInfo();
		ThrowOnError(JetRetrieveColumn(sessionId_, columnList.tableid,
		                               columnList.columnidCp, &codePage,
		                               JET_coltypLong, &actualSize, 0, &retInfo));
		return codePage;
	}

	EseColumn* EseTable::Impl::GetColumnDefinition(const JET_COLUMNLIST& columnList) const {
		auto columnName = GetColumnName(columnList);
		auto colType = GetColumnType(columnList);
		auto columnId = GetColumnId(columnList);
		auto codePage = GetCodePage(columnList);
		return new EseColumn(columnId, columnName, colType, codePage != 1252);
	}

	void EseTable::MoveFirstRecord() const {
		pimpl->log_->info("Moving to the first record in table {}...", pimpl->tableName_);
		ThrowOnError(JetMove(pimpl->sessionId_, pimpl->tableId_, JET_MoveFirst, 0));
	}

	bool EseTable::MoveNextRecord() const {
		pimpl->log_->debug("Moving to the next record in table {}...", pimpl->tableName_);
		auto error = JetMove(pimpl->sessionId_, pimpl->tableId_, JET_MoveNext, 0);
		if (error == JET_errNoCurrentRecord) {
			return false;
		}

		ThrowOnError(error);
		return true;
	}

	void EseTable::Move(uint rowIndex) const {
		pimpl->log_->debug("Moving to the record index {} in table {}...", rowIndex, pimpl->tableName_);
		ThrowOnError(JetMove(pimpl->sessionId_, pimpl->tableId_, JET_MoveFirst, 0));
		ThrowOnError(JetMove(pimpl->sessionId_, pimpl->tableId_, rowIndex, 0));
	}

	vector<byte> EseTable::Impl::GetColumnData(uint columnIndex, uint itagSequence) {
		ulong actualSize = 0;
		auto retInfo = InitRetInfo(itagSequence);
		JetRetrieveColumn(sessionId_, tableId_, columns_[columnIndex]->GetId(),
		                  nullptr, 0, &actualSize, 0, &retInfo);
		if (actualSize == 0) {
			return vector<byte>{};
		}

		vector<byte> buf(actualSize);
		auto jeterr = JetRetrieveColumn(sessionId_, tableId_, columns_[columnIndex]->GetId(),
		                                buf.data(), actualSize, nullptr, 0, &retInfo);
		if (JET_errSuccess != jeterr) {
			throw runtime_error(GetJetErrorMessage(jeterr));
		}

		return buf;
	}

	int EseTable::Impl::CountColumnValue(uint columnIndex) const {
		JET_RETRIEVECOLUMN retrieveColumn{0};
		retrieveColumn.columnid = columns_[columnIndex]->GetId();
		auto jeterr = JetRetrieveColumns(sessionId_, tableId_, &retrieveColumn, 1);
		if (JET_errSuccess != jeterr && JET_wrnBufferTruncated != jeterr) {
			throw runtime_error(GetJetErrorMessage(jeterr));
		}

		return retrieveColumn.itagSequence;
	}

	uint EseTable::GetColumnCount() const {
		return static_cast<uint>(pimpl->columns_.size());
	}

	wstring EseTable::GetColumnName(uint columnIndex) const {
		auto name = pimpl->columns_[columnIndex]->GetName();
		return wstring(name.begin(), name.end());
	}

	EseColumnData* EseTable::GetColumnData(unsigned int columnIndex) const {
		auto size = pimpl->CountColumnValue(columnIndex);
		vector<vector<uchar>> v;
		for (auto i = 1; i <= size; i++) {
			auto d = pimpl->GetColumnData(columnIndex, i);
			v.push_back(d);
		}

		EseType type;
		auto rawType = pimpl->columns_[columnIndex]->GetType();
		switch (rawType) {
		case JET_coltypNil:	type = EseType::Nil; break;
		case JET_coltypBit: type = EseType::Bit; break;
		case JET_coltypUnsignedByte: type = EseType::UnsignedByte; break;
		case JET_coltypShort: type = EseType::Short; break;
		case JET_coltypLong: type = EseType::Long; break;
		case JET_coltypCurrency: type = EseType::Currency; break;
		case JET_coltypIEEESingle: type = EseType::IEEESingle; break;
		case JET_coltypIEEEDouble: type = EseType::IEEEDouble; break;
		case JET_coltypDateTime:type = EseType::DateTime; break;
		case JET_coltypBinary: type = EseType::Binary; break;
		case JET_coltypLongBinary:type = EseType::LongBinary; break;
		case JET_coltypText: type = EseType::Text; break;
		case JET_coltypLongText: type = EseType::LongText; break;
		case JET_coltypUnsignedLong:type = EseType::UnsignedLong; break;
		case JET_coltypLongLong:type = EseType::LongLong; break;
		case JET_coltypGUID:type = EseType::GUID; break;
		case JET_coltypUnsignedShort:type = EseType::UnsignedShort; break;
		default:type = EseType::Nil;
		}

		return new EseColumnData(type, v, pimpl->columns_[columnIndex]->IsUnicode());
	}
}
