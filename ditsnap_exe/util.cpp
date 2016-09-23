#include "stdafx.h"
#include "util.h"

void ShowMessageBox(const char* message) {
	MessageBoxA(nullptr, message, "Ditsnap", MB_ICONWARNING | MB_OK);
}

void ShowMessageBox(const wchar_t* message) {
	MessageBoxW(nullptr, message, L"Ditsnap", MB_ICONWARNING | MB_OK);
}

wstring JoinString(vector<wstring> vs, wstring separator) {
	wstring columnValues;
	for (auto i = 0; i < vs.size(); ++i) {
		columnValues += vs[i];
		if (vs.size() > 1 && i != vs.size()) {
			columnValues += separator;
		}
	}
	return columnValues;
}