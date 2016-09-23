#pragma once
#define WIN32_LEAN_AND_MEAN		
#include <tchar.h>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>
#include <sstream>
#include "windows.h"
#include "atlbase.h"

using std::vector;
using std::string;
using std::wstring;
using std::runtime_error;
using std::to_string;
using std::to_wstring;
using std::unique_ptr;

using uchar = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;
using ulong = unsigned long;
using ulonglong = unsigned long long;;

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName&);                 \
void operator=(const TypeName&)
