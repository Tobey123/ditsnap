#pragma once

void ShowMessageBox(const char* message);
void ShowMessageBox(const wchar_t* message);
wstring JoinString(vector<wstring> vs, wstring separator);
wstring FileTimeToString(long long int ft);
wstring BytesToGuidString(vector<uchar> bytes);
