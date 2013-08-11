#pragma once

#include <unzip.h>
#include <map>
#include "myTexture.h"
#include "myEffect.h"
#include "myMesh.h"
#include "mySkeleton.h"
#include "myFont.h"
#include <boost/weak_ptr.hpp>

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

		typedef std::map<std::string, ResourceDirPtr> ResourceDirPtrMap;

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

	class ResourceMgr
		: public ArchiveDirMgr
		, public ID3DXInclude
	{
	protected:
		CComPtr<ID3DXEffectPool> m_EffectPool;

		std::string m_EffectInclude;

		std::map<LPCVOID, CachePtr> m_cacheSet;

		typedef std::map<std::string, boost::weak_ptr<DeviceRelatedObjectBase> > DeviceRelatedResourceSet;

		DeviceRelatedResourceSet m_resourceSet;

	public:
		ResourceMgr(void)
		{
		}

		virtual ~ResourceMgr(void)
		{
		}

		virtual __declspec(nothrow) HRESULT __stdcall Open(
			D3DXINCLUDE_TYPE IncludeType,
			LPCSTR pFileName,
			LPCVOID pParentData,
			LPCVOID * ppData,
			UINT * pBytes);

		virtual __declspec(nothrow) HRESULT __stdcall Close(
			LPCVOID pData);

		HRESULT OnCreateDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		HRESULT OnResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		void OnLostDevice(void);

		void OnDestroyDevice(void);

		template <class ResourceType>
		boost::shared_ptr<ResourceType> GetDeviceRelatedResource(const std::string & key, bool reload)
		{
			DeviceRelatedResourceSet::const_iterator res_iter = m_resourceSet.find(key);
			if(m_resourceSet.end() != res_iter)
			{
				boost::shared_ptr<DeviceRelatedObjectBase> res = res_iter->second.lock();
				if(res)
				{
					if(reload)
						res->OnDestroyDevice();

					return boost::dynamic_pointer_cast<ResourceType>(res);
				}
			}

			boost::shared_ptr<ResourceType> res(new ResourceType());
			m_resourceSet[key] = res;
			return res;
		}

		TexturePtr LoadTexture(const std::string & path, bool reload = false);

		CubeTexturePtr LoadCubeTexture(const std::string & path, bool reload = false);

		OgreMeshPtr LoadMesh(const std::string & path, bool reload = false);

		OgreSkeletonAnimationPtr LoadSkeleton(const std::string & path, bool reload = false);

		typedef std::vector<std::pair<std::string, std::string> > string_pair_list;

		EffectPtr LoadEffect(const std::string & path, const string_pair_list & macros, bool reload = false);

		FontPtr LoadFont(const std::string & path, int height, bool reload = false);
	};
};
