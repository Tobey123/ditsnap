
#include "SnapshotWizard.h"
#include "../VssCopy/VssCopy.h"
#include "util.h"

BOOL SnapshotWizardPage1::OnInitDialog(CWindow wndFocus, LPARAM lInitParam) {
	sourceEdit_ = GetDlgItem(IDC_SOURCE_EDIT);
	destinationEdit_ = GetDlgItem(IDC_DEST_EDIT);

	sourceEdit_.SetWindowTextW(L"%systemroot%\\NTDS\\ntds.dit");
	wchar_t destinationPath[MAX_PATH];
	if (::GetModuleFileName(nullptr, destinationPath, MAX_PATH)) {
		CPath tempPath(destinationPath);
		tempPath.RemoveFileSpec();
		tempPath.Append(L"\\ntdsSnapshot.dit");
		destinationEdit_.SetWindowTextW(static_cast<const wchar_t*>(tempPath));
	}
	return TRUE;
}

LRESULT SnapshotWizardPage1::OnWizardNext() {
	CAutoPtr<wchar_t> sourcePath(new wchar_t[sourceEdit_.GetWindowTextLengthW() + 1]);
	sourceEdit_.GetWindowTextW(sourcePath, sourceEdit_.GetWindowTextLengthW() + 1);
	CAutoPtr<wchar_t> destinationPath(new wchar_t[destinationEdit_.GetWindowTextLengthW() + 1]);
	destinationEdit_.GetWindowTextW(destinationPath, destinationEdit_.GetWindowTextLengthW() + 1);
	wchar_t expandedSourcePath[MAX_PATH];
	wchar_t expandedDestinationPath[MAX_PATH];
	if (0 == ::ExpandEnvironmentStrings(sourcePath, expandedSourcePath, MAX_PATH)) {
		ShowMessageBox(L"Bad source path format.");
		return -1;
	}

	if (0 == ::ExpandEnvironmentStrings(destinationPath, expandedDestinationPath, MAX_PATH)) {
		ShowMessageBox(L"Bad destination path format.");
		return -1;
	}

	if (!ATLPath::FileExists(expandedSourcePath)) {
		ShowMessageBox(L"The source file does not exists.");
		return -1;
	}

	if (ATLPath::FileExists(expandedDestinationPath)) {
		ShowMessageBox(L"The Destination file already exists.");
		return -1;
	}

	auto hr = Vss::CopyFileFromSnapshot(sourcePath, destinationPath);
	if (FAILED(hr)) {
		CString errorMessage;
		errorMessage.Format(L"%s : %d", L"Vss copy Failed.", hr);
		ShowMessageBox(errorMessage);
		return -1;
	}

	if (!InvokeEsentutilP(destinationPath)) {
		CString errorMessage;
		errorMessage.Format(L"%s : %d", L"Database repair Failed. : ", GetLastError());
		ShowMessageBox(errorMessage);
		return -1;
	}

	wcscpy_s(sharedStringPage1_, MAX_PATH + 1, destinationPath);
	return 0;
}

BOOL SnapshotWizardPage1::InvokeEsentutilP(const wchar_t* targetDbPath) const {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof( si ) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof( pi ) );
	wchar_t* esentutilCommand = L"\"%systemroot%\\system32\\esentutl.exe\" /p /8 /o ";
	wchar_t eseutilCommandWithDbPath[MAX_PATH * 2 + 4];
	swprintf_s(eseutilCommandWithDbPath, MAX_PATH * 2 + 4, L"%s\"%s\"", esentutilCommand, targetDbPath);
	wchar_t expandedCmdLine[MAX_PATH * 2 + 4];
	::ExpandEnvironmentStrings(eseutilCommandWithDbPath, expandedCmdLine, MAX_PATH * 2 + 4);
	if (!::CreateProcess(nullptr, expandedCmdLine, nullptr, nullptr, FALSE,
	                     CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi)) {
		return FALSE;
	}

	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return TRUE;
}

BOOL SnapshotWizardPage2::OnSetActive() {
	SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);
	return TRUE;
}

CSnapshotWizard::CSnapshotWizard(_U_STRINGorID title, UINT uStartPage, HWND hWndParent) : CPropertySheetImpl<CSnapshotWizard>(title, uStartPage, hWndParent), sharedString_(new wchar_t[MAX_PATH + 1]) {
	SetWizardMode();
	AddPage(page1_);
	AddPage(page2_);
	page1_.SetSharedString(sharedString_);
	page2_.SetSharedString(sharedString_);
}
