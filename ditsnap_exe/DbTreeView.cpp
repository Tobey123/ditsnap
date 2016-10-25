#include "DbTreeView.h"
#include "../EseDataAccess/EseDataAccess.h"
#include "resource.h"
#include "util.h"

DbTreeView::DbTreeView(EseRepository& eseRepository) : eseRepository_(eseRepository) {
	eseRepository_.RegisterDbObserver(this);
}

DbTreeView::~DbTreeView(void) {
	eseRepository_.RemoveDbObserver(this);
}

LRESULT DbTreeView::OnTreeDoubleClick(LPNMHDR pnmh) const {
	uint uFlag;
	CPoint pt = GetMessagePos();
	ScreenToClient(&pt);
	auto hItem = HitTest(pt, &uFlag);
	if (hItem == nullptr || !(uFlag & TVHT_ONITEM)) {
		return 0;
	}

	wchar_t tableName[1024];
	if (GetItemText(hItem, tableName, sizeof(tableName) / sizeof(tableName[0]))) {
		try {
			eseRepository_.SetTable(tableName);
		}
		catch (runtime_error& e) {
			ShowMessageBox(e.what());
		}
	}

	return 0;
}

void DbTreeView::LoadEseRepository() {
	DeleteAllItems();
	CImageList images;
	images.CreateFromImage(IDB_BITMAP1, 16, 0, RGB( 255, 0, 255 ), IMAGE_BITMAP, LR_CREATEDIBSECTION);
	SetImageList(images);
	auto hRootItem = InsertItem(eseRepository_.GetFilePath().c_str(), 0, 0, TVI_ROOT, TVI_LAST);
	if (hRootItem != nullptr) {
		SetItemData(hRootItem, reinterpret_cast<DWORD_PTR>(hRootItem));
	}

	try {
		auto tableNames = eseRepository_.GetTableNames();
		for (auto& tableName : tableNames) {
			auto hItem = InsertItem(tableName.c_str(), 1, 1, hRootItem, TVI_LAST);
			if (hItem != nullptr) {
				SetItemData(hItem, reinterpret_cast<DWORD_PTR>(hItem));
				EnsureVisible(hItem);
			}
		}
	}
	catch (runtime_error& e) {
		ShowMessageBox(e.what());
	}
}
