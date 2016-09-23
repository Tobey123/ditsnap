#pragma once
#include "resource.h"
#include "TableListView.h"
#include "DbTreeView.h"

class EseDbManager;

class MainFrame : public CFrameWindowImpl<MainFrame>, public CUpdateUI<MainFrame>
{
public:
	DECLARE_FRAME_WND_CLASS(nullptr, IDR_MAINFRAME)

	BEGIN_UPDATE_UI_MAP(MainFrame)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
		END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP_EX(MainFrame)
		MSG_WM_CREATE(OnCreate)
		COMMAND_ID_HANDLER_EX(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER_EX(ID_FILE_OPEN, OnFileOpen)
		COMMAND_ID_HANDLER_EX(ID_FILE_TAKESNAPSHOTANDOPEN, OnFileSnapshot)
		COMMAND_ID_HANDLER_EX(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER_EX(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER_EX(ID_TOOL_FILTER, OnToolFilter)
		CHAIN_MSG_MAP(CUpdateUI<MainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<MainFrame>)
		CHAIN_CLIENT_COMMANDS() // Command chains views
		REFLECT_NOTIFICATIONS() // Message Reflection
		END_MSG_MAP()

	explicit MainFrame(EseDbManager* eseDbManager);

	LRESULT OnCreate(LPCREATESTRUCT lpcs);
	void OnFileExit(UINT uCode, int nID, HWND hwndCtrl);
	void OnFileOpen(UINT uCode, int nID, HWND hwndCtrl) const;
	void OnFileSnapshot(UINT uCode, int nID, HWND hwndCtrl) const;
	void OnViewStatusBar(UINT uCode, int nID, HWND hwndCtrl);
	static void OnAppAbout(UINT uCode, int nID, HWND hwndCtrl);
	void OnToolFilter(UINT uCode, int nID, HWND hwndCtrl);

private:
	TableListView tableListView_;
	DbTreeView dbTreeView_;
	CSplitterWindow splitter_;
	EseDbManager* eseDbManager_;

	DISALLOW_COPY_AND_ASSIGN(MainFrame);
};
