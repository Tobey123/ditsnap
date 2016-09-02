#include "stdafx.h"
#include "EseDataAccess.h"
#include "atlbase.h"

namespace EseDataAccess
{
	string GetJetErrorMessage(JET_ERR err)
	{
		auto bufsize = 512;
		vector<char> buf(bufsize);
		auto r = JetGetSystemParameter(NULL, JET_sesidNil, JET_paramErrorToString,
			reinterpret_cast<ULONG_PTR *>(&err), buf.data(), bufsize);
		if (r == JET_errSuccess)
		{
			return string(buf.data());
		}

		return string("Unknown Error.");
	}

	EseInstance::EseInstance() : jetInstance_(0), sessionId_(0), pageSize_(DEFAULT_ESE_PAGE_SIZE)
	{
	}

	EseInstance::EseInstance(uint pageSize) : jetInstance_(0), sessionId_(0), pageSize_(pageSize)
	{
	}

	EseInstance::~EseInstance()
	{
		if (sessionId_ != 0)
		{
			JetEndSession(sessionId_, 0);
		}

		if (jetInstance_ != 0)
		{
			JetTerm(jetInstance_);
		}
	}

	EseInstance* EseInstance::CreateInstance(uint pageSize)
	{
		auto eseInstance = new EseInstance(pageSize);
		eseInstance->Init();
		return eseInstance;
	}

	void EseInstance::Init()
	{
		auto instanceName = "ditsnap";
		ThrowOnError(JetSetSystemParameter(&jetInstance_, 0,
		                                   JET_paramDatabasePageSize, pageSize_, nullptr));
		ThrowOnError(JetCreateInstance(&jetInstance_, instanceName));
		ThrowOnError(JetInit(&jetInstance_));
		ThrowOnError(JetBeginSession(jetInstance_, &sessionId_, nullptr, nullptr));
	}

	EseDatabase* EseInstance::OpenDatabase(const wstring dbPath) const
	{
		auto db = new EseDatabase(this, string(CW2A(dbPath.c_str())));
		db->Init();
		return db;
	}

	EseDatabase::EseDatabase(const EseInstance* const eseInstance, const string& dbPath)
		: eseInstance_(eseInstance), sessionId_(eseInstance->GetSessionId()),
		  dbId_(0), dbPath_(dbPath), tableCount_(-1)
	{
	}

	EseDatabase::~EseDatabase(void)
	{
		if (dbId_ != 0)
		{
			JetCloseDatabase(sessionId_, dbId_, 0);
		}

		JetDetachDatabase(sessionId_, dbPath_.c_str());
	}

	void EseDatabase::Init()
	{
		ThrowOnError(JetAttachDatabase(sessionId_,
		                               dbPath_.c_str(), JET_bitDbReadOnly));
		ThrowOnError(JetOpenDatabase(sessionId_, dbPath_.c_str(), nullptr,
		                             &dbId_, JET_bitDbReadOnly));
	}

	EseTable* EseDatabase::OpenTable(const wstring tableName) const
	{
		auto table = new EseTable(this, string(CW2A(tableName.c_str())));
		table->Init();
		return table;
	}

	vector<wstring> EseDatabase::GetTableNames()
	{
		//Get a temporary table which contains all table names.
		JET_OBJECTLIST tableList = {0};
		ThrowOnError(JetGetObjectInfo(sessionId_, dbId_, JET_objtypTable,
		                              nullptr, nullptr, &tableList, sizeof(JET_OBJECTLIST), JET_ObjInfoList));
		tableCount_ = tableList.cRecord;
		vector<wstring> tableNames(tableList.cRecord);
		for (auto& tableName : tableNames)
		{
			unsigned long actualSize = 0;
			JET_RETINFO retInfo = {0};
			retInfo.cbStruct = sizeof(JET_RETINFO);
			retInfo.itagSequence = 1;
			vector<char> tableNameBuffer(JET_cbNameMost + 1);
			try
			{
				ThrowOnError(JetRetrieveColumn(sessionId_, tableList.tableid,
				                               tableList.columnidobjectname, tableNameBuffer.data(), JET_cbNameMost, &actualSize, 0, &retInfo));
				tableNameBuffer[actualSize] = NULL;
				tableName = wstring(tableNameBuffer.begin(), tableNameBuffer.end());
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

	EseTable::EseTable(const EseDatabase* const eseDatabase, const string& tableName)
		: eseDatabase_(eseDatabase), sessionId_(eseDatabase->GetEseInstance()->GetSessionId()),
		  dbId_(eseDatabase->GetDbId()), tableId_(0), tableName_(tableName)
	{
	}

	EseTable::~EseTable(void)
	{
		for (auto& column : columns_)
		{
			delete column;
		}

		if (0 != tableId_)
		{
			JetCloseTable(sessionId_, tableId_);
		}
	}

	void EseTable::Init()
	{
		JET_COLUMNLIST columnList{0};
		try
		{
			ThrowOnError(JetOpenTable(sessionId_, dbId_, tableName_.c_str(), nullptr, 0,
			                          JET_bitTableReadOnly, &tableId_));
			// Open a temporary table that contains column definitions
			ThrowOnError(JetGetTableColumnInfo(sessionId_, tableId_, nullptr,
			                                   &columnList, sizeof(JET_COLUMNLIST), JET_ColInfoList));
			columns_.reserve(columnList.cRecord);
			ThrowOnError(JetMove(sessionId_, columnList.tableid, JET_MoveFirst, 0));
			JET_ERR ret;
			do
			{
				columns_.push_back(RetrieveColumnDefinition(columnList));
			}
			while (JET_errSuccess == (ret = JetMove(sessionId_, columnList.tableid, JET_MoveNext, 0)));

			//if cursor don't reach to the end of records, throw exception
			if (ret != JET_errNoCurrentRecord)
			{
				throw runtime_error(GetJetErrorMessage(ret));
			}

			// close the temporary table 
			JetCloseTable(sessionId_, columnList.tableid);
		}
		catch (runtime_error&)
		{
			if (0 != columnList.tableid)
			{
				JetCloseTable(sessionId_, columnList.tableid);
			}
			throw;
		}
	}

	void EseTable::RetrieveColumnName(const JET_COLUMNLIST& columnList, vector<char>& columnName) const
	{
		unsigned long actualSize = 0;
		JET_RETINFO retInfo = { 0 };
		retInfo.cbStruct = sizeof(JET_RETINFO);
		retInfo.itagSequence = 1;
		ThrowOnError(JetRetrieveColumn(sessionId_, columnList.tableid,
		                               columnList.columnidBaseColumnName, columnName.data(),
		                               columnName.size(), &actualSize, 0, &retInfo));
	}

	void EseTable::RetrieveColumnType(const JET_COLUMNLIST& columnList, JET_COLTYP* colType) const
	{
		unsigned long actualSize = 0;
		JET_RETINFO retInfo = { 0 };
		retInfo.cbStruct = sizeof(JET_RETINFO);
		retInfo.itagSequence = 1;
		ThrowOnError(JetRetrieveColumn(sessionId_, columnList.tableid,
		                               columnList.columnidcoltyp, colType,
		                               JET_coltypLong, &actualSize, 0, &retInfo));
	}

	void EseTable::RetrieveColumnId(const JET_COLUMNLIST& columnList, JET_COLUMNID* columnId) const
	{
		unsigned long actualSize = 0;
		JET_RETINFO retInfo = { 0 };
		retInfo.cbStruct = sizeof(JET_RETINFO);
		retInfo.itagSequence = 1;
		ThrowOnError(JetRetrieveColumn(sessionId_, columnList.tableid,
		                               columnList.columnidcolumnid, columnId,
		                               JET_coltypLong, &actualSize, 0, &retInfo));
	}

	void EseTable::RetrieveCodePage(const JET_COLUMNLIST& columnList, unsigned short* codePage) const
	{
		unsigned long actualSize = 0;
		JET_RETINFO retInfo = { 0 };
		retInfo.cbStruct = sizeof(JET_RETINFO);
		retInfo.itagSequence = 1;
		ThrowOnError(JetRetrieveColumn(sessionId_, columnList.tableid,
		                               columnList.columnidCp, codePage,
		                               JET_coltypLong, &actualSize, 0, &retInfo));
	}

	EseColumn* EseTable::RetrieveColumnDefinition(const JET_COLUMNLIST& columnList) const
	{
		vector<char> columnName(JET_cbColumnMost);
		RetrieveColumnName(columnList, columnName);
		JET_COLTYP colType = 0;
		RetrieveColumnType(columnList, &colType);
		JET_COLUMNID columnId = 0;
		RetrieveColumnId(columnList, &columnId);
		unsigned short codePage = 0;
		RetrieveCodePage(columnList, &codePage);
		return new EseColumn(columnId, string(columnName.data()), colType, codePage != 1252);
	}

	void EseTable::MoveFirstRecord() const
	{
		ThrowOnError(JetMove(sessionId_, tableId_, JET_MoveFirst, 0));
	}

	bool EseTable::MoveNextRecord() const
	{
		auto error = JetMove(sessionId_, tableId_, JET_MoveNext, 0);
		if (error == JET_errNoCurrentRecord)
		{
			return false;
		}

		ThrowOnError(error);
		return true;
	}

	void EseTable::Move(uint rowIndex) const
	{
		ThrowOnError(JetMove(sessionId_, tableId_, JET_MoveFirst, 0));
		ThrowOnError(JetMove(sessionId_, tableId_, rowIndex, 0));
	}

	vector<char> EseTable::RetrieveColumnData(uint columnIndex, uint itagSequence)
	{
		unsigned long actualSize = 0;
		JET_RETINFO retInfo{0};
		retInfo.cbStruct = sizeof(JET_RETINFO);
		retInfo.itagSequence = itagSequence;
		JetRetrieveColumn(sessionId_, tableId_, columns_[columnIndex]->GetId(),
		                  nullptr, 0, &actualSize, 0, &retInfo);
		if (actualSize == 0)
		{
			return vector<char>{};
		}

		vector<char> buf(actualSize);
		auto jeterr = JetRetrieveColumn(sessionId_, tableId_, columns_[columnIndex]->GetId(),
		                                buf.data(), actualSize, nullptr, 0, &retInfo);
		if (JET_errSuccess != jeterr)
		{
			throw runtime_error(GetJetErrorMessage(jeterr));
		}

		return buf;
	}

	int EseTable::CountColumnValue(uint columnIndex) const
	{
		JET_RETRIEVECOLUMN retrieveColumn = {0};
		retrieveColumn.columnid = columns_[columnIndex]->GetId();
		auto jeterr = JetRetrieveColumns(sessionId_, tableId_, &retrieveColumn, 1);
		if (JET_errSuccess != jeterr && JET_wrnBufferTruncated != jeterr)
		{
			throw runtime_error(GetJetErrorMessage(jeterr));
		}

		return retrieveColumn.itagSequence;
	}

	wstring EseTable::RetrieveColumnDataAsString(uint columnIndex, uint itagSequence)
	{
		auto v = RetrieveColumnData(columnIndex, itagSequence);
		if (v.size() == 0)
		{
			return wstring(L"");
		}

		switch (columns_[columnIndex]->GetType())
		{
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
				for (auto& c : v)
				{
					ss << std::setfill(L'0') << std::setw(2);
					ss << std::uppercase << std::hex << static_cast<unsigned char>(c) << L" ";
				}
				return ss.str();
			}
		case JET_coltypText: /* ANSI text, case insensitive, < 255 bytes */
		case JET_coltypLongText: /* ANSI text, long value  */
			{
				if (columns_[columnIndex]->IsUnicode())
				{
					// Ensure L'\0' terminated
					v.push_back(0);
					v.push_back(0);
					return wstring{reinterpret_cast<wchar_t*>(v.data())};
				}
				return wstring(v.begin(), v.end());
			}
		case JET_coltypUnsignedLong:
			return to_wstring(*reinterpret_cast<unsigned long long int*>(v.data()));
		case JET_coltypLongLong:
			return to_wstring(*reinterpret_cast<long long int*>(v.data()));
		case JET_coltypGUID:
			return wstring(L"(GUID type)");
		case JET_coltypUnsignedShort:
			return to_wstring(*reinterpret_cast<unsigned short*>(v.data()));
		default:
			return wstring(L"unknown type");
		}
	}

	uint EseTable::GetColumnCount() const
	{
		return static_cast<uint>(columns_.size());
	}

	wstring EseTable::GetColumnName(uint columnIndex) const
	{
		auto name = columns_[columnIndex]->GetName();
		return wstring(name.begin(), name.end());
	}

	EseColumn::EseColumn(uint id, const string& name, uint type, bool isUnicode) :
		id_(id), name_(name), type_(type), isUnicode_(isUnicode)
	{
	}
} // name space EseDataAccess
