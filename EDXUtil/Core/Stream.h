#pragma once

#include "Types.h"
#include "Template.h"

namespace EDX
{
	/**
	* Base class for archives that can be used for loading, saving in a byte order neutral way.
	*/
	class Stream
	{
	public:

		/** Default constructor. */
		Stream() = default;

		/** Copy constructor. */
		Stream(const Stream&) = default;

		/**
		* Copy assignment operator.
		*
		* @param ArchiveToCopy The archive to copy from.
		*/
		Stream& operator=(const Stream& ArchiveToCopy) = default;

		/** Destructor. */
		virtual ~Stream() = default;

	public:

		virtual int64 Tell()
		{
			return INDEX_NONE;
		}

		virtual int64 TotalSize()
		{
			return INDEX_NONE;
		}

		virtual bool AtEnd()
		{
			int64 Pos = Tell();

			return ((Pos != INDEX_NONE) && (Pos >= TotalSize()));
		}

		virtual void Seek(int64 InPos)
		{
		}

		virtual void Flush()
		{
		}

		virtual void Close()
		{
		}

		/**
		* Serializes an ANSICHAR value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator<<(Stream& stream, ANSICHAR& Value)
		{
			stream.Write(&Value, 1);
			return stream;
		}

		/**
		* Serializes a WIDECHAR value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator<<(Stream& stream, WIDECHAR& Value)
		{
			stream.ByteOrderWrite(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes an unsigned 8-bit integer value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator<<(Stream& stream, uint8& Value)
		{
			stream.ByteOrderWrite(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes an enumeration value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		//template<class TEnum>
		//__forceinline friend Stream& operator<<(Stream& stream, TEnumAsByte<TEnum>& Value)
		//{
		//	stream.ByteOrderWrite(&Value, sizeof(Value));
		//	return stream;
		//}

		/**
		* Serializes a signed 8-bit integer value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator<<(Stream& stream, int8& Value)
		{
			stream.Write(&Value, 1);
			return stream;
		}

		/**
		* Serializes an unsigned 16-bit integer value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator<<(Stream& stream, uint16& Value)
		{
			stream.ByteOrderWrite(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes a signed 16-bit integer value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator<<(Stream& stream, int16& Value)
		{
			stream.ByteOrderWrite(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes an unsigned 32-bit integer value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator<<(Stream& stream, uint32& Value)
		{
			stream.ByteOrderWrite(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes a Boolean value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator<<(Stream& stream, bool& D)
		{
			uint32 OldUBoolValue = D ? 1 : 0;
			stream.Write(&OldUBoolValue, sizeof(OldUBoolValue));
			D = !!OldUBoolValue;
			return stream;
		}

		/**
		* Serializes a signed 32-bit integer value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator<<(Stream& stream, int32& Value)
		{
			stream.ByteOrderWrite(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes a long integer value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator<<(Stream& stream, long& Value)
		{
			stream.ByteOrderWrite(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes a single precision floating point value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator<<(Stream& stream, float& Value)
		{
			stream.ByteOrderWrite(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes a double precision floating point value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator<<(Stream& stream, double& Value)
		{
			stream.ByteOrderWrite(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes a unsigned 64-bit integer value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator<<(Stream &stream, uint64& Value)
		{
			stream.ByteOrderWrite(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes a signed 64-bit integer value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		/*__forceinline*/friend Stream& operator<<(Stream& stream, int64& Value)
		{
			stream.ByteOrderWrite(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes enum classes as their underlying type.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		template <
			typename EnumType,
			typename = typename EnableIf<IsEnumClass<EnumType>::Value>::Type
		>
			__forceinline friend Stream& operator<<(Stream& stream, EnumType& Value)
		{
			return stream << (__underlying_type(EnumType)&)Value;
		}
		/**
		* Serializes an FString value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		friend Stream& operator<<(Stream& stream, class String& Value);


		/**
		* Serializes an ANSICHAR value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator>>(Stream& stream, ANSICHAR& Value)
		{
			stream.Read(&Value, 1);
			return stream;
		}

		/**
		* Serializes a WIDECHAR value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator>>(Stream& stream, WIDECHAR& Value)
		{
			stream.ByteOrderRead(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes an unsigned 8-bit integer value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator>>(Stream& stream, uint8& Value)
		{
			stream.ByteOrderRead(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes an enumeration value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		//template<class TEnum>
		//__forceinline friend Stream& operator>>(Stream& stream, TEnumAsByte<TEnum>& Value)
		//{
		//	stream.ByteOrderRead(&Value, sizeof(Value));
		//	return stream;
		//}

		/**
		* Serializes a signed 8-bit integer value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator>>(Stream& stream, int8& Value)
		{
			stream.Read(&Value, 1);
			return stream;
		}

		/**
		* Serializes an unsigned 16-bit integer value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator>>(Stream& stream, uint16& Value)
		{
			stream.ByteOrderRead(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes a signed 16-bit integer value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator>>(Stream& stream, int16& Value)
		{
			stream.ByteOrderRead(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes an unsigned 32-bit integer value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator>>(Stream& stream, uint32& Value)
		{
			stream.ByteOrderRead(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes a Boolean value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator>>(Stream& stream, bool& D)
		{
			uint32 OldUBoolValue = D ? 1 : 0;
			stream.Read(&OldUBoolValue, sizeof(OldUBoolValue));
			D = !!OldUBoolValue;
			return stream;
		}

		/**
		* Serializes a signed 32-bit integer value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator>>(Stream& stream, int32& Value)
		{
			stream.ByteOrderRead(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes a long integer value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator>>(Stream& stream, long& Value)
		{
			stream.ByteOrderRead(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes a single precision floating point value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator>>(Stream& stream, float& Value)
		{
			stream.ByteOrderRead(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes a double precision floating point value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator>>(Stream& stream, double& Value)
		{
			stream.ByteOrderRead(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes a unsigned 64-bit integer value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		__forceinline friend Stream& operator>>(Stream &stream, uint64& Value)
		{
			stream.ByteOrderRead(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes a signed 64-bit integer value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		/*__forceinline*/friend Stream& operator>>(Stream& stream, int64& Value)
		{
			stream.ByteOrderRead(&Value, sizeof(Value));
			return stream;
		}

		/**
		* Serializes enum classes as their underlying type.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		template <
			typename EnumType,
			typename = typename EnableIf<IsEnumClass<EnumType>::Value>::Type
		>
			__forceinline friend Stream& operator>>(Stream& stream, EnumType& Value)
		{
			return stream >> (__underlying_type(EnumType)&)Value;
		}
		/**
		* Serializes an FString value from or into an archive.
		*
		* @param stream The archive to serialize from or to.
		* @param Value The value to serialize.
		*/
		friend Stream& operator>>(Stream& stream, class String& Value);
	public:

		virtual void Write(void* V, int64 Length) = 0;
		virtual void Read(void* V, int64 Length) = 0;

		virtual void SerializeBits(void* V, int64 LengthBits)
		{
			Write(V, (LengthBits + 7) / 8);
		}

		virtual void LoadBits(void* V, int64 LengthBits)
		{
			Read(V, (LengthBits + 7) / 8);

			if ((LengthBits % 8) != 0)
			{
				((uint8*)V)[LengthBits / 8] &= ((1 << (LengthBits & 7)) - 1);
			}
		}

		virtual void SerializeInt(uint32& Value, uint32 Max)
		{
			ByteOrderWrite(&Value, sizeof(Value));
		}

		virtual void LoadInt(uint32& Value, uint32 Max)
		{
			ByteOrderRead(&Value, sizeof(Value));
		}

		__forceinline bool IsByteSwapping()
		{
			return false;
		}

		// Used to do byte swapping on small items. This does not happen usually, so we don't want it inline
		void ByteSwap(void* V, int32 Length);

		__forceinline Stream& ByteOrderWrite(void* V, int32 Length)
		{
			Write(V, Length);
			return *this;
		}

		__forceinline Stream& ByteOrderRead(void* V, int32 Length)
		{
			Read(V, Length);
			if (IsByteSwapping())
			{
				// Transferring between memory and file, so flip the byte order.
				ByteSwap(V, Length);
			}
			return *this;
		}
	};
}