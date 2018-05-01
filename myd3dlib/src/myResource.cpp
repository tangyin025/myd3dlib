#include "stdafx.h"
#include "myResource.h"
#include "myDxutApp.h"
#include "libc.h"
#include <strstream>
#include <boost/bind.hpp>
#include <zzip/file.h>
#include <fstream>
#include <SYS\Stat.h>
#include "myMesh.h"
#include "mySkeleton.h"
#include "myEffect.h"
#include "myFont.h"
#include "myEmitter.h"
#include <boost/algorithm/string.hpp>

using namespace my;

CachePtr my::IStream::GetWholeCache(void)
{
	CachePtr cache(new Cache(GetSize()));
	if(0 == cache->size())
	{
		THROW_CUSEXCEPTION("read stream cache failed");
	}

	int ret = read(&(*cache)[0], cache->size());
	if(ret != cache->size())
	{
		THROW_CUSEXCEPTION("read stream cache failed");
	}
	return cache;
}

ZipIStream::ZipIStream(ZZIP_FILE * fp, CriticalSection & DirSec)
	: m_fp(fp)
	, m_DirSec(DirSec)
{
	_ASSERT(NULL != m_fp);
}

ZipIStream::~ZipIStream(void)
{
	CriticalSectionLock lock(m_DirSec);
	zzip_file_close(m_fp);
}

int ZipIStream::read(void * buff, unsigned read_size)
{
	CriticalSectionLock lock(m_DirSec);
	return zzip_file_read(m_fp, buff, read_size);
}

long ZipIStream::seek(long offset)
{
	CriticalSectionLock lock(m_DirSec);
	return zzip_seek(m_fp, offset, SEEK_SET);
}

long ZipIStream::tell(void)
{
	CriticalSectionLock lock(m_DirSec);
	return zzip_tell(m_fp);
}

unsigned long ZipIStream::GetSize(void)
{
	CriticalSectionLock lock(m_DirSec);
	return m_fp->usize;
}

FileIStream::FileIStream(int fp)
	: m_fp(fp)
{
	_ASSERT(NULL != m_fp);
}

FileIStream::~FileIStream(void)
{
	_close(m_fp);
}

IStreamPtr FileIStream::Open(LPCTSTR pFilename)
{
	int fp;
	errno_t err = _tsopen_s(&fp, pFilename, _O_RDONLY | _O_BINARY, _SH_DENYWR, 0);
	if (0 != err)
	{
		THROW_CUSEXCEPTION(str_printf("cannot open file archive: %S", pFilename));
	}
	return IStreamPtr(new FileIStream(fp));
}

int FileIStream::read(void * buff, unsigned read_size)
{
	return _read(m_fp, buff, read_size);
}

long FileIStream::seek(long offset)
{
	return _lseek(m_fp, offset, SEEK_SET);
}

long FileIStream::tell(void)
{
	return _tell(m_fp);
}

unsigned long FileIStream::GetSize(void)
{
	return _filelength(m_fp);
}

FileOStream::FileOStream(int fp)
	: m_fp(fp)
{
	_ASSERT(NULL != m_fp);
}

FileOStream::~FileOStream(void)
{
	_close(m_fp);
}

OStreamPtr FileOStream::Open(LPCTSTR pFilename)
{
	int fp;
	errno_t err = _tsopen_s(&fp, pFilename, _O_WRONLY | _O_BINARY | _O_CREAT | _O_TRUNC, _SH_DENYRW, _S_IWRITE);
	if (0 != err)
	{
		THROW_CUSEXCEPTION(str_printf("cannot open file archive: %S", pFilename));
	}
	return OStreamPtr(new FileOStream(fp));
}

int FileOStream::write(const void * buff, unsigned write_size)
{
	return _write(m_fp, buff, write_size);
}

std::string StreamDir::ReplaceSlash(const char * path)
{
	size_t pos = 0;
	std::string ret(path);
	while(std::string::npos != (pos = ret.find('/', pos)))
	{
		ret.replace(pos++, 1, 1, '\\');
	}
	return ret;
}

std::string StreamDir::ReplaceBackslash(const char * path)
{
	size_t pos = 0;
	std::string ret(path);
	while(std::string::npos != (pos = ret.find('\\', pos)))
	{
		ret.replace(pos++, 1, 1, '/');
	}
	return ret;
}

ZipIStreamDir::ZipIStreamDir(const std::string & dir)
	: StreamDir(dir)
{
	int fd;
	errno_t err = _sopen_s(&fd, m_dir.c_str(), O_RDONLY|O_BINARY, _SH_DENYWR, _S_IREAD);
	if (0 != err)
	{
		THROW_CUSEXCEPTION(str_printf("cannot open zip archive: %s", m_dir.c_str()));
	}
	zzip_error_t rv;
	m_zipdir = zzip_dir_fdopen(fd, &rv);
	if (!m_zipdir)
	{
		THROW_CUSEXCEPTION(str_printf("cannot open zip archive: %s", m_dir.c_str()));
	}
}

ZipIStreamDir::~ZipIStreamDir(void)
{
	zzip_dir_close(m_zipdir);
}

bool ZipIStreamDir::CheckPath(const char * path)
{
	CriticalSectionLock lock(m_DirSec);
	ZZIP_FILE * zfile = zzip_file_open(m_zipdir, ReplaceBackslash(path).c_str(), ZZIP_CASEINSENSITIVE);
	if(NULL == zfile)
	{
		return false;
	}

	zzip_file_close(zfile);
	return true;
}

std::string ZipIStreamDir::GetFullPath(const char * path)
{
	return std::string();
}

IStreamPtr ZipIStreamDir::OpenIStream(const char * path)
{
	CriticalSectionLock lock(m_DirSec);
	ZZIP_FILE * zfile = zzip_file_open(m_zipdir, ReplaceBackslash(path).c_str(), ZZIP_CASEINSENSITIVE);
	if(NULL == zfile)
	{
		THROW_CUSEXCEPTION(str_printf("cannot open zip file: %s", path));
	}

	return IStreamPtr(new ZipIStream(zfile, m_DirSec));
}

bool FileIStreamDir::CheckPath(const char * path)
{
	return !GetFullPath(path).empty();
}

std::string FileIStreamDir::GetFullPath(const char * path)
{
	std::string fullPath;
	char * lpFilePath;
	DWORD dwLen = MAX_PATH;
	do
	{
		fullPath.resize(dwLen);
		dwLen = SearchPathA(m_dir.c_str(), path, NULL, fullPath.size(), &fullPath[0], &lpFilePath);
	}
	while(dwLen > fullPath.size());

	fullPath.resize(dwLen);
	return fullPath;
}

IStreamPtr FileIStreamDir::OpenIStream(const char * path)
{
	std::string fullPath = GetFullPath(path);
	if(fullPath.empty())
	{
		THROW_CUSEXCEPTION(str_printf("cannot open file archive: %s", path));
	}

	return FileIStream::Open(ms2ts(fullPath).c_str());
}

void StreamDirMgr::RegisterZipDir(const std::string & zip_path)
{
	if (PathFileExistsA(zip_path.c_str()))
	{
		m_DirList.push_back(ResourceDirPtr(new ZipIStreamDir(zip_path)));
	}
}

void StreamDirMgr::RegisterFileDir(const std::string & dir)
{
	m_DirList.push_back(ResourceDirPtr(new FileIStreamDir(dir)));
}

bool StreamDirMgr::CheckPath(const char * path)
{
	if(!PathIsRelativeA(path))
	{
		return PathFileExistsA(path);
	}

	ResourceDirPtrList::iterator dir_iter = m_DirList.begin();
	for(; dir_iter != m_DirList.end(); dir_iter++)
	{
		if((*dir_iter)->CheckPath(path))
		{
			return true;
		}
	}

	return false;
}

std::string StreamDirMgr::GetFullPath(const char * path)
{
	if(!PathIsRelativeA(path))
	{
		return path;
	}

	ResourceDirPtrList::iterator dir_iter = m_DirList.begin();
	for(; dir_iter != m_DirList.end(); dir_iter++)
	{
		std::string ret = (*dir_iter)->GetFullPath(path);
		if(!ret.empty())
		{
			return ret;
		}
	}

	// ! will return the default first combined path
	if(!m_DirList.empty())
	{
		std::string ret(MAX_PATH, '\0');
		PathCombineA(&ret[0], m_DirList.front()->m_dir.c_str(), path);
		ret = ZipIStreamDir::ReplaceSlash(ret.c_str());
		std::string full_path;
		full_path.resize(MAX_PATH);
		GetFullPathNameA(ret.c_str(), full_path.size(), &full_path[0], NULL);
		return full_path;
	}

	return std::string();
}

IStreamPtr StreamDirMgr::OpenIStream(const char * path)
{
	if(!PathIsRelativeA(path))
	{
		return FileIStream::Open(ms2ts(path).c_str());
	}

	ResourceDirPtrList::iterator dir_iter = m_DirList.begin();
	for(; dir_iter != m_DirList.end(); dir_iter++)
	{
		if((*dir_iter)->CheckPath(path))
		{
			return (*dir_iter)->OpenIStream(path);
		}
	}

	THROW_CUSEXCEPTION(str_printf("cannot find specified file: %s", path));
}

AsynchronousIOMgr::AsynchronousIOMgr(void)
	: m_bStopped(false)
	, m_Thread(boost::bind(&AsynchronousIOMgr::IORequestProc, this))
{
}

DWORD AsynchronousIOMgr::IORequestProc(void)
{
	m_IORequestListMutex.Wait(INFINITE);
	while(!m_bStopped)
	{
		IORequestPtrPairList::iterator req_iter = m_IORequestList.begin();
		for(; req_iter != m_IORequestList.end(); req_iter++)
		{
			if(!req_iter->second->m_LoadEvent.Wait(0))
			{
				_ASSERT(!req_iter->second->m_callbacks.empty());
				break;
			}
		}
		if(req_iter != m_IORequestList.end())
		{
			// ! req_iter will be invalid after release mutex
			IORequestPtr request = req_iter->second;

			m_IORequestListMutex.Release();

			// ! HAVENT HANDLED EXCEPTION YET
			request->LoadResource();

			// ! request list will be modified when set event, shared_ptr must be thread safe
			request->m_LoadEvent.SetEvent();

			m_IORequestListMutex.Wait(INFINITE);
		}
		else
		{
			m_IORequestListCondition.Sleep(m_IORequestListMutex, INFINITE);
		}
	}
	m_IORequestListMutex.Release();

	return 0;
}

AsynchronousIOMgr::IORequestPtrPairList::iterator AsynchronousIOMgr::PushIORequest(const std::string & key, my::IORequestPtr request)
{
	_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);
	m_IORequestListMutex.Wait(INFINITE);

	IORequest::IResourceCallbackSet::iterator callback_iter = request->m_callbacks.begin();
	for (; callback_iter != request->m_callbacks.end(); callback_iter++)
	{
		_ASSERT(!(*callback_iter)->IsRequested());
		(*callback_iter)->m_Requested = true;
	}

	IORequestPtrPairList::iterator req_iter = m_IORequestList.begin();
	for(; req_iter != m_IORequestList.end(); req_iter++)
	{
		if(req_iter->first == key)
		{
			req_iter->second->m_callbacks.insert(request->m_callbacks.begin(), request->m_callbacks.end());
			m_IORequestListMutex.Release();
			return req_iter;
		}
	}

	m_IORequestList.push_back(std::make_pair(key, request));
	m_IORequestListMutex.Release();
	m_IORequestListCondition.Wake(1);
	return --m_IORequestList.end();
}

void AsynchronousIOMgr::RemoveIORequestCallback(const std::string & key, IResourceCallback * callback)
{
	_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);
	m_IORequestListMutex.Wait(INFINITE);

	IORequestPtrPairList::iterator req_iter = m_IORequestList.begin();
	for(; req_iter != m_IORequestList.end(); req_iter++)
	{
		if(req_iter->first == key)
		{
			IORequest::IResourceCallbackSet::iterator callback_iter = req_iter->second->m_callbacks.find(callback);
			if (callback_iter != req_iter->second->m_callbacks.end())
			{
				_ASSERT((*callback_iter)->IsRequested());
				(*callback_iter)->m_Requested = false;
				req_iter->second->m_callbacks.erase(callback_iter);
				if (req_iter->second->m_callbacks.empty())
				{
					m_IORequestList.erase(req_iter);
				}
			}
			break;
		}
	}

	m_IORequestListMutex.Release();
}

void AsynchronousIOMgr::StartIORequestProc(void)
{
	m_bStopped = false;
	m_Thread.CreateThread();
	m_Thread.ResumeThread();
}

void AsynchronousIOMgr::StopIORequestProc(void)
{
	_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);
	m_IORequestListMutex.Wait(INFINITE);
	m_bStopped = true;
	m_IORequestListMutex.Release();
	m_IORequestListCondition.Wake(1);
}

IStreamBuff::IStreamBuff(IStreamPtr fptr, size_t buff_sz, size_t put_back)
	: fptr_(fptr)
	, put_back_(std::max(put_back, size_t(1)))
	, buffer_(std::max(buff_sz, put_back_) + put_back_)
{
	char *end = &buffer_.front() + buffer_.size();
	setg(end, end, end);
}

std::streambuf::int_type IStreamBuff::underflow(void)
{
	if (gptr() < egptr()) // buffer not exhausted
		return traits_type::to_int_type(*gptr());

	char *base = &buffer_.front();
	char *start = base;

	if (eback() == base) // true when this isn't the first fill
	{
		// Make arrangements for putback characters
		std::memmove(base, egptr() - put_back_, put_back_);
		start += put_back_;
	}

	// start is now the start of the buffer, proper.
	// Read from fptr_ in to the provided buffer
	size_t n = fptr_->read(start, buffer_.size() - (start - base));
	if (n == 0)
		return traits_type::eof();

	// Set buffer pointers
	setg(base, start, start + n);

	return traits_type::to_int_type(*gptr());
}

HRESULT ResourceMgr::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	HRESULT hr;
	if(FAILED(hr = D3DXCreateEffectPool(&m_EffectPool)))
	{
		return hr;
	}

	StartIORequestProc();

	return S_OK;
}

HRESULT ResourceMgr::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	return S_OK;
}

void ResourceMgr::OnLostDevice(void)
{
}

void ResourceMgr::OnDestroyDevice(void)
{
	StopIORequestProc();

	m_ResourceWeakSet.clear();

	m_EffectPool.Release();

	m_Thread.WaitForThreadStopped(INFINITE);

	m_Thread.CloseThread();

	m_IORequestList.clear();
}

HRESULT ResourceMgr::Open(
	D3DXINCLUDE_TYPE IncludeType,
	LPCSTR pFileName,
	LPCVOID pParentData,
	LPCVOID * ppData,
	UINT * pBytes)
{
	CachePtr cache;
	std::string path(MAX_PATH, '\0');
	HeaderMap::const_iterator header_iter = m_HeaderMap.find(pFileName);
	if (header_iter != m_HeaderMap.end())
	{
		path = header_iter->second;
	}
	else
	{
		PathCombineA(&path[0], m_EffectInclude.c_str(), pFileName);
	}
	switch(IncludeType)
	{
	case D3DXINC_SYSTEM:
	case D3DXINC_LOCAL:
		if(CheckPath(path.c_str()))
		{
			cache = OpenIStream(path.c_str())->GetWholeCache();
			*ppData = &(*cache)[0];
			*pBytes = cache->size();
			_ASSERT(m_CacheSet.end() == m_CacheSet.find(*ppData));
			m_CacheSet[*ppData] = cache;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT ResourceMgr::Close(
	LPCVOID pData)
{
	_ASSERT(m_CacheSet.end() != m_CacheSet.find(pData));
	m_CacheSet.erase(pData);
	return S_OK;
}

DeviceResourceBasePtr ResourceMgr::GetResource(const std::string & key)
{
	DeviceResourceBaseWeakPtrSet::iterator res_iter = m_ResourceWeakSet.find(key);
	if(res_iter != m_ResourceWeakSet.end())
	{
		DeviceResourceBasePtr res = res_iter->second.lock();
		if(res)
		{
			return res;
		}
		else
			m_ResourceWeakSet.erase(res_iter);
	}
	return DeviceResourceBasePtr();
}

void ResourceMgr::AddResource(const std::string & key, DeviceResourceBasePtr res)
{
	_ASSERT(!GetResource(key));

	m_ResourceWeakSet[key] = res;
}

std::string ResourceMgr::GetResourceKey(DeviceResourceBasePtr res) const
{
	DeviceResourceBaseWeakPtrSet::const_iterator res_iter = m_ResourceWeakSet.begin();
	for(; res_iter != m_ResourceWeakSet.end(); res_iter++)
	{
		if(res == res_iter->second.lock())
		{
			return res_iter->first;
		}
	}
	return std::string();
}

AsynchronousIOMgr::IORequestPtrPairList::iterator ResourceMgr::LoadIORequestAsync(const std::string & key, IORequestPtr request)
{
	DeviceResourceBasePtr res = GetResource(key);
	if (res)
	{
		request->m_res = res;

		request->m_LoadEvent.SetEvent();

		OnIORequestReady(key, request);

		return m_IORequestList.end();
	}

	return PushIORequest(key, request);
}

void ResourceMgr::LoadIORequestAndWait(const std::string & key, IORequestPtr request)
{
	_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);

	IORequestPtrPairList::iterator req_iter = LoadIORequestAsync(key, request);
	if (req_iter != m_IORequestList.end())
	{
		if (req_iter->second->m_LoadEvent.Wait(INFINITE))
		{
			OnIORequestIteratorReady(req_iter);
		}
	}
}

bool ResourceMgr::CheckIORequests(void)
{
	_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);

	while (true)
	{
		IORequestPtrPairList::iterator req_iter = m_IORequestList.begin();
		if (req_iter != m_IORequestList.end() && req_iter->second->m_LoadEvent.Wait(0))
		{
			OnIORequestIteratorReady(req_iter);
		}
		else
			break;
	}

	return !m_IORequestList.empty();
}

void ResourceMgr::OnIORequestIteratorReady(IORequestPtrPairList::iterator req_iter)
{
	IORequestPtrPair pair = *req_iter;
	m_IORequestListMutex.Wait(INFINITE);
	m_IORequestList.erase(req_iter);
	m_IORequestListMutex.Release();

	IORequest::IResourceCallbackSet::iterator callback_iter = pair.second->m_callbacks.begin();
	for (; callback_iter != pair.second->m_callbacks.end(); callback_iter++)
	{
		_ASSERT((*callback_iter)->IsRequested());
		(*callback_iter)->m_Requested = false;
	}

	OnIORequestReady(pair.first, pair.second);
}

void ResourceMgr::OnIORequestReady(const std::string & key, IORequestPtr request)
{
	_ASSERT(request->m_LoadEvent.Wait(0));

	if(!request->m_res)
	{
		// ! havent handled exception
		try
		{
			_ASSERT(D3DContext::getSingleton().m_DeviceObjectsCreated);

			request->CreateResource(D3DContext::getSingleton().m_d3dDevice);

			if (request->m_res && D3DContext::getSingleton().m_DeviceObjectsReset)
			{
				request->m_res->OnResetDevice();
			}

			AddResource(key, request->m_res);
		}
		catch(const Exception & e)
		{
			D3DContext::getSingleton().m_EventLog(e.what().c_str());
		}
		catch(const std::exception & e)
		{
			D3DContext::getSingleton().m_EventLog(str_printf("%s error: %s", key.c_str(), e.what()).c_str());
		}
	}

	if (request->m_res)
	{
		IORequest::IResourceCallbackSet::iterator callback_iter = request->m_callbacks.begin();
		for(; callback_iter != request->m_callbacks.end(); callback_iter++)
		{
			_ASSERT(*callback_iter);

			(*callback_iter)->OnReady(request->m_res);
		}
	}
}

void ResourceMgr::LoadTextureAsync(const char * path, IResourceCallback * callback)
{
	IORequestPtr request(new TextureIORequest(path));
	request->m_callbacks.insert(callback);
	LoadIORequestAsync(path, request);
}

class SimpleResourceCallback : public IResourceCallback
{
public:
	DeviceResourceBasePtr m_res;

	virtual void OnReady(DeviceResourceBasePtr res)
	{
		m_res = res;
	}
};

boost::shared_ptr<BaseTexture> ResourceMgr::LoadTexture(const char * path)
{
	SimpleResourceCallback cb;
	IORequestPtr request(new TextureIORequest(path));
	request->m_callbacks.insert(&cb);
	LoadIORequestAndWait(path, request);
	return boost::dynamic_pointer_cast<BaseTexture>(request->m_res);
}

void ResourceMgr::LoadMeshAsync(const char * path, IResourceCallback * callback)
{
	IORequestPtr request(new MeshIORequest(path));
	request->m_callbacks.insert(callback);
	LoadIORequestAsync(path, request);
}

boost::shared_ptr<OgreMesh> ResourceMgr::LoadMesh(const char * path)
{
	SimpleResourceCallback cb;
	IORequestPtr request(new MeshIORequest(path));
	request->m_callbacks.insert(&cb);
	LoadIORequestAndWait(path, request);
	return boost::dynamic_pointer_cast<OgreMesh>(request->m_res);
}

void ResourceMgr::LoadSkeletonAsync(const char * path, IResourceCallback * callback)
{
	IORequestPtr request(new SkeletonIORequest(path));
	request->m_callbacks.insert(callback);
	LoadIORequestAsync(path, request);
}

boost::shared_ptr<OgreSkeletonAnimation> ResourceMgr::LoadSkeleton(const char * path)
{
	SimpleResourceCallback cb;
	IORequestPtr request(new SkeletonIORequest(path));
	request->m_callbacks.insert(&cb);
	LoadIORequestAndWait(path, request);
	return boost::dynamic_pointer_cast<OgreSkeletonAnimation>(request->m_res);
}

void ResourceMgr::LoadEffectAsync(const char * path, const char * macros, IResourceCallback * callback)
{
	std::string key = EffectIORequest::BuildKey(path, macros);
	IORequestPtr request(new EffectIORequest(path, macros));
	request->m_callbacks.insert(callback);
	LoadIORequestAsync(key, request);
}

boost::shared_ptr<Effect> ResourceMgr::LoadEffect(const char * path, const char * macros)
{
	std::string key = EffectIORequest::BuildKey(path, macros);
	SimpleResourceCallback cb;
	IORequestPtr request(new EffectIORequest(path, macros));
	request->m_callbacks.insert(&cb);
	LoadIORequestAndWait(key, request);
	return boost::dynamic_pointer_cast<Effect>(request->m_res);
}

void ResourceMgr::LoadFontAsync(const char * path, int height, IResourceCallback * callback)
{
	std::string key = FontIORequest::BuildKey(path, height);
	IORequestPtr request(new FontIORequest(path, height));
	request->m_callbacks.insert(callback);
	LoadIORequestAsync(key, request);
}

boost::shared_ptr<Font> ResourceMgr::LoadFont(const char * path, int height)
{
	std::string key = FontIORequest::BuildKey(path, height);
	SimpleResourceCallback cb;
	IORequestPtr request(new FontIORequest(path, height));
	request->m_callbacks.insert(&cb);
	LoadIORequestAndWait(key, request);
	return boost::dynamic_pointer_cast<Font>(request->m_res);
}

void TextureIORequest::LoadResource(void)
{
	if(ResourceMgr::getSingleton().CheckPath(m_path.c_str()))
	{
		m_cache = ResourceMgr::getSingleton().OpenIStream(m_path.c_str())->GetWholeCache();
	}
}

void TextureIORequest::CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
{
	if(!m_cache)
	{
		THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
	}
	D3DXIMAGE_INFO imif;
	HRESULT hr = D3DXGetImageInfoFromFileInMemory(&(*m_cache)[0], m_cache->size(), &imif);
	if(FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}
	switch(imif.ResourceType)
	{
	case D3DRTYPE_TEXTURE:
		{
			Texture2DPtr res(new Texture2D());
			res->CreateTextureFromFileInMemory(&(*m_cache)[0], m_cache->size());
			m_res = res;
		}
		break;
	case D3DRTYPE_CUBETEXTURE:
		{
			CubeTexturePtr res(new CubeTexture());
			res->CreateCubeTextureFromFileInMemory(&(*m_cache)[0], m_cache->size());
			m_res = res;
		}
		break;
	default:
		THROW_CUSEXCEPTION(str_printf("unsupported d3d texture format %u", imif.ResourceType));
	}
}

void MeshIORequest::LoadResource(void)
{
	if(ResourceMgr::getSingleton().CheckPath(m_path.c_str()))
	{
		m_cache = ResourceMgr::getSingleton().OpenIStream(m_path.c_str())->GetWholeCache();
		m_cache->push_back(0);
		try
		{
			m_doc.parse<0>((char *)&(*m_cache)[0]);
		}
		catch(rapidxml::parse_error &)
		{
			m_doc.clear();
		}
	}
}

void MeshIORequest::CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
{
	if(!m_doc.first_node())
	{
		THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
	}
	OgreMeshPtr res(new OgreMesh());
	res->CreateMeshFromOgreXml(&m_doc);
	m_res = res;
}

void SkeletonIORequest::LoadResource(void)
{
	if(ResourceMgr::getSingleton().CheckPath(m_path.c_str()))
	{
		m_cache = ResourceMgr::getSingleton().OpenIStream(m_path.c_str())->GetWholeCache();
		m_cache->push_back(0);
		try
		{
			m_doc.parse<0>((char *)&(*m_cache)[0]);
		}
		catch(rapidxml::parse_error &)
		{
			m_doc.clear();
		}
	}
}

void SkeletonIORequest::CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
{
	if(!m_doc.first_node())
	{
		THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
	}
	OgreSkeletonAnimationPtr res(new OgreSkeletonAnimation());
	res->CreateOgreSkeletonAnimation(&m_doc);
	m_res = res;
}

EffectIORequest::EffectIORequest(const char * path, std::string macros)
	: m_path(path)
{
	boost::algorithm::split(m_macros, macros, boost::algorithm::is_any_of(" \t"), boost::algorithm::token_compress_on);

	std::vector<std::string>::const_iterator macro_iter = m_macros.begin();
	for(; macro_iter != m_macros.end(); )
	{
		D3DXMACRO d3dmacro;
		d3dmacro.Name = (macro_iter++)->c_str();
		d3dmacro.Definition = macro_iter != m_macros.end() ? (macro_iter++)->c_str() : NULL;
		m_d3dmacros.push_back(d3dmacro);
	}
	D3DXMACRO end = {0};
	m_d3dmacros.push_back(end);
}

void EffectIORequest::LoadResource(void)
{
	if(ResourceMgr::getSingleton().CheckPath(m_path.c_str()))
	{
		m_cache = ResourceMgr::getSingleton().OpenIStream(m_path.c_str())->GetWholeCache();
	}
}

void EffectIORequest::CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
{
	if(!m_cache)
	{
		THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
	}
	EffectPtr res(new Effect());
	ResourceMgr::getSingleton().m_EffectInclude = ZipIStreamDir::ReplaceSlash(m_path.c_str());
	PathRemoveFileSpecA(&ResourceMgr::getSingleton().m_EffectInclude[0]);
	res->CreateEffect(&(*m_cache)[0], m_cache->size(), &m_d3dmacros[0], ResourceMgr::getSingletonPtr(), 0, ResourceMgr::getSingleton().m_EffectPool);
	m_res = res;
}

std::string EffectIORequest::BuildKey(const char * path, const char * macros)
{
	return str_printf("%s %s", path, macros);
}

void FontIORequest::LoadResource(void)
{
	if(ResourceMgr::getSingleton().CheckPath(m_path.c_str()))
	{
		m_cache = ResourceMgr::getSingleton().OpenIStream(m_path.c_str())->GetWholeCache();
	}
}

void FontIORequest::CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
{
	if(!m_cache)
	{
		THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
	}
	FontPtr res(new Font());
	res->CreateFontFromFileInCache(m_cache, m_height);
	m_res = res;
}

std::string FontIORequest::BuildKey(const char * path, int height)
{
	return str_printf("%s %d", path, height);
}
