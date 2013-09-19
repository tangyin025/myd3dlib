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
#include <D3DX9Shader.h>

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
		enum IORequestState
		{
			IORequestStateNone = 0,
			IORequestStateLoaded,
		};

		IORequestState m_state;

		typedef std::vector<ResourceCallback> ResourceCallbackList;

		ResourceCallbackList m_callbacks;

		DeviceRelatedObjectBasePtr m_res;

	public:
		IORequest(void)
			: m_state(IORequestStateNone)
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

		void PushIORequestResource(const std::string & key, my::IORequestPtr request);

		void StopIORequestProc(void);
	};

	class DeviceRelatedResourceMgr : public AsynchronousIOMgr, public ID3DXInclude
	{
	protected:
		typedef boost::weak_ptr<DeviceRelatedObjectBase> DeviceRelatedObjectBaseWeakPtr;

		typedef boost::unordered_map<std::string, DeviceRelatedObjectBaseWeakPtr> DeviceRelatedObjectBaseWeakPtrSet;

		DeviceRelatedObjectBaseWeakPtrSet m_ResourceWeakSet;

		std::string m_EffectInclude;

		boost::unordered_map<LPCVOID, CachePtr> m_CacheSet;

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

		__declspec(nothrow) HRESULT __stdcall Open(
			D3DXINCLUDE_TYPE IncludeType,
			LPCSTR pFileName,
			LPCVOID pParentData,
			LPCVOID * ppData,
			UINT * pBytes);

		__declspec(nothrow) HRESULT __stdcall Close(
			LPCVOID pData);
	};

	class AsynchronousResourceMgr : public DeviceRelatedResourceMgr
	{
	protected:
		CComPtr<ID3DXEffectPool> m_EffectPool;

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

		void LoadResource(const std::string & key, IORequestPtr request);

		void CheckResource(void);

		void LoadTexture(const std::string & path, const ResourceCallback & callback);

		void LoadMesh(const std::string & path, const ResourceCallback & callback);

		void LoadSkeleton(const std::string & path, const ResourceCallback & callback);

		typedef std::pair<std::string, std::string> EffectMacroPair;

		typedef std::vector<EffectMacroPair> EffectMacroPairList;

		void LoadEffect(const std::string & path, const EffectMacroPairList & macros, const ResourceCallback & callback);

		void LoadFont(const std::string & path, int height, const ResourceCallback & callback);
	};
};
