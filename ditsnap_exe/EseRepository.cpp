#include "StdAfx.h"
#include "EseRepository.h"
#include "../EseDataAccess/EseDataAccess.h"
#include "util.h"

using namespace Ese;

EseRepository::EseRepository() : eseInstance_(nullptr),
                               eseDatabase_(nullptr),
                               eseTable_(nullptr) {}

EseRepository::~EseRepository() {
	CleanupEse();
}

void EseRepository::RegisterTableObserver(ITableObserver* o) {
	tableObservers_.push_back(o);
}

void EseRepository::RemoveTableObserver(ITableObserver* o) {
	tableObservers_.remove(o);
}

void EseRepository::NotifyTableObservers() {
	for (auto& o : tableObservers_) {
		o->LoadEseTable();
	}
}

void EseRepository::RegisterDbObserver(IDbObserver* o) {
	tableNameObservers_.push_back(o);
}

void EseRepository::RemoveDbObserver(IDbObserver* o) {
	tableNameObservers_.remove(o);
}

void EseRepository::NotifyDbObservers() {
	for (auto& o : tableNameObservers_) {
		o->LoadEseRepository();
	}
}

void EseRepository::OpenFile(wstring path) {
	filePath_ = path;
	CleanupEse();
	eseInstance_ = new EseInstance();
	eseDatabase_ = eseInstance_->OpenDatabase(path);
	tableNames_ = eseDatabase_->GetTableNames();
	NotifyDbObservers();
}

wstring EseRepository::GetFilePath() const {
	return filePath_;
}

void EseRepository::SetTable(wstring name) {
	eseTable_ = eseDatabase_->OpenTable(name);
	currentTableName_ = name;
	NotifyTableObservers();
}

wstring EseRepository::GetCurrentTableName() const {
	return currentTableName_;
}

vector<wstring> EseRepository::GetTableNames() const {
	return tableNames_;
}

void EseRepository::MoveFirstRecord() const {
	eseTable_->MoveFirstRecord();
}

BOOL EseRepository::MoveNextRecord() const {
	return eseTable_->MoveNextRecord();
}

void EseRepository::Move(uint rowIndex) const {
	return eseTable_->Move(rowIndex);
}

wstring EseRepository::GetColumnDataAsString(uint columnIndex) const {
	auto colData = GetColumnData(columnIndex);
	return JoinString(colData->GetValuesAsString());
}

uint EseRepository::GetColumnCount() const {
	return eseTable_->GetColumnCount();
}

wstring EseRepository::GetColumnName(uint columnIndex) const {
	return eseTable_->GetColumnName(columnIndex);
}

unique_ptr<EseColumnData> EseRepository::GetColumnData(uint columnIndex) const {
	return unique_ptr<EseColumnData>(eseTable_->GetColumnData(columnIndex));
}

void EseRepository::CleanupEse() {
	delete eseInstance_;
	eseInstance_ = nullptr;
	delete eseDatabase_;
	eseDatabase_ = nullptr;
	delete eseTable_;
	eseTable_ = nullptr;
}
