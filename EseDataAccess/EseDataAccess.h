#pragma once
#include "esent.h"
#include <string>
#include <memory>
#include <vector>

namespace Ese
{
	class EseInstance;
	class EseDatabase;
	class EseTable;
	class EseColumn;
	class EseColumnData;
	enum class EseType;

	class EseInstance
	{
	public:
		explicit EseInstance(unsigned int pageSize = DEFAULT_ESE_PAGE_SIZE);
		~EseInstance();
		EseDatabase* OpenDatabase(std::wstring dbPath) const;
		JET_SESID GetSessionId() const;
		JET_INSTANCE GetJetInstance() const;
		const static unsigned int DEFAULT_ESE_PAGE_SIZE = 8 * 1024;

	private:
		class Impl;
		std::unique_ptr<Impl> pimpl;

		DISALLOW_COPY_AND_ASSIGN(EseInstance);
	};

	class EseDatabase
	{
	public:
		EseDatabase(const EseInstance* const parent, std::string dbPath);
		~EseDatabase();
		EseTable* OpenTable(std::wstring tableName) const;
		std::vector<std::wstring> GetTableNames() const;
		const EseInstance* GetEseInstance() const;
		JET_DBID GetDbId() const;

	private:
		class Impl;
		std::unique_ptr<Impl> pimpl;

		DISALLOW_COPY_AND_ASSIGN(EseDatabase);
	};

	enum class EseType
	{
		Nil,
		Bit,
		UnsignedByte,
		Short,
		Long,
		Currency,
		IEEESingle,
		IEEEDouble,
		DateTime,
		Binary,
		Text,
		LongBinary,
		LongText,
		SLV,
		UnsignedLong,
		LongLong,
		GUID,
		UnsignedShort
	};

	class EseColumnData
	{
	public:
		EseColumnData(EseType type, std::vector<std::vector<uchar>> values, bool isUnicode);
		~EseColumnData();
		EseType GetType() const;
		std::wstring GetColumnTypeString() const;
		std::vector<std::vector<uchar>> GetValues() const;
		std::vector<std::wstring> GetValuesAsString() const;
	private:
		class Impl;
		std::unique_ptr<Impl> pimpl;

		DISALLOW_COPY_AND_ASSIGN(EseColumnData);
	};

	class EseTable
	{
	public:
		EseTable(const EseDatabase* const eseDatabase, std::string tableName);
		~EseTable();
		void MoveFirstRecord() const;
		bool MoveNextRecord() const;
		void Move(unsigned int rowIndex) const;
		unsigned int GetColumnCount() const;
		std::wstring GetColumnName(unsigned int columnIndex) const;
		EseColumnData* GetColumnData(unsigned int columnIndex) const;

	private:
		class Impl;
		std::unique_ptr<Impl> pimpl;

		DISALLOW_COPY_AND_ASSIGN(EseTable);
	};

	class EseColumn
	{
	public:
		EseColumn(unsigned int id, std::string name, unsigned int type, bool isUnicode);
		~EseColumn();
		unsigned int GetId() const;
		std::string GetName() const;
		unsigned int GetType() const;
		bool IsUnicode() const;

	private:
		class Impl;
		std::unique_ptr<Impl> pimpl;

		DISALLOW_COPY_AND_ASSIGN(EseColumn);
	};
} // name space Ese
