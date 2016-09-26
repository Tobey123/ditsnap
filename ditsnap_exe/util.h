#pragma once

void ShowMessageBox(const char* message);
void ShowMessageBox(const wchar_t* message);
wstring JoinString(vector<wstring> vs, wstring separator = L"; ");
wstring FileTimeToString(long long int ft);
wstring BytesToGuidString(vector<uchar> bytes);
wstring GetFlagString(int flag, map<wstring, int> flagMap);
wstring GetUserFlagString(int flag);
wstring GetSamAccountTypeString(int flag);
wstring GetSystemFlagString(int flag);
wstring GetSearchFlagString(int flag);