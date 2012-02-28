
#pragma once

namespace my
{
	typedef std::vector<unsigned char> Cache;

	typedef boost::shared_ptr<Cache> CachePtr;

	class ArchiveStream
	{
	public:
		virtual ~ArchiveStream(void)
		{
		}

		virtual CachePtr GetWholeCache(void) = 0;
	};

	typedef boost::shared_ptr<ArchiveStream> ArchiveStreamPtr;

	class ZipArchiveStream
		: public ArchiveStream
	{
	protected:
		unzFile m_zFile;

		unz_file_info m_zFileInfo;

	public:
		ZipArchiveStream(unzFile zFile);

		~ZipArchiveStream(void);

		virtual CachePtr GetWholeCache(void);
	};

	class FileArchiveStream
		: public ArchiveStream
	{
	protected:
		FILE * m_fp;

	public:
		FileArchiveStream(FILE * fp);

		~FileArchiveStream(void);

		virtual CachePtr GetWholeCache(void);
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
	protected:
		std::string m_password;

	public:
		ZipArchiveDir(const std::string & dir, const std::string & password = "");

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
		void RegisterZipArchive(const std::string & zip_path, const std::string & password = "");

		void RegisterFileDir(const std::string & dir);

		bool CheckArchivePath(const std::string & path);

		ArchiveStreamPtr OpenArchiveStream(const std::string & path);
	};
};
