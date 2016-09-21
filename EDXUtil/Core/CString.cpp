#include "CString.h"

namespace EDX
{
	template <> const TCHAR* CStringUtil::GetEncodingTypeName<ANSICHAR>() { return EDX_TEXT("ANSICHAR"); }
	template <> const TCHAR* CStringUtil::GetEncodingTypeName<WIDECHAR>() { return EDX_TEXT("WIDECHAR"); }

	// 4 lines of 64 chars each, plus a null terminator
	template <>
	const ANSICHAR CStringSpcHelper<ANSICHAR>::SpcArray[MAX_SPACES + 1] =
		"                                                                "
		"                                                                "
		"                                                                "
		"                                                               ";

	template <>
	const WIDECHAR CStringSpcHelper<WIDECHAR>::SpcArray[MAX_SPACES + 1] =
		L"                                                                "
		L"                                                                "
		L"                                                                "
		L"                                                               ";

	template <>
	const ANSICHAR CStringSpcHelper<ANSICHAR>::TabArray[MAX_TABS + 1] =
		"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
		"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
		"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
		"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

	template <>
	const WIDECHAR CStringSpcHelper<WIDECHAR>::TabArray[MAX_TABS + 1] =
		L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
		L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
		L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
		L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

	bool ToBoolHelper::FromCStringAnsi(const ANSICHAR* String)
	{
		if (CStringAnsi::Stricmp(String, ("True")) == 0
			|| CStringAnsi::Stricmp(String, ("Yes")) == 0
			|| CStringAnsi::Stricmp(String, ("On")) == 0)
		{
			return true;
		}
		else if (CStringAnsi::Stricmp(String, ("False")) == 0
			|| CStringAnsi::Stricmp(String, ("No")) == 0
			|| CStringAnsi::Stricmp(String, ("Off")) == 0)
		{
			return false;
		}
		else
		{
			return CStringAnsi::Atoi(String) ? true : false;
		}
	}

	bool ToBoolHelper::FromCStringWide(const WIDECHAR* String)
	{
		if (CStringWide::Stricmp(String, (L"True")) == 0
			|| CStringWide::Stricmp(String, (L"Yes")) == 0
			|| CStringWide::Stricmp(String, (L"On")) == 0)
		{
			return true;
		}
		else if (CStringWide::Stricmp(String, (L"False")) == 0
			|| CStringWide::Stricmp(String, (L"No")) == 0
			|| CStringWide::Stricmp(String, (L"Off")) == 0)
		{
			return false;
		}
		else
		{
			return CStringWide::Atoi(String) ? true : false;
		}
	}

}