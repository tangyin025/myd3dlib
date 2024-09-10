// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <d3d9.h>
#include <d3dx9shader.h>
#include <boost/unordered_map.hpp>
#include "myThread.h"
#include "mySingleton.h"

struct zip_file;

struct zip_stat;

struct zip;

namespace my
{
	class ZipIStream : public IStream
	{
	protected:
		boost::shared_ptr<zip_stat> m_stat;

		zip_file * m_zipf;

		CriticalSection & m_DirSec;

	public:
		ZipIStream(boost::shared_ptr<zip_stat> stat, zip_file * zipf, CriticalSection & DirSec);

		~ZipIStream(void);

		virtual int read(void * buff, unsigned int read_size);

		virtual long seek(long offset, int origin);

		virtual long tell(void);

		virtual size_t GetSize(void);
	};

	class FileIStream : public IStream
	{
	protected:
		int m_fp;

	public:
		FileIStream(int fp);

		~FileIStream(void);

		static IStreamPtr Open(LPCTSTR pFilename);

		virtual int read(void * buff, unsigned int read_size);

		virtual long seek(long offset, int origin);

		virtual long tell(void);

		virtual size_t GetSize(void);
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

		virtual bool CheckPath(const char * path) = 0;

		virtual std::string GetFullPath(const char * path) = 0;

		virtual std::string GetRelativePath(const char * path) = 0;

		virtual IStreamPtr OpenIStream(const char * path) = 0;
	};

	class ZipIStreamDir : public StreamDir
	{
	protected:
		zip * m_archive;

		CriticalSection m_DirSec;

	public:
		ZipIStreamDir(const std::string & dir);

		~ZipIStreamDir(void);

		bool CheckPath(const char * path);

		std::string GetFullPath(const char * path);

		std::string GetRelativePath(const char * path);

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

		std::string GetRelativePath(const char * path);

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

		std::string GetRelativePath(const char * path);

		IStreamPtr OpenIStream(const char * path);
	};

	class IORequest
	{
	public:
		friend class AsynchronousIOMgr;

		friend class ResourceMgr;

		int m_Priority;

		Event m_PreLoadEvent;

		Event m_PostLoadEvent;

		typedef boost::function<void(my::DeviceResourceBasePtr)> ResourceCallback;

		typedef std::vector<ResourceCallback> ResourceCallbackSet;

		ResourceCallbackSet m_callbacks;

		DeviceResourceBasePtr m_res;

	public:
		IORequest(int Priority)
			: m_Priority(Priority)
			, m_PreLoadEvent(NULL, TRUE, FALSE, NULL)
			, m_PostLoadEvent(NULL, TRUE, FALSE, NULL)
		{
		}

		virtual ~IORequest(void)
		{
		}

		virtual void LoadResource(void) = 0;

		virtual void CreateResource(LPDIRECT3DDEVICE9 pd3dDevice) = 0;
	};

	typedef boost::shared_ptr<IORequest> IORequestPtr;

	class AsynchronousIOMgr
	{
	protected:
		typedef std::map<std::string, IORequestPtr> IORequestPtrPairList;

		IORequestPtrPairList m_IORequestList;

		Mutex m_IORequestListMutex;

		ConditionVariable m_IORequestListCondition;

		bool m_bStopped;

		typedef std::vector<ThreadPtr> ThreadPtrList;

		ThreadPtrList m_Threads;

		static bool IsMainThread(void);

	public:
		AsynchronousIOMgr(void);

		DWORD IORequestProc(void);

		template <typename T>
		IORequestPtrPairList::iterator PushIORequest(const std::string & key, my::IORequestPtr request, const T & callback)
		{
			_ASSERT(IsMainThread());

			_ASSERT(request->m_callbacks.empty());

			MutexLock lock(m_IORequestListMutex);

			std::pair<IORequestPtrPairList::iterator, bool> res = m_IORequestList.insert(std::make_pair(key, request));
			if (!res.second)
			{
				_ASSERT(std::find(res.first->second->m_callbacks.begin(), res.first->second->m_callbacks.end(), callback) == res.first->second->m_callbacks.end());
				res.first->second->m_callbacks.push_back(callback);
				return res.first;
			}

			_ASSERT(std::find(res.first->second->m_callbacks.begin(), res.first->second->m_callbacks.end(), callback) == res.first->second->m_callbacks.end());
			res.first->second->m_callbacks.push_back(callback);
			lock.Unlock();

			m_IORequestListCondition.Wake(1);

			return res.first;
		}

		template <typename T>
		void RemoveIORequestCallback(const std::string& key, const T & callback)
		{
			_ASSERT(IsMainThread());

			MutexLock lock(m_IORequestListMutex);

			IORequestPtrPairList::iterator req_iter = m_IORequestList.find(key);
			if (req_iter != m_IORequestList.end())
			{
				IORequest::ResourceCallbackSet::iterator callback_iter = std::find(req_iter->second->m_callbacks.begin(), req_iter->second->m_callbacks.end(), callback);
				if (callback_iter != req_iter->second->m_callbacks.end())
				{
					req_iter->second->m_callbacks.erase(callback_iter);
					if (req_iter->second->m_callbacks.empty())
					{
						m_IORequestList.erase(req_iter);
					}
				}
			}
		}

		//void RemoveAllIORequest(void);

		bool FindIORequest(const std::string & key);

		template <typename T>
		bool FindIORequestCallback(const T & callback) const
		{
			_ASSERT(IsMainThread());

			IORequestPtrPairList::const_iterator req_iter = m_IORequestList.begin();
			for (; req_iter != m_IORequestList.end(); req_iter++)
			{
				IORequest::ResourceCallbackSet::iterator callback_iter = std::find(req_iter->second->m_callbacks.begin(), req_iter->second->m_callbacks.end(), callback);
				if (callback_iter != req_iter->second->m_callbacks.end())
				{
					return true;
				}
			}
			return false;
		}

		void StartIORequestProc(LONG lMaximumCount);

		void StopIORequestProc(void);
	};

	class BaseTexture;

	class VertexBuffer;

	class OgreMesh;

	class OgreMeshSet;

	class OgreSkeletonAnimation;

	class Effect;

	class Font;

	class Wav;

	template <class _Elem>
	class IStreamBuff : public std::basic_streambuf<_Elem, std::char_traits<_Elem>>
	{
	protected:
		IStreamPtr fptr_;
		const std::size_t put_back_;
		std::vector<_Elem> buffer_;

	public:
		IStreamBuff(IStreamPtr fptr, size_t buff_sz = 1024, size_t put_back = 1)
			: fptr_(fptr)
			, put_back_(max(put_back, size_t(1)))
			, buffer_(max(buff_sz, put_back_) + put_back_)
		{
			_Elem*end = &buffer_.front() + buffer_.size();
			setg(end, end, end);
		}

		virtual int_type underflow(void)
		{
			if (gptr() < egptr()) // buffer not exhausted
				return traits_type::to_int_type(*gptr());

			_Elem* base = &buffer_.front();
			_Elem* start = base;

			if (eback() == base) // true when this isn't the first fill
			{
				// Make arrangements for putback characters
				std::memmove(base, egptr() - put_back_, put_back_ * sizeof(_Elem));
				start += put_back_;
			}

			// start is now the start of the buffer, proper.
			// Read from fptr_ in to the provided buffer
			size_t n = fptr_->read(start, (buffer_.size() - (start - base)) * sizeof(_Elem));
			if (n == 0)
				return traits_type::eof();

			// Set buffer pointers
			setg(base, start, start + n);

			return traits_type::to_int_type(*gptr());
		}
	};

	class ResourceMgr
		: public SingletonInstance<ResourceMgr>
		, public StreamDirMgr
		, public AsynchronousIOMgr
		, public ID3DXInclude
	{
	public:
		friend class EffectIORequest;

		typedef std::map<std::string, DeviceResourceBasePtr> DeviceResourceBasePtrSet;

		DeviceResourceBasePtrSet m_ResourceSet;

		CComPtr<ID3DXEffectPool> m_EffectPool;

		std::string m_LocalInclude;

		std::vector<std::string> m_SystemIncludes;

		typedef boost::unordered_map<LPCVOID, CachePtr> CacheSet;

		CacheSet m_CacheSet;

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

		DeviceResourceBasePtr AddResource(const std::string & key, DeviceResourceBasePtr res);

		template <typename T>
		IORequestPtrPairList::iterator LoadIORequestAsync(const std::string & key, IORequestPtr request, const T & callback)
		{
			_ASSERT(IsMainThread());

			_ASSERT(!key.empty());

			DeviceResourceBasePtr res = GetResource(key);
			if (res)
			{
				request->m_res = res;

				request->m_callbacks.push_back(callback);

				request->m_PostLoadEvent.SetEvent();

				OnIORequestCallback(request);

				return m_IORequestList.end();
			}

			return PushIORequest(key, request, callback);
		}

		template <typename T>
		void LoadIORequestAndWait(const std::string& key, IORequestPtr request, const T & callback)
		{
			_ASSERT(IsMainThread());

			D3DContext::getSingleton().m_d3dDeviceSec.Leave();

			IORequestPtrPairList::iterator req_iter = LoadIORequestAsync(key, request, callback);
			if (req_iter != m_IORequestList.end())
			{
				if (req_iter->second->m_PostLoadEvent.Wait(INFINITE))
				{
					OnIORequestIteratorReady(req_iter);
				}
			}

			D3DContext::getSingleton().m_d3dDeviceSec.Enter();
		}

		bool CheckIORequests(DWORD dwMilliseconds);

		IORequestPtrPairList::iterator OnIORequestIteratorReady(IORequestPtrPairList::iterator req_iter);

		void OnIORequestReady(const std::string & key, IORequestPtr request);

		void OnIORequestCallback(IORequestPtr request);

		template <typename T>
		void LoadTextureAsync(const char * path, const T & callback, int Priority = 0)
		{
			IORequestPtr request(new TextureIORequest(path, Priority));
			LoadIORequestAsync(path, request, callback);
		}

		boost::shared_ptr<BaseTexture> LoadTexture(const char * path);

		template <typename T>
		void LoadMeshAsync(const char * path, const T & callback, int Priority = 0)
		{
			IORequestPtr request(new MeshIORequest(path, Priority));
			LoadIORequestAsync(path, request, callback);
		}

		boost::shared_ptr<OgreMesh> LoadMesh(const char * path);

		template <typename T>
		void LoadSkeletonAsync(const char * path, const T & callback, int Priority = 0)
		{
			IORequestPtr request(new SkeletonIORequest(path, Priority));
			LoadIORequestAsync(path, request, callback);
		}

		boost::shared_ptr<OgreSkeletonAnimation> LoadSkeleton(const char * path);

		template <typename T>
		void LoadEffectAsync(const char * path, const char * macros, const T & callback, int Priority = 0)
		{
			std::string key = EffectIORequest::BuildKey(path, macros);
			IORequestPtr request(new EffectIORequest(path, macros, Priority));
			LoadIORequestAsync(key, request, callback);
		}

		boost::shared_ptr<Effect> LoadEffect(const char * path, const char * macros);

		template <typename T>
		void LoadFontAsync(const char * path, int height, int face_index, const T & callback, int Priority = 0)
		{
			std::string key = FontIORequest::BuildKey(path, height, face_index);
			IORequestPtr request(new FontIORequest(path, height, face_index, Priority));
			LoadIORequestAsync(key, request, callback);
		}

		boost::shared_ptr<Font> LoadFont(const char * path, int height, int face_index);

		template <typename T>
		void LoadWavAsync(const char * path, const T & callback, int Priority = 0)
		{
			IORequestPtr request(new WavIORequest(path, Priority));
			LoadIORequestAsync(path, request, callback);
		}

		boost::shared_ptr<Wav> LoadWav(const char * path);
	};

	class SimpleResourceCallback
	{
	public:
		DeviceResourceBasePtr m_res;

		void OnResourceReady(DeviceResourceBasePtr res)
		{
			m_res = res;
		}
	};

	class TextureIORequest : public IORequest
	{
	protected:
		std::string m_path;

	public:
		TextureIORequest(const char * path, int Priority)
			: IORequest(Priority)
			, m_path(path)
		{
		}

		virtual void LoadResource(void);

		virtual void CreateResource(LPDIRECT3DDEVICE9 pd3dDevice);
	};

	class MeshIORequest : public IORequest
	{
	protected:
		std::string m_path;

	public:
		MeshIORequest(const char * path, int Priority);

		virtual void LoadResource(void);

		virtual void CreateResource(LPDIRECT3DDEVICE9 pd3dDevice);
	};

	class SkeletonIORequest : public IORequest
	{
	protected:
		std::string m_path;

	public:
		SkeletonIORequest(const char * path, int Priority)
			: IORequest(Priority)
			, m_path(path)
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
		EffectIORequest(const char * path, std::string macros, int Priority);

		virtual void LoadResource(void);

		virtual void CreateResource(LPDIRECT3DDEVICE9 pd3dDevice);

		static std::string BuildKey(const char * path, const char * macros);
	};

	class FontIORequest : public IORequest
	{
	protected:
		std::string m_path;

		int m_height;

		int m_face_index;

		CachePtr m_cache;

	public:
		FontIORequest(const char * path, int height, int face_index, int Priority)
			: IORequest(Priority)
			, m_path(path)
			, m_height(height)
			, m_face_index(face_index)
		{
		}

		virtual void LoadResource(void);

		virtual void CreateResource(LPDIRECT3DDEVICE9 pd3dDevice);

		static std::string BuildKey(const char * path, int height, int face_index);
	};

	class WavIORequest : public IORequest
	{
	protected:
		std::string m_path;

	public:
		WavIORequest(const char * path, int Priority)
			: IORequest(Priority)
			, m_path(path)
		{
		}

		virtual void LoadResource(void);

		virtual void CreateResource(LPDIRECT3DDEVICE9 pd3dDevice);
	};
}
