
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

wstring FileTimeToString(long long int ll) {
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

wstring GetFlagString(int flag, map<wstring, int> flagMap) {
	wstring s = L"";
	for (auto pair : flagMap) {
		if ((flag & pair.second) == pair.second) {
			s += pair.first;
			s += L"|";
		}
	}

	if (s.length() > 0 && s.back() == L'|') {
		s.pop_back();
	}

	return s;
}

wstring GetUserFlagString(int flag) {
	map<wstring, int> flagMap;
	flagMap[L"SCRIPT"] = 1; // 0x1
	flagMap[L"ACCOUNTDISABLE"] = 2; // 0x2
	flagMap[L"HOMEDIR_REQUIRED"] = 8; // 0x8
	flagMap[L"LOCKOUT"] = 16; // 0x10
	flagMap[L"PASSWD_NOTREQD"] = 32; // 0x20
	flagMap[L"PASSWD_CANT_CHANGE"] = 64; // 0x40
	flagMap[L"ENCRYPTED_TEXT_PASSWORD_ALLOWED"] = 128; // 0x80
	flagMap[L"TEMP_DUPLICATE_ACCOUNT"] = 256; // 0x100
	flagMap[L"NORMAL_ACCOUNT"] = 512; // 0x200
	flagMap[L"INTERDOMAIN_TRUST_ACCOUNT"] = 2048; // 0x800
	flagMap[L"WORKSTATION_TRUST_ACCOUNT"] = 4096; // 0x1000
	flagMap[L"SERVER_TRUST_ACCOUNT"] = 8192; // 0x2000
	flagMap[L"DONT_EXPIRE_PASSWD"] = 65536; // 0x10000
	flagMap[L"MNS_LOGON_ACCOUNT"] = 131072; // 0x20000
	flagMap[L"SMARTCARD_REQUIRED"] = 262144; // 0x40000
	flagMap[L"TRUSTED_FOR_DELEGATION"] = 524288; // 0x80000
	flagMap[L"NOT_DELEGATED"] = 1048576; // 0x100000
	flagMap[L"USE_DES_KEY_ONLY"] = 2097152; // 0x200000
	flagMap[L"DONT_REQUIRE_PREAUTH"] = 4194304; // 0x400000
	flagMap[L"PASSWORD_EXPIRED"] = 8388608; // 0x800000
	flagMap[L"TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION"] = 16777216; // 0x1000000
	return GetFlagString(flag, flagMap);
}

wstring GetSamAccountTypeString(int flag) {
	map<wstring, int> flagMap;
	flagMap[L"DOMAIN_OBJECT"] = 0x0;
	flagMap[L"GROUP_OBJECT"] = 0x10000000;
	flagMap[L"NON_SECURITY_GROUP_OBJECT"] = 0x10000001;
	flagMap[L"ALIAS_OBJECT"] = 0x20000000;
	flagMap[L"NON_SECURITY_ALIAS_OBJECT"]= 0x20000001;
	flagMap[L"USER_OBJECT"] = 0x30000000;
	flagMap[L"NORMAL_USER_ACCOUNT"] = 0x30000000;
	flagMap[L"MACHINE_ACCOUNT"] = 0x30000001;
	flagMap[L"TRUST_ACCOUNT"] = 0x30000002;
	flagMap[L"APP_BASIC_GROUP"] = 0x40000000;
	flagMap[L"APP_QUERY_GROUP"] = 0x40000001;
	flagMap[L"SAM_ACCOUNT_TYPE_MAX"] = 0x7fffffff;
	return GetFlagString(flag, flagMap);
}

wstring GetSystemFlagString(int flag) {
	map<wstring, int> flagMap;
	flagMap[L"FLAG_ATTR_IS_OPERATIONAL"] = 0x00000008;
	flagMap[L"FLAG_SCHEMA_BASE_OBJECT"] = 0x00000010;
	flagMap[L"FLAG_ATTR_IS_RDN"] = 0x00000020;
	flagMap[L"FLAG_DISALLOW_MOVE_ON_DELETE"] = 0x02000000;
	flagMap[L"FLAG_DOMAIN_DISALLOW_MOVE"] = 0x04000000;
	flagMap[L"FLAG_DOMAIN_DISALLOW_RENAME"] = 0x08000000;
	flagMap[L"FLAG_CONFIG_ALLOW_LIMITED_MOVE"] = 0x10000000;
	flagMap[L"FLAG_CONFIG_ALLOW_MOVE"] = 0x20000000;
	flagMap[L"FLAG_CONFIG_ALLOW_RENAME"] = 0x40000000;
	flagMap[L"FLAG_DISALLOW_DELETE"] = 0x80000000;
	if ((flag & 0x00000020) == 0x00000020) {
		flagMap[L"FLAG_ATTR_NOT_REPLICATED"] = 0x00000001;
		flagMap[L"FLAG_ATTR_REQ_PARTIAL_SET_MEMBER"] = 0x00000002;
		flagMap[L"FLAG_ATTR_IS_CONSTRUCTED"] = 0x00000004;
	} else {
		flagMap[L"FLAG_CR_NTDS_NC"] = 0x00000001;
		flagMap[L"FLAG_CR_NTDS_DOMAIN"] = 0x00000002;
		flagMap[L"FLAG_CR_NTDS_NOT_GC_REPLICATED"] = 0x00000004;
	}
	return GetFlagString(flag, flagMap);
}

wstring GetSearchFlagString(int flag) {
	map<wstring, int> flagMap;
	flagMap[L"fATTINDEX"] = 0x00000001;
	flagMap[L"fPDNTATTINDEX"] = 0x00000002;
	flagMap[L"fANR"] = 0x00000004;
	flagMap[L"fPRESERVEONDELETE"] = 0x00000008;
	flagMap[L"fCOPY"] = 0x00000010;
	flagMap[L"fTUPLEINDEX"] = 0x00000020;
	flagMap[L"fSUBTREEATTINDEX"] = 0x00000040;
	flagMap[L"fCONFIDENTIAL"] = 0x00000080;
	flagMap[L"fNEVERVALUEAUDIT"] = 0x00000100;
	flagMap[L"fRODCFilteredAttribute"] = 0x00000200;
	flagMap[L"fEXTENDEDLINKTRACKING"] = 0x00000400;
	flagMap[L"fBASEONLY"] = 0x00000800;
	flagMap[L"fPARTITIONSECRET"] = 0x00001000;
	return GetFlagString(flag, flagMap);
}
