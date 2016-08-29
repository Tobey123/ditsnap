#include "stdafx.h"
#include "MainFrame.h"
#include "SnapshotWizard.h"
#include "AboutDlg.h"
#include "FilterDialog.h"

using namespace EseDataAccess;

CMainFrame::CMainFrame(EseDbManager* eseDbManager)
	: tableListView_(CTableListView(eseDbManager)),
	  dbTreeView_(CDbTreeView(eseDbManager)),
	  eseDbManager_(eseDbManager)
{
}

LRESULT CMainFrame::OnCreate(LPCREATESTRUCT lpcs)
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

void CMainFrame::OnFileExit(UINT uCode, int nID, HWND hwndCtrl)
{
	PostMessage(WM_CLOSE);
}

void CMainFrame::OnFileOpen(UINT uCode, int nID, HWND hwndCtrl)
{
	CFileDialog fileDialog(TRUE, _T("txt"), nullptr, OFN_HIDEREADONLY | OFN_CREATEPROMPT,
	                           _T("dit file (*.dit)\0*.dit\0all file (*.*)\0*.*\0\0"));
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
			MessageBoxA(nullptr, e.what(), "Ditsnap", MB_ICONWARNING | MB_OK);
		}
	}
}

void CMainFrame::OnViewStatusBar(UINT uCode, int nID, HWND hwndCtrl)
{
	auto isVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, isVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, isVisible);
	UpdateLayout();
}

void CMainFrame::OnAppAbout(UINT uCode, int nID, HWND hwndCtrl)
{
	CAboutDlg dlg;
	dlg.DoModal();
}

void CMainFrame::OnFileSnapshot(UINT uCode, int nID, HWND hwndCtrl)
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

void CMainFrame::OnToolFilter(UINT uCode, int nID, HWND hwndCtrl)
{
	CFilterDialog filterDialog(&tableListView_);
	filterDialog.DoModal();
}
