#pragma once
#define WINVER		0x0500
#define _WIN32_WINNT	0x0501
#define _WIN32_IE	0x0501
#define _RICHEDIT_VER	0x0200

// ATL headers
#include <atlbase.h>
#include <atlstr.h>
#include <atlpath.h>
#include <atlwin.h>

// WTL headers
#define _WTL_NO_CSTRING
#pragma warning( disable : 4996 )
#include "atlapp.h"

extern CAppModule _Module;

#include "atlcrack.h"
#include "atlmisc.h"
#include "atlctrls.h"
#include "atlframe.h"
#include "atldlgs.h"
#include "atlctrlx.h" 
#include "atlsplit.h"

#include <string>
#include <vector>
#include <list>
#include <map>
#include <memory>

using std::map;
using std::string;
using std::wstring;
using std::pair;
using std::vector;
using std::list;
using std::runtime_error;
using std::unique_ptr;
using std::make_unique;

inline wstring to_w(const char* s) {
	string str(s);
	return wstring(str.begin(), str.end());
}

using uchar = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;
using ulong = unsigned long;
using ulonglong = unsigned long long;

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName&);                 \
void operator=(const TypeName&)

#pragma warning( default : 4996 )

#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
