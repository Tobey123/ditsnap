#include "stdafx.h"
#include "DetailDialog.h"
#include "TableListView.h"
#include "util.h"
#include "EseDbManager.h"

using namespace Ese;

DetailDialog::DetailDialog(EseDbManager* eseDbManager,
                           TableListView* parent,
                           int rowIndex) : m_bMsgHandled(0), eseDbManager_(eseDbManager), parent_(parent), rowIndex_(rowIndex) {}

DetailDialog::~DetailDialog() {}

LRESULT DetailDialog::OnInitDialog(HWND hWnd, LPARAM lParam) {
	CenterWindow();
	auto hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR,
	                                           GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);
	auto hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR,
	                                                GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);
	detailListView_ = GetDlgItem(IDC_LIST1);
	checkBox_ = GetDlgItem(IDC_CHECK1);
	detailListView_.SetExtendedListViewStyle(LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT);
	CRect rcList;
	detailListView_.GetWindowRect(rcList);
	auto nScrollWidth = GetSystemMetrics(SM_CXVSCROLL);
	auto n3DEdge = GetSystemMetrics(SM_CXEDGE);
	detailListView_.InsertColumn(0, L"Name", LVCFMT_LEFT, 100, -1);
	detailListView_.InsertColumn(1, L"Description", LVCFMT_LEFT, 200, -1);
	detailListView_.InsertColumn(2, L"Value", LVCFMT_LEFT,
	                             rcList.Width() - 300 - nScrollWidth - n3DEdge * 2, -1);
	checkBox_.SetCheck(1);

	try {
		eseDbManager_->Move(rowIndex_);
	}
	catch (runtime_error& e) {
		ShowMessageBox(e.what());
	}

	SetupTopLabel();
	SetupListItems();
	return TRUE;
}

void DetailDialog::OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl) {
	DestroyWindow();
}

void DetailDialog::OnShowAllCheckBoxToggled(UINT uNotifyCode, int nID, CWindow wndCtl) {
	SetupListItems();
}

void DetailDialog::SetupTopLabel() const {
	auto RDN = parent_->GetColumnIdFromColumnName(L"ATTm589825");
	auto rdnLabel = GetDlgItem(IDC_RDN);

	try {
		auto rdn = eseDbManager_->RetrieveColumnDataAsString(RDN);
		rdnLabel.SetWindowTextW(rdn.c_str());
	}
	catch (runtime_error& e) {
		ShowMessageBox(e.what());
	}
}

void DetailDialog::SetupListItems() {
	try {
		detailListView_.DeleteAllItems();
		auto filterNoValue = !!(checkBox_.GetCheck());
		auto visibleColumnIndex = 0;
		auto nColumn = eseDbManager_->GetColumnCount();
		for (uint columnIndex = 0; columnIndex < nColumn; ++columnIndex) {
			auto columnValues = GetColumnValueString(columnIndex);
			auto columnName = eseDbManager_->GetColumnName(columnIndex);
			auto adName = parent_->GetAdNameFromColumnName(columnName);
			if (0 == columnValues.size()) {
				if (!filterNoValue) {
					AddRow(visibleColumnIndex, columnName, adName, L"<not set>");
					++visibleColumnIndex;
				}
			}
			else {
				AddRow(visibleColumnIndex, columnName, adName, columnValues);
				++visibleColumnIndex;
			}
		}
	}
	catch (runtime_error& e) {
		ShowMessageBox(e.what());
	}
}

void DetailDialog::AddRow(int index, wstring col1, wstring col2, wstring col3) {
	detailListView_.AddItem(index, 0, col1.c_str());
	detailListView_.AddItem(index, 1, col2.c_str());
	detailListView_.AddItem(index, 2, col3.c_str());
}

wstring DetailDialog::GetColumnValueString(uint columnIndex) const {
	wstring columnValues;
	auto numberOfColumnValue = eseDbManager_->CountColumnValue(columnIndex);
	for (auto itagSequence = 1; itagSequence <= numberOfColumnValue; ++itagSequence) {
		auto columnValue = eseDbManager_->RetrieveColumnDataAsString(columnIndex, itagSequence);
		columnValues += columnValue;
		if (numberOfColumnValue != itagSequence) {
			columnValues += L"; ";
		}
	}

	return columnValues;
}

LRESULT DetailDialog::OnCopyAllButtonClicked(UINT uNotifyCode, int nID, CWindow wndCtl) {
	CString copyText;
	for (auto i = 0; i < detailListView_.GetItemCount(); ++i) {
		CString s;
		detailListView_.GetItemText(i, 0, s);
		copyText.Append(s);

		CString temp;
		detailListView_.GetItemText(i, 1, temp);
		if (temp.IsEmpty()) {
			s.Format(L": ");
		}
		else {
			s.Format(L" ( %s ): ", static_cast<const wchar_t*>(temp));
		}
		copyText.Append(s);

		detailListView_.GetItemText(i, 2, s);
		copyText.Append(s);
		copyText.Append(L"\r\n");
	}

	if (!OpenClipboard()) {
		ShowMessageBox(L"Cannot open clipboard.");
		return -1;
	}
	int bufSize = (copyText.GetLength() + 1) * sizeof(wchar_t);
	auto hBuf = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, bufSize);
	auto pBuf = static_cast<wchar_t*>(GlobalLock(hBuf));
	memcpy(pBuf, static_cast<LPCTSTR>(copyText), bufSize);
	GlobalUnlock(hBuf);

	if (!EmptyClipboard()) {
		ShowMessageBox(L"Cannot empty clipboard.");
		return -1;
	}

	if (nullptr == SetClipboardData(CF_UNICODETEXT, hBuf)) {
		CloseClipboard();
		return 1;
	}
	CloseClipboard();

	return 0;
}
