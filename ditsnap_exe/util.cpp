#include "stdafx.h"
#include "util.h"
#include <iomanip>
#include <sstream>

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
		if (i != vs.size() - 1) {
			columnValues += separator;
		}
	}
	return columnValues;
}

wstring FileTimeToString(long long ll) {
	if (ll == 0) {
		return{};
	}
	FILETIME ft;
	memcpy(&ft, &ll, sizeof ft);
	SYSTEMTIME st;
	FileTimeToSystemTime(&ft, &st);
	std::wstringstream ss;
	ss << st.wYear << L"-"
		<< std::setw(2) << std::setfill(L'0') << st.wMonth << L"-"
		<< std::setw(2) << std::setfill(L'0') << st.wDay << L" "
		<< std::setw(2) << std::setfill(L'0') << st.wHour << L":"
		<< std::setw(2) << std::setfill(L'0') << st.wMinute << L":"
		<< std::setw(2) << std::setfill(L'0') << st.wSecond << L"."
		<< std::setw(3) << std::setfill(L'0') << st.wMilliseconds;
	return ss.str();
}

wstring BytesToGuidString(vector<uchar> bytes) {
	std::wstringstream ss;
	ss << L"{";
	for (auto i = 0; i < bytes.size(); i++) {
		ss << std::setfill(L'0') << std::setw(2);
		ss << std::uppercase << std::hex << static_cast<uchar>(bytes[i]);
		if (i == 3 || i == 5 || i== 7 || i == 9) {
			ss << L"-";
		}
	}
	ss << L"}";
	return ss.str();
}
