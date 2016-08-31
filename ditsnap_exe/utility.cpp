#include "stdafx.h"
#include "utility.h"

void ShowMessageBox(const char* message)
{
	MessageBoxA(nullptr, message, "Ditsnap", MB_ICONWARNING | MB_OK);
}

void ShowMessageBox(const wchar_t* message)
{
	MessageBoxW(nullptr, message, L"Ditsnap", MB_ICONWARNING | MB_OK);
}