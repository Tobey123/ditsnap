#include "stdafx.h"
#include "MainFrame.h"
#include "SnapshotWizard.h"
#include "AboutDlg.h"
#include "FilterDialog.h"
#include "utility.h"

using namespace EseDataAccess;

MainFrame::MainFrame(EseDbManager* eseDbManager)
	: m_bMsgHandled(0), tableListView_(TableListView(eseDbManager)),
	  dbTreeView_(DbTreeView(eseDbManager)),
	  eseDbManager_(eseDbManager)
{
}

LRESULT MainFrame::OnCreate(LPCREATESTRUCT lpcs)
{
	CreateSimpleStatusBar();
	UISetCheck(ID_VIEW_STATUS_BAR, true);
	m_hWndClient = splitter_.Create(m_hWnd, rcDefault, nullptr,
	                                WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	splitter_.SetSplitterExtendedStyle(0);
	dbTreeView_.Create(splitter_, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TVS_HASLINES |
		TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE);
	splitter_.SetSplitterPane(SPLIT_PANE_LEFT, dbTreeView_);
	tableListView_.Create(splitter_, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_REPORT |
		LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE);
	splitter_.SetSplitterPane(SPLIT_PANE_RIGHT, tableListView_);
	UpdateLayout();
	splitter_.SetSplitterPosPct(25);
	return 0;
}

void MainFrame::OnFileExit(UINT uCode, int nID, HWND hwndCtrl)
{
	PostMessage(WM_CLOSE);
}

void MainFrame::OnFileOpen(UINT uCode, int nID, HWND hwndCtrl) const
{
	auto ext = L"txt";
	auto filter = L"dit file (*.dit)\0*.dit\0all file (*.*)\0*.*\0\0";
	CFileDialog fileDialog(TRUE, ext, nullptr, OFN_HIDEREADONLY | OFN_CREATEPROMPT, filter);
	wchar_t moduleName[MAX_PATH];
	if (::GetModuleFileName(nullptr, moduleName, sizeof(moduleName)) > 0)
	{
		auto modulePath = CPath(moduleName);
		if (modulePath.RemoveFileSpec())
			fileDialog.m_ofn.lpstrInitialDir = static_cast<LPCWSTR>(modulePath.m_strPath);
	}
	if (fileDialog.DoModal() == IDOK)
	{
		try
		{
			eseDbManager_->OpenFile(fileDialog.m_szFileName);
		}
		catch (runtime_error& e)
		{
			ShowMessageBox(e.what());
		}
	}
}

void MainFrame::OnViewStatusBar(UINT uCode, int nID, HWND hwndCtrl)
{
	auto isVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, isVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, isVisible);
	UpdateLayout();
}

void MainFrame::OnAppAbout(UINT uCode, int nID, HWND hwndCtrl)
{
	AboutDialog dlg;
	dlg.DoModal();
}

void MainFrame::OnFileSnapshot(UINT uCode, int nID, HWND hwndCtrl) const
{
	CSnapshotWizard snapshotWizard;
	if (snapshotWizard.DoModal() != IDOK)
		return;
	auto snapshotFilePath = snapshotWizard.GetSnapshotFilePath();
	if (nullptr != snapshotFilePath)
	{
		eseDbManager_->OpenFile(snapshotFilePath);
	}
}

void MainFrame::OnToolFilter(UINT uCode, int nID, HWND hwndCtrl)
{
	FilterDialog filterDialog(&tableListView_);
	filterDialog.DoModal();
}
