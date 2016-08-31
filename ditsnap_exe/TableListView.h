#pragma once
#include "Interfaces.h"
#include "EseDbManager.h"

class DetailDialog;

class TableListView : public CWindowImpl<TableListView, CListViewCtrl>,
                       ITableObserver, IDbObserver
{
public:
	enum
	{
		CLASSSCHEMA = 0x00000001,
		ATTRIBUTESCHEMA = 0x00000002,
		SUBSCHEMA = 0x00000004,
		DISPLAYSPECIFIER = 0x00000008,
		OTHERS = 0x00000010,
	};

	DECLARE_WND_SUPERCLASS(nullptr, CListViewCtrl::GetWndClassName())

	BEGIN_MSG_MAP_EX(TableListView)
		MSG_WM_CREATE(OnCreate)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_DBLCLK, OnListDoubleClick)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	TableListView(EseDbManager* eseDbManager);
	~TableListView();

	LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
	LRESULT OnListDoubleClick(LPNMHDR pnmh);
	void LoadTable();
	void LoadDatatable();
	void FilterTable(int filterFlag);
	wstring GetAdNameFromColumnName(wstring columnName);
	int GetColumnIdFromColumnName(wstring columnName);
	virtual void LoadEseTable() override;
	virtual void LoadEseDbManager() override;

private:
	DetailDialog* detailDialog_;
	map<wstring, int> columnMap_;
	map<wstring, wstring> adNameMap_;
	map<int, int> listItemIdToEseRowIndex_;
	EseDbManager* eseDbManager_;

	wstring RetrieveColumnData(wstring columnName);
	void CleanupTable();
	void CleanupDetailDialog();
	void InsertColumnHelper(int nCol, wstring ATT, int nWidth = 200);
	void AddItemHelper(int nItem, int nSubItem, wstring ATT);
	bool MapColumnNameToColumnIndex(map<wstring, int>* columnMap) const;
	void MapColumnNameToAdName(map<wstring, wstring>* pAdNameMap) const;

	static constexpr const char* PROGRAM_NAME = "Ditsnap";
	static constexpr const wchar_t* NOT_SET = L"<not set>";
	static constexpr const wchar_t* DATATABLE = L"datatable";
};
