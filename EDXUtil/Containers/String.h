#pragma once

#include "Array.h"
#include "Map.h"
#include "../Core/Crc.h"
#include "../Core/CString.h"
#include "../Core/Template.h"
#include "../Math/EDXMath.h"

namespace EDX
{
	/** Determines case sensitivity options for string comparisons. */
	enum class ESearchCase
	{
		/** Case sensitive. Upper/lower casing must match for strings to be considered equal. */
		CaseSensitive,

		/** Ignore case. Upper/lower casing does not matter when making a comparison. */
		IgnoreCase,
	};

	/** Determines search direction for string operations. */
	enum class ESearchDir
	{
		/** Search from the start, moving forward through the string. */
		FromStart,

		/** Search from the end, moving backward through the string. */
		FromEnd,
	};

	/**
	* A dynamically sizeable string.
	*/
	class String
	{
	private:
		friend struct ContainerTraits<String>;

		/** Array holding the character data */
		typedef Array<TCHAR> DataType;
		DataType Data;

	public:

		String() = default;
		String(String&&) = default;
		String(const String&) = default;
		String& operator=(String&&) = default;
		String& operator=(const String&) = default;

		/**
		* Create a copy of the Other string with extra space for characters at the end of the string
		*
		* @param Other the other string to create a new copy from
		* @param ExtraSlack number of extra characters to add to the end of the other string in this string
		*/
		__forceinline String(const String& Other, int32 ExtraSlack)
			: Data(Other.Data, ExtraSlack + ((Other.Data.Size() || !ExtraSlack) ? 0 : 1)) // Add 1 if the source string array is empty and we want some slack, because we'll need to include a null terminator which is currently missing
		{
		}

		/**
		* Create a copy of the Other string with extra space for characters at the end of the string
		*
		* @param Other the other string to create a new copy from
		* @param ExtraSlack number of extra characters to add to the end of the other string in this string
		*/
		__forceinline String(String&& Other, int32 ExtraSlack)
			: Data(Move(Other.Data), ExtraSlack + ((Other.Data.Size() || !ExtraSlack) ? 0 : 1)) // Add 1 if the source string array is empty and we want some slack, because we'll need to include a null terminator which is currently missing
		{
		}

		/**
		* Constructor using an array of TCHAR
		*
		* @param In array of TCHAR
		*/
		template <typename CharType>
		__forceinline String(const CharType* Src, typename EnableIf<IsCharType<CharType>::Value>::Type* Dummy = nullptr) // This EnableIf is to ensure we don't instantiate this constructor for non-char types, like id* in Obj-C
		{
			if (Src && *Src)
			{
				int32 SrcLen = TCString<CharType>::Strlen(Src) + 1;
				int32 DestLen = CStringUtil::ConvertedLength<TCHAR>(Src, SrcLen);
				Data.AddUninitialized(DestLen);

				CStringUtil::Convert(Data.Data(), DestLen, Src, SrcLen);
			}
		}

		/**
		* Constructor to create String with specified number of characters from another string with additional character zero
		*
		* @param InCount how many characters to copy
		* @param InSrc String to copy from
		*/
		__forceinline explicit String(int32 InCount, const TCHAR* InSrc)
		{
			Data.AddUninitialized(InCount ? InCount + 1 : 0);

			if (Data.Size() > 0)
			{
				CString::Strncpy(Data.Data(), InCount + 1, InSrc, InCount);
			}
		}

		/**
		* Copy Assignment from array of TCHAR
		*
		* @param Other array of TCHAR
		*/
		__forceinline String& operator=(const TCHAR* Other)
		{
			if (Data.Data() != Other)
			{
				int32 Len = (Other && *Other) ? CString::Strlen(Other) + 1 : 0;
				Data.Clear(Len);
				Data.AddUninitialized(Len);

				if (Len)
				{
					Memory::Memcpy(Data.Data(), Other, Len * sizeof(TCHAR));
				}
			}
			return *this;
		}

		/**
		* Return specific character from this string
		*
		* @param Index into string
		* @return Character at Index
		*/
		__forceinline TCHAR& operator[](int32 Index)
		{
			Assertf(IsValidIndex(Index), EDX_TEXT("String index out of bounds: Index %i from a string with a length of %i"), Index, Len());
			return Data.Data()[Index];
		}

		/**
		* Return specific const character from this string
		*
		* @param Index into string
		* @return const Character at Index
		*/
		__forceinline const TCHAR& operator[](int32 Index) const
		{
			Assertf(IsValidIndex(Index), EDX_TEXT("String index out of bounds: Index %i from a string with a length of %i"), Index, Len());
			return Data.Data()[Index];
		}

		/**
		* Iterator typedefs
		*/
		typedef Array<TCHAR>::Iterator      Iterator;
		typedef Array<TCHAR>::ConstIterator ConstIterator;

		/** Creates an iterator for the characters in this string */
		__forceinline Iterator CreateIterator()
		{
			return Data.CreateIterator();
		}

		/** Creates a const iterator for the characters in this string */
		__forceinline ConstIterator CreateConstIterator() const
		{
			return Data.CreateConstIterator();
		}

	private:
		/**
		* DO NOT USE DIRECTLY
		* STL-like iterators to enable range-based for loop support.
		*/
		__forceinline friend DataType::RangedForIteratorType      begin(String& Str) { auto Result = begin(Str.Data); return Result; }
		__forceinline friend DataType::RangedForConstIteratorType begin(const String& Str) { auto Result = begin(Str.Data); return Result; }
		__forceinline friend DataType::RangedForIteratorType      end(String& Str) { auto Result = end(Str.Data); if (Str.Data.Size()) { --Result; } return Result; }
		__forceinline friend DataType::RangedForConstIteratorType end(const String& Str) { auto Result = end(Str.Data); if (Str.Data.Size()) { --Result; } return Result; }

	public:
		__forceinline uint32 GetAllocatedSize() const
		{
			return Data.GetAllocatedSize();
		}

		/**
		* Run slow checks on this string
		*/
		__forceinline void CheckInvariants() const
		{
			int32 Num = Data.Size();
			Assert(Num >= 0);
			Assert(!Num || !Data.Data()[Num - 1]);
			Assert(Data.GetSlack() >= 0);
		}

		/**
		* Create empty string of given size with zero terminating character
		*
		* @param Slack length of empty string to create
		*/
		__forceinline void Clear(int32 Slack = 0)
		{
			Data.Clear(Slack);
		}

		/**
		* Test whether this string is empty
		*
		* @return true if this string is empty, otherwise return false.
		*/
		__forceinline bool IsEmpty() const
		{
			return Data.Size() <= 1;
		}

		/**
		* Empties the string, but doesn't change memory allocation, unless the new size is larger than the current string.
		*
		* @param NewReservedSize The expected usage size (in characters, not including the terminator) after calling this function.
		*/
		__forceinline void Reset(int32 NewReservedSize = 0)
		{
			const int32 NewSizeIncludingTerminator = (NewReservedSize > 0) ? (NewReservedSize + 1) : 0;
			Data.Reset(NewSizeIncludingTerminator);
		}

		/**
		* Remove unallocated empty character space from the end of this string
		*/
		__forceinline void Shrink()
		{
			Data.Shrink();
		}

		/**
		* Tests if index is valid, i.e. greater than or equal to zero, and less than the number of characters in this string (excluding the null terminator).
		*
		* @param Index Index to test.
		*
		* @returns True if index is valid. False otherwise.
		*/
		__forceinline bool IsValidIndex(int32 Index) const
		{
			return Index >= 0 && Index < Len();
		}

		/**
		* Get pointer to the string
		*
		* @Return Pointer to Array of TCHAR if Num, otherwise the empty string
		*/
		__forceinline const TCHAR* operator*() const
		{
			return Data.Size() ? Data.Data() : EDX_TEXT("");
		}

		/**
		*Get string as array of TCHARS
		*
		* @warning: Operations on the Array<*CHAR> can be unsafe, such as adding
		*		non-terminating 0's or removing the terminating zero.
		*/
		__forceinline DataType& GetCharArray()
		{
			return Data;
		}

		/** Get string as const array of TCHARS */
		__forceinline const DataType& GetCharArray() const
		{
			return Data;
		}

		/**
		* Appends an array of characters to the string.
		*
		* @param Array A pointer to the start of an array of characters to append.  This array need not be null-terminated, and null characters are not treated specially.
		* @param Count The number of characters to copy from Array.
		*/
		__forceinline void AppendChars(const TCHAR* Array, int32 Count)
		{
			Assert(Count >= 0);

			if (!Count)
				return;

			Assert(Array);

			int32 Index = Data.Size();

			// Reserve enough space - including an extra gap for a null terminator if we don't already have a string allocated
			Data.AddUninitialized(Count + (Index ? 0 : 1));

			TCHAR* EndPtr = Data.Data() + Index - (Index ? 1 : 0);

			// Copy characters to end of string, overwriting null terminator if we already have one
			CopyAssignItems(EndPtr, Array, Count);

			// (Re-)establish the null terminator
			*(EndPtr + Count) = 0;
		}

		/**
		* Concatenate this with given string
		*
		* @param Str array of TCHAR to be concatenated onto the end of this
		* @return reference to this
		*/
		__forceinline String& operator+=(const TCHAR* Str)
		{
			Assert(Str);
			CheckInvariants();

			AppendChars(Str, CString::Strlen(Str));

			return *this;
		}

		/**
		* Concatenate this with given char
		*
		* @param inChar other Char to be concatenated onto the end of this string
		* @return reference to this
		*/
		__forceinline String& operator+=(const TCHAR InChar)
		{
			CheckInvariants();

			if (InChar != 0)
			{
				// position to insert the character.  
				// At the end of the string if we have existing characters, otherwise at the 0 position
				int32 InsertIndex = (Data.Size() > 0) ? Data.Size() - 1 : 0;

				// number of characters to add.  If we don't have any existing characters, 
				// we'll need to append the terminating zero as well.
				int32 InsertCount = (Data.Size() > 0) ? 1 : 2;

				Data.AddUninitialized(InsertCount);
				Data[InsertIndex] = InChar;
				Data[InsertIndex + 1] = 0;
			}
			return *this;
		}

		/**
		* Concatenate this with given char
		*
		* @param InChar other Char to be concatenated onto the end of this string
		* @return reference to this
		*/
		__forceinline String& AppendChar(const TCHAR InChar)
		{
			*this += InChar;
			return *this;
		}

		__forceinline String& Append(const String& EDX_TEXT)
		{
			*this += EDX_TEXT;
			return *this;
		}

		String& Append(const TCHAR* EDX_TEXT, int32 Count)
		{
			CheckInvariants();

			if (Count != 0)
			{
				// position to insert the character.  
				// At the end of the string if we have existing characters, otherwise at the 0 position
				int32 InsertIndex = (Data.Size() > 0) ? Data.Size() - 1 : 0;

				// number of characters to add.  If we don't have any existing characters, 
				// we'll need to append the terminating zero as well.
				int32 FinalCount = (Data.Size() > 0) ? Count : Count + 1;

				Data.AddUninitialized(FinalCount);

				for (int32 Index = 0; Index < Count; Index++)
				{
					Data[InsertIndex + Index] = EDX_TEXT[Index];
				}

				Data[Data.Size() - 1] = 0;
			}
			return *this;
		}

		/**
		* Removes characters within the string.
		*
		* @param Index           The index of the first character to remove.
		* @param Count           The number of characters to remove.
		* @param bAllowShrinking Whether or not to reallocate to shrink the storage after removal.
		*/
		__forceinline void RemoveAt(int32 Index, int32 Count = 1, bool bAllowShrinking = true)
		{
			Data.RemoveAt(Index, Count, bAllowShrinking);
		}

		__forceinline void InsertAt(int32 Index, TCHAR Character)
		{
			if (Character != 0)
			{
				if (Data.Size() == 0)
				{
					*this += Character;
				}
				else
				{
					Data.Insert(Character, Index);
				}
			}
		}

		__forceinline void InsertAt(int32 Index, const String& Characters)
		{
			if (Characters.Len())
			{
				if (Data.Size() == 0)
				{
					*this += Characters;
				}
				else
				{
					Data.Insert(Characters.Data.Data(), Characters.Len(), Index);
				}
			}
		}

		/**
		* Removes the text from the start of the string if it exists.
		*
		* @param InPrefix the prefix to search for at the start of the string to remove.
		* @return true if the prefix was removed, otherwise false.
		*/
		bool RemoveFromStart(const String& InPrefix, ESearchCase SearchCase = ESearchCase::IgnoreCase);

		/**
		* Removes the text from the end of the string if it exists.
		*
		* @param InSuffix the suffix to search for at the end of the string to remove.
		* @return true if the suffix was removed, otherwise false.
		*/
		bool RemoveFromEnd(const String& InSuffix, ESearchCase SearchCase = ESearchCase::IgnoreCase);

		/**
		* Concatenate this with given string
		*
		* @param Str other string to be concatenated onto the end of this
		* @return reference to this
		*/
		__forceinline String& operator+=(const String& Str)
		{
			CheckInvariants();
			Str.CheckInvariants();

			AppendChars(Str.Data.Data(), Str.Len());

			return *this;
		}

		/**
		* Concatenates an String with a TCHAR.
		*
		* @param Lhs The String on the left-hand-side of the expression.
		* @param Rhs The char on the right-hand-side of the expression.
		*
		* @return The concatenated string.
		*/
		__forceinline friend String operator+(const String& Lhs, TCHAR Rhs)
		{
			Lhs.CheckInvariants();

			String Result(Lhs, 1);
			Result += Rhs;

			return Result;
		}

		/**
		* Concatenates an String with a TCHAR.
		*
		* @param Lhs The String on the left-hand-side of the expression.
		* @param Rhs The char on the right-hand-side of the expression.
		*
		* @return The concatenated string.
		*/
		__forceinline friend String operator+(String&& Lhs, TCHAR Rhs)
		{
			Lhs.CheckInvariants();

			String Result(Move(Lhs), 1);
			Result += Rhs;

			return Result;
		}

	private:
		template <typename LhsType, typename RhsType>
		__forceinline static String ConcatFStrings(typename Identity<LhsType>::Type Lhs, typename Identity<RhsType>::Type Rhs)
		{
			Lhs.CheckInvariants();
			Rhs.CheckInvariants();

			if (Lhs.IsEmpty())
				return Move(Rhs);

			int32 RhsLen = Rhs.Len();

			String Result(Move(Lhs), RhsLen);
			Result.AppendChars(Rhs.Data.Data(), RhsLen);

			return Result;
		}

		template <typename RhsType>
		__forceinline static String ConcatTCHARsToFString(const TCHAR* Lhs, typename Identity<RhsType>::Type Rhs)
		{
			Assert(Lhs);
			Rhs.CheckInvariants();

			if (!Lhs || !*Lhs)
				return Move(Rhs);

			int32 LhsLen = CString::Strlen(Lhs);
			int32 RhsLen = Rhs.Len();

			// This is not entirely optimal, as if the Rhs is an rvalue and has enough slack space to hold Lhs, then
			// the memory could be reused here without constructing a new object.  However, until there is proof otherwise,
			// I believe this will be relatively rare and isn't worth making the code a lot more complex right now.
			String Result;
			Result.Data.AddUninitialized(LhsLen + RhsLen + 1);

			TCHAR* ResultData = Result.Data.Data();
			CopyAssignItems(ResultData, Lhs, LhsLen);
			CopyAssignItems(ResultData + LhsLen, Rhs.Data.Data(), RhsLen);
			*(ResultData + LhsLen + RhsLen) = 0;

			return Result;
		}

		template <typename LhsType>
		__forceinline static String ConcatFStringToTCHARs(typename Identity<LhsType>::Type Lhs, const TCHAR* Rhs)
		{
			Lhs.CheckInvariants();
			Assert(Rhs);

			if (!Rhs || !*Rhs)
				return Move(Lhs);

			int32 RhsLen = CString::Strlen(Rhs);

			String Result(Move(Lhs), RhsLen);
			Result.AppendChars(Rhs, RhsLen);

			return Result;
		}

	public:
		/**
		* Concatenate two FStrings.
		*
		* @param Lhs The String on the left-hand-side of the expression.
		* @param Rhs The String on the right-hand-side of the expression.
		*
		* @return The concatenated string.
		*/
		__forceinline friend String operator+(const String& Lhs, const String& Rhs)
		{
			return ConcatFStrings<const String&, const String&>(Lhs, Rhs);
		}

		/**
		* Concatenate two FStrings.
		*
		* @param Lhs The String on the left-hand-side of the expression.
		* @param Rhs The String on the right-hand-side of the expression.
		*
		* @return The concatenated string.
		*/
		__forceinline friend String operator+(String&& Lhs, const String& Rhs)
		{
			return ConcatFStrings<String&&, const String&>(Move(Lhs), Rhs);
		}

		/**
		* Concatenate two FStrings.
		*
		* @param Lhs The String on the left-hand-side of the expression.
		* @param Rhs The String on the right-hand-side of the expression.
		*
		* @return The concatenated string.
		*/
		__forceinline friend String operator+(const String& Lhs, String&& Rhs)
		{
			return ConcatFStrings<const String&, String&&>(Lhs, Move(Rhs));
		}

		/**
		* Concatenate two FStrings.
		*
		* @param Lhs The String on the left-hand-side of the expression.
		* @param Rhs The String on the right-hand-side of the expression.
		*
		* @return The concatenated string.
		*/
		__forceinline friend String operator+(String&& Lhs, String&& Rhs)
		{
			return ConcatFStrings<String&&, String&&>(Move(Lhs), Move(Rhs));
		}

		/**
		* Concatenates a TCHAR array to an String.
		*
		* @param Lhs The TCHAR array on the left-hand-side of the expression.
		* @param Rhs The String on the right-hand-side of the expression.
		*
		* @return The concatenated string.
		*/
		__forceinline friend String operator+(const TCHAR* Lhs, const String& Rhs)
		{
			return ConcatTCHARsToFString<const String&>(Lhs, Rhs);
		}

		/**
		* Concatenates a TCHAR array to an String.
		*
		* @param Lhs The TCHAR array on the left-hand-side of the expression.
		* @param Rhs The String on the right-hand-side of the expression.
		*
		* @return The concatenated string.
		*/
		__forceinline friend String operator+(const TCHAR* Lhs, String&& Rhs)
		{
			return ConcatTCHARsToFString<String&&>(Lhs, Move(Rhs));
		}

		/**
		* Concatenates an String to a TCHAR array.
		*
		* @param Lhs The String on the left-hand-side of the expression.
		* @param Rhs The TCHAR array on the right-hand-side of the expression.
		*
		* @return The concatenated string.
		*/
		__forceinline friend String operator+(const String& Lhs, const TCHAR* Rhs)
		{
			return ConcatFStringToTCHARs<const String&>(Lhs, Rhs);
		}

		/**
		* Concatenates an String to a TCHAR array.
		*
		* @param Lhs The String on the left-hand-side of the expression.
		* @param Rhs The TCHAR array on the right-hand-side of the expression.
		*
		* @return The concatenated string.
		*/
		__forceinline friend String operator+(String&& Lhs, const TCHAR* Rhs)
		{
			return ConcatFStringToTCHARs<String&&>(Move(Lhs), Rhs);
		}

		/**
		* Concatenate this path with given path ensuring the / character is used between them
		*
		* @param Str path array of TCHAR to be concatenated onto the end of this
		* @return reference to path
		*/
		__forceinline String& operator/=(const TCHAR* Str)
		{
			if (Data.Size() > 1 && Data[Data.Size() - 2] != EDX_TEXT('/') && Data[Data.Size() - 2] != EDX_TEXT('\\') && (!Str || *Str != EDX_TEXT('/')))
			{
				*this += EDX_TEXT("/");
			}
			return *this += Str;
		}

		/**
		* Concatenate this path with given path ensuring the / character is used between them
		*
		* @param Str path String to be concatenated onto the end of this
		* @return reference to path
		*/
		__forceinline String& operator/=(const String& Str)
		{
			return operator/=(*Str);
		}

		/**
		* Concatenate this path with given path ensuring the / character is used between them
		*
		* @param Lhs Path to concatenate onto.
		* @param Rhs Path to concatenate.
		* @return new String of the path
		*/
		__forceinline friend String operator/(const String& Lhs, const TCHAR* Rhs)
		{
			return String(Lhs) /= Rhs;
		}

		/**
		* Concatenate this path with given path ensuring the / character is used between them
		*
		* @param Lhs Path to concatenate onto.
		* @param Rhs Path to concatenate.
		* @return new String of the path
		*/
		__forceinline friend String operator/(String&& Lhs, const TCHAR* Rhs)
		{
			return String(Move(Lhs)) /= Rhs;
		}

		/**
		* Concatenate this path with given path ensuring the / character is used between them
		*
		* @param Lhs Path to concatenate onto.
		* @param Rhs Path to concatenate.
		* @return new String of the path
		*/
		__forceinline friend String operator/(const String& Lhs, const String& Rhs)
		{
			return String(Lhs) /= *Rhs;
		}

		/**
		* Concatenate this path with given path ensuring the / character is used between them
		*
		* @param Lhs Path to concatenate onto.
		* @param Rhs Path to concatenate.
		* @return new String of the path
		*/
		__forceinline friend String operator/(String&& Lhs, const String& Rhs)
		{
			return String(Move(Lhs)) /= *Rhs;
		}

		/**
		* Concatenate this path with given path ensuring the / character is used between them
		*
		* @param Lhs Path to concatenate onto.
		* @param Rhs Path to concatenate.
		* @return new String of the path
		*/
		__forceinline friend String operator/(const TCHAR* Lhs, const String& Rhs)
		{
			return String(Lhs) /= *Rhs;
		}

		/**
		* Lexicographically test whether the left string is <= the right string
		*
		* @param Lhs String to compare against.
		* @param Rhs String to compare against.
		* @return true if the left string is lexicographically <= the right string, otherwise false
		* @note case insensitive
		*/
		__forceinline friend bool operator<=(const String& Lhs, const String& Rhs)
		{
			return CStringUtil::Stricmp(*Lhs, *Rhs) <= 0;
		}

		/**
		* Lexicographically test whether the left string is <= the right string
		*
		* @param Lhs String to compare against.
		* @param Rhs String to compare against.
		* @return true if the left string is lexicographically <= the right string, otherwise false
		* @note case insensitive
		*/
		template <typename CharType>
		__forceinline friend bool operator<=(const String& Lhs, const CharType* Rhs)
		{
			return CStringUtil::Stricmp(*Lhs, Rhs) <= 0;
		}

		/**
		* Lexicographically test whether the left string is <= the right string
		*
		* @param Lhs String to compare against.
		* @param Rhs String to compare against.
		* @return true if the left string is lexicographically <= the right string, otherwise false
		* @note case insensitive
		*/
		template <typename CharType>
		__forceinline friend bool operator<=(const CharType* Lhs, const String& Rhs)
		{
			return CStringUtil::Stricmp(Lhs, *Rhs) <= 0;
		}

		/**
		* Lexicographically test whether the left string is < the right string
		*
		* @param Lhs String to compare against.
		* @param Rhs String to compare against.
		* @return true if the left string is lexicographically < the right string, otherwise false
		* @note case insensitive
		*/
		__forceinline friend bool operator<(const String& Lhs, const String& Rhs)
		{
			return CStringUtil::Stricmp(*Lhs, *Rhs) < 0;
		}

		/**
		* Lexicographically test whether the left string is < the right string
		*
		* @param Lhs String to compare against.
		* @param Rhs String to compare against.
		* @return true if the left string is lexicographically < the right string, otherwise false
		* @note case insensitive
		*/
		template <typename CharType>
		__forceinline friend bool operator<(const String& Lhs, const CharType* Rhs)
		{
			return CStringUtil::Stricmp(*Lhs, Rhs) < 0;
		}

		/**
		* Lexicographically test whether the left string is < the right string
		*
		* @param Lhs String to compare against.
		* @param Rhs String to compare against.
		* @return true if the left string is lexicographically < the right string, otherwise false
		* @note case insensitive
		*/
		template <typename CharType>
		__forceinline friend bool operator<(const CharType* Lhs, const String& Rhs)
		{
			return CStringUtil::Stricmp(Lhs, *Rhs) < 0;
		}

		/**
		* Lexicographically test whether the left string is >= the right string
		*
		* @param Lhs String to compare against.
		* @param Rhs String to compare against.
		* @return true if the left string is lexicographically >= the right string, otherwise false
		* @note case insensitive
		*/
		__forceinline friend bool operator>=(const String& Lhs, const String& Rhs)
		{
			return CStringUtil::Stricmp(*Lhs, *Rhs) >= 0;
		}

		/**
		* Lexicographically test whether the left string is >= the right string
		*
		* @param Lhs String to compare against.
		* @param Rhs String to compare against.
		* @return true if the left string is lexicographically >= the right string, otherwise false
		* @note case insensitive
		*/
		template <typename CharType>
		__forceinline friend bool operator>=(const String& Lhs, const CharType* Rhs)
		{
			return CStringUtil::Stricmp(*Lhs, Rhs) >= 0;
		}

		/**
		* Lexicographically test whether the left string is >= the right string
		*
		* @param Lhs String to compare against.
		* @param Rhs String to compare against.
		* @return true if the left string is lexicographically >= the right string, otherwise false
		* @note case insensitive
		*/
		template <typename CharType>
		__forceinline friend bool operator>=(const CharType* Lhs, const String& Rhs)
		{
			return CStringUtil::Stricmp(Lhs, *Rhs) >= 0;
		}

		/**
		* Lexicographically test whether the left string is > the right string
		*
		* @param Lhs String to compare against.
		* @param Rhs String to compare against.
		* @return true if the left string is lexicographically > the right string, otherwise false
		* @note case insensitive
		*/
		__forceinline friend bool operator>(const String& Lhs, const String& Rhs)
		{
			return CStringUtil::Stricmp(*Lhs, *Rhs) > 0;
		}

		/**
		* Lexicographically test whether the left string is > the right string
		*
		* @param Lhs String to compare against.
		* @param Rhs String to compare against.
		* @return true if the left string is lexicographically > the right string, otherwise false
		* @note case insensitive
		*/
		template <typename CharType>
		__forceinline friend bool operator>(const String& Lhs, const CharType* Rhs)
		{
			return CStringUtil::Stricmp(*Lhs, Rhs) > 0;
		}

		/**
		* Lexicographically test whether the left string is > the right string
		*
		* @param Lhs String to compare against.
		* @param Rhs String to compare against.
		* @return true if the left string is lexicographically > the right string, otherwise false
		* @note case insensitive
		*/
		template <typename CharType>
		__forceinline friend bool operator>(const CharType* Lhs, const String& Rhs)
		{
			return CStringUtil::Stricmp(Lhs, *Rhs) > 0;
		}

		/**
		* Lexicographically test whether the left string is == the right string
		*
		* @param Lhs String to compare against.
		* @param Rhs String to compare against.
		* @return true if the left string is lexicographically == the right string, otherwise false
		* @note case insensitive
		*/
		__forceinline friend bool operator==(const String& Lhs, const String& Rhs)
		{
			return CStringUtil::Stricmp(*Lhs, *Rhs) == 0;
		}

		/**
		* Lexicographically test whether the left string is == the right string
		*
		* @param Lhs String to compare against.
		* @param Rhs String to compare against.
		* @return true if the left string is lexicographically == the right string, otherwise false
		* @note case insensitive
		*/
		template <typename CharType>
		__forceinline friend bool operator==(const String& Lhs, const CharType* Rhs)
		{
			return CStringUtil::Stricmp(*Lhs, Rhs) == 0;
		}

		/**
		* Lexicographically test whether the left string is == the right string
		*
		* @param Lhs String to compare against.
		* @param Rhs String to compare against.
		* @return true if the left string is lexicographically == the right string, otherwise false
		* @note case insensitive
		*/
		template <typename CharType>
		__forceinline friend bool operator==(const CharType* Lhs, const String& Rhs)
		{
			return CStringUtil::Stricmp(Lhs, *Rhs) == 0;
		}

		/**
		* Lexicographically test whether the left string is != the right string
		*
		* @param Lhs String to compare against.
		* @param Rhs String to compare against.
		* @return true if the left string is lexicographically != the right string, otherwise false
		* @note case insensitive
		*/
		__forceinline friend bool operator!=(const String& Lhs, const String& Rhs)
		{
			return CStringUtil::Stricmp(*Lhs, *Rhs) != 0;
		}

		/**
		* Lexicographically test whether the left string is != the right string
		*
		* @param Lhs String to compare against.
		* @param Rhs String to compare against.
		* @return true if the left string is lexicographically != the right string, otherwise false
		* @note case insensitive
		*/
		template <typename CharType>
		__forceinline friend bool operator!=(const String& Lhs, const CharType* Rhs)
		{
			return CStringUtil::Stricmp(*Lhs, Rhs) != 0;
		}

		/**
		* Lexicographically test whether the left string is != the right string
		*
		* @param Lhs String to compare against.
		* @param Rhs String to compare against.
		* @return true if the left string is lexicographically != the right string, otherwise false
		* @note case insensitive
		*/
		template <typename CharType>
		__forceinline friend bool operator!=(const CharType* Lhs, const String& Rhs)
		{
			return CStringUtil::Stricmp(Lhs, *Rhs) != 0;
		}

		/** Get the length of the string, excluding terminating character */
		__forceinline int32 Len() const
		{
			return Data.Size() ? Data.Size() - 1 : 0;
		}

		/** @return the left most given number of characters */
		__forceinline String Left(int32 Count) const
		{
			return String(Math::Clamp(Count, 0, Len()), **this);
		}

		/** @return the left most characters from the string chopping the given number of characters from the end */
		__forceinline String LeftChop(int32 Count) const
		{
			return String(Math::Clamp(Len() - Count, 0, Len()), **this);
		}

		/** @return the string to the right of the specified location, counting back from the right (end of the word). */
		__forceinline String Right(int32 Count) const
		{
			return String(**this + Len() - Math::Clamp(Count, 0, Len()));
		}

		/** @return the string to the right of the specified location, counting forward from the left (from the beginning of the word). */
		__forceinline String RightChop(int32 Count) const
		{
			return String(**this + Len() - Math::Clamp(Len() - Count, 0, Len()));
		}

		/** @return the substring from Start position for Count characters. */
		__forceinline String Mid(int32 Start, int32 Count = int32(Math::EDX_INFINITY)) const
		{
			uint32 End = Start + Count;
			Start = Math::Clamp((uint32)Start, (uint32)0, (uint32)Len());
			End = Math::Clamp((uint32)End, (uint32)Start, (uint32)Len());
			return String(End - Start, **this + Start);
		}

		/**
		* Searches the string for a substring, and returns index into this string
		* of the first found instance. Can search from beginning or end, and ignore case or not.
		*
		* @param SubStr			The string array of TCHAR to search for
		* @param StartPosition		The start character position to search from
		* @param SearchCase		Indicates whether the search is case sensitive or not
		* @param SearchDir			Indicates whether the search starts at the begining or at the end.
		*/
		int32 Find(const TCHAR* SubStr, ESearchCase SearchCase = ESearchCase::IgnoreCase,
			ESearchDir SearchDir = ESearchDir::FromStart, int32 StartPosition = INDEX_NONE) const;

		/**
		* Searches the string for a substring, and returns index into this string
		* of the first found instance. Can search from beginning or end, and ignore case or not.
		*
		* @param SubStr			The string to search for
		* @param StartPosition		The start character position to search from
		* @param SearchCase		Indicates whether the search is case sensitive or not ( defaults to ESearchCase::IgnoreCase )
		* @param SearchDir			Indicates whether the search starts at the begining or at the end ( defaults to ESearchDir::FromStart )
		*/
		__forceinline int32 Find(const String& SubStr, ESearchCase SearchCase = ESearchCase::IgnoreCase,
			ESearchDir SearchDir = ESearchDir::FromStart, int32 StartPosition = INDEX_NONE) const
		{
			return Find(*SubStr, SearchCase, SearchDir, StartPosition);
		}

		/**
		* Returns whether this string contains the specified substring.
		*
		* @param SubStr			Find to search for
		* @param SearchCase		Indicates whether the search is case sensitive or not ( defaults to ESearchCase::IgnoreCase )
		* @param SearchDir			Indicates whether the search starts at the begining or at the end ( defaults to ESearchDir::FromStart )
		* @return					Returns whether the string contains the substring
		**/
		__forceinline bool Contains(const TCHAR* SubStr, ESearchCase SearchCase = ESearchCase::IgnoreCase,
			ESearchDir SearchDir = ESearchDir::FromStart) const
		{
			return Find(SubStr, SearchCase, SearchDir) != INDEX_NONE;
		}

		/**
		* Returns whether this string contains the specified substring.
		*
		* @param SubStr			Find to search for
		* @param SearchCase		Indicates whether the search is case sensitive or not ( defaults to ESearchCase::IgnoreCase )
		* @param SearchDir			Indicates whether the search starts at the begining or at the end ( defaults to ESearchDir::FromStart )
		* @return					Returns whether the string contains the substring
		**/
		__forceinline bool Contains(const String& SubStr, ESearchCase SearchCase = ESearchCase::IgnoreCase,
			ESearchDir SearchDir = ESearchDir::FromStart) const
		{
			return Find(*SubStr, SearchCase, SearchDir) != INDEX_NONE;
		}

		/**
		* Searches the string for a character
		*
		* @param InChar the character to search for
		* @param Index out the position the character was found at, INDEX_NONE if return is false
		* @return true if character was found in this string, otherwise false
		*/
		__forceinline bool FindChar(TCHAR InChar, int32& Index) const
		{
			return Data.Find(InChar, Index);
		}

		/**
		* Searches the string for the last occurrence of a character
		*
		* @param InChar the character to search for
		* @param Index out the position the character was found at, INDEX_NONE if return is false
		* @return true if character was found in this string, otherwise false
		*/
		__forceinline bool FindLastChar(TCHAR InChar, int32& Index) const
		{
			return Data.FindLast(InChar, Index);
		}

		/**
		* Searches the string for the last occurrence of a character
		*
		* @param Pred Predicate that takes TCHAR and returns true if TCHAR matches search criteria, false otherwise.
		* @param StartIndex Index of element from which to start searching. Defaults to last TCHAR in string.
		*
		* @return Index of found TCHAR, INDEX_NONE otherwise.
		*/
		template <typename Predicate>
		__forceinline int32 FindLastCharByPredicate(Predicate Pred, int32 StartIndex) const
		{
			Assert(StartIndex >= 0 && StartIndex <= this->Len());
			return Data.FindLastByPredicate(Pred, StartIndex);
		}

		/**
		* Searches the string for the last occurrence of a character
		*
		* @param Pred Predicate that takes TCHAR and returns true if TCHAR matches search criteria, false otherwise.
		* @param StartIndex Index of element from which to start searching. Defaults to last TCHAR in string.
		*
		* @return Index of found TCHAR, INDEX_NONE otherwise.
		*/
		template <typename Predicate>
		__forceinline int32 FindLastCharByPredicate(Predicate Pred) const
		{
			return Data.FindLastByPredicate(Pred, this->Len());
		}

		/**
		* Lexicographically tests whether this string is equivalent to the Other given string
		*
		* @param Other 	The string test against
		* @param SearchCase 	Whether or not the comparison should ignore case
		* @return true if this string is lexicographically equivalent to the other, otherwise false
		*/
		__forceinline bool Equals(const String& Other, ESearchCase SearchCase = ESearchCase::CaseSensitive) const
		{
			if (SearchCase == ESearchCase::CaseSensitive)
			{
				return CString::Strcmp(**this, *Other) == 0;
			}
			else
			{
				return CString::Stricmp(**this, *Other) == 0;
			}
		}

		/**
		* Lexicographically tests how this string compares to the Other given string
		*
		* @param Other 	The string test against
		* @param SearchCase 	Whether or not the comparison should ignore case
		* @return 0 if equal, negative if less than, positive if greater than
		*/
		__forceinline int32 Compare(const String& Other, ESearchCase SearchCase = ESearchCase::CaseSensitive) const
		{
			if (SearchCase == ESearchCase::CaseSensitive)
			{
				return CString::Strcmp(**this, *Other);
			}
			else
			{
				return CString::Stricmp(**this, *Other);
			}
		}

		/**
		* Splits this string at given string position case sensitive.
		*
		* @param InStr The string to search and split at
		* @param LeftS out the string to the left of InStr, not updated if return is false
		* @param RightS out the string to the right of InStr, not updated if return is false
		* @param SearchCase		Indicates whether the search is case sensitive or not ( defaults to ESearchCase::IgnoreCase )
		* @param SearchDir			Indicates whether the search starts at the begining or at the end ( defaults to ESearchDir::FromStart )
		* @return true if string is split, otherwise false
		*/
		bool Split(const String& InS, String* LeftS, String* RightS, ESearchCase SearchCase = ESearchCase::IgnoreCase,
			ESearchDir SearchDir = ESearchDir::FromStart) const
		{
			int32 InPos = Find(InS, SearchCase, SearchDir);

			if (InPos < 0) { return false; }

			if (LeftS) { *LeftS = Left(InPos); }
			if (RightS) { *RightS = Mid(InPos + InS.Len()); }

			return true;
		}

		/** @return a new string with the characters of this converted to uppercase */
		String ToUpper() const;

		/** Converts all characters in this string to uppercase */
		void ToUpperInline();

		/** @return a new string with the characters of this converted to lowercase */
		String ToLower() const;

		/** Converts all characters in this string to lowercase */
		void ToLowerInline();

		/** Pad the left of this string for ChCount characters */
		String LeftPad(int32 ChCount) const;

		/** Pad the right of this string for ChCount characters */
		String RightPad(int32 ChCount) const;

		/** @return true if the string only contains numeric characters */
		bool IsNumeric() const;

		/**
		* Constructs String object similarly to how classic sprintf works.
		*
		* @param Format	Format string that specifies how String should be built optionally using additional args. Refer to standard printf format.
		* @param ...		Depending on format function may require additional arguments to build output object.
		*
		* @returns String object that was constructed using format and additional parameters.
		*/
		VARARG_DECL(static String, static String, return, Printf, VARARG_NONE, const TCHAR*, VARARG_NONE, VARARG_NONE);

		///**
		//* Format the specified string using the specified arguments. Replaces instances of { Argument } with keys in the map matching 'Argument'
		//* @param InFormatString		A string representing the format expression
		//* @param InNamedArguments		A map of named arguments that match the tokens specified in InExpression
		//* @return A string containing the formatted text
		//*/
		//static String Format(const TCHAR* InFormatString, const Map<String, FStringFormatArg>& InNamedArguments);

		///**
		//* Format the specified string using the specified arguments. Replaces instances of {0} with indices from the given array matching the index specified in the token
		//* @param InFormatString		A string representing the format expression
		//* @param InOrderedArguments	An array of ordered arguments that match the tokens specified in InExpression
		//* @return A string containing the formatted text
		//*/
		//static String Format(const TCHAR* InFormatString, const Array<FStringFormatArg>& InOrderedArguments);

		// @return string with Ch character
		static String Chr(TCHAR Ch);

		/**
		* Returns a string that is full of a variable number of characters
		*
		* @param NumCharacters Number of characters to put into the string
		* @param Char Character to put into the string
		*
		* @return The string of NumCharacters characters.
		*/
		static String ChrN(int32 NumCharacters, TCHAR Char);

		/**
		* Serializes the string.
		*
		* @param Ar Reference to the serialization archive.
		* @param S Reference to the string being serialized.
		*
		* @return Reference to the Archive after serialization.
		*/
		//friend CORE_API FArchive& operator<<(FArchive& Ar, String& S);


		/**
		* Test whether this string starts with given string.
		*
		* @param SearchCase		Indicates whether the search is case sensitive or not ( defaults to ESearchCase::IgnoreCase )
		* @return true if this string begins with specified text, false otherwise
		*/
		bool StartsWith(const String& InPrefix, ESearchCase SearchCase = ESearchCase::IgnoreCase) const;

		/**
		* Test whether this string ends with given string.
		*
		* @param SearchCase		Indicates whether the search is case sensitive or not ( defaults to ESearchCase::IgnoreCase )
		* @return true if this string ends with specified text, false otherwise
		*/
		bool EndsWith(const String& InSuffix, ESearchCase SearchCase = ESearchCase::IgnoreCase) const;

		/**
		* Searches this string for a given wild card
		*
		* @param Wildcard		*?-type wildcard
		* @param SearchCase	Indicates whether the search is case sensitive or not ( defaults to ESearchCase::IgnoreCase )
		* @return true if this string matches the *?-type wildcard given.
		* @warning This is a simple, SLOW routine. Use with caution
		*/
		bool MatchesWildcard(const String& Wildcard, ESearchCase SearchCase = ESearchCase::IgnoreCase) const;

		/**
		* Removes whitespace characters from the front of this string.
		*/
		String Trim();

		/**
		* Removes trailing whitespace characters
		*/
		String TrimTrailing(void);

		/**
		* Trims the inner array after the null terminator.
		*/
		void TrimToNullTerminator();

		/**
		* Returns a copy of this string with wrapping quotation marks removed.
		*/
		String TrimQuotes(bool* bQuotesRemoved = nullptr) const;

		/**
		* Breaks up a delimited string into elements of a string array.
		*
		* @param	InArray		The array to fill with the string pieces
		* @param	pchDelim	The string to delimit on
		* @param	InCullEmpty	If 1, empty strings are not added to the array
		*
		* @return	The number of elements in InArray
		*/
		int32 ParseIntoArray(Array<String>& OutArray, const TCHAR* pchDelim, bool InCullEmpty = true) const;

		/**
		* Breaks up a delimited string into elements of a string array, using any whitespace and an
		* optional extra delimter, like a ","
		* @warning Caution!! this routine is O(N^2) allocations...use it for parsing very short text or not at all!
		*
		* @param	InArray			The array to fill with the string pieces
		* @param	pchExtraDelim	The string to delimit on
		*
		* @return	The number of elements in InArray
		*/
		int32 ParseIntoArrayWS(Array<String>& OutArray, const TCHAR* pchExtraDelim = nullptr, bool InCullEmpty = true) const;

		/**
		* Breaks up a delimited string into elements of a string array, using line ending characters
		* @warning Caution!! this routine is O(N^2) allocations...use it for parsing very short text or not at all!
		*
		* @param	InArray			The array to fill with the string pieces
		*
		* @return	The number of elements in InArray
		*/
		int32 ParseIntoArrayLines(Array<String>& OutArray, bool InCullEmpty = true) const;

		/**
		* Breaks up a delimited string into elements of a string array, using the given delimiters
		* @warning Caution!! this routine is O(N^2) allocations...use it for parsing very short text or not at all!
		*
		* @param	InArray			The array to fill with the string pieces
		* @param	DelimArray		The strings to delimit on
		* @param	NumDelims		The number of delimiters.
		*
		* @return	The number of elements in InArray
		*/
		int32 ParseIntoArray(Array<String>& OutArray, const TCHAR** DelimArray, int32 NumDelims, bool InCullEmpty = true) const;

		/**
		* Takes an array of strings and removes any zero length entries.
		*
		* @param	InArray	The array to cull
		*
		* @return	The number of elements left in InArray
		*/
		static int32 CullArray(Array<String>* InArray);

		/**
		* Returns a copy of this string, with the characters in reverse order
		*/
		__forceinline String Reverse() const;

		/**
		* Reverses the order of characters in this string
		*/
		void ReverseString();

		/**
		* Replace all occurrences of a substring in this string
		*
		* @param From substring to replace
		* @param To substring to replace From with
		* @param SearchCase	Indicates whether the search is case sensitive or not ( defaults to ESearchCase::IgnoreCase )
		* @return a copy of this string with the replacement made
		*/
		String Replace(const TCHAR* From, const TCHAR* To, ESearchCase SearchCase = ESearchCase::IgnoreCase) const;

		/**
		* Replace all occurrences of SearchText with ReplacementText in this string.
		*
		* @param	SearchText	the text that should be removed from this string
		* @param	ReplacementText		the text to insert in its place
		* @param SearchCase	Indicates whether the search is case sensitive or not ( defaults to ESearchCase::IgnoreCase )
		*
		* @return	the number of occurrences of SearchText that were replaced.
		*/
		int32 ReplaceInline(const TCHAR* SearchText, const TCHAR* ReplacementText, ESearchCase SearchCase = ESearchCase::IgnoreCase);

		/**
		* Returns a copy of this string with all quote marks escaped (unless the quote is already escaped)
		*/
		String ReplaceQuotesWithEscapedQuotes() const;

		/**
		* Replaces certain characters with the "escaped" version of that character (i.e. replaces "\n" with "\\n").
		* The characters supported are: { \n, \r, \t, \', \", \\ }.
		*
		* @param	Chars	by default, replaces all supported characters; this parameter allows you to limit the replacement to a subset.
		*
		* @return	a string with all control characters replaced by the escaped version.
		*/
		String ReplaceCharWithEscapedChar(const Array<TCHAR>* Chars = nullptr) const;

		/**
		* Removes the escape backslash for all supported characters, replacing the escape and character with the non-escaped version.  (i.e.
		* replaces "\\n" with "\n".  Counterpart to ReplaceCharWithEscapedChar().
		* @return copy of this string with replacement made
		*/
		String ReplaceEscapedCharWithChar(const Array<TCHAR>* Chars = nullptr) const;

		/**
		* Replaces all instances of '\t' with TabWidth number of spaces
		* @param InSpacesPerTab - Number of spaces that a tab represents
		* @return copy of this string with replacement made
		*/
		String ConvertTabsToSpaces(const int32 InSpacesPerTab);

		// Takes the number passed in and formats the string in comma format ( 12345 becomes "12,345")
		static String FormatAsNumber(int32 InNumber);

		// To allow more efficient memory handling, automatically adds one for the string termination.
		__forceinline void Reserve(const uint32 CharacterCount)
		{
			Data.Reserve(CharacterCount + 1);
		}

		/**
		* Serializes a string as ANSI char array.
		*
		* @param	String			String to serialize
		* @param	Ar				Archive to serialize with
		* @param	MinCharacters	Minimum number of characters to serialize.
		*/
		//void SerializeAsANSICharArray(FArchive& Ar, int32 MinCharacters = 0) const;


		/** Converts an integer to a string. */
		static __forceinline String FromInt(int32 Num)
		{
			String Ret;
			Ret.AppendInt(Num);
			return Ret;
		}

		/** appends the integer InNum to this string */
		void AppendInt(int32 InNum);

		/**
		* Converts a string into a boolean value
		*   1, "True", "Yes", GTrue, GYes, and non-zero integers become true
		*   0, "False", "No", GFalse, GNo, and unparsable values become false
		*
		* @return The boolean value
		*/
		bool ToBool() const;

		/**
		* Converts a buffer to a string
		*
		* @param SrcBuffer the buffer to stringify
		* @param SrcSize the number of bytes to convert
		*
		* @return the blob in string form
		*/
		static String FromBlob(const uint8* SrcBuffer, const uint32 SrcSize);

		/**
		* Converts a string into a buffer
		*
		* @param DestBuffer the buffer to fill with the string data
		* @param DestSize the size of the buffer in bytes (must be at least string len / 3)
		*
		* @return true if the conversion happened, false otherwise
		*/
		static bool ToBlob(const String& Source, uint8* DestBuffer, const uint32 DestSize);

		/**
		* Converts a buffer to a string by hex-ifying the elements
		*
		* @param SrcBuffer the buffer to stringify
		* @param SrcSize the number of bytes to convert
		*
		* @return the blob in string form
		*/
		static String FromHexBlob(const uint8* SrcBuffer, const uint32 SrcSize);

		/**
		* Converts a string into a buffer
		*
		* @param DestBuffer the buffer to fill with the string data
		* @param DestSize the size of the buffer in bytes (must be at least string len / 2)
		*
		* @return true if the conversion happened, false otherwise
		*/
		static bool ToHexBlob(const String& Source, uint8* DestBuffer, const uint32 DestSize);

		/**
		* Converts a float string with the trailing zeros stripped
		* For example - 1.234 will be "1.234" rather than "1.234000"
		*
		* @param	InFloat		The float to sanitize
		* @returns sanitized string version of float
		*/
		static String SanitizeFloat(double InFloat);

		/**
		* Joins an array of 'something that can be concatentated to strings with +=' together into a single string with separators.
		*
		* @param	Array		The array of 'things' to concatenate.
		* @param	Separator	The string used to separate each element.
		*
		* @return	The final, joined, separated string.
		*/
		template <typename T, typename Allocator>
		static String Join(const Array<T, Allocator>& Array, const TCHAR* Separator)
		{
			String Result;
			bool    First = true;
			for (const T& Element : Array)
			{
				if (First)
				{
					First = false;
				}
				else
				{
					Result += Separator;
				}

				Result += Element;
			}

			return Result;
		}
	};

	template<>
	struct ContainerTraits<String> : public ContainerTraitsBase<String>
	{
		enum { MoveWillEmptyContainer = ContainerTraits<String::DataType>::MoveWillEmptyContainer };
	};

	template<> struct IsZeroConstructType<String> { enum { Value = true }; };
	Expose_TNameOf(String)

		/** Case insensitive string hash function. */
	__forceinline uint32 GetTypeHash(const String& S)
	{
		return Crc::StrHash(*S);
	}

	/**
	* Convert an array of bytes to a TCHAR
	* @param In byte array values to convert
	* @param Count number of bytes to convert
	* @return Valid string representing bytes.
	*/
	inline String BytesToString(const uint8* In, int32 Count)
	{
		String Result;
		Result.Clear(Count);

		while (Count)
		{
			// Put the byte into an int16 and add 1 to it, this keeps anything from being put into the string as a null terminator
			int16 Value = *In;
			Value += 1;

			Result += TCHAR(Value);

			++In;
			Count--;
		}
		return Result;
	}

	/**
	* Convert String of bytes into the byte array.
	* @param String		The String of byte values
	* @param OutBytes		Ptr to memory must be preallocated large enough
	* @param MaxBufferSize	Max buffer size of the OutBytes array, to prevent overflow
	* @return	The number of bytes copied
	*/
	inline int32 StringToBytes(const String& String, uint8* OutBytes, int32 MaxBufferSize)
	{
		int32 NumBytes = 0;
		const TCHAR* CharPos = *String;

		while (*CharPos && NumBytes < MaxBufferSize)
		{
			OutBytes[NumBytes] = (int8)(*CharPos - 1);
			CharPos++;
			++NumBytes;
		}
		return NumBytes - 1;
	}

	/** @return Char value of Nibble */
	inline TCHAR NibbleToTChar(uint8 Num)
	{
		if (Num > 9)
		{
			return EDX_TEXT('A') + TCHAR(Num - 10);
		}
		return EDX_TEXT('0') + TCHAR(Num);
	}

	/**
	* Convert a byte to hex
	* @param In byte value to convert
	* @param Result out hex value output
	*/
	inline void ByteToHex(uint8 In, String& Result)
	{
		Result += NibbleToTChar(In >> 4);
		Result += NibbleToTChar(In & 15);
	}

	/**
	* Convert an array of bytes to hex
	* @param In byte array values to convert
	* @param Count number of bytes to convert
	* @return Hex value in string.
	*/
	inline String BytesToHex(const uint8* In, int32 Count)
	{
		String Result;
		Result.Clear(Count * 2);

		while (Count)
		{
			ByteToHex(*In++, Result);
			Count--;
		}
		return Result;
	}

	/**
	* Checks if the TChar is a valid hex character
	* @param Char		The character
	* @return	True if in 0-9 and A-F ranges
	*/
	inline const bool CheckTCharIsHex(const TCHAR Char)
	{
		return (Char >= EDX_TEXT('0') && Char <= EDX_TEXT('9')) || (Char >= EDX_TEXT('A') && Char <= EDX_TEXT('F')) || (Char >= EDX_TEXT('a') && Char <= EDX_TEXT('f'));
	}

	/**
	* Convert a TChar to equivalent hex value as a uint8
	* @param Char		The character
	* @return	The uint8 value of a hex character
	*/
	inline const uint8 TCharToNibble(const TCHAR Char)
	{
		Assert(CheckTCharIsHex(Char));
		if (Char >= EDX_TEXT('0') && Char <= EDX_TEXT('9'))
		{
			return Char - EDX_TEXT('0');
		}
		else if (Char >= EDX_TEXT('A') && Char <= EDX_TEXT('F'))
		{
			return (Char - EDX_TEXT('A')) + 10;
		}
		return (Char - EDX_TEXT('a')) + 10;
	}

	/**
	* Convert String of Hex digits into the byte array.
	* @param HexString		The String of Hex values
	* @param OutBytes		Ptr to memory must be preallocated large enough
	* @return	The number of bytes copied
	*/
	inline int32 HexToBytes(const String& HexString, uint8* OutBytes)
	{
		int32 NumBytes = 0;
		const bool bPadNibble = (HexString.Len() % 2) == 1;
		const TCHAR* CharPos = *HexString;
		if (bPadNibble)
		{
			OutBytes[NumBytes++] = TCharToNibble(*CharPos++);
		}
		while (*CharPos)
		{
			OutBytes[NumBytes] = TCharToNibble(*CharPos++) << 4;
			OutBytes[NumBytes] += TCharToNibble(*CharPos++);
			++NumBytes;
		}
		return NumBytes;
	}

	/** Namespace that houses lexical conversion for various types. User defined conversions can be implemented externally */
	namespace LexicalConversion
	{
		/**
		*	Expected functions in this namespace are as follows:
		*		static bool		TryParseString(T& OutValue, const TCHAR* Buffer);
		*		static void 	FromString(T& OutValue, const TCHAR* Buffer);
		*		static String	ToString(const T& OutValue);
		*
		*	Implement custom functionality externally.
		*/

		/** Covert a string buffer to intrinsic types */
		inline void FromString(int8& OutValue, const TCHAR* Buffer) { OutValue = CString::Atoi(Buffer); }
		inline void FromString(int16& OutValue, const TCHAR* Buffer) { OutValue = CString::Atoi(Buffer); }
		inline void FromString(int32& OutValue, const TCHAR* Buffer) { OutValue = CString::Atoi(Buffer); }
		inline void FromString(int64& OutValue, const TCHAR* Buffer) { OutValue = CString::Atoi64(Buffer); }
		inline void FromString(uint8& OutValue, const TCHAR* Buffer) { OutValue = CString::Atoi(Buffer); }
		inline void FromString(uint16& OutValue, const TCHAR* Buffer) { OutValue = CString::Atoi(Buffer); }
		inline void FromString(uint32& OutValue, const TCHAR* Buffer) { OutValue = CString::Atoi64(Buffer); }	//64 because this unsigned and so Atoi might overflow
		inline void FromString(uint64& OutValue, const TCHAR* Buffer) { OutValue = CString::Strtoui64(Buffer, nullptr, 0); }
		inline void FromString(float& OutValue, const TCHAR* Buffer) { OutValue = CString::Atof(Buffer); }
		inline void FromString(double& OutValue, const TCHAR* Buffer) { OutValue = CString::Atod(Buffer); }
		inline void FromString(bool& OutValue, const TCHAR* Buffer) { OutValue = CString::ToBool(Buffer); }
		inline void FromString(String& OutValue, const TCHAR* Buffer) { OutValue = Buffer; }

		/** Convert numeric types to a string */
		template<typename T>
		typename EnableIf<IsArithmeticType<T>::Value, String>::Type
			ToString(const T& Value)
		{
			return String::Printf(FormatSpecifier<T>::GetFormatSpecifier(), Value);
		}

		/** Helper template to convert to sanitized strings */
		template<typename T>
		String ToSanitizedString(const T& Value)
		{
			return ToString(Value);
		}

		/** Specialized for floats */
		template<>
		inline String ToSanitizedString<float>(const float& Value)
		{
			return String::SanitizeFloat(Value);
		}


		/** Parse a string into this type, returning whether it was successful */
		/** Specialization for arithmetic types */
		template<typename T>
		static typename EnableIf<IsArithmeticType<T>::Value, bool>::Type
			TryParseString(T& OutValue, const TCHAR* Buffer)
		{
			if (CString::IsNumeric(Buffer))
			{
				FromString(OutValue, Buffer);
				return true;
			}
			return false;
		}

		/** Try and parse a bool - always returns true */
		static bool TryParseString(bool& OutValue, const TCHAR* Buffer)
		{
			FromString(OutValue, Buffer);
			return true;
		}
	}

	/** Shorthand legacy use for LexicalConversion functions */
	template<typename T>
	struct TypeToString
	{
		static String ToString(const T& Value) { return LexicalConversion::ToString(Value); }
		static String ToSanitizedString(const T& Value) { return LexicalConversion::ToSanitizedString(Value); }
	};
	template<typename T>
	struct TypeFromString
	{
		static void FromString(T& Value, const TCHAR* Buffer) { return LexicalConversion::FromString(Value, Buffer); }
	};

}