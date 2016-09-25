#pragma once
#include "resource.h"

namespace Ese
{
	class EseColumnData;
	class EseTable;
}

class TableListView;
class EseDbManager;

class DetailDialog : public CDialogImpl<DetailDialog>
{
public:
	enum
	{
		IDD = IDD_DETAIL_DIALOG
	};

	BEGIN_MSG_MAP_EX(DetailDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER_EX(IDC_CHECK1, OnShowAllCheckBoxToggled);
		COMMAND_HANDLER_EX(IDC_BUTTON_COPYALL, BN_CLICKED, OnCopyAllButtonClicked)
		END_MSG_MAP()

	DetailDialog(EseDbManager* eseDbManager, TableListView* paBrent, int rowIndex);
	~DetailDialog();
	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	void OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnShowAllCheckBoxToggled(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnCopyAllButtonClicked(UINT uNotifyCode, int nID, CWindow wndCtl);

private:
	EseDbManager* eseDbManager_;
	TableListView* parent_;
	CListViewCtrl detailListView_;
	CButton checkBox_;
	int rowIndex_;

	void SetupTopLabel() const;
	void SetupListItems();
	wstring GetColumnValueString(uint columnIndex) const;
	void AddRow(int index, wstring name, wstring desc, wstring type, wstring value, wstring intepreted = L"");
	wstring Interpret(Ese::EseColumnData* colData, wstring adName) const;

	DISALLOW_COPY_AND_ASSIGN(DetailDialog);
};
