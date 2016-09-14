#pragma once
#include "stdafx.h"

namespace EseDataAccess
{
	class EseInstance;
	class EseDatabase;
	class EseTable;
	class EseColumn;

	class EseInstance
	{
	public:
		explicit EseInstance(uint pageSize = DEFAULT_ESE_PAGE_SIZE);
		~EseInstance();
		EseDatabase* OpenDatabase(wstring dbPath) const;
		JET_SESID GetSessionId() const;
		JET_INSTANCE GetJetInstance() const;
		const static uint DEFAULT_ESE_PAGE_SIZE = 8 * 1024;

	private:		
		class Impl;
		unique_ptr<Impl> pimpl;

		DISALLOW_COPY_AND_ASSIGN(EseInstance);
	};

	class EseDatabase
	{
	public:
		EseDatabase(const EseInstance* const parent, string dbPath);
		~EseDatabase();
		EseTable* OpenTable(wstring tableName) const;
		vector<wstring> GetTableNames() const;
		const EseInstance* GetEseInstance() const;
		JET_DBID GetDbId() const;

	private:
		class Impl;
		unique_ptr<Impl> pimpl;

		DISALLOW_COPY_AND_ASSIGN(EseDatabase);
	};

	class EseTable
	{
	public:
		EseTable(const EseDatabase* const eseDatabase, string tableName);
		~EseTable();
		void MoveFirstRecord() const;
		bool MoveNextRecord() const;
		void Move(uint rowIndex) const;
		int CountColumnValue(uint columnIndex) const;
		wstring RetrieveColumnDataAsString(uint columnIndex, uint itagSequence = 1);
		uint GetColumnCount() const;
		wstring GetColumnName(uint columnIndex) const;

	private:
		class Impl;
		unique_ptr<Impl> pimpl;

		DISALLOW_COPY_AND_ASSIGN(EseTable);
	};

	class EseColumn
	{
	public:
		EseColumn(uint id, string name, uint type, bool isUnicode);
		~EseColumn();
		uint GetId() const;
		string GetName() const;
		uint GetType() const;
		bool IsUnicode() const;

	private:
		class Impl;
		unique_ptr<Impl> pimpl;

		DISALLOW_COPY_AND_ASSIGN(EseColumn);
	};	
} // name space EseDataAccess

