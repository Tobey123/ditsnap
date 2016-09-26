#include "stdafx.h"
#include "MainFrame.h"
#include "EseRepository.h"

CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/  = nullptr, int nCmdShow = SW_SHOWDEFAULT) {
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);
	auto eseRepository = new EseRepository();
	auto wndMain = new MainFrame(eseRepository);
	if (wndMain->CreateEx() == nullptr) {
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	wndMain->ShowWindow(nCmdShow);
	auto nRet = theLoop.Run();
	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow) {
	HRESULT hRes = CoInitialize(nullptr);
	ATLASSERT(SUCCEEDED(hRes));
	::DefWindowProc(nullptr, 0, 0, 0L);
	AtlInitCommonControls(ICC_BAR_CLASSES);
	hRes = _Module.Init(nullptr, hInstance);
	ATLASSERT(SUCCEEDED(hRes));
	auto nRet = Run(lpstrCmdLine, nCmdShow);
	_Module.Term();
	CoUninitialize();
	return nRet;
}
