#include "Base.h"
#include "FileStream.h"
#include "../Containers/String.h"

namespace EDX
{
	bool FileExists(const String& fileName)
	{
#if _WIN64
		struct stat statVar;
		return ::stat(fileName.GetCString(), &statVar) == 0;
#else
		struct _stat32 statVar;
		return ::_wstat32(((String)fileName).ToWString(), &statVar) != -1;
#endif
	}

	FileStream::FileStream(const String & fileName, FileMode fileMode)
	{
		Init(fileName, fileMode, fileMode == FileMode::Open ? FileAccess::Read : FileAccess::Write, FileShare::None);
	}
	FileStream::FileStream(const String & fileName, FileMode fileMode, FileAccess access, FileShare share)
	{
		Init(fileName, fileMode, access, share);
	}
	void FileStream::Init(const String & fileName, FileMode fileMode, FileAccess access, FileShare share)
	{
		const wchar_t * mode = L"rt";
		const char* modeMBCS = "rt";
		switch (fileMode)
		{
		case FileMode::Create:
			if (access == FileAccess::Read)
				throw std::exception("Read-only access is incompatible with Create mode.");
			else if (access == FileAccess::ReadWrite)
			{
				mode = L"w+b";
				modeMBCS = "w+b";
				this->mFileAccess = FileAccess::ReadWrite;
			}
			else
			{
				mode = L"wb";
				modeMBCS = "wb";
				this->mFileAccess = FileAccess::Write;
			}
			break;
		case FileMode::Open:
			if (access == FileAccess::Read)
			{
				mode = L"rb";
				modeMBCS = "rb";
				this->mFileAccess = FileAccess::Read;
			}
			else if (access == FileAccess::ReadWrite)
			{
				mode = L"r+b";
				modeMBCS = "r+b";
				this->mFileAccess = FileAccess::ReadWrite;
			}
			else
			{
				mode = L"wb";
				modeMBCS = "wb";
				this->mFileAccess = FileAccess::Write;
			}
			break;
		case FileMode::CreateNew:
			if (FileExists(fileName))
			{
				throw std::exception(("Failed opening '" + fileName + "', file already exists.").GetCString());
			}
			if (access == FileAccess::Read)
				throw std::exception("Read-only access is incompatible with Create mode.");
			else if (access == FileAccess::ReadWrite)
			{
				mode = L"w+b";
				this->mFileAccess = FileAccess::ReadWrite;
			}
			else
			{
				mode = L"wb";
				this->mFileAccess = FileAccess::Write;
			}
			break;
		case FileMode::Append:
			if (access == FileAccess::Read)
				throw std::exception("Read-only access is incompatible with Append mode.");
			else if (access == FileAccess::ReadWrite)
			{
				mode = L"a+b";
				this->mFileAccess = FileAccess::ReadWrite;
			}
			else
			{
				mode = L"ab";
				this->mFileAccess = FileAccess::Write;
			}
			break;
		default:
			break;
		}
		int shFlag;
#ifdef _WIN32
		switch (share)
		{
		case FileShare::None:
			shFlag = _SH_DENYRW;
			break;
		case FileShare::ReadOnly:
			shFlag = _SH_DENYWR;
			break;
		case FileShare::WriteOnly:
			shFlag = _SH_DENYRD;
			break;
		case FileShare::ReadWrite:
			shFlag = _SH_DENYNO;
			break;
		default:
			throw std::exception("Invalid file share mode.");
			break;
		}
#if USE_UNICODE
		handle = _wfsopen(fileName.GetCString(), mode, shFlag);
#else
		mHandle = _fsopen(fileName.GetCString(), modeMBCS, shFlag);
#endif
#else
		handle = fopen(fileName.Buffer(), modeMBCS);
#endif
		if (!mHandle)
		{
			throw std::exception(("Cannot open file '" + fileName + "'").GetCString());
		}
	}
	FileStream::~FileStream()
	{
		Close();
	}
	int64 FileStream::Tell()
	{
#ifdef _WIN32
		fpos_t pos;
		fgetpos(mHandle, &pos);
		return pos;
#else
		fpos64_t pos;
		fgetpos64(handle, &pos);
		return *(int64*)(&pos);
#endif
	}
	void FileStream::Seek(int64 offset)
	{
		int rs = _fseeki64(mHandle, offset, SEEK_SET);
		if (rs != 0)
		{
			throw std::exception("FileStream seek failed.");
		}
	}
	void FileStream::Read(void* buffer, int64 length)
	{
		auto bytes = fread_s(buffer, (size_t)length, 1, (size_t)length, mHandle);
		if (bytes == 0 && length > 0)
		{
			if (!feof(mHandle))
				throw std::exception("FileStream read failed.");
			else if (mEOF)
				throw std::exception("End of file is reached.");
			mEOF = true;
		}
	}
	void FileStream::Write(void* buffer, int64 length)
	{
		auto bytes = (int64)fwrite(buffer, 1, (size_t)length, mHandle);
		if (bytes < length)
		{
			throw std::exception("FileStream write failed.");
		}
	}
	void FileStream::Close()
	{
		if (mHandle)
		{
			fclose(mHandle);
			mHandle = 0;
		}
	}
	bool FileStream::AtEnd()
	{
		return mEOF;
	}
}