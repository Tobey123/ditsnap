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
		EseDatabase* OpenDatabase(const wstring dbPath) const;
		JET_SESID GetSessionId() const;
		JET_INSTANCE GetJetInstance() const;

	private:
		const static uint DEFAULT_ESE_PAGE_SIZE = 8 * 1024;
		class Impl;
		unique_ptr<Impl> pimpl;

		DISALLOW_COPY_AND_ASSIGN(EseInstance);
	};

	class EseDatabase
	{
	public:
		EseDatabase(const EseInstance* const parent, const string& dbPath);
		~EseDatabase();
		EseTable* OpenTable(const wstring tableName) const;
		vector<wstring> GetTableNames();
		const EseInstance* GetEseInstance() const;
		JET_DBID GetDbId() const;

	private:
		const EseInstance* const eseInstance_;
		const JET_SESID sessionId_;
		JET_DBID dbId_;
		const string dbPath_;
		int tableCount_;

		DISALLOW_COPY_AND_ASSIGN(EseDatabase);
	};

	class EseTable
	{
	public:
		EseTable(const EseDatabase* const eseDatabase, const string& tableName);
		~EseTable();
		void MoveFirstRecord() const;
		bool MoveNextRecord() const;
		void Move(uint rowIndex) const;
		int CountColumnValue(uint columnIndex) const;
		wstring RetrieveColumnDataAsString(uint columnIndex, uint itagSequence = 1);
		uint GetColumnCount() const;
		wstring GetColumnName(uint columnIndex) const;

	private:
		const EseDatabase* const eseDatabase_;
		const JET_SESID sessionId_;
		const JET_DBID dbId_;
		JET_TABLEID tableId_;
		const string tableName_;
		vector<EseColumn*> columns_;
		vector<char> RetrieveColumnName(const JET_COLUMNLIST& columnList) const;
		JET_COLTYP RetrieveColumnType(const JET_COLUMNLIST& columnList) const;
		JET_COLUMNID RetrieveColumnId(const JET_COLUMNLIST& columnList) const;
		unsigned short RetrieveCodePage(const JET_COLUMNLIST& columnList) const;
		EseColumn* RetrieveColumnDefinition(const JET_COLUMNLIST& columnList) const;
		vector<char> RetrieveColumnData(uint columnIndex, uint itagSequence);

		DISALLOW_COPY_AND_ASSIGN(EseTable);
	};

	class EseColumn
	{
	public:
		EseColumn(uint id, const string& name, uint type, bool isUnicode);
		~EseColumn();
		uint GetId() const;
		string GetName() const;
		uint GetType() const;
		bool IsUnicode() const;

	private:
		uint id_;
		const string name_;
		uint type_;
		bool isUnicode_;

		DISALLOW_COPY_AND_ASSIGN(EseColumn);
	};	
} // name space EseDataAccess

