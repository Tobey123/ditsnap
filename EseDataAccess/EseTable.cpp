#include "stdafx.h"
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
		std::shared_ptr<spdlog::logger> log_;

		DISALLOW_COPY_AND_ASSIGN(EseTable::Impl);
	};

	EseTable::EseTable(const EseDatabase* eseDatabase, string tableName) : pimpl(new Impl) {
		pimpl->eseDatabase_ = eseDatabase;
		pimpl->sessionId_ = eseDatabase->GetEseInstance()->GetSessionId();
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

	int EseTable::CountColumnValue(uint columnIndex) const {
		JET_RETRIEVECOLUMN retrieveColumn{0};
		retrieveColumn.columnid = pimpl->columns_[columnIndex]->GetId();
		auto jeterr = JetRetrieveColumns(pimpl->sessionId_, pimpl->tableId_, &retrieveColumn, 1);
		if (JET_errSuccess != jeterr && JET_wrnBufferTruncated != jeterr) {
			throw runtime_error(GetJetErrorMessage(jeterr));
		}

		return retrieveColumn.itagSequence;
	}

	wstring EseTable::RetrieveColumnDataAsString(uint columnIndex, uint itagSequence) const {
		pimpl->log_->debug("Retrieving column data at index {}, itag {}...", columnIndex, itagSequence);
		auto v = pimpl->GetColumnData(columnIndex, itagSequence);
		if (v.size() == 0) {
			return wstring(L"");
		}

		switch (pimpl->columns_[columnIndex]->GetType()) {
		case JET_coltypNil:
			return wstring(L"");

		case JET_coltypBit: /* True or False, Never NULL */
			return to_wstring(*reinterpret_cast<int*>(v.data()));

		case JET_coltypUnsignedByte: /* 1-byte integer, unsigned */
			return to_wstring(*reinterpret_cast<uint*>(v.data()));

		case JET_coltypShort: /* 2-byte integer, signed */
			return to_wstring(*reinterpret_cast<int*>(v.data()));

		case JET_coltypLong: /* 4-byte integer, signed */
			return to_wstring(*reinterpret_cast<long*>(v.data()));

		case JET_coltypCurrency: /* 8 byte integer, signed */
			return to_wstring(*reinterpret_cast<long long int*>(v.data()));

		case JET_coltypIEEESingle: /* 4-byte IEEE single precision */
			return to_wstring(*reinterpret_cast<float*>(v.data()));

		case JET_coltypIEEEDouble: /* 8-byte IEEE double precision */
			return to_wstring(*reinterpret_cast<double*>(v.data()));

		case JET_coltypDateTime:
			{/* This column type is identical to the variant date type.*/
				SYSTEMTIME st{};
				VariantTimeToSystemTime(*reinterpret_cast<double*>(v.data()), &st);
				std::wstringstream ss;
				ss << st.wYear << L"-"
					<< std::setw(2) << std::setfill(L'0') << st.wMonth << L"-"
					<< std::setw(2) << std::setfill(L'0') << st.wDay << L" "
					<< std::setw(2) << std::setfill(L'0') << st.wHour << L":"
					<< std::setw(2) << std::setfill(L'0') << st.wMinute << L":"
					<< std::setw(2) << std::setfill(L'0') << st.wSecond << L"."
					<< std::setw(3) << std::setfill(L'0') << st.wMilliseconds;
				return ss.str();
			}
		case JET_coltypBinary: /* Binary data, < 255 bytes */
		case JET_coltypLongBinary: /* Binary data, long value */
			{
				std::wstringstream ss;
				for (auto& c : v) {
					ss << std::setfill(L'0') << std::setw(2);
					ss << std::uppercase << std::hex << static_cast<uchar>(c) << L" ";
				}
				return ss.str();
			}
		case JET_coltypText: /* ANSI text, case insensitive, < 255 bytes */
		case JET_coltypLongText: /* ANSI text, long value  */
			{
				if (pimpl->columns_[columnIndex]->IsUnicode()) {
					// Ensure L'\0' terminated
					v.push_back(0);
					v.push_back(0);
					return wstring{reinterpret_cast<wchar_t*>(v.data())};
				}
				return wstring(v.begin(), v.end());
			}
		case JET_coltypUnsignedLong:
			return to_wstring(*reinterpret_cast<ulong*>(v.data()));
		case JET_coltypLongLong:
			return to_wstring(*reinterpret_cast<long long int*>(v.data()));
		case JET_coltypGUID:
			return wstring(L"(GUID type)");
		case JET_coltypUnsignedShort:
			return to_wstring(*reinterpret_cast<ushort*>(v.data()));
		default:
			return wstring(L"unknown type");
		}
	}

	uint EseTable::GetColumnCount() const {
		return static_cast<uint>(pimpl->columns_.size());
	}

	wstring EseTable::GetColumnName(uint columnIndex) const {
		auto name = pimpl->columns_[columnIndex]->GetName();
		return wstring(name.begin(), name.end());
	}

	EseColumnData* EseTable::GetColumnData(unsigned int columnIndex) const {
		auto size = CountColumnValue(columnIndex);
		vector<vector<unsigned char>> v(size);
		for (auto i = 0; i < size; i++) {
			auto d = pimpl->GetColumnData(columnIndex, i);
			v.push_back(d);
		}

		EseType type;
		switch (pimpl->columns_[columnIndex]->GetType()) {
		case JET_coltypNil:	type = EseType::Nil;
		case JET_coltypBit: type = EseType::Bit;
		case JET_coltypUnsignedByte: type = EseType::UnsignedByte;
		case JET_coltypShort: type = EseType::Short;
		case JET_coltypLong: type = EseType::Long;
		case JET_coltypCurrency: type = EseType::Currency;
		case JET_coltypIEEESingle: type = EseType::IEEESingle;
		case JET_coltypIEEEDouble: type = EseType::IEEEDouble;
		case JET_coltypDateTime:type = EseType::DateTime;
		case JET_coltypBinary: type = EseType::Binary;
		case JET_coltypLongBinary:type = EseType::LongBinary;
		case JET_coltypText: type = EseType::Text;
		case JET_coltypLongText: type = EseType::LongText;
		case JET_coltypUnsignedLong:type = EseType::UnsignedLong;
		case JET_coltypLongLong:type = EseType::LongLong;
		case JET_coltypGUID:type = EseType::GUID;
		case JET_coltypUnsignedShort:type = EseType::UnsignedShort;
		default:type = EseType::Nil;
		}

		return new EseColumnData(type, v, pimpl->columns_[columnIndex]->IsUnicode());
	}
}
