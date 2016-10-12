#pragma once
#include "Interfaces.h"

// Defined in EseDataAccess.h 
namespace Ese
{
	class EseTable;
	class EseDatabase;
	class EseInstance;
	class EseColumnData;
}

class EseRepository :public ITableObservable, IDbObservable
{
public:
	EseRepository();
	~EseRepository();

	void RegisterTableObserver(ITableObserver* o) override;
	void RemoveTableObserver(ITableObserver* o) override;
	void NotifyTableObservers() override;
	void RegisterDbObserver(IDbObserver* o) override;
	void RemoveDbObserver(IDbObserver* o) override;
	void NotifyDbObservers() override;
	void OpenFile(wstring path);
	wstring GetFilePath() const;
	void SetTable(wstring name);
	wstring GetCurrentTableName() const;
	vector<wstring> GetTableNames() const;
	void MoveFirstRecord() const;
	BOOL MoveNextRecord() const;
	void Move(uint rowIndex) const;
	wstring GetColumnDataAsString(uint columnIndex) const;
	vector<wstring> GetColumnNames() const;
	unique_ptr<Ese::EseColumnData> GetColumnData(uint columnIndex) const;

private:
	list<ITableObserver*> tableObservers_;
	list<IDbObserver*> tableNameObservers_;
	Ese::EseInstance* eseInstance_;
	Ese::EseDatabase* eseDatabase_;
	Ese::EseTable* eseTable_;
	wstring filePath_;
	vector<wstring> tableNames_;
	wstring currentTableName_;

	void CleanupEse();

	DISALLOW_COPY_AND_ASSIGN(EseRepository);
};
