
#pragma once

#include <vector>
#include <boost/shared_ptr.hpp>
#include <unzip.h>
#include "mySingleton.h"
#include <ft2build.h>
#include FT_FREETYPE_H
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

	class ResourceMgr
		: public Singleton<ResourceMgr>
	{
	public:
		FT_Library m_library;

		CComPtr<IDirect3DStateBlock9> m_stateBlock;

		boost::weak_ptr<Control> m_ControlFocus;

		typedef boost::weak_ptr<DeviceRelatedObjectBase> WeakDeviceRelatedObjectBasePtr;

		typedef std::set<WeakDeviceRelatedObjectBasePtr> WeakDeviceRelatedObjectBasePtrSet;

		WeakDeviceRelatedObjectBasePtrSet m_DeviceRelatedObjs;

		typedef boost::shared_ptr<ResourceDir> ResourceDirPtr;

		typedef std::map<std::string, ResourceDirPtr> ResourceDirPtrMap;

		ResourceDirPtrMap m_dirMap;

		template <typename T>
		boost::shared_ptr<T> RegisterDeviceRelatedObject(boost::shared_ptr<T> obj_ptr)
		{
			_ASSERT(m_DeviceRelatedObjs.end() == m_DeviceRelatedObjs.find(obj_ptr));

			m_DeviceRelatedObjs.insert(obj_ptr);

			return obj_ptr;
		}

	public:
		ResourceMgr(void);

		~ResourceMgr(void);

		void OnResetDevice(void);

		void OnLostDevice(void);

		void OnDestroyDevice(void);

		void RegisterZipArchive(const std::string & zip_path, const std::string & password = "");

		void RegisterFileDir(const std::string & dir);

		bool CheckArchivePath(const std::string & path);

		std::string GetFullPath(const std::string & path);

		ArchiveStreamPtr OpenArchiveStream(const std::string & path);
	};

	typedef boost::shared_ptr<ResourceMgr> ResourceMgrPtr;
};
