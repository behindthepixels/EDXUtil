#include "Debug.h"
#include "../Core/CString.h"

namespace EDX
{
	void Debug::AssertFailedMessage(const char* Msg, const char* File, int Line, const TCHAR* Format, ...)
	{
		TCHAR DescriptionString[4096];
		GET_VARARGS(DescriptionString, ARRAY_COUNT(DescriptionString), ARRAY_COUNT(DescriptionString) - 1, Format, Format);

		TCHAR DebugString[MAX_SPRINTF];
		CString::Sprintf(DebugString, EDX_TEXT("Assertion \"%s\" failed at %s line %i\n%s"),
			StringCast<TCHAR>(static_cast<const ANSICHAR*>(Msg)).Get(),
			StringCast<TCHAR>(static_cast<const ANSICHAR*>(File)).Get(),
			Line,
			StringCast<TCHAR>(static_cast<const ANSICHAR*>(DescriptionString)).Get());

		DebugMessageBox(DebugString, EDX_TEXT("Assertion Failed"));

		WriteLine(DebugString);
	}
}