#pragma once
#include "Interfaces.h"

class EseRepository;
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

	TableListView(EseRepository* eseRepository);
	~TableListView();

	LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
	LRESULT OnListDoubleClick(LPNMHDR pnmh);
	void LoadTable();
	void LoadDatatable();
	void FilterTable(int filterFlag);
	wstring GetAdNameFromColumnName(wstring columnName);
	int GetColumnIdFromColumnName(wstring columnName);
	wstring GetRdnFromDnt(int dnt);
	void LoadEseTable() override;
	void LoadEseRepository() override;

private:
	DetailDialog* detailDialog_;
	map<wstring, int> columnMap_;
	map<wstring, wstring> adNameMap_;
	map<int, int> listItemIdToEseRowIndex_;
	map<int, wstring> dntRdnMap_;
	EseRepository* eseRepository_;

	wstring GetColumnData(wstring columnName);
	void CleanupTable();
	void CleanupDetailDialog();
	void InsertColumnHelper(int nCol, wstring ATT, int nWidth = 200);
	bool MapColumnNameToColumnIndex(map<wstring, int>* columnMap) const;
	void MapColumnNameToAdName(map<wstring, wstring>* pAdNameMap) const;

	static constexpr const char* PROGRAM_NAME = "Ditsnap";
	static constexpr const wchar_t* NOT_SET = L"<not set>";
	static constexpr const wchar_t* DATATABLE = L"datatable";
};
