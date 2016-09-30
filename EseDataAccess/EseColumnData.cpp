#include "EseDataAccess.h"

namespace Ese
{
	class EseColumnData::Impl
	{
	public:
		Impl() {}
		EseType type_{EseType::Nil};
		vector<vector<uchar>> values_;
		bool isUnicode_{false};
		wstring ConvertToString(vector<uchar> v) const;

		DISALLOW_COPY_AND_ASSIGN(EseColumnData::Impl);
	};

	EseColumnData::EseColumnData(EseType type, vector<vector<uchar>> values, bool isUnicode): pimpl(new Impl) {
		pimpl->type_ = type;
		pimpl->values_ = values;
		pimpl->isUnicode_ = isUnicode;
	}

	EseColumnData::~EseColumnData() {}

	EseType EseColumnData::GetType() const {
		return pimpl->type_;
	}

	wstring EseColumnData::GetColumnTypeString() const {
		switch (pimpl->type_) {
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

	vector<vector<uchar>> EseColumnData::GetValues() const {
		return pimpl->values_;
	}

	vector<wstring> EseColumnData::GetValuesAsString() const {
		vector<wstring> vals;
		for (auto v : pimpl->values_) {
			vals.push_back(pimpl->ConvertToString(v));
		}
		return vals;
	}
		
	wstring EseColumnData::Impl::ConvertToString(vector<uchar> v) const {
		if (v.size() == 0) {
			return wstring(L"");
		}

		switch (type_) {
		case EseType::Nil:
			return wstring(L"");
		case EseType::Bit: /* True or False, Never NULL */
			return to_wstring(*reinterpret_cast<int*>(v.data()));
		case EseType::UnsignedByte: /* 1-byte integer, unsigned */
			return to_wstring(*reinterpret_cast<uint*>(v.data()));
		case EseType::Short: /* 2-byte integer, signed */
			return to_wstring(*reinterpret_cast<int*>(v.data()));
		case EseType::Long: /* 4-byte integer, signed */
			return to_wstring(*reinterpret_cast<int*>(v.data()));
		case EseType::Currency: /* 8 byte integer, signed */
			return to_wstring(*reinterpret_cast<long long int*>(v.data()));
		case EseType::IEEESingle: /* 4-byte IEEE single precision */
			return to_wstring(*reinterpret_cast<float*>(v.data()));
		case EseType::IEEEDouble: /* 8-byte IEEE double precision */
			return to_wstring(*reinterpret_cast<double*>(v.data()));
		case EseType::DateTime:
		{/* This column type is identical to the variant date type.*/
			SYSTEMTIME st{};
			VariantTimeToSystemTime(*reinterpret_cast<double*>(v.data()), &st);
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
		case EseType::Binary: /* Binary data, < 255 bytes */
		case EseType::LongBinary: /* Binary data, long value */
		{
			std::wstringstream ss;
			for (auto& c : v) {
				ss << std::setfill(L'0') << std::setw(2);
				ss << std::uppercase << std::hex << static_cast<uchar>(c) << L" ";
			}
			return ss.str();
		}
		case EseType::Text: /* ANSI text, case insensitive, < 255 bytes */
		case EseType::LongText: /* ANSI text, long value  */
		{
			if (isUnicode_) {
				// Ensure L'\0' terminated
				v.push_back(0);
				v.push_back(0);
				return wstring{ reinterpret_cast<wchar_t*>(v.data()) };
			}
			return wstring(v.begin(), v.end());
		}
		case EseType::UnsignedLong:
			return to_wstring(*reinterpret_cast<ulong*>(v.data()));
		case EseType::LongLong:
			return to_wstring(*reinterpret_cast<long long int*>(v.data()));
		case EseType::GUID:
			return wstring(L"(GUID type)");
		case EseType::UnsignedShort:
			return to_wstring(*reinterpret_cast<ushort*>(v.data()));
		default:
			return wstring(L"unknown type");
		}
	}
}
