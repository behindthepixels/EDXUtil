#include "Debug.h"
#include "../Core/CString.h"

namespace EDX
{
	void Debug::AssertFailedMessage(const char* Msg, const char* File, int Line, const TCHAR* Format, ...)
	{
		TCHAR DescriptionString[4096];
		GET_VARARGS(DescriptionString, ARRAY_COUNT(DescriptionString), ARRAY_COUNT(DescriptionString) - 1, Format, Format);

		TCHAR ErrorString[MAX_SPRINTF];
		CString::Sprintf(ErrorString, EDX_TEXT("Assertion failed: %s"), StringCast<TCHAR>(static_cast<const ANSICHAR*>(Msg)).Get());

		DebugMessageBox(DescriptionString, ErrorString);

		TCHAR DebugString[MAX_SPRINTF];
		CString::Sprintf(DebugString, EDX_TEXT("Assertion %s failed at %s line %i."), StringCast<TCHAR>(static_cast<const ANSICHAR*>(Msg)).Get(), StringCast<TCHAR>(static_cast<const ANSICHAR*>(File)).Get(), Line);

		WriteLine(DebugString);
		WriteLine(DescriptionString);
	}
}