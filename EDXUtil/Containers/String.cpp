#include "String.h"

namespace EDX
{
	/* String implementation
	*****************************************************************************/

	void String::TrimToNullTerminator()
	{
		if (Data.Size())
		{
			int32 DataLen = CString::Strlen(Data.Data());
			Assert(DataLen == 0 || DataLen < Data.Size());
			int32 Len = DataLen > 0 ? DataLen + 1 : 0;

			Assert(Len <= Data.Size())
				Data.RemoveAt(Len, Data.Size() - Len);
		}
	}


	int32 String::Find(const TCHAR* SubStr, ESearchCase SearchCase, ESearchDir SearchDir, int32 StartPosition) const
	{
		if (SubStr == nullptr)
		{
			return INDEX_NONE;
		}
		if (SearchDir == ESearchDir::FromStart)
		{
			const TCHAR* Start = **this;
			if (StartPosition != INDEX_NONE)
			{
				Start += Math::Clamp(StartPosition, 0, Len() - 1);
			}
			const TCHAR* Tmp = SearchCase == ESearchCase::IgnoreCase
				? CString::Stristr(Start, SubStr)
				: CString::Strstr(Start, SubStr);

			return Tmp ? (Tmp - **this) : INDEX_NONE;
		}
		else
		{
			// if ignoring, do a onetime ToUpper on both strings, to avoid ToUppering multiple
			// times in the loop below
			if (SearchCase == ESearchCase::IgnoreCase)
			{
				return ToUpper().Find(String(SubStr).ToUpper(), ESearchCase::CaseSensitive, SearchDir, StartPosition);
			}
			else
			{
				const int32 SearchStringLength = Math::Max(1, CString::Strlen(SubStr));

				if (StartPosition == INDEX_NONE)
				{
					StartPosition = Len();
				}

				for (int32 i = StartPosition - SearchStringLength; i >= 0; i--)
				{
					int32 j;
					for (j = 0; SubStr[j]; j++)
					{
						if ((*this)[i + j] != SubStr[j])
						{
							break;
						}
					}

					if (!SubStr[j])
					{
						return i;
					}
				}
				return INDEX_NONE;
			}
		}
	}

	String String::ToUpper() const
	{
		String New(**this);
		New.ToUpperInline();
		return New;
	}

	void String::ToUpperInline()
	{
		const int32 StringLength = Len();
		TCHAR* RawData = Data.Data();
		for (int32 i = 0; i < StringLength; ++i)
		{
			RawData[i] = FChar::ToUpper(RawData[i]);
		}
	}


	String String::ToLower() const
	{
		String New(**this);
		New.ToLowerInline();
		return New;
	}

	void String::ToLowerInline()
	{
		const int32 StringLength = Len();
		TCHAR* RawData = Data.Data();
		for (int32 i = 0; i < StringLength; ++i)
		{
			RawData[i] = FChar::ToLower(RawData[i]);
		}
	}

	bool String::StartsWith(const String& InPrefix, ESearchCase SearchCase) const
	{
		if (SearchCase == ESearchCase::IgnoreCase)
		{
			return InPrefix.Len() > 0 && !CString::Strnicmp(**this, *InPrefix, InPrefix.Len());
		}
		else
		{
			return InPrefix.Len() > 0 && !CString::Strncmp(**this, *InPrefix, InPrefix.Len());
		}
	}

	bool String::EndsWith(const String& InSuffix, ESearchCase SearchCase) const
	{
		if (SearchCase == ESearchCase::IgnoreCase)
		{
			return InSuffix.Len() > 0 &&
				Len() >= InSuffix.Len() &&
				!CString::Stricmp(&(*this)[Len() - InSuffix.Len()], *InSuffix);
		}
		else
		{
			return InSuffix.Len() > 0 &&
				Len() >= InSuffix.Len() &&
				!CString::Strcmp(&(*this)[Len() - InSuffix.Len()], *InSuffix);
		}
	}

	bool String::RemoveFromStart(const String& InPrefix, ESearchCase SearchCase)
	{
		if (InPrefix.IsEmpty())
		{
			return false;
		}

		if (StartsWith(InPrefix, SearchCase))
		{
			RemoveAt(0, InPrefix.Len());
			return true;
		}

		return false;
	}

	bool String::RemoveFromEnd(const String& InSuffix, ESearchCase SearchCase)
	{
		if (InSuffix.IsEmpty())
		{
			return false;
		}

		if (EndsWith(InSuffix, SearchCase))
		{
			RemoveAt(Len() - InSuffix.Len(), InSuffix.Len());
			return true;
		}

		return false;
	}

	String String::Trim()
	{
		int32 Pos = 0;
		while (Pos < Len())
		{
			if (FChar::IsWhitespace((*this)[Pos]))
			{
				Pos++;
			}
			else
			{
				break;
			}
		}

		*this = Right(Len() - Pos);

		return *this;
	}

	String String::TrimTrailing(void)
	{
		int32 Pos = Len() - 1;
		while (Pos >= 0)
		{
			if (!FChar::IsWhitespace((*this)[Pos]))
			{
				break;
			}

			Pos--;
		}

		*this = Left(Pos + 1);

		return(*this);
	}

	String String::TrimQuotes(bool* bQuotesRemoved) const
	{
		bool bQuotesWereRemoved = false;
		int32 Start = 0, Count = Len();
		if (Count > 0)
		{
			if ((*this)[0] == TCHAR('"'))
			{
				Start++;
				Count--;
				bQuotesWereRemoved = true;
			}

			if (Len() > 1 && (*this)[Len() - 1] == TCHAR('"'))
			{
				Count--;
				bQuotesWereRemoved = true;
			}
		}

		if (bQuotesRemoved != nullptr)
		{
			*bQuotesRemoved = bQuotesWereRemoved;
		}
		return Mid(Start, Count);
	}

	int32 String::CullArray(Array<String>* InArray)
	{
		Assert(InArray);
		String Empty;
		InArray->Remove(Empty);
		return InArray->Size();
	}

	String String::Reverse() const
	{
		String New(*this);
		New.ReverseString();
		return New;
	}

	void String::ReverseString()
	{
		if (Len() > 0)
		{
			TCHAR* StartChar = &(*this)[0];
			TCHAR* EndChar = &(*this)[Len() - 1];
			TCHAR TempChar;
			do
			{
				TempChar = *StartChar;	// store the current value of StartChar
				*StartChar = *EndChar;	// change the value of StartChar to the value of EndChar
				*EndChar = TempChar;	// change the value of EndChar to the character that was previously at StartChar

				StartChar++;
				EndChar--;

			} while (StartChar < EndChar);	// repeat until we've reached the midpoint of the string
		}
	}

	String String::FormatAsNumber(int32 InNumber)
	{
		String Number = String::FromInt(InNumber), Result;

		int32 dec = 0;
		for (int32 x = Number.Len() - 1; x > -1; --x)
		{
			Result += Number.Mid(x, 1);

			dec++;
			if (dec == 3 && x > 0)
			{
				Result += EDX_TEXT(",");
				dec = 0;
			}
		}

		return Result.Reverse();
	}

	///**
	//* Serializes a string as ANSI char array.
	//*
	//* @param	String			String to serialize
	//* @param	stream				Archive to serialize with
	//* @param	MinCharacters	Minimum number of characters to serialize.
	//*/
	//void String::SerializeAsANSICharArray(FArchive& stream, int32 MinCharacters) const
	//{
	//	int32	Length = Math::Max(Len(), MinCharacters);
	//	stream << Length;

	//	for (int32 CharIndex = 0; CharIndex<Len(); CharIndex++)
	//	{
	//		ANSICHAR AnsiChar = CharCast<ANSICHAR>((*this)[CharIndex]);
	//		stream << AnsiChar;
	//	}

	//	// Zero pad till minimum number of characters are written.
	//	for (int32 i = Len(); i<Length; i++)
	//	{
	//		ANSICHAR NullChar = 0;
	//		stream << NullChar;
	//	}
	//}

	void String::AppendInt(int32 InNum)
	{
		int64 Num = InNum; // This avoids having to deal with negating -int32(Math::EDX_INFINITY)-1
		const TCHAR* NumberChar[11] = { EDX_TEXT("0"), EDX_TEXT("1"), EDX_TEXT("2"), EDX_TEXT("3"), EDX_TEXT("4"), EDX_TEXT("5"), EDX_TEXT("6"), EDX_TEXT("7"), EDX_TEXT("8"), EDX_TEXT("9"), EDX_TEXT("-") };
		bool bIsNumberNegative = false;
		TCHAR TempNum[16];				// 16 is big enough
		int32 TempAt = 16; // fill the temp string from the top down.

						   // Correctly handle negative numbers and convert to positive integer.
		if (Num < 0)
		{
			bIsNumberNegative = true;
			Num = -Num;
		}

		TempNum[--TempAt] = 0; // NULL terminator

							   // Convert to string assuming base ten and a positive integer.
		do
		{
			TempNum[--TempAt] = *NumberChar[Num % 10];
			Num /= 10;
		} while (Num);

		// Append sign as we're going to reverse string afterwards.
		if (bIsNumberNegative)
		{
			TempNum[--TempAt] = *NumberChar[10];
		}

		*this += TempNum + TempAt;
	}


	bool String::ToBool() const
	{
		return CString::ToBool(**this);
	}

	String String::FromBlob(const uint8* SrcBuffer, const uint32 SrcSize)
	{
		String Result;
		Result.Reserve(SrcSize * 3);
		// Convert and append each byte in the buffer
		for (uint32 Count = 0; Count < SrcSize; Count++)
		{
			Result += String::Printf(EDX_TEXT("%03d"), (uint8)SrcBuffer[Count]);
		}
		return Result;
	}

	bool String::ToBlob(const String& Source, uint8* DestBuffer, const uint32 DestSize)
	{
		// Make sure the buffer is at least half the size and that the string is an
		// even number of characters long
		if (DestSize >= (uint32)(Source.Len() / 3) &&
			(Source.Len() % 3) == 0)
		{
			TCHAR ConvBuffer[4];
			ConvBuffer[3] = EDX_TEXT('\0');
			int32 WriteIndex = 0;
			// Walk the string 3 chars at a time
			for (int32 Index = 0; Index < Source.Len(); Index += 3, WriteIndex++)
			{
				ConvBuffer[0] = Source[Index];
				ConvBuffer[1] = Source[Index + 1];
				ConvBuffer[2] = Source[Index + 2];
				DestBuffer[WriteIndex] = CString::Atoi(ConvBuffer);
			}
			return true;
		}
		return false;
	}

	String String::FromHexBlob(const uint8* SrcBuffer, const uint32 SrcSize)
	{
		String Result;
		Result.Reserve(SrcSize * 2);
		// Convert and append each byte in the buffer
		for (uint32 Count = 0; Count < SrcSize; Count++)
		{
			Result += String::Printf(EDX_TEXT("%02X"), (uint8)SrcBuffer[Count]);
		}
		return Result;
	}

	bool String::ToHexBlob(const String& Source, uint8* DestBuffer, const uint32 DestSize)
	{
		// Make sure the buffer is at least half the size and that the string is an
		// even number of characters long
		if (DestSize >= (uint32)(Source.Len() / 2) &&
			(Source.Len() % 2) == 0)
		{
			TCHAR ConvBuffer[3];
			ConvBuffer[2] = EDX_TEXT('\0');
			int32 WriteIndex = 0;
			// Walk the string 2 chars at a time
			TCHAR* End = nullptr;
			for (int32 Index = 0; Index < Source.Len(); Index += 2, WriteIndex++)
			{
				ConvBuffer[0] = Source[Index];
				ConvBuffer[1] = Source[Index + 1];
				DestBuffer[WriteIndex] = CString::Strtoi(ConvBuffer, &End, 16);
			}
			return true;
		}
		return false;
	}

	String String::SanitizeFloat(double InFloat)
	{
		// Avoids negative zero
		if (InFloat == 0)
		{
			InFloat = 0;
		}

		String TempString;
		// First create the string
		TempString = String::Printf(EDX_TEXT("%f"), InFloat);
		const Array< TCHAR >& Chars = TempString.GetCharArray();
		const TCHAR Zero = '0';
		const TCHAR Period = '.';
		int32 TrimIndex = 0;
		// Find the first non-zero char in the array
		for (int32 Index = Chars.Size() - 2; Index >= 2; --Index)
		{
			const TCHAR EachChar = Chars[Index];
			const TCHAR NextChar = Chars[Index - 1];
			if ((EachChar != Zero) || (NextChar == Period))
			{
				TrimIndex = Index;
				break;
			}
		}
		// If we changed something trim the string
		if (TrimIndex != 0)
		{
			TempString = TempString.Left(TrimIndex + 1);
		}
		return TempString;
	}

	String String::Chr(TCHAR Ch)
	{
		TCHAR Temp[2] = { Ch,0 };
		return String(Temp);
	}


	String String::ChrN(int32 NumCharacters, TCHAR Char)
	{
		Assert(NumCharacters >= 0);

		String Temp;
		Temp.Data.AddUninitialized(NumCharacters + 1);
		for (int32 Cx = 0; Cx < NumCharacters; ++Cx)
		{
			Temp[Cx] = Char;
		}
		Temp.Data[NumCharacters] = 0;
		return Temp;
	}

	String String::LeftPad(int32 ChCount) const
	{
		int32 Pad = ChCount - Len();

		if (Pad > 0)
		{
			return ChrN(Pad, ' ') + *this;
		}
		else
		{
			return *this;
		}
	}
	String String::RightPad(int32 ChCount) const
	{
		int32 Pad = ChCount - Len();

		if (Pad > 0)
		{
			return *this + ChrN(Pad, ' ');
		}
		else
		{
			return *this;
		}
	}

	bool String::IsNumeric() const
	{
		if (IsEmpty())
		{
			return 0;
		}

		return CString::IsNumeric(Data.Data());
	}

	/**
	* Breaks up a delimited string into elements of a string array.
	*
	* @param	InArray		The array to fill with the string pieces
	* @param	pchDelim	The string to delimit on
	* @param	InCullEmpty	If 1, empty strings are not added to the array
	*
	* @return	The number of elements in InArray
	*/
	int32 String::ParseIntoArray(Array<String>& OutArray, const TCHAR* pchDelim, bool InCullEmpty) const
	{
		// Make sure the delimit string is not null or empty
		Assert(pchDelim);
		OutArray.Clear();
		const TCHAR *Start = Data.Data();
		int32 DelimLength = CString::Strlen(pchDelim);
		if (Start && DelimLength)
		{
			while (const TCHAR *At = CString::Strstr(Start, pchDelim))
			{
				if (!InCullEmpty || At - Start)
				{
					new (OutArray) String(At - Start, Start);
				}
				Start = At + DelimLength;
			}
			if (!InCullEmpty || *Start)
			{
				new(OutArray) String(Start);
			}

		}
		return OutArray.Size();
	}

	bool String::MatchesWildcard(const String& InWildcard, ESearchCase SearchCase) const
	{
		String Wildcard(InWildcard);
		String Target(*this);
		int32 IndexOfStar = Wildcard.Find(EDX_TEXT("*"), ESearchCase::CaseSensitive, ESearchDir::FromEnd); // last occurance
		int32 IndexOfQuestion = Wildcard.Find(EDX_TEXT("?"), ESearchCase::CaseSensitive, ESearchDir::FromEnd); // last occurance
		int32 Suffix = Math::Max<int32>(IndexOfStar, IndexOfQuestion);
		if (Suffix == INDEX_NONE)
		{
			// no wildcards
			if (SearchCase == ESearchCase::IgnoreCase)
			{
				return CString::Stricmp(*Target, *Wildcard) == 0;
			}
			else
			{
				return CString::Strcmp(*Target, *Wildcard) == 0;
			}
		}
		else
		{
			if (Suffix + 1 < Wildcard.Len())
			{
				String SuffixString = Wildcard.Mid(Suffix + 1);
				if (!Target.EndsWith(SuffixString, SearchCase))
				{
					return false;
				}
				Wildcard = Wildcard.Left(Suffix + 1);
				Target = Target.Left(Target.Len() - SuffixString.Len());
			}
			int32 PrefixIndexOfStar = Wildcard.Find(EDX_TEXT("*"), ESearchCase::CaseSensitive);
			int32 PrefixIndexOfQuestion = Wildcard.Find(EDX_TEXT("?"), ESearchCase::CaseSensitive);
			int32 Prefix = Math::Min(PrefixIndexOfStar < 0 ? int32(Math::EDX_INFINITY) : PrefixIndexOfStar, PrefixIndexOfQuestion < 0 ? int32(Math::EDX_INFINITY) : PrefixIndexOfQuestion);
			Assert(Prefix >= 0 && Prefix < Wildcard.Len());
			if (Prefix > 0)
			{
				String PrefixString = Wildcard.Left(Prefix);
				if (!Target.StartsWith(PrefixString, SearchCase))
				{
					return false;
				}
				Wildcard = Wildcard.Mid(Prefix);
				Target = Target.Mid(Prefix);
			}
		}
		// This routine is very slow, though it does ok with one wildcard
		Assert(Wildcard.Len());
		TCHAR FirstWild = Wildcard[0];
		Wildcard = Wildcard.Right(Wildcard.Len() - 1);
		if (FirstWild == EDX_TEXT('*') || FirstWild == EDX_TEXT('?'))
		{
			if (!Wildcard.Len())
			{
				if (FirstWild == EDX_TEXT('*') || Target.Len() < 2)
				{
					return true;
				}
			}
			int32 MaxNum = Math::Min(Target.Len(), FirstWild == EDX_TEXT('?') ? 1 : int32(Math::EDX_INFINITY));
			for (int32 Index = 0; Index <= MaxNum; Index++)
			{
				if (Target.Right(Target.Len() - Index).MatchesWildcard(Wildcard, SearchCase))
				{
					return true;
				}
			}
			return false;
		}
		else
		{
			Assert(0); // we should have dealt with prefix comparison above
			return false;
		}
	}


	/** Caution!! this routine is O(N^2) allocations...use it for parsing very short text or not at all */
	int32 String::ParseIntoArrayWS(Array<String>& OutArray, const TCHAR* pchExtraDelim, bool InCullEmpty) const
	{
		// default array of White Spaces, the last entry can be replaced with the optional pchExtraDelim string
		// (if you want to split on white space and another character)
		static const TCHAR* WhiteSpace[] =
		{
			EDX_TEXT(" "),
			EDX_TEXT("\t"),
			EDX_TEXT("\r"),
			EDX_TEXT("\n"),
			EDX_TEXT(""),
		};

		// start with just the standard whitespaces
		int32 NumWhiteSpaces = ARRAY_COUNT(WhiteSpace) - 1;
		// if we got one passed in, use that in addition
		if (pchExtraDelim && *pchExtraDelim)
		{
			WhiteSpace[NumWhiteSpaces++] = pchExtraDelim;
		}

		return ParseIntoArray(OutArray, WhiteSpace, NumWhiteSpaces, InCullEmpty);
	}

	int32 String::ParseIntoArrayLines(Array<String>& OutArray, bool InCullEmpty) const
	{
		// default array of LineEndings
		static const TCHAR* LineEndings[] =
		{
			EDX_TEXT("\r\n"),
			EDX_TEXT("\r"),
			EDX_TEXT("\n"),
		};

		// start with just the standard line endings
		int32 NumLineEndings = ARRAY_COUNT(LineEndings);
		return ParseIntoArray(OutArray, LineEndings, NumLineEndings, InCullEmpty);
	}

	int32 String::ParseIntoArray(Array<String>& OutArray, const TCHAR** DelimArray, int32 NumDelims, bool InCullEmpty) const
	{
		// Make sure the delimit string is not null or empty
		Assert(DelimArray);
		OutArray.Clear();
		const TCHAR *Start = Data.Data();
		const int32 Length = Len();
		if (Start)
		{
			int32 SubstringBeginIndex = 0;

			// Iterate through string.
			for (int32 i = 0; i < Len();)
			{
				int32 SubstringEndIndex = INDEX_NONE;
				int32 DelimiterLength = 0;

				// Attempt each delimiter.
				for (int32 DelimIndex = 0; DelimIndex < NumDelims; ++DelimIndex)
				{
					DelimiterLength = CString::Strlen(DelimArray[DelimIndex]);

					// If we found a delimiter...
					if (CString::Strncmp(Start + i, DelimArray[DelimIndex], DelimiterLength) == 0)
					{
						// Mark the end of the substring.
						SubstringEndIndex = i;
						break;
					}
				}

				if (SubstringEndIndex != INDEX_NONE)
				{
					const int32 SubstringLength = SubstringEndIndex - SubstringBeginIndex;
					// If we're not culling empty strings or if we are but the string isn't empty anyways...
					if (!InCullEmpty || SubstringLength != 0)
					{
						// ... add new string from substring beginning up to the beginning of this delimiter.
						new (OutArray) String(SubstringEndIndex - SubstringBeginIndex, Start + SubstringBeginIndex);
					}
					// Next substring begins at the end of the discovered delimiter.
					SubstringBeginIndex = SubstringEndIndex + DelimiterLength;
					i = SubstringBeginIndex;
				}
				else
				{
					++i;
				}
			}

			// Add any remaining characters after the last delimiter.
			const int32 SubstringLength = Length - SubstringBeginIndex;
			// If we're not culling empty strings or if we are but the string isn't empty anyways...
			if (!InCullEmpty || SubstringLength != 0)
			{
				// ... add new string from substring beginning up to the beginning of this delimiter.
				new (OutArray) String(Start + SubstringBeginIndex);
			}
		}

		return OutArray.Size();
	}

	String String::Replace(const TCHAR* From, const TCHAR* To, ESearchCase SearchCase) const
	{
		// Previous code used to accidentally accept a nullptr replacement string - this is no longer accepted.
		Assert(To);

		if (IsEmpty() || !From || !*From)
		{
			return *this;
		}

		// get a pointer into the character data
		const TCHAR* Travel = Data.Data();

		// precalc the lengths of the replacement strings
		int32 FromLength = CString::Strlen(From);
		int32 ToLength = CString::Strlen(To);

		String Result;
		while (true)
		{
			// look for From in the remaining string
			const TCHAR* FromLocation = SearchCase == ESearchCase::IgnoreCase ? CString::Stristr(Travel, From) : CString::Strstr(Travel, From);
			if (!FromLocation)
				break;

			// copy everything up to FromLocation
			Result.AppendChars(Travel, FromLocation - Travel);

			// copy over the To
			Result.AppendChars(To, ToLength);

			Travel = FromLocation + FromLength;
		}

		// copy anything left over
		Result += Travel;

		return Result;
	}

	int32 String::ReplaceInline(const TCHAR* SearchText, const TCHAR* ReplacementText, ESearchCase SearchCase)
	{
		int32 ReplacementCount = 0;

		if (Len() > 0
			&& SearchText != nullptr && *SearchText != 0
			&& ReplacementText != nullptr && (SearchCase == ESearchCase::IgnoreCase || CString::Strcmp(SearchText, ReplacementText) != 0))
		{
			const int32 NumCharsToReplace = CString::Strlen(SearchText);
			const int32 NumCharsToInsert = CString::Strlen(ReplacementText);

			if (NumCharsToInsert == NumCharsToReplace)
			{
				TCHAR* Pos = SearchCase == ESearchCase::IgnoreCase ? CString::Stristr(&(*this)[0], SearchText) : CString::Strstr(&(*this)[0], SearchText);
				while (Pos != nullptr)
				{
					ReplacementCount++;

					// CString::Strcpy now inserts a terminating zero so can't use that
					for (int32 i = 0; i < NumCharsToInsert; i++)
					{
						Pos[i] = ReplacementText[i];
					}

					if (Pos + NumCharsToReplace - **this < Len())
					{
						Pos = SearchCase == ESearchCase::IgnoreCase ? CString::Stristr(Pos + NumCharsToReplace, SearchText) : CString::Strstr(Pos + NumCharsToReplace, SearchText);
					}
					else
					{
						break;
					}
				}
			}
			else if (Contains(SearchText, SearchCase))
			{
				String Copy(*this);
				Clear(Len());

				// get a pointer into the character data
				TCHAR* WritePosition = (TCHAR*)Copy.Data.Data();
				// look for From in the remaining string
				TCHAR* SearchPosition = SearchCase == ESearchCase::IgnoreCase ? CString::Stristr(WritePosition, SearchText) : CString::Strstr(WritePosition, SearchText);
				while (SearchPosition != nullptr)
				{
					ReplacementCount++;

					// replace the first letter of the From with 0 so we can do a strcpy (String +=)
					*SearchPosition = 0;

					// copy everything up to the SearchPosition
					(*this) += WritePosition;

					// copy over the ReplacementText
					(*this) += ReplacementText;

					// restore the letter, just so we don't have 0's in the string
					*SearchPosition = *SearchText;

					WritePosition = SearchPosition + NumCharsToReplace;
					SearchPosition = SearchCase == ESearchCase::IgnoreCase ? CString::Stristr(WritePosition, SearchText) : CString::Strstr(WritePosition, SearchText);
				}

				// copy anything left over
				(*this) += WritePosition;
			}
		}

		return ReplacementCount;
	}


	/**
	* Returns a copy of this string with all quote marks escaped (unless the quote is already escaped)
	*/
	String String::ReplaceQuotesWithEscapedQuotes() const
	{
		if (Contains(EDX_TEXT("\""), ESearchCase::CaseSensitive))
		{
			String Result;

			const TCHAR* pChar = **this;

			bool bEscaped = false;
			while (*pChar != 0)
			{
				if (bEscaped)
				{
					bEscaped = false;
				}
				else if (*pChar == TCHAR('\\'))
				{
					bEscaped = true;
				}
				else if (*pChar == TCHAR('"'))
				{
					Result += TCHAR('\\');
				}

				Result += *pChar++;
			}

			return Result;
		}

		return *this;
	}

	static const TCHAR* CharToEscapeSeqMap[][2] =
	{
		// Always replace \\ first to avoid double-escaping characters
		{ EDX_TEXT("\\"), EDX_TEXT("\\\\") },
		{ EDX_TEXT("\n"), EDX_TEXT("\\n") },
		{ EDX_TEXT("\r"), EDX_TEXT("\\r") },
		{ EDX_TEXT("\t"), EDX_TEXT("\\t") },
		{ EDX_TEXT("\'"), EDX_TEXT("\\'") },
		{ EDX_TEXT("\""), EDX_TEXT("\\\"") }
	};

	static const uint32 MaxSupportedEscapeChars = ARRAY_COUNT(CharToEscapeSeqMap);

	/**
	* Replaces certain characters with the "escaped" version of that character (i.e. replaces "\n" with "\\n").
	* The characters supported are: { \n, \r, \t, \', \", \\ }.
	*
	* @param	Chars	by default, replaces all supported characters; this parameter allows you to limit the replacement to a subset.
	*
	* @return	a string with all control characters replaced by the escaped version.
	*/
	String String::ReplaceCharWithEscapedChar(const Array<TCHAR>* Chars/*=nullptr*/) const
	{
		if (Len() > 0 && (Chars == nullptr || Chars->Size() > 0))
		{
			String Result(*this);
			for (int32 ChIdx = 0; ChIdx < MaxSupportedEscapeChars; ChIdx++)
			{
				if (Chars == nullptr || Chars->Contains(*(CharToEscapeSeqMap[ChIdx][0])))
				{
					// use ReplaceInline as that won't create a copy of the string if the character isn't found
					Result.ReplaceInline(CharToEscapeSeqMap[ChIdx][0], CharToEscapeSeqMap[ChIdx][1]);
				}
			}
			return Result;
		}

		return *this;
	}
	/**
	* Removes the escape backslash for all supported characters, replacing the escape and character with the non-escaped version.  (i.e.
	* replaces "\\n" with "\n".  Counterpart to ReplaceCharWithEscapedChar().
	*/
	String String::ReplaceEscapedCharWithChar(const Array<TCHAR>* Chars/*=nullptr*/) const
	{
		if (Len() > 0 && (Chars == nullptr || Chars->Size() > 0))
		{
			String Result(*this);
			// Spin CharToEscapeSeqMap backwards to ensure we're doing the inverse of ReplaceCharWithEscapedChar
			for (int32 ChIdx = MaxSupportedEscapeChars - 1; ChIdx >= 0; ChIdx--)
			{
				if (Chars == nullptr || Chars->Contains(*(CharToEscapeSeqMap[ChIdx][0])))
				{
					// use ReplaceInline as that won't create a copy of the string if the character isn't found
					Result.ReplaceInline(CharToEscapeSeqMap[ChIdx][1], CharToEscapeSeqMap[ChIdx][0]);
				}
			}
			return Result;
		}

		return *this;
	}

	/**
	* Replaces all instances of '\t' with TabWidth number of spaces
	* @param InSpacesPerTab - Number of spaces that a tab represents
	*/
	String String::ConvertTabsToSpaces(const int32 InSpacesPerTab)
	{
		//must call this with at least 1 space so the modulus operation works
		Assert(InSpacesPerTab > 0);

		String FinalString = *this;
		int32 TabIndex;
		while ((TabIndex = FinalString.Find(EDX_TEXT("\t"))) != INDEX_NONE)
		{
			String LeftSide = FinalString.Left(TabIndex);
			String RightSide = FinalString.Mid(TabIndex + 1);

			FinalString = LeftSide;
			//for a tab size of 4, 
			int32 LineBegin = LeftSide.Find(EDX_TEXT("\n"), ESearchCase::IgnoreCase, ESearchDir::FromEnd, TabIndex);
			if (LineBegin == INDEX_NONE)
			{
				LineBegin = 0;
			}
			int32 CharactersOnLine = (LeftSide.Len() - LineBegin);

			int32 NumSpacesForTab = InSpacesPerTab - (CharactersOnLine % InSpacesPerTab);
			for (int32 i = 0; i < NumSpacesForTab; ++i)
			{
				FinalString.AppendChar(' ');
			}
			FinalString += RightSide;
		}

		return FinalString;
	}

	// This starting size catches 99.97% of printf calls - there are about 700k printf calls per level
#define STARTING_BUFFER_SIZE		512

	VARARG_BODY(String, String::Printf, const TCHAR*, VARARG_NONE)
	{
		int32		BufferSize = STARTING_BUFFER_SIZE;
		TCHAR	StartingBuffer[STARTING_BUFFER_SIZE];
		TCHAR*	Buffer = StartingBuffer;
		int32		Result = -1;

		// First try to print to a stack allocated location 
		GET_VARARGS_RESULT(Buffer, BufferSize, BufferSize - 1, Fmt, Fmt, Result);

		// If that fails, start allocating regular memory
		if (Result == -1)
		{
			Buffer = nullptr;
			while (Result == -1)
			{
				BufferSize *= 2;
				Buffer = (TCHAR*)Memory::AlignedRealloc(Buffer, BufferSize * sizeof(TCHAR));
				GET_VARARGS_RESULT(Buffer, BufferSize, BufferSize - 1, Fmt, Fmt, Result);
			};
		}

		Buffer[Result] = 0;

		String ResultString(Buffer);

		if (BufferSize != STARTING_BUFFER_SIZE)
		{
			Memory::Free(Buffer);
		}

		return ResultString;
	}

	Stream& operator >> (Stream& stream, String& A)
	{
		int32 SaveNum;
		stream >> SaveNum;

		bool LoadUCS2Char = SaveNum < 0;
		if (LoadUCS2Char)
		{
			SaveNum = -SaveNum;
		}

		// If SaveNum is still less than 0, they must have passed in MIN_INT. Archive is corrupted.
		if (SaveNum < 0)
		{
			AssertNoEntry();
		}

		//auto MaxSerializeSize = stream.GetMaxSerializeSize();
		//// Protect against network packets allocating too much memory
		//if ((MaxSerializeSize > 0) && (SaveNum > MaxSerializeSize))
		//{
		//	stream.ArIsError = 1;
		//	stream.ArIsCriticalError = 1;
		//	UE_LOG(LogNetSerialization, Error, EDX_TEXT("String is too large"));
		//	return stream;
		//}

		// Resize the array only if it passes the above tests to prevent rogue packets from crashing
		A.Data.Clear(SaveNum);
		A.Data.AddUninitialized(SaveNum);

		if (SaveNum)
		{
			if (LoadUCS2Char)
			{
				// read in the unicode string and byteswap it, etc
				stream.Read(A.Data.Data(), SaveNum * sizeof(WIDECHAR));
				// Ensure the string has a null terminator
				A[SaveNum - 1] = '\0';


				// Since Microsoft's vsnwprintf implementation raises an invalid parameter warning
				// with a character of 0xffff, scan for it and terminate the string there.
				// 0xffff isn't an actual Unicode character anyway.
				int Index = 0;
				if (A.FindChar(TCHAR(0xffff), Index))
				{
					A[Index] = '\0';
					A.TrimToNullTerminator();
				}
			}
			else
			{
				stream.Read(A.Data.Data(), SaveNum * sizeof(ANSICHAR));
				// Ensure the string has a null terminator
				A[SaveNum - 1] = '\0';
			}

			// Throw away empty string.
			if (SaveNum == 1)
			{
				A.Data.Clear();
			}
		}

		return stream;
	}

	Stream& operator<<(Stream& stream, String& A)
	{
		// > 0 for ANSICHAR, < 0 for UCS2CHAR serialization
		bool  SaveUCS2Char = !CString::IsPureAnsi(*A);
		int32 Num = A.Data.Size();
		int32 SaveNum = SaveUCS2Char ? -Num : Num;

		stream << SaveNum;

		if (SaveNum)
		{
			if (SaveUCS2Char)
			{
				stream.Write((void*)StringCast<WIDECHAR>(A.Data.Data(), Num).Get(), sizeof(WIDECHAR)* Num);
			}
			else
			{
				stream.Write((void*)StringCast<ANSICHAR>(A.Data.Data(), Num).Get(), sizeof(ANSICHAR)* Num);
			}
		}

		return stream;
	}

	int32 FindMatchingClosingParenthesis(const String& TargetString, const int32 StartSearch)
	{
		Assert(StartSearch >= 0 && StartSearch <= TargetString.Len());// Check for usage, we do not accept INDEX_NONE like other string functions

		const TCHAR* const StartPosition = (*TargetString) + StartSearch;
		const TCHAR* CurrPosition = StartPosition;
		int32 ParenthesisCount = 0;

		// Move to first open parenthesis
		while (*CurrPosition != 0 && *CurrPosition != EDX_TEXT('('))
		{
			++CurrPosition;
		}

		// Did we find the open parenthesis
		if (*CurrPosition == EDX_TEXT('('))
		{
			++ParenthesisCount;
			++CurrPosition;

			while (*CurrPosition != 0 && ParenthesisCount > 0)
			{
				if (*CurrPosition == EDX_TEXT('('))
				{
					++ParenthesisCount;
				}
				else if (*CurrPosition == EDX_TEXT(')'))
				{
					--ParenthesisCount;
				}
				++CurrPosition;
			}

			// Did we find the matching close parenthesis
			if (ParenthesisCount == 0 && *(CurrPosition - 1) == EDX_TEXT(')'))
			{
				return StartSearch + ((CurrPosition - 1) - StartPosition);
			}
		}

		return INDEX_NONE;
	}

}