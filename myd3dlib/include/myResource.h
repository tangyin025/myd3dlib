
#pragma once

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
		std::string m_dir;

	public:
		ResourceDir(const std::string & dir);

		virtual ~ResourceDir(void);

		virtual bool CheckArchivePath(const std::string & path) = 0;

		virtual ArchiveStreamPtr OpenArchiveStream(const std::string & path) = 0;
	};

	typedef boost::shared_ptr<ResourceDir> ResourceDirPtr;

	typedef std::vector<ResourceDirPtr> ResourceDirPtrList;

	class ZipArchiveDir
		: public ResourceDir
	{
	public:
		ZipArchiveDir(const std::string & dir);

		bool CheckArchivePath(const std::string & path);

		ArchiveStreamPtr OpenArchiveStream(const std::string & path);
	};

	class FileArchiveDir
		: public ResourceDir
	{
	protected:
		std::string GetFullPath(const std::string & path);

	public:
		FileArchiveDir(const std::string & dir);

		bool CheckArchivePath(const std::string & path);

		ArchiveStreamPtr OpenArchiveStream(const std::string & path);
	};

	class ResourceMgr
		: public Singleton<ResourceMgr>
	{
	protected:
		ResourceDirPtrList m_dirList;

	public:
		void RegisterZipArchive(const std::string & zip_path);

		void RegisterFileDir(const std::string & dir);

		ArchiveStreamPtr OpenArchiveStream(const std::string & path);
	};

	typedef std::vector<unsigned char> Cache;

	typedef boost::shared_ptr<Cache> CachePtr;

	CachePtr ReadWholeCacheFromStream(ArchiveStreamPtr stream);
};
