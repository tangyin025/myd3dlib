#pragma once

#include <unzip.h>
#include "myThread.h"
#include <vector>
#include <list>
#include <boost/unordered_map.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/function.hpp>
#include "mySingleton.h"
#include <d3d9.h>

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

	class ArchiveDir
	{
	protected:
		std::string m_dir;

	public:
		ArchiveDir(const std::string & dir)
			: m_dir(dir)
		{
		}

		virtual ~ArchiveDir(void)
		{
		}

		virtual bool CheckArchivePath(const std::string & path) = 0;

		virtual std::string GetFullPath(const std::string & path) = 0;

		virtual ArchiveStreamPtr OpenArchiveStream(const std::string & path) = 0;
	};

	class ZipArchiveDir
		: public ArchiveDir
	{
	protected:
		std::string m_password;

		bool m_UsePassword;

	public:
		ZipArchiveDir(const std::string & dir)
			: ArchiveDir(dir)
			, m_UsePassword(false)
		{
		}

		ZipArchiveDir(const std::string & dir, const std::string & password)
			: ArchiveDir(dir)
			, m_UsePassword(true)
			, m_password(password)
		{
		}

		static std::string ReplaceSlash(const std::string & path);

		static std::string ReplaceBackslash(const std::string & path);

		bool CheckArchivePath(const std::string & path);

		std::string GetFullPath(const std::string & path);

		ArchiveStreamPtr OpenArchiveStream(const std::string & path);
	};

	class FileArchiveDir
		: public ArchiveDir
	{
	public:
		FileArchiveDir(const std::string & dir)
			: ArchiveDir(dir)
		{
		}

		bool CheckArchivePath(const std::string & path);

		std::string GetFullPath(const std::string & path);

		ArchiveStreamPtr OpenArchiveStream(const std::string & path);
	};

	class ArchiveDirMgr
	{
	protected:
		typedef boost::shared_ptr<ArchiveDir> ResourceDirPtr;

		typedef boost::unordered_map<std::string, ResourceDirPtr> ResourceDirPtrMap;

		ResourceDirPtrMap m_dirMap;

	public:
		ArchiveDirMgr(void)
		{
		}

		virtual ~ArchiveDirMgr(void)
		{
		}

		void RegisterZipArchive(const std::string & zip_path);

		void RegisterZipArchive(const std::string & zip_path, const std::string & password);

		void RegisterFileDir(const std::string & dir);

		bool CheckArchivePath(const std::string & path);

		std::string GetFullPath(const std::string & path);

		ArchiveStreamPtr OpenArchiveStream(const std::string & path);
	};

	//class DeviceRelatedResourceMgr
	//	: public ArchiveDirMgr
	//	, public ID3DXInclude
	//{
	//protected:
	//	CComPtr<ID3DXEffectPool> m_EffectPool;

	//	std::string m_EffectInclude;

	//	boost::unordered_map<LPCVOID, CachePtr> m_cacheSet;

	//	typedef boost::unordered_map<std::string, boost::weak_ptr<DeviceRelatedObjectBase> > DeviceRelatedResourceSet;

	//	DeviceRelatedResourceSet m_resourceSet;

	//public:
	//	DeviceRelatedResourceMgr(void)
	//	{
	//	}

	//	virtual ~DeviceRelatedResourceMgr(void)
	//	{
	//	}

	//	virtual __declspec(nothrow) HRESULT __stdcall Open(
	//		D3DXINCLUDE_TYPE IncludeType,
	//		LPCSTR pFileName,
	//		LPCVOID pParentData,
	//		LPCVOID * ppData,
	//		UINT * pBytes);

	//	virtual __declspec(nothrow) HRESULT __stdcall Close(
	//		LPCVOID pData);

	//	HRESULT OnCreateDevice(
	//		IDirect3DDevice9 * pd3dDevice,
	//		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	//	HRESULT OnResetDevice(
	//		IDirect3DDevice9 * pd3dDevice,
	//		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	//	void OnLostDevice(void);

	//	void OnDestroyDevice(void);

	//	template <class ResourceType>
	//	boost::shared_ptr<ResourceType> GetDeviceRelatedResource(const std::string & key, bool reload)
	//	{
	//		DeviceRelatedResourceSet::const_iterator res_iter = m_resourceSet.find(key);
	//		if(m_resourceSet.end() != res_iter)
	//		{
	//			DeviceRelatedObjectBasePtr res = res_iter->second.lock();
	//			if(res)
	//			{
	//				if(reload)
	//					res->OnDestroyDevice();

	//				boost::shared_ptr<ResourceType> ret = boost::dynamic_pointer_cast<ResourceType>(res); _ASSERT(ret); return ret;
	//			}
	//		}

	//		boost::shared_ptr<ResourceType> res(new ResourceType());
	//		m_resourceSet[key] = res;
	//		return res;
	//	}

	//	TexturePtr LoadTexture(const std::string & path, bool reload = false);

	//	CubeTexturePtr LoadCubeTexture(const std::string & path, bool reload = false);

	//	OgreMeshPtr LoadMesh(const std::string & path, bool reload = false);

	//	OgreSkeletonAnimationPtr LoadSkeleton(const std::string & path, bool reload = false);

	//	typedef std::vector<std::pair<std::string, std::string> > string_pair_list;

	//	EffectPtr LoadEffect(const std::string & path, const string_pair_list & macros, bool reload = false);

	//	FontPtr LoadFont(const std::string & path, int height, bool reload = false);
	//};

	typedef boost::function<void (DeviceRelatedObjectBasePtr)> IOResourceCallback;

	class IOResource
	{
	public:
		IOResourceCallback m_callback;

	public:
		IOResource(const IOResourceCallback & callback)
			: m_callback(callback)
		{
		}

		virtual ~IOResource(void)
		{
		}

		virtual std::string GetKey(void) = 0;

		virtual void DoLoad(void) = 0;

		virtual DeviceRelatedObjectBasePtr GetResource(LPDIRECT3DDEVICE9 pd3dDevice) = 0;
	};

	typedef boost::shared_ptr<IOResource> IOResourcePtr;

	class IOResourceMgr
		: public ArchiveDirMgr
		, public Thread
	{
	protected:
		typedef std::list<IOResourcePtr> IOResourcePtrList;

		IOResourcePtrList m_IORequestList;

		CriticalSection m_IORequestListSection;

		ConditionVariable m_IORequestListNotEmpty;

		bool m_IOStop;

		IOResourcePtrList m_IOReadyList;

		CriticalSection m_IOReadyListSection;

		ConditionVariable m_IOReadyListNotEmpty;

	public:
		IOResourceMgr(void)
			: m_IOStop(false)
		{
		}

	protected:
		virtual DWORD OnProc(void);

		void PushIORequestResource(my::IOResourcePtr io_res);

		void PushIOReadyResource(my::IOResourcePtr io_res);

		void Stop(void);
	};

	class DeviceRelatedResourceMgr
		: public IOResourceMgr
	{
	protected:
		typedef boost::unordered_map<std::string, boost::weak_ptr<DeviceRelatedObjectBase> > DeviceRelatedResourceSet;

		DeviceRelatedResourceSet m_resourceSet;

	public:
		DeviceRelatedResourceMgr(void)
		{
		}

		HRESULT OnCreateDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		HRESULT OnResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		void OnLostDevice(void);

		void OnDestroyDevice(void);

		void LoadResource(IOResourcePtr Request);

		void CheckResource(void);

		void LoadTexture(const std::string & path, IOResourceCallback callback);
	};
};
