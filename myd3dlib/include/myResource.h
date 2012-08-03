
#pragma once

#include <vector>
#include <boost/shared_ptr.hpp>
#include <unzip.h>
#include "mySingleton.h"
#include <atlbase.h>
#include <d3d9.h>
#include "myUi.h"
#include <map>

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

		virtual int read(void * buff, unsigned read_size) = 0;

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

		virtual int read(void * buff, unsigned read_size);

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

		virtual int read(void * buff, unsigned read_size);

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

		virtual std::string GetFullPath(const std::string & path) = 0;

		virtual ArchiveStreamPtr OpenArchiveStream(const std::string & path) = 0;
	};

	class ZipArchiveDir
		: public ResourceDir
	{
	protected:
		std::string m_password;

	public:
		ZipArchiveDir(const std::string & dir, const std::string & password = "");

		bool CheckArchivePath(const std::string & path);

		std::string GetFullPath(const std::string & path);

		ArchiveStreamPtr OpenArchiveStream(const std::string & path);
	};

	class FileArchiveDir
		: public ResourceDir
	{
	public:
		FileArchiveDir(const std::string & dir);

		bool CheckArchivePath(const std::string & path);

		std::string GetFullPath(const std::string & path);

		ArchiveStreamPtr OpenArchiveStream(const std::string & path);
	};

	class ResourceMgrBase
	{
	protected:
		typedef boost::shared_ptr<ResourceDir> ResourceDirPtr;

		typedef std::map<std::string, ResourceDirPtr> ResourceDirPtrMap;

		ResourceDirPtrMap m_dirMap;

	public:
		ResourceMgrBase(void);

		virtual ~ResourceMgrBase(void);

		void RegisterZipArchive(const std::string & zip_path, const std::string & password = "");

		void RegisterFileDir(const std::string & dir);

		bool CheckArchivePath(const std::string & path);

		std::string GetFullPath(const std::string & path);

		ArchiveStreamPtr OpenArchiveStream(const std::string & path);
	};

	class IncludeFromResource
		: public ResourceMgrBase
		, public ID3DXInclude
	{
	protected:
		std::map<LPCVOID, CachePtr> m_cacheSet;

	public:
		virtual __declspec(nothrow) HRESULT __stdcall Open(
			D3DXINCLUDE_TYPE IncludeType,
			LPCSTR pFileName,
			LPCVOID pParentData,
			LPCVOID * ppData,
			UINT * pBytes);

		virtual __declspec(nothrow) HRESULT __stdcall Close(
			LPCVOID pData);
	};

	typedef boost::shared_ptr<IncludeFromResource> IncludeFromResourcePtr;

	class ResourceMgr
		: public IncludeFromResource
		, public Singleton<ResourceMgr>
	{
	public:
		ResourceMgr(void);

		~ResourceMgr(void);
	};

	typedef boost::shared_ptr<ResourceMgr> ResourceMgrPtr;
};
