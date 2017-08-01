#pragma once

#include "../Core/Stream.h"

namespace EDX
{
	enum class FileMode
	{
		Create, Open, CreateNew, Append
	};

	enum class FileAccess
	{
		Read = 1, Write = 2, ReadWrite = 3
	};

	enum class FileShare
	{
		None, ReadOnly, WriteOnly, ReadWrite
	};

	class FileStream : public Stream
	{
	private:
		FILE* mHandle;
		FileAccess mFileAccess;
		bool mEOF = false;
		void Init(const String & fileName, FileMode fileMode, FileAccess access, FileShare share);

	public:
		FileStream(const String & fileName, FileMode fileMode = FileMode::Open);
		FileStream(const String & fileName, FileMode fileMode, FileAccess access, FileShare share);
		~FileStream();

	public:
		virtual void Read(void* V, int64 Length);
		virtual void Write(void* V, int64 Length);
		virtual int64 Tell();
		virtual void Seek(int64 offset);
		virtual void Close();
		virtual bool AtEnd();
	};
}