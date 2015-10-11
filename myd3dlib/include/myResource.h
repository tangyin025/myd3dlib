#pragma once

#include <unzip.h>
#include <list>
#include "myThread.h"
#include <boost/weak_ptr.hpp>
#include "mySingleton.h"
#include "zzip/zzip.h"

namespace my
{
	typedef std::vector<unsigned char> Cache;

	typedef boost::shared_ptr<Cache> CachePtr;

	class IStream
	{
	public:
		virtual ~IStream(void)
		{
		}

		virtual int read(void * buff, unsigned read_size) = 0;

		virtual long seek(long offset) = 0;

		virtual long tell(void) = 0;

		virtual unsigned long GetSize(void) = 0;

		virtual CachePtr GetWholeCache(void);
	};

	typedef boost::shared_ptr<IStream> IStreamPtr;

	class OStream
	{
	public:
		virtual ~OStream(void)
		{
		}

		virtual int write(const void * buff, unsigned write_size) = 0;
	};

	typedef boost::shared_ptr<OStream> OStreamPtr;

	class ZipIStream : public IStream
	{
	protected:
		ZZIP_FILE * m_fp;

		CriticalSection & m_DirSec;

	public:
		ZipIStream(ZZIP_FILE * fp, CriticalSection & DirSec);

		~ZipIStream(void);

		virtual int read(void * buff, unsigned read_size);

		virtual long seek(long offset);

		virtual long tell(void);

		virtual unsigned long GetSize(void);
	};

	class FileIStream : public IStream
	{
	protected:
		int m_fp;

	public:
		FileIStream(int fp);

		~FileIStream(void);

		static IStreamPtr Open(LPCTSTR pFilename);

		virtual int read(void * buff, unsigned read_size);

		virtual long seek(long offset);

		virtual long tell(void);

		virtual unsigned long GetSize(void);
	};

	class FileOStream : public OStream
	{
	protected:
		int m_fp;

	public:
		FileOStream(int fp);

		~FileOStream(void);

		static OStreamPtr Open(LPCTSTR pFilename);

		virtual int write(const void * buff, unsigned write_size);
	};

	class MemoryIStream : public IStream
	{
	protected:
		unsigned char * m_buffer;

		long m_size;

		long m_tell;

	public:
		MemoryIStream(void * buffer, size_t size);

		virtual int read(void * buff, unsigned read_size);

		virtual long seek(long offset);

		virtual long tell(void);

		virtual unsigned long GetSize(void);
	};

	class MemoryOStream : public OStream
	{
	public:
		CachePtr m_cache;

	public:
		MemoryOStream(void);

		virtual int write(const void * buff, unsigned write_size);
	};

	typedef boost::shared_ptr<MemoryOStream> MemoryOStreamPtr;

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

		static std::string ReplaceSlash(const std::string & path);

		static std::string ReplaceBackslash(const std::string & path);

		virtual bool CheckPath(const std::string & path) = 0;

		virtual std::string GetFullPath(const std::string & path) = 0;

		virtual IStreamPtr OpenIStream(const std::string & path) = 0;
	};

	class ZipIStreamDir : public StreamDir
	{
	protected:
		ZZIP_DIR * m_zipdir;

		CriticalSection m_DirSec;

	public:
		ZipIStreamDir(const std::string & dir);

		~ZipIStreamDir(void);

		bool CheckPath(const std::string & path);

		std::string GetFullPath(const std::string & path);

		IStreamPtr OpenIStream(const std::string & path);
	};

	class FileIStreamDir : public StreamDir
	{
	public:
		FileIStreamDir(const std::string & dir)
			: StreamDir(dir)
		{
		}

		bool CheckPath(const std::string & path);

		std::string GetFullPath(const std::string & path);

		IStreamPtr OpenIStream(const std::string & path);
	};

	class StreamDirMgr
	{
	protected:
		typedef boost::shared_ptr<StreamDir> ResourceDirPtr;

		typedef std::vector<ResourceDirPtr> ResourceDirPtrList;

		ResourceDirPtrList m_DirList;

	public:
		StreamDirMgr(void)
		{
		}

		virtual ~StreamDirMgr(void)
		{
		}

		void RegisterZipDir(const std::string & zip_path);

		void RegisterFileDir(const std::string & dir);

		bool CheckPath(const std::string & path);

		std::string GetFullPath(const std::string & path);

		IStreamPtr OpenIStream(const std::string & path);
	};

	typedef boost::function<void (DeviceRelatedObjectBasePtr)> ResourceCallback;

	class IORequest
	{
	public:
		Event m_LoadEvent;

		typedef std::list<ResourceCallback> ResourceCallbackList;

		ResourceCallbackList m_callbacks;

		DeviceRelatedObjectBasePtr m_res;

	public:
		IORequest(const ResourceCallback & callback)
			: m_LoadEvent(NULL, TRUE, FALSE, NULL)
		{
			m_callbacks.push_back(callback);
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

		DeviceRelatedObjectBasePtr GetResource(const std::string & key);

		void AddResource(const std::string & key, DeviceRelatedObjectBasePtr res);

		std::string GetResourceKey(DeviceRelatedObjectBasePtr res) const;
	};

	class BaseTexture;

	class OgreMesh;

	class OgreMeshSet;

	class OgreSkeletonAnimation;

	class Effect;

	class Font;

	class Emitter;

	class membuf : public std::streambuf
	{
	public:
		membuf(const char * buff, size_t size)
		{
			char * p = const_cast<char *>(buff);
			setg(p, p, p + size);
		}
	};

	class ResourceCallbackBoundle
	{
	public:
		DeviceRelatedObjectBasePtr m_res;

		IORequest::ResourceCallbackList m_callbacks;

		ResourceCallbackBoundle(DeviceRelatedObjectBasePtr res)
			: m_res(res)
		{
		}

		~ResourceCallbackBoundle(void)
		{
			IORequest::ResourceCallbackList::const_iterator callback_iter = m_callbacks.begin();
			for(; callback_iter != m_callbacks.end(); callback_iter++)
			{
				if(*callback_iter)
				{
					(*callback_iter)(m_res);
				}
			}
		}
	};

	typedef boost::shared_ptr<ResourceCallbackBoundle> ResourceCallbackBoundlePtr;

	class ResourceMgr : public AsynchronousIOMgr, public DeviceRelatedResourceMgr, public ID3DXInclude
	{
	protected:
		CComPtr<ID3DXEffectPool> m_EffectPool;

		std::string m_EffectInclude;

		typedef boost::unordered_map<LPCVOID, CachePtr> CacheSet;

		CacheSet m_CacheSet;

		typedef boost::unordered_map<std::string, const char *> HeaderMap;

		HeaderMap m_HeaderMap;

	public:
		ResourceMgr(void)
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

		bool CheckRequests(void);

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

		virtual void OnResourceFailed(const std::string & error_str) = 0;

		void LoadTextureAsync(const std::string & path, const ResourceCallback & callback);

		boost::shared_ptr<BaseTexture> LoadTexture(const std::string & path);

		void LoadMeshAsync(const std::string & path, const ResourceCallback & callback);

		boost::shared_ptr<OgreMesh> LoadMesh(const std::string & path);

		void LoadMeshSetAsync(const std::string & path, const ResourceCallback & callback);

		boost::shared_ptr<OgreMeshSet> LoadMeshSet(const std::string & path);

		void LoadSkeletonAsync(const std::string & path, const ResourceCallback & callback);

		boost::shared_ptr<OgreSkeletonAnimation> LoadSkeleton(const std::string & path);

		class EffectIORequest : public IORequest
		{
		protected:
			std::string m_path;

			std::list<std::string> m_macros;

			std::vector<D3DXMACRO> m_d3dmacros;

			ResourceMgr * m_arc;

			CachePtr m_cache;

		public:
			EffectIORequest(const ResourceCallback & callback, const std::string & path, std::string macros, ResourceMgr * arc);

			virtual void DoLoad(void);

			virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice);

			static std::string BuildKey(const std::string & path, const std::string & macros);
		};

		void LoadEffectAsync(const std::string & path, const std::string & macros, const ResourceCallback & callback);

		boost::shared_ptr<Effect> LoadEffect(const std::string & path, const std::string & macros);

		void LoadFontAsync(const std::string & path, int height, const ResourceCallback & callback);

		boost::shared_ptr<Font> LoadFont(const std::string & path, int height);

		void SaveMesh(const std::string & path, boost::shared_ptr<OgreMesh> mesh);

		void SaveSimplyMesh(const std::string & path, boost::shared_ptr<OgreMesh> mesh, DWORD MinFaces);
	};
}
