#pragma once

#include "Types.h"
#include <ctype.h>
#include <wctype.h>
#include <tchar.h>

namespace EDX
{
	/*-----------------------------------------------------------------------------
	Character type functions.
	-----------------------------------------------------------------------------*/

	/**
	* Templated literal struct to allow selection of wide or ansi string literals
	* based on the character type provided, and not on compiler switches.
	*/
	template <typename T> struct Literal
	{
		static const ANSICHAR  Select(const ANSICHAR  ansi, const WIDECHAR) { return ansi; }
		static const ANSICHAR* Select(const ANSICHAR* ansi, const WIDECHAR*) { return ansi; }
	};

	template <> struct Literal<WIDECHAR>
	{
		static const WIDECHAR  Select(const ANSICHAR, const WIDECHAR  wide) { return wide; }
		static const WIDECHAR* Select(const ANSICHAR*, const WIDECHAR* wide) { return wide; }
	};

#define LITERAL(CharType, StringLiteral) Literal<CharType>::Select(StringLiteral, L##StringLiteral)

	/**
	* TChar
	* Set of utility functions operating on a single character. The functions
	* are specialized for ANSICHAR and TCHAR character types. You can use the
	* typedefs FChar and FCharAnsi for convenience.
	*/

	template <typename T, const unsigned int Size>
	struct CharBase
	{
		typedef T CharType;

		static const CharType LineFeed = L'\x000A';
		static const CharType VerticalTab = L'\x000B';
		static const CharType FormFeed = L'\x000C';
		static const CharType CarriageReturn = L'\x000D';
		static const CharType NextLine = L'\x0085';
		static const CharType LineSeparator = L'\x2028';
		static const CharType ParagraphSeparator = L'\x2029';
	};

	template <typename T>
	struct CharBase<T, 1>
	{
		typedef T CharType;

		static const CharType LineFeed = '\x000A';
		static const CharType VerticalTab = '\x000B';
		static const CharType FormFeed = '\x000C';
		static const CharType CarriageReturn = '\x000D';
		static const CharType NextLine = '\x0085';
	};

	template <typename T, const unsigned int Size>
	struct LineBreakImplementation
	{
		typedef T CharType;
		static inline bool IsLinebreak(CharType c)
		{
			return	c == CharBase<CharType, Size>::LineFeed
				|| c == CharBase<CharType, Size>::VerticalTab
				|| c == CharBase<CharType, Size>::FormFeed
				|| c == CharBase<CharType, Size>::CarriageReturn
				|| c == CharBase<CharType, Size>::NextLine
				|| c == CharBase<CharType, Size>::LineSeparator
				|| c == CharBase<CharType, Size>::ParagraphSeparator;
		}
	};

	template <typename T>
	struct LineBreakImplementation<T, 1>
	{
		typedef T CharType;
		static inline bool IsLinebreak(CharType c)
		{
			return	c == CharBase<CharType, 1>::LineFeed
				|| c == CharBase<CharType, 1>::VerticalTab
				|| c == CharBase<CharType, 1>::FormFeed
				|| c == CharBase<CharType, 1>::CarriageReturn
				|| c == CharBase<CharType, 1>::NextLine;
		}
	};

	template <typename T>
	struct TChar : public CharBase<T, sizeof(T)>
	{
		typedef T CharType;
	public:
		static inline CharType ToUpper(CharType Char);
		static inline CharType ToLower(CharType Char);
		static inline bool IsUpper(CharType Char);
		static inline bool IsLower(CharType Char);
		static inline bool IsAlpha(CharType Char);
		static inline bool IsPunct(CharType Char);

		static inline bool IsAlnum(CharType Char);
		static inline bool IsDigit(CharType Char);
		static inline bool IsOctDigit(CharType Char)
		{
			return Char >= '0' && Char <= '7';
		}
		static inline bool IsHexDigit(CharType Char);
		static inline int32 ConvertCharDigitToInt(CharType Char)
		{
			return static_cast<int32>(Char) - static_cast<int32>('0');
		}
		static inline bool IsWhitespace(CharType Char);
		static inline bool IsIdentifier(CharType Char)
		{
			return IsAlnum(Char) || IsUnderscore(Char);
		}
		static inline bool IsUnderscore(CharType Char) { return Char == LITERAL(CharType, '_'); }

	public:
		static inline bool IsLinebreak(CharType Char) { return LineBreakImplementation<CharType, sizeof(CharType)>::IsLinebreak(Char); }
	};

	typedef TChar<TCHAR>    FChar;
	typedef TChar<WIDECHAR> FCharWide;
	typedef TChar<ANSICHAR> FCharAnsi;

	/*-----------------------------------------------------------------------------
	TCHAR specialized functions
	-----------------------------------------------------------------------------*/

	template <> inline TChar<WIDECHAR>::CharType TChar<WIDECHAR>::ToUpper(CharType Char) { return ::towupper(Char); }
	template <> inline TChar<WIDECHAR>::CharType TChar<WIDECHAR>::ToLower(CharType Char) { return ::towlower(Char); }
	template <> inline bool TChar<WIDECHAR>::IsUpper(CharType Char) { return ::iswupper(Char) != 0; }
	template <> inline bool TChar<WIDECHAR>::IsLower(CharType Char) { return ::iswlower(Char) != 0; }
	template <> inline bool TChar<WIDECHAR>::IsAlpha(CharType Char) { return ::iswalpha(Char) != 0; }
	template <> inline bool TChar<WIDECHAR>::IsPunct(CharType Char) { return ::iswpunct(Char) != 0; }
	template <> inline bool TChar<WIDECHAR>::IsAlnum(CharType Char) { return ::iswalnum(Char) != 0; }
	template <> inline bool TChar<WIDECHAR>::IsDigit(CharType Char) { return ::iswdigit(Char) != 0; }
	template <> inline bool TChar<WIDECHAR>::IsHexDigit(CharType Char) { return ::iswxdigit(Char) != 0; }
	template <> inline bool TChar<WIDECHAR>::IsWhitespace(CharType Char) { return ::iswspace(Char) != 0; }

	/*-----------------------------------------------------------------------------
	ANSICHAR specialized functions
	-----------------------------------------------------------------------------*/
	template <> inline TChar<ANSICHAR>::CharType TChar<ANSICHAR>::ToUpper(CharType Char) { return ::toupper(Char); }
	template <> inline TChar<ANSICHAR>::CharType TChar<ANSICHAR>::ToLower(CharType Char) { return ::tolower(Char); }
	template <> inline bool TChar<ANSICHAR>::IsUpper(CharType Char) { return ::isupper((unsigned char)Char) != 0; }
	template <> inline bool TChar<ANSICHAR>::IsLower(CharType Char) { return ::islower((unsigned char)Char) != 0; }
	template <> inline bool TChar<ANSICHAR>::IsAlpha(CharType Char) { return ::isalpha((unsigned char)Char) != 0; }
	template <> inline bool TChar<ANSICHAR>::IsPunct(CharType Char) { return ::ispunct((unsigned char)Char) != 0; }
	template <> inline bool TChar<ANSICHAR>::IsAlnum(CharType Char) { return ::isalnum((unsigned char)Char) != 0; }
	template <> inline bool TChar<ANSICHAR>::IsDigit(CharType Char) { return ::isdigit((unsigned char)Char) != 0; }
	template <> inline bool TChar<ANSICHAR>::IsHexDigit(CharType Char) { return ::isxdigit((unsigned char)Char) != 0; }
	template <> inline bool TChar<ANSICHAR>::IsWhitespace(CharType Char) { return ::isspace((unsigned char)Char) != 0; }
}