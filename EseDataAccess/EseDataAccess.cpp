#include "EseDataAccess.h"
namespace Ese {
	std::wstring ToString(EseType eseType)
	{
		switch (eseType) {
		case EseType::Nil: return L"Nil";
		case EseType::Bit: return L"Bit";
		case EseType::UnsignedByte: return L"UnsignedByte";
		case EseType::Short: return L"Short";
		case EseType::Long: return L"Long";
		case EseType::Currency: return L"Currency";
		case EseType::IEEESingle: return L"IEEESingle";
		case EseType::IEEEDouble: return L"IEEEDouble";
		case EseType::DateTime: return L"DateTime";
		case EseType::Binary: return L"Binary";
		case EseType::Text: return L"Text";
		case EseType::LongBinary: return L"LongBinary";
		case EseType::LongText: return L"LongText";
		case EseType::SLV: return L"SLV";
		case EseType::UnsignedLong: return L"UnsignedLong";
		case EseType::LongLong: return L"LongLong";
		case EseType::GUID: return L"GUID";
		case EseType::UnsignedShort: return L"UnsignedShort";
		default: return L"Unknown";
		}
	}
}