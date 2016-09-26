#include "stdafx.h"
#include "DetailDialog.h"
#include "TableListView.h"
#include "util.h"
#include "EseRepository.h"
#include "../EseDataAccess/EseDataAccess.h"

using namespace Ese;

DetailDialog::DetailDialog(EseRepository* eseRepository,
                           TableListView* parent,
                           int rowIndex) : m_bMsgHandled(0), eseRepository_(eseRepository), parent_(parent), rowIndex_(rowIndex) {}

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
	detailListView_.InsertColumn(0, L"Name", LVCFMT_LEFT, 100);
	detailListView_.InsertColumn(1, L"Description", LVCFMT_LEFT, 200);
	detailListView_.InsertColumn(2, L"Type", LVCFMT_LEFT, 100);
	detailListView_.InsertColumn(3, L"Value", LVCFMT_LEFT, 200);
	detailListView_.InsertColumn(4, L"Intepreted Value", LVCFMT_LEFT, 200);
	checkBox_.SetCheck(1);

	try {
		eseRepository_->Move(rowIndex_);
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
		auto rdn = eseRepository_->GetColumnDataAsString(RDN);
		rdnLabel.SetWindowTextW(rdn.c_str());
	}
	catch (runtime_error& e) {
		ShowMessageBox(e.what());
	}
}

void DetailDialog::SetupListItems() {
	try {
		detailListView_.DeleteAllItems();
		auto filterNoValue = !!checkBox_.GetCheck();
		auto visibleColumnIndex = 0;
		auto nColumn = eseRepository_->GetColumnCount();
		for (uint columnIndex = 0; columnIndex < nColumn; ++columnIndex) {
			auto columnName = eseRepository_->GetColumnName(columnIndex);
			auto adName = parent_->GetAdNameFromColumnName(columnName);
			auto colData = eseRepository_->GetColumnData(columnIndex);
			auto value = JoinString(colData->GetValuesAsString());
			auto type = colData->GetColumnTypeString();
			auto interpreted = Interpret(colData.get(), adName);
			if (0 == value.size()) {
				if (!filterNoValue) {
					AddRow(visibleColumnIndex, columnName, adName, type, L"<not set>", interpreted);
					++visibleColumnIndex;
				}
			}
			else {
				AddRow(visibleColumnIndex, columnName, adName, type, value, interpreted);
				++visibleColumnIndex;
			}
		}
	}
	catch (runtime_error& e) {
		ShowMessageBox(e.what());
	}
}

void DetailDialog::AddRow(int index, wstring name, wstring desc, wstring type, wstring value, wstring intepreted) {
	detailListView_.AddItem(index, 0, name.c_str());
	detailListView_.AddItem(index, 1, desc.c_str());
	detailListView_.AddItem(index, 2, type.c_str());
	detailListView_.AddItem(index, 3, value.c_str());
	detailListView_.AddItem(index, 4, intepreted.c_str());
}

wstring DetailDialog::Interpret(EseColumnData* colData, wstring adName) const {
	auto shortFtType = vector<wstring>{ L"WHEN_CREATED", L"WHEN_CHANGED" };
	auto ftType = vector<wstring>{ L"PWD_LAST_SET", L"LAST_LOGON", L"LAST_LOGOFF", L"ACCOUNT_EXPIRES" };
	auto interpreted = wstring(L"");
	auto vd = colData->GetValues();
	if (vd.size() == 0) {
		return interpreted;
	}

	if (find(shortFtType.begin(), shortFtType.end(), adName) != shortFtType.end()) {
		auto ll = *reinterpret_cast<long long*>(vd[0].data());
		interpreted = FileTimeToString(ll * 10000000);
	}
	else if (find(ftType.begin(), ftType.end(), adName) != ftType.end()) {
		auto ll = *reinterpret_cast<long long*>(vd[0].data());
		interpreted = FileTimeToString(ll);
	} else if (adName.find(L"GUID") != string::npos) {
		for (auto& d : vd) {
			auto guidString = BytesToGuidString(d);
			interpreted += guidString;
		}
	} else if (adName == L"USER_ACCOUNT_CONTROL") {
		auto i = *reinterpret_cast<int*>(vd[0].data());
		interpreted = GetUserFlagString(i);
	} else if (adName == L"SAM_ACCOUNT_TYPE") {
		auto i = *reinterpret_cast<int*>(vd[0].data());
		interpreted = GetSamAccountTypeString(i);
	} else if (adName == L"SYSTEM_FLAGS") {
		auto i = *reinterpret_cast<int*>(vd[0].data());
		interpreted = GetSystemFlagString(i);
	} else if (adName == L"SEARCH_FLAGS") {
		auto i = *reinterpret_cast<int*>(vd[0].data());
		interpreted = GetSearchFlagString(i);
	} else if (adName == L"OBJECT_CATEGORY") {
		auto i = *reinterpret_cast<int*>(vd[0].data());
		interpreted = parent_->GetRdnFromDnt(i);
	} else if (adName == L"OBJECT_CLASS") {
		vector<wstring> classes;
		for (auto& iter : vd) {
			auto i = *reinterpret_cast<int*>(iter.data());
			classes.push_back(parent_->GetRdnFromGovernId(i));
		}
		interpreted = JoinString(classes);
	}

	return interpreted;
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
