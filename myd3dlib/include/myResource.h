#pragma once

#include <unzip.h>
#include <list>
#include "myThread.h"
#include <boost/weak_ptr.hpp>
#include "mySingleton.h"
#include "zzip/zzip.h"
#include "rapidxml.hpp"

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

		static std::string ReplaceSlash(const char * path);

		static std::string ReplaceBackslash(const char * path);

		virtual bool CheckPath(const char * path) = 0;

		virtual std::string GetFullPath(const char * path) = 0;

		virtual IStreamPtr OpenIStream(const char * path) = 0;
	};

	class ZipIStreamDir : public StreamDir
	{
	protected:
		ZZIP_DIR * m_zipdir;

		CriticalSection m_DirSec;

	public:
		ZipIStreamDir(const std::string & dir);

		~ZipIStreamDir(void);

		bool CheckPath(const char * path);

		std::string GetFullPath(const char * path);

		IStreamPtr OpenIStream(const char * path);
	};

	class FileIStreamDir : public StreamDir
	{
	public:
		FileIStreamDir(const std::string & dir)
			: StreamDir(dir)
		{
		}

		bool CheckPath(const char * path);

		std::string GetFullPath(const char * path);

		IStreamPtr OpenIStream(const char * path);
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

		bool CheckPath(const char * path);

		std::string GetFullPath(const char * path);

		IStreamPtr OpenIStream(const char * path);
	};

	class IResourceCallback
	{
	protected:
		friend class AsynchronousIOMgr;

		friend class ResourceMgr;

		bool m_Requested;

	public:
		IResourceCallback(void)
			: m_Requested(false)
		{
		}

		virtual ~IResourceCallback(void)
		{
			_ASSERT(!IsRequested());
		}

		bool IsRequested(void) const
		{
			return m_Requested;
		}

		virtual void OnReady(DeviceResourceBasePtr res) = 0;
	};

	class IORequest
	{
	protected:
		friend class AsynchronousIOMgr;

		friend class ResourceMgr;

		Event m_LoadEvent;

		typedef std::set<IResourceCallback *> IResourceCallbackSet;

		IResourceCallbackSet m_callbacks;

		DeviceResourceBasePtr m_res;

	public:
		IORequest(void)
			: m_LoadEvent(NULL, TRUE, FALSE, NULL)
		{
		}

		virtual ~IORequest(void)
		{
		}

		virtual void LoadResource(void) = 0;

		virtual void CreateResource(LPDIRECT3DDEVICE9 pd3dDevice) = 0;
	};

	typedef boost::shared_ptr<IORequest> IORequestPtr;

	class AsynchronousIOMgr : public StreamDirMgr
	{
	protected:
		typedef std::pair<std::string, IORequestPtr> IORequestPtrPair;

		typedef std::list<IORequestPtrPair> IORequestPtrPairList;

		IORequestPtrPairList m_IORequestList;

		Mutex m_IORequestListMutex;

		ConditionVariable m_IORequestListCondition;

		bool m_bStopped;

		Thread m_Thread;

	public:
		AsynchronousIOMgr(void);

		DWORD IORequestProc(void);

		IORequestPtrPairList::iterator PushIORequest(const std::string & key, my::IORequestPtr request);

		void RemoveIORequestCallback(const std::string & key, IResourceCallback * callback);

		void StartIORequestProc(void);

		void StopIORequestProc(void);
	};

	class BaseTexture;

	class OgreMesh;

	class OgreMeshSet;

	class OgreSkeletonAnimation;

	class Effect;

	class Font;

	class Emitter;

	class IStreamBuff : public std::streambuf
	{
	protected:
		IStreamPtr fptr_;
		const std::size_t put_back_;
		std::vector<char> buffer_;

	public:
		IStreamBuff(IStreamPtr fptr, size_t buff_sz = 1024, size_t put_back = 1);

		std::streambuf::int_type underflow(void);
	};

	class ResourceMgr
		: public SingleInstance<ResourceMgr>
		, public AsynchronousIOMgr
		, public ID3DXInclude
	{
	protected:
		friend class EffectIORequest;

		typedef boost::weak_ptr<DeviceResourceBase> DeviceResourceBaseWeakPtr;

		typedef boost::unordered_map<std::string, DeviceResourceBaseWeakPtr> DeviceResourceBaseWeakPtrSet;

		DeviceResourceBaseWeakPtrSet m_ResourceWeakSet;

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

		DeviceResourceBasePtr GetResource(const std::string & key);

		void AddResource(const std::string & key, DeviceResourceBasePtr res);

		std::string GetResourceKey(DeviceResourceBasePtr res) const;

		IORequestPtrPairList::iterator LoadIORequestAsync(const std::string & key, IORequestPtr request);

		void LoadIORequestAndWait(const std::string & key, IORequestPtr request);

		bool CheckIORequests(void);

		void OnIORequestIteratorReady(IORequestPtrPairList::iterator req_iter);

		void OnIORequestReady(const std::string & key, IORequestPtr request);

		void LoadTextureAsync(const char * path, IResourceCallback * callback);

		boost::shared_ptr<BaseTexture> LoadTexture(const char * path);

		void LoadMeshAsync(const char * path, IResourceCallback * callback);

		boost::shared_ptr<OgreMesh> LoadMesh(const char * path);

		void LoadSkeletonAsync(const char * path, IResourceCallback * callback);

		boost::shared_ptr<OgreSkeletonAnimation> LoadSkeleton(const char * path);

		void LoadEffectAsync(const char * path, const char * macros, IResourceCallback * callback);

		boost::shared_ptr<Effect> LoadEffect(const char * path, const char * macros);

		void LoadFontAsync(const char * path, int height, IResourceCallback * callback);

		boost::shared_ptr<Font> LoadFont(const char * path, int height);
	};

	class TextureIORequest : public IORequest
	{
	protected:
		std::string m_path;

		CachePtr m_cache;

	public:
		TextureIORequest(const char * path)
			: m_path(path)
		{
		}

		virtual void LoadResource(void);

		virtual void CreateResource(LPDIRECT3DDEVICE9 pd3dDevice);
	};

	class MeshIORequest : public IORequest
	{
	protected:
		std::string m_path;

		CachePtr m_cache;

		rapidxml::xml_document<char> m_doc;

	public:
		MeshIORequest(const char * path)
			: m_path(path)
		{
		}

		virtual void LoadResource(void);

		virtual void CreateResource(LPDIRECT3DDEVICE9 pd3dDevice);
	};

	class SkeletonIORequest : public IORequest
	{
	protected:
		std::string m_path;

		CachePtr m_cache;

		rapidxml::xml_document<char> m_doc;

	public:
		SkeletonIORequest(const char * path)
			: m_path(path)
		{
		}

		virtual void LoadResource(void);

		virtual void CreateResource(LPDIRECT3DDEVICE9 pd3dDevice);
	};

	class EffectIORequest : public IORequest
	{
	protected:
		std::string m_path;

		std::vector<std::string> m_macros;

		std::vector<D3DXMACRO> m_d3dmacros;

		CachePtr m_cache;

	public:
		EffectIORequest(const char * path, std::string macros);

		virtual void LoadResource(void);

		virtual void CreateResource(LPDIRECT3DDEVICE9 pd3dDevice);

		static std::string BuildKey(const char * path, const char * macros);
	};

	class FontIORequest : public IORequest
	{
	protected:
		std::string m_path;

		int m_height;

		CachePtr m_cache;

	public:
		FontIORequest(const char * path, int height)
			: m_path(path)
			, m_height(height)
		{
		}

		virtual void LoadResource(void);

		virtual void CreateResource(LPDIRECT3DDEVICE9 pd3dDevice);

		static std::string BuildKey(const char * path, int height);
	};
}
