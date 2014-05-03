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

	class IOStream
	{
	public:
		virtual ~IOStream(void)
		{
		}

		virtual int read(void * buff, unsigned read_size) = 0;

		virtual CachePtr GetWholeCache(void) = 0;
	};

	typedef boost::shared_ptr<IOStream> IOStreamPtr;

	class ZipStream : public IOStream
	{
	protected:
		unzFile m_zFile;

		unz_file_info m_zFileInfo;

	public:
		ZipStream(unzFile zFile);

		~ZipStream(void);

		virtual int read(void * buff, unsigned read_size);

		virtual CachePtr GetWholeCache(void);
	};

	class FileStream : public IOStream
	{
	protected:
		FILE * m_fp;

	public:
		FileStream(FILE * fp);

		~FileStream(void);

		virtual int read(void * buff, unsigned read_size);

		virtual CachePtr GetWholeCache(void);
	};

	class StreamDir
	{
	public:
		const std::string m_dir;

	public:
		StreamDir(const std::string & dir)
			: m_dir(dir)
		{
		}

		virtual ~StreamDir(void)
		{
		}

		virtual bool CheckPath(const std::string & path) = 0;

		virtual std::string GetFullPath(const std::string & path) = 0;

		virtual IOStreamPtr OpenStream(const std::string & path) = 0;
	};

	class ZipStreamDir : public StreamDir
	{
	protected:
		std::string m_password;

		bool m_UsePassword;

	public:
		ZipStreamDir(const std::string & dir)
			: StreamDir(dir)
			, m_UsePassword(false)
		{
		}

		ZipStreamDir(const std::string & dir, const std::string & password)
			: StreamDir(dir)
			, m_UsePassword(true)
			, m_password(password)
		{
		}

		static std::string ReplaceSlash(const std::string & path);

		static std::string ReplaceBackslash(const std::string & path);

		bool CheckPath(const std::string & path);

		std::string GetFullPath(const std::string & path);

		IOStreamPtr OpenStream(const std::string & path);
	};

	class FileStreamDir : public StreamDir
	{
	public:
		FileStreamDir(const std::string & dir)
			: StreamDir(dir)
		{
		}

		bool CheckPath(const std::string & path);

		std::string GetFullPath(const std::string & path);

		IOStreamPtr OpenStream(const std::string & path);
	};

	class StreamDirMgr
	{
	protected:
		typedef boost::shared_ptr<StreamDir> ResourceDirPtr;

		typedef std::vector<ResourceDirPtr> ResourceDirPtrList;

		ResourceDirPtrList m_DirList;

		CriticalSection m_DirListSection;

	public:
		StreamDirMgr(void)
		{
		}

		virtual ~StreamDirMgr(void)
		{
		}

		void RegisterZipDir(const std::string & zip_path);

		void RegisterZipDir(const std::string & zip_path, const std::string & password);

		void RegisterFileDir(const std::string & dir);

		bool CheckPath(const std::string & path);

		std::string GetFullPath(const std::string & path);

		IOStreamPtr OpenStream(const std::string & path);
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

	class AsynchronousIOMgr : public StreamDirMgr
	{
	protected:
		typedef std::list<std::pair<std::string, IORequestPtr> > IORequestPtrPairList;

		IORequestPtrPairList m_IORequestList;

		Mutex m_IORequestListMutex;

		ConditionVariable m_IORequestListCondition;

		bool m_bStopped;

		Thread m_Thread;

	public:
		AsynchronousIOMgr(void);

		DWORD IORequestProc(void);

		IORequestPtrPairList::iterator PushIORequestResource(const std::string & key, my::IORequestPtr request);

		void StartIORequestProc(void);

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

		std::string GetResourceKey(DeviceRelatedObjectBasePtr res) const;
	};

	typedef std::pair<std::string, std::string> EffectMacroPair;

	typedef std::vector<EffectMacroPair> EffectMacroPairList;

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

		IORequestPtrPairList::iterator LoadResourceAsync(const std::string & key, IORequestPtr request);

		void CheckRequests(void);

		bool CheckResource(const std::string & key, IORequestPtr request, DWORD timeout);

		template <typename T>
		boost::shared_ptr<T> LoadResource(const std::string & key, IORequestPtr request)
		{
			IORequestPtrPairList::iterator req_iter = LoadResourceAsync(key, request);
			if (req_iter != m_IORequestList.end())
			{
				CheckResource(req_iter->first, req_iter->second, INFINITE);

				boost::shared_ptr<T> ret = boost::dynamic_pointer_cast<T>(req_iter->second->m_res);

				MutexLock lock(m_IORequestListMutex);

				m_IORequestList.erase(req_iter);

				return ret;
			}
			return boost::dynamic_pointer_cast<T>(request->m_res);
		}

		virtual void OnResourceFailed(const std::basic_string<TCHAR> & error_str);

		void LoadTextureAsync(const std::string & path, const ResourceCallback & callback);

		BaseTexturePtr LoadTexture(const std::string & path);

		void LoadMeshAsync(const std::string & path, const ResourceCallback & callback);

		OgreMeshPtr LoadMesh(const std::string & path);

		void LoadMeshSetAsync(const std::string & path, const ResourceCallback & callback);

		OgreMeshSetPtr LoadMeshSet(const std::string & path);

		void LoadSkeletonAsync(const std::string & path, const ResourceCallback & callback);

		OgreSkeletonAnimationPtr LoadSkeleton(const std::string & path);

		class EffectIORequest;

		void LoadEffectAsync(const std::string & path, const EffectMacroPairList & macros, const ResourceCallback & callback);

		EffectPtr LoadEffect(const std::string & path, const EffectMacroPairList & macros);

		void LoadFontAsync(const std::string & path, int height, const ResourceCallback & callback);

		FontPtr LoadFont(const std::string & path, int height);
	};
};
