#pragma once

class AboutDialog : public CDialogImpl<AboutDialog>
{
public:
	enum
	{
		IDD = IDD_ABOUTBOX
	};

	BEGIN_MSG_MAP_EX(AboutDialog)
	MSG_WM_INITDIALOG(OnInitDialog)
	COMMAND_ID_HANDLER_EX(IDOK, OnClose)
	COMMAND_ID_HANDLER_EX(IDCANCEL, OnClose)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam) {
		CenterWindow(GetParent());
		return TRUE;
	}

	void OnClose(UINT uNotifyCode, int nID, HWND hWndCtl) {
		EndDialog(nID);
	}
};
