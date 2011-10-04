
#pragma once

#include <vector>
#include <boost/shared_ptr.hpp>
#include <string>
#include <tchar.h>
#include <zzip/lib.h>
#include "mySingleton.h"

namespace my
{
	class ArchiveStream
	{
	public:
		enum SeekType
		{
			CUR,
			END,
			SET
		};

	public:
		virtual ~ArchiveStream(void)
		{
		}

		virtual size_t Read(void * buffer, size_t size, size_t count) = 0;

		virtual size_t Write(void * buffer, size_t size, size_t count) = 0;

		virtual long Seek(long offset, SeekType origin) = 0;

		virtual long Tell(void) = 0;
	};

	typedef boost::shared_ptr<ArchiveStream> ArchiveStreamPtr;

	class ZipArchiveStream
		: public ArchiveStream
	{
	protected:
		ZZIP_FILE * m_fp;

	public:
		static int ToZipSeekType(SeekType origin);

	public:
		ZipArchiveStream(ZZIP_FILE * fp);

		~ZipArchiveStream(void);

		size_t Read(void * buffer, size_t size, size_t count);

		size_t Write(void * buffer, size_t size, size_t count);

		long Seek(long offset, SeekType origin);

		long Tell(void);
	};

	class FileArchiveStream
		: public ArchiveStream
	{
	protected:
		FILE * m_fp;

	public:
		FileArchiveStream(FILE * fp);

		~FileArchiveStream(void);

		size_t Read(void * buffer, size_t size, size_t count);

		size_t Write(void * buffer, size_t size, size_t count);

		long Seek(long offset, SeekType origin);

		long Tell(void);
	};

	class ResourceDir
	{
	protected:
		std::basic_string<_TCHAR> m_dir;

	public:
		ResourceDir(const std::basic_string<_TCHAR> & dir);

		virtual ~ResourceDir(void);

		virtual bool CheckArchivePath(const std::basic_string<_TCHAR> & path) = 0;

		virtual ArchiveStreamPtr OpenArchiveStream(const std::basic_string<_TCHAR> & path) = 0;
	};

	typedef boost::shared_ptr<ResourceDir> ResourceDirPtr;

	typedef std::vector<ResourceDirPtr> ResourceDirPtrList;

	class ZipArchiveDir
		: public ResourceDir
	{
	public:
		ZipArchiveDir(const std::basic_string<_TCHAR> & dir);

		bool CheckArchivePath(const std::basic_string<_TCHAR> & path);

		ArchiveStreamPtr OpenArchiveStream(const std::basic_string<_TCHAR> & path);
	};

	class FileArchiveDir
		: public ResourceDir
	{
	protected:
		std::basic_string<_TCHAR> GetFullPath(const std::basic_string<_TCHAR> & path);

	public:
		FileArchiveDir(const std::basic_string<_TCHAR> & dir);

		bool CheckArchivePath(const std::basic_string<_TCHAR> & path);

		ArchiveStreamPtr OpenArchiveStream(const std::basic_string<_TCHAR> & path);
	};

	class ResourceMgr
		: public Singleton<ResourceMgr>
	{
	protected:
		ResourceDirPtrList m_dirList;

	public:
		void RegisterZipArchive(const std::basic_string<_TCHAR> & zip_path);

		void RegisterFileDir(const std::basic_string<_TCHAR> & dir);

		ArchiveStreamPtr OpenArchiveStream(const std::basic_string<_TCHAR> & path);
	};

	typedef std::vector<unsigned char> Cache;

	typedef boost::shared_ptr<Cache> CachePtr;

	CachePtr ReadWholeCacheFromStream(ArchiveStreamPtr stream);
};
