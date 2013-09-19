#pragma once

#include <unzip.h>
#include "myThread.h"
#include "mySingleton.h"
#include "myMesh.h"
#include "mySkeleton.h"
#include "myEffect.h"
#include "myFont.h"

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

	class ZipArchiveStream : public ArchiveStream
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

	class FileArchiveStream : public ArchiveStream
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

	class ZipArchiveDir : public ArchiveDir
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

	class FileArchiveDir : public ArchiveDir
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

		ResourceDirPtrMap m_DirMap;

		CriticalSection m_DirMapSection;

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

	typedef boost::function<void (DeviceRelatedObjectBasePtr)> ResourceCallback;

	class IORequest
	{
	public:
		Event m_LoadEvent;

		typedef std::vector<ResourceCallback> ResourceCallbackList;

		ResourceCallbackList m_callbacks;

		DeviceRelatedObjectBasePtr m_res;

	public:
		IORequest(void)
			: m_LoadEvent(NULL, TRUE, FALSE, NULL)
		{
		}

		virtual ~IORequest(void)
		{
		}

		virtual void DoLoad(void) = 0;

		virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice) = 0;
	};

	typedef boost::shared_ptr<IORequest> IORequestPtr;

	class AsynchronousIOMgr : public ArchiveDirMgr , public Thread
	{
	protected:
		typedef std::list<std::pair<std::string, IORequestPtr> > IORequestPtrPairList;

		IORequestPtrPairList m_IORequestList;

		CriticalSection m_IORequestListSection;

		ConditionVariable m_IORequestListCondition;

		bool m_bStopped;

	public:
		AsynchronousIOMgr(void)
			: m_bStopped(false)
		{
		}

		virtual DWORD OnProc(void);

		my::IORequestPtr PushIORequestResource(const std::string & key, my::IORequestPtr request);

		void StopIORequestProc(void);
	};

	class DeviceRelatedResourceMgr
	{
	protected:
		typedef boost::weak_ptr<DeviceRelatedObjectBase> DeviceRelatedObjectBaseWeakPtr;

		typedef boost::unordered_map<std::string, DeviceRelatedObjectBaseWeakPtr> DeviceRelatedObjectBaseWeakPtrSet;

		DeviceRelatedObjectBaseWeakPtrSet m_ResourceWeakSet;

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
	};

	class AsynchronousResourceMgr : public AsynchronousIOMgr, public DeviceRelatedResourceMgr, public ID3DXInclude
	{
	protected:
		CComPtr<ID3DXEffectPool> m_EffectPool;

		std::string m_EffectInclude;

		boost::unordered_map<LPCVOID, CachePtr> m_CacheSet;

	public:
		AsynchronousResourceMgr(void)
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

		__declspec(nothrow) HRESULT __stdcall Open(
			D3DXINCLUDE_TYPE IncludeType,
			LPCSTR pFileName,
			LPCVOID pParentData,
			LPCVOID * ppData,
			UINT * pBytes);

		__declspec(nothrow) HRESULT __stdcall Close(
			LPCVOID pData);

		IORequestPtr LoadResourceAsync(const std::string & key, IORequestPtr request);

		void CheckResource(void);

		bool CheckRequest(const std::string & key, IORequestPtr request, DWORD timeout);

		void LoadTextureAsync(const std::string & path, const ResourceCallback & callback);

		BaseTexturePtr LoadTexture(const std::string & path);

		void LoadMeshAsync(const std::string & path, const ResourceCallback & callback);

		OgreMeshPtr LoadMesh(const std::string & path);

		void LoadSkeletonAsync(const std::string & path, const ResourceCallback & callback);

		OgreSkeletonAnimationPtr LoadSkeleton(const std::string & path);

		class EffectIORequest;

		typedef std::pair<std::string, std::string> EffectMacroPair;

		typedef std::vector<EffectMacroPair> EffectMacroPairList;

		void LoadEffectAsync(const std::string & path, const EffectMacroPairList & macros, const ResourceCallback & callback);

		EffectPtr LoadEffect(const std::string & path, const EffectMacroPairList & macros);

		void LoadFontAsync(const std::string & path, int height, const ResourceCallback & callback);

		FontPtr LoadFont(const std::string & path, int height);
	};
};
