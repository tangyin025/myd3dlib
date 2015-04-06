#include "stdafx.h"
#include "myResource.h"
#include "myDxutApp.h"
#include "libc.h"
#include <strstream>
#include <boost/bind.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <zzip/file.h>
#include <fstream>
#include <SYS\Stat.h>
#include "myMesh.h"
#include "mySkeleton.h"
#include "myEffect.h"
#include "myFont.h"
#include "myEmitter.h"

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

ZipIStream::ZipIStream(ZZIP_FILE * fp)
	: m_fp(fp)
{
	_ASSERT(NULL != m_fp);
}

ZipIStream::~ZipIStream(void)
{
	zzip_file_close(m_fp);
}

int ZipIStream::read(void * buff, unsigned read_size)
{
	return zzip_file_read(m_fp, buff, read_size);
}

long ZipIStream::seek(long offset)
{
	return zzip_seek(m_fp, offset, SEEK_SET);
}

long ZipIStream::tell(void)
{
	return zzip_tell(m_fp);
}

unsigned long ZipIStream::GetSize(void)
{
	return m_fp->usize;
}

FileIStream::FileIStream(FILE * fp)
	: m_fp(fp)
{
	_ASSERT(NULL != m_fp);
}

FileIStream::~FileIStream(void)
{
	fclose(m_fp);
}

IStreamPtr FileIStream::Open(LPCTSTR pFilename)
{
	FILE * fp;
	if(0 != _tfopen_s(&fp, pFilename, _T("rb")))
	{
		THROW_CUSEXCEPTION(str_printf("cannot open file archive: %s", pFilename));
	}
	return IStreamPtr(new FileIStream(fp));
}

int FileIStream::read(void * buff, unsigned read_size)
{
	return ::fread(buff, 1, read_size, m_fp);
}

long FileIStream::seek(long offset)
{
	return ::fseek(m_fp, offset, SEEK_SET);
}

long FileIStream::tell(void)
{
	return ::ftell(m_fp);
}

unsigned long FileIStream::GetSize(void)
{
	long pos = ::ftell(m_fp);
	::fseek(m_fp, 0, SEEK_END);
	long len = ::ftell(m_fp);
	::fseek(m_fp, pos, SEEK_SET);
	return len;
}

FileOStream::FileOStream(FILE * fp)
	: m_fp(fp)
{
	_ASSERT(NULL != m_fp);
}

FileOStream::~FileOStream(void)
{
	fclose(m_fp);
}

OStreamPtr FileOStream::Open(LPCTSTR pFilename)
{
	FILE * fp;
	if(0 != _tfopen_s(&fp, pFilename, _T("wb")))
	{
		THROW_CUSEXCEPTION(str_printf("cannot open file archive: %s", pFilename));
	}
	return OStreamPtr(new FileOStream(fp));
}

int FileOStream::write(const void * buff, unsigned write_size)
{
	return fwrite(buff, 1, write_size, m_fp);
}

MemoryIStream::MemoryIStream(void * buffer, size_t size)
	: m_buffer((unsigned char *)buffer)
	, m_size(size)
	, m_tell(0)
{
}

int MemoryIStream::read(void * buff, unsigned read_size)
{
	int copy_size = Min<int>(read_size, m_size - m_tell);
	if (copy_size > 0)
	{
		memcpy(buff, m_buffer + m_tell, copy_size);
		m_tell += copy_size;
	}
	return copy_size;
}

long MemoryIStream::seek(long offset)
{
	m_tell = Min<long>(m_size, offset);
	return 0;
}

long MemoryIStream::tell(void)
{
	return m_tell;
}

unsigned long MemoryIStream::GetSize(void)
{
	return m_size;
}

MemoryOStream::MemoryOStream(void)
	: m_cache(new Cache)
{
	_ASSERT(m_cache->size() == 0);
}

int MemoryOStream::write(const void * buff, unsigned write_size)
{
	if (write_size > 0)
	{
		size_t prev_size = m_cache->size();
		m_cache->resize(prev_size + write_size);
		memcpy(&(*m_cache)[prev_size], buff, write_size);
	}
	return write_size;
}

ZipIStreamDir::ZipIStreamDir(const std::string & dir)
	: StreamDir(dir)
	, m_UsePassword(false)
{
	int fd;
	_sopen_s(&fd, m_dir.c_str(), O_RDONLY|O_BINARY, _SH_DENYWR, _S_IREAD);
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

std::string ZipIStreamDir::ReplaceSlash(const std::string & path)
{
	size_t pos = 0;
	std::string ret = path;
	while(std::string::npos != (pos = ret.find('/', pos)))
	{
		ret.replace(pos++, 1, 1, '\\');
	}
	return ret;
}

std::string ZipIStreamDir::ReplaceBackslash(const std::string & path)
{
	size_t pos = 0;
	std::string ret = path;
	while(std::string::npos != (pos = ret.find('\\', pos)))
	{
		ret.replace(pos++, 1, 1, '/');
	}
	return ret;
}

bool ZipIStreamDir::CheckPath(const std::string & path)
{
	ZZIP_FILE * zfile = zzip_file_open(m_zipdir, ReplaceBackslash(path).c_str(), ZZIP_CASEINSENSITIVE);
	if(NULL == zfile)
	{
		return false;
	}

	zzip_file_close(zfile);
	return true;
}

std::string ZipIStreamDir::GetFullPath(const std::string & path)
{
	return std::string();
}

IStreamPtr ZipIStreamDir::OpenIStream(const std::string & path)
{
	ZZIP_FILE * zfile = zzip_file_open(m_zipdir, ReplaceBackslash(path).c_str(), ZZIP_CASEINSENSITIVE);
	if(NULL == zfile)
	{
		THROW_CUSEXCEPTION(str_printf("cannot open zip file: %s", path.c_str()));
	}

	return IStreamPtr(new ZipIStream(zfile));
}

bool FileIStreamDir::CheckPath(const std::string & path)
{
	return !GetFullPath(path).empty();
}

std::string FileIStreamDir::GetFullPath(const std::string & path)
{
	std::string fullPath;
	char * lpFilePath;
	DWORD dwLen = MAX_PATH;
	do
	{
		fullPath.resize(dwLen);
		dwLen = SearchPathA(m_dir.c_str(), path.c_str(), NULL, fullPath.size(), &fullPath[0], &lpFilePath);
	}
	while(dwLen > fullPath.size());

	fullPath.resize(dwLen);
	return fullPath;
}

IStreamPtr FileIStreamDir::OpenIStream(const std::string & path)
{
	std::string fullPath = GetFullPath(path);
	if(fullPath.empty())
	{
		THROW_CUSEXCEPTION(str_printf("cannot open file archive: %s", path.c_str()));
	}

	return FileIStream::Open(ms2ts(fullPath).c_str());
}

void StreamDirMgr::RegisterZipDir(const std::string & zip_path)
{
	CriticalSectionLock lock(m_DirListSection);
	m_DirList.push_back(ResourceDirPtr(new ZipIStreamDir(zip_path)));
}
//
//void StreamDirMgr::RegisterZipDir(const std::string & zip_path, const std::string & password)
//{
//	CriticalSectionLock lock(m_DirListSection);
//	m_DirList.push_back(ResourceDirPtr(new ZipIStreamDir(zip_path, password)));
//}

void StreamDirMgr::RegisterFileDir(const std::string & dir)
{
	CriticalSectionLock lock(m_DirListSection);
	m_DirList.push_back(ResourceDirPtr(new FileIStreamDir(dir)));
}

bool StreamDirMgr::CheckPath(const std::string & path)
{
	if(!PathIsRelativeA(path.c_str()))
	{
		return PathFileExistsA(path.c_str());
	}

	CriticalSectionLock lock(m_DirListSection);
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

std::string StreamDirMgr::GetFullPath(const std::string & path)
{
	if(!PathIsRelativeA(path.c_str()))
	{
		return path;
	}

	CriticalSectionLock lock(m_DirListSection);
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
		std::string ret;
		ret.resize(MAX_PATH);
		PathCombineA(&ret[0], m_DirList.front()->m_dir.c_str(), path.c_str());
		ret = ZipIStreamDir::ReplaceSlash(ret);
		std::string full_path;
		full_path.resize(MAX_PATH);
		GetFullPathNameA(ret.c_str(), full_path.size(), &full_path[0], NULL);
		return full_path;
	}

	return std::string();
}

IStreamPtr StreamDirMgr::OpenIStream(const std::string & path)
{
	if(!PathIsRelativeA(path.c_str()))
	{
		return FileIStream::Open(ms2ts(path).c_str());
	}

	CriticalSectionLock lock(m_DirListSection);
	ResourceDirPtrList::iterator dir_iter = m_DirList.begin();
	for(; dir_iter != m_DirList.end(); dir_iter++)
	{
		if((*dir_iter)->CheckPath(path))
		{
			return (*dir_iter)->OpenIStream(path);
		}
	}

	THROW_CUSEXCEPTION(str_printf("cannot find specified file: %s", path.c_str()));
}

AsynchronousIOMgr::AsynchronousIOMgr(void)
	: m_bStopped(false)
	, m_Thread(boost::bind(&AsynchronousIOMgr::IORequestProc, this))
{
}

DWORD AsynchronousIOMgr::IORequestProc(void)
{
	m_IORequestListMutex.Wait();
	while(!m_bStopped)
	{
		IORequestPtrPairList::iterator req_iter = m_IORequestList.begin();
		for(; req_iter != m_IORequestList.end(); req_iter++)
		{
			if(!req_iter->second->m_LoadEvent.Wait(0))
			{
				break;
			}
		}
		if(req_iter != m_IORequestList.end())
		{
			// ! req_iter will be invalid after release mutex
			IORequestPtr request = req_iter->second;

			m_IORequestListMutex.Release();

			// ! HAVENT HANDLED EXCEPTION YET
			request->DoLoad();

			// ! request will be decrease after set event, shared_ptr must be thread safe
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

AsynchronousIOMgr::IORequestPtrPairList::iterator AsynchronousIOMgr::PushIORequestResource(const std::string & key, my::IORequestPtr request, bool abort)
{
	m_IORequestListMutex.Wait(INFINITE);

	IORequestPtrPairList::iterator req_iter = m_IORequestList.begin();
	for(; req_iter != m_IORequestList.end(); req_iter++)
	{
		if(req_iter->first == key)
		{
			break;
		}
	}

	if(req_iter != m_IORequestList.end())
	{
		if (!abort)
		{
			req_iter->second->m_callbacks.insert(
				req_iter->second->m_callbacks.end(), request->m_callbacks.begin(), request->m_callbacks.end());
		}
		m_IORequestListMutex.Release();
		return req_iter;
	}

	m_IORequestList.push_back(std::make_pair(key, request));
	m_IORequestListMutex.Release();
	m_IORequestListCondition.Wake(1);
	return --m_IORequestList.end();
}

void AsynchronousIOMgr::StartIORequestProc(void)
{
	m_bStopped = false;
	m_Thread.CreateThread();
	m_Thread.ResumeThread();
}

void AsynchronousIOMgr::StopIORequestProc(void)
{
	m_IORequestListMutex.Wait(INFINITE);
	m_bStopped = true;
	m_IORequestListMutex.Release();
	m_IORequestListCondition.Wake(1);
}

HRESULT DeviceRelatedResourceMgr::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	return S_OK;
}

HRESULT DeviceRelatedResourceMgr::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	DeviceRelatedObjectBaseWeakPtrSet::iterator res_iter = m_ResourceWeakSet.begin();
	for(; res_iter != m_ResourceWeakSet.end();)
	{
		DeviceRelatedObjectBasePtr res = res_iter->second.lock();
		if(res)
		{
			res->OnResetDevice();
			res_iter++;
		}
		else
		{
			res_iter = m_ResourceWeakSet.erase(res_iter);
		}
	}
	return S_OK;
}

void DeviceRelatedResourceMgr::OnLostDevice(void)
{
	DeviceRelatedObjectBaseWeakPtrSet::iterator res_iter = m_ResourceWeakSet.begin();
	for(; res_iter != m_ResourceWeakSet.end();)
	{
		DeviceRelatedObjectBasePtr res = res_iter->second.lock();
		if(res)
		{
			res->OnLostDevice();
			res_iter++;
		}
		else
		{
			res_iter = m_ResourceWeakSet.erase(res_iter);
		}
	}
}

void DeviceRelatedResourceMgr::OnDestroyDevice(void)
{
	DeviceRelatedObjectBaseWeakPtrSet::iterator res_iter = m_ResourceWeakSet.begin();
	for(; res_iter != m_ResourceWeakSet.end(); res_iter++)
	{
		DeviceRelatedObjectBasePtr res = res_iter->second.lock();
		if(res)
		{
			res->OnDestroyDevice();
		}
	}
	m_ResourceWeakSet.clear();
}

void DeviceRelatedResourceMgr::AddResource(const std::string & key, DeviceRelatedObjectBasePtr res)
{
	_ASSERT(m_ResourceWeakSet.end() == m_ResourceWeakSet.find(key));

	m_ResourceWeakSet[key] = res;
}

std::string DeviceRelatedResourceMgr::GetResourceKey(DeviceRelatedObjectBasePtr res) const
{
	DeviceRelatedObjectBaseWeakPtrSet::const_iterator res_iter = m_ResourceWeakSet.begin();
	for(; res_iter != m_ResourceWeakSet.end(); res_iter++)
	{
		if(res == res_iter->second.lock())
		{
			return res_iter->first;
		}
	}
	return std::string();
}

HRESULT ResourceMgr::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	HRESULT hr;
	if(FAILED(hr = DeviceRelatedResourceMgr::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

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
	HRESULT hr;
	if(FAILED(hr = DeviceRelatedResourceMgr::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}
	return S_OK;
}

void ResourceMgr::OnLostDevice(void)
{
	DeviceRelatedResourceMgr::OnLostDevice();
}

void ResourceMgr::OnDestroyDevice(void)
{
	StopIORequestProc();

	DeviceRelatedResourceMgr::OnDestroyDevice();

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
	std::string path;
	path.resize(MAX_PATH);
	PathCombineA(&path[0], m_EffectInclude.c_str(), pFileName);
	switch(IncludeType)
	{
	case D3DXINC_SYSTEM:
	case D3DXINC_LOCAL:
		if(CheckPath(path))
		{
			cache = OpenIStream(path)->GetWholeCache();
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

AsynchronousIOMgr::IORequestPtrPairList::iterator ResourceMgr::LoadResourceAsync(const std::string & key, IORequestPtr request, bool abort)
{
	DeviceRelatedObjectBaseWeakPtrSet::iterator res_iter = m_ResourceWeakSet.find(key);
	if(res_iter != m_ResourceWeakSet.end())
	{
		DeviceRelatedObjectBasePtr res = res_iter->second.lock();
		if(res)
		{
			request->m_res = res;

			request->m_LoadEvent.SetEvent();

			CheckResource(key, request, 0);

			return m_IORequestList.end();
		}
		else
			m_ResourceWeakSet.erase(res_iter);
	}

	return PushIORequestResource(key, request, abort);
}

bool ResourceMgr::CheckRequests(void)
{
	MutexLock lock(m_IORequestListMutex);
	IORequestPtrPairList::iterator req_iter = m_IORequestList.begin();
	for(; req_iter != m_IORequestList.end(); )
	{
		if(CheckResource(req_iter->first, req_iter->second, 0))
		{
			req_iter = m_IORequestList.erase(req_iter);
		}
		else
		{
			break;
		}
	}
	return !m_IORequestList.empty();
}

bool ResourceMgr::CheckResource(const std::string & key, IORequestPtr request, DWORD timeout)
{
	if(request->m_LoadEvent.Wait(timeout))
	{
		if(!request->m_res)
		{
			// ! havent handled exception
			try
			{
				_ASSERT(D3DContext::getSingleton().m_DeviceObjectsCreated);

				request->BuildResource(D3DContext::getSingleton().m_d3dDevice);

				if (D3DContext::getSingleton().m_DeviceObjectsReset)
				{
					request->m_res->OnResetDevice();
				}

				AddResource(key, request->m_res);
			}
			catch(const Exception & e)
			{
				OnResourceFailed(e.what());
			}
			catch(const std::exception & e)
			{
				OnResourceFailed(str_printf("%s error: %s", key.c_str(), e.what()));
			}
		}

		if (request->m_res)
		{
			IORequest::ResourceCallbackList::iterator callback_iter = request->m_callbacks.begin();
			for(; callback_iter != request->m_callbacks.end(); callback_iter++)
			{
				if(*callback_iter)
					(*callback_iter)(request->m_res);
			}
		}
		return true;
	}
	return false;
}

void ResourceMgr::OnResourceFailed(const std::string & error_str)
{
}

class TextureIORequest : public IORequest
{
protected:
	std::string m_path;

	StreamDirMgr * m_arc;

	CachePtr m_cache;

public:
	TextureIORequest(const ResourceCallback & callback, const std::string & path, StreamDirMgr * arc)
		: m_path(path)
		, m_arc(arc)
	{
		if(callback)
		{
			m_callbacks.push_back(callback);
		}
	}

	virtual void DoLoad(void)
	{
		if(m_arc->CheckPath(m_path))
		{
			m_cache = m_arc->OpenIStream(m_path)->GetWholeCache();
		}
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
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
				res->CreateTextureFromFileInMemory(pd3dDevice, &(*m_cache)[0], m_cache->size());
				m_res = res;
			}
			break;
		case D3DRTYPE_CUBETEXTURE:
			{
				CubeTexturePtr res(new CubeTexture());
				res->CreateCubeTextureFromFileInMemory(pd3dDevice, &(*m_cache)[0], m_cache->size());
				m_res = res;
			}
			break;
		default:
			THROW_CUSEXCEPTION(str_printf("unsupported d3d texture format %u", imif.ResourceType));
		}
	}
};

void ResourceMgr::LoadTextureAsync(const std::string & path, const ResourceCallback & callback)
{
	LoadResourceAsync(path, IORequestPtr(new TextureIORequest(callback, path, this)), false);
}

boost::shared_ptr<BaseTexture> ResourceMgr::LoadTexture(const std::string & path)
{
	return LoadResource<BaseTexture>(path, IORequestPtr(new TextureIORequest(ResourceCallback(), path, this)));
}

class MeshIORequest : public IORequest
{
protected:
	std::string m_path;

	StreamDirMgr * m_arc;

	CachePtr m_cache;

	rapidxml::xml_document<char> m_doc;

public:
	MeshIORequest(const ResourceCallback & callback, const std::string & path, StreamDirMgr * arc)
		: m_path(path)
		, m_arc(arc)
	{
		if(callback)
		{
			m_callbacks.push_back(callback);
		}
	}

	virtual void DoLoad(void)
	{
		if(m_arc->CheckPath(m_path))
		{
			m_cache = m_arc->OpenIStream(m_path)->GetWholeCache();
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

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(!m_doc.first_node())
		{
			THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
		}
		OgreMeshPtr res(new OgreMesh());
		res->CreateMeshFromOgreXml(pd3dDevice, &m_doc);
		m_res = res;
	}
};

void ResourceMgr::LoadMeshAsync(const std::string & path, const ResourceCallback & callback)
{
	LoadResourceAsync(path, IORequestPtr(new MeshIORequest(callback, path, this)), false);
}

boost::shared_ptr<OgreMesh> ResourceMgr::LoadMesh(const std::string & path)
{
	return LoadResource<OgreMesh>(path, IORequestPtr(new MeshIORequest(ResourceCallback(), path, this)));
}

class MeshSetIORequest : public IORequest
{
protected:
	std::string m_path;

	StreamDirMgr * m_arc;

	CachePtr m_cache;

	rapidxml::xml_document<char> m_doc;

public:
	MeshSetIORequest(const ResourceCallback & callback, const std::string & path, StreamDirMgr * arc)
		: m_path(path)
		, m_arc(arc)
	{
		if(callback)
		{
			m_callbacks.push_back(callback);
		}
	}

	virtual void DoLoad(void)
	{
		if(m_arc->CheckPath(m_path))
		{
			m_cache = m_arc->OpenIStream(m_path)->GetWholeCache();
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

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(!m_doc.first_node())
		{
			THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
		}
		OgreMeshSetPtr res(new OgreMeshSet());
		res->CreateMeshSetFromOgreXml(pd3dDevice, &m_doc);
		m_res = res;
	}
};

void ResourceMgr::LoadMeshSetAsync(const std::string & path, const ResourceCallback & callback)
{
	LoadResourceAsync(path, IORequestPtr(new MeshSetIORequest(callback, path, this)), false);
}

boost::shared_ptr<OgreMeshSet> ResourceMgr::LoadMeshSet(const std::string & path)
{
	return LoadResource<OgreMeshSet>(path, IORequestPtr(new MeshSetIORequest(ResourceCallback(), path, this)));
}

class SkeletonIORequest : public IORequest
{
protected:
	std::string m_path;

	StreamDirMgr * m_arc;

	CachePtr m_cache;

	rapidxml::xml_document<char> m_doc;

public:
	SkeletonIORequest(const ResourceCallback & callback, const std::string & path, StreamDirMgr * arc)
		: m_path(path)
		, m_arc(arc)
	{
		if(callback)
		{
			m_callbacks.push_back(callback);
		}
	}

	virtual void DoLoad(void)
	{
		if(m_arc->CheckPath(m_path))
		{
			m_cache = m_arc->OpenIStream(m_path)->GetWholeCache();
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

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(!m_doc.first_node())
		{
			THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
		}
		OgreSkeletonAnimationPtr res(new OgreSkeletonAnimation());
		res->CreateOgreSkeletonAnimation(&m_doc);
		m_res = res;
	}
};

void ResourceMgr::LoadSkeletonAsync(const std::string & path, const ResourceCallback & callback)
{
	LoadResourceAsync(path, IORequestPtr(new SkeletonIORequest(callback, path, this)), false);
}

boost::shared_ptr<OgreSkeletonAnimation> ResourceMgr::LoadSkeleton(const std::string & path)
{
	return LoadResource<OgreSkeletonAnimation>(path, IORequestPtr(new SkeletonIORequest(ResourceCallback(), path, this)));
}

ResourceMgr::EffectIORequest::EffectIORequest(const ResourceCallback & callback, const std::string & path, std::string macros, ResourceMgr * arc)
	: m_path(path)
	, m_arc(arc)
{
	boost::regex_split(std::back_inserter(m_macros), macros);

	std::list<std::string>::const_iterator macro_iter = m_macros.begin();
	for(; macro_iter != m_macros.end(); )
	{
		D3DXMACRO d3dmacro;
		d3dmacro.Name = (macro_iter++)->c_str();
		d3dmacro.Definition = macro_iter != m_macros.end() ? (macro_iter++)->c_str() : NULL;
		m_d3dmacros.push_back(d3dmacro);
	}
	D3DXMACRO end = {0};
	m_d3dmacros.push_back(end);

	if(callback)
	{
		m_callbacks.push_back(callback);
	}
}

void ResourceMgr::EffectIORequest::DoLoad(void)
{
	if(m_arc->CheckPath(m_path))
	{
		m_cache = m_arc->OpenIStream(m_path)->GetWholeCache();
	}
}

void ResourceMgr::EffectIORequest::BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
{
	if(!m_cache)
	{
		THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
	}
	EffectPtr res(new Effect());
	m_arc->m_EffectInclude = ZipIStreamDir::ReplaceSlash(m_path);
	PathRemoveFileSpecA(&m_arc->m_EffectInclude[0]);
	res->CreateEffect(pd3dDevice, &(*m_cache)[0], m_cache->size(), &m_d3dmacros[0], m_arc, 0, m_arc->m_EffectPool);
	m_res = res;
}

std::string ResourceMgr::EffectIORequest::BuildKey(const std::string & path, const std::string & macros)
{
	return str_printf("%s, %s", path.c_str(), macros.c_str());
}

void ResourceMgr::LoadEffectAsync(const std::string & path, const std::string & macros, const ResourceCallback & callback)
{
	std::string key = EffectIORequest::BuildKey(path, macros);

	LoadResourceAsync(key, IORequestPtr(new EffectIORequest(callback, path, macros, this)), false);
}

boost::shared_ptr<Effect> ResourceMgr::LoadEffect(const std::string & path, const std::string & macros)
{
	std::string key = EffectIORequest::BuildKey(path, macros);

	return LoadResource<Effect>(key, IORequestPtr(new EffectIORequest(ResourceCallback(), path, macros, this)));
}

class FontIORequest : public IORequest
{
protected:
	std::string m_path;

	int m_height;

	StreamDirMgr * m_arc;

	CachePtr m_cache;

public:
	FontIORequest(const ResourceCallback & callback, const std::string & path, int height, StreamDirMgr * arc)
		: m_path(path)
		, m_height(height)
		, m_arc(arc)
	{
		if(callback)
		{
			m_callbacks.push_back(callback);
		}
	}

	virtual void DoLoad(void)
	{
		if(m_arc->CheckPath(m_path))
		{
			m_cache = m_arc->OpenIStream(m_path)->GetWholeCache();
		}
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(!m_cache)
		{
			THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
		}
		FontPtr res(new Font());
		res->CreateFontFromFileInCache(pd3dDevice, m_cache, m_height);
		m_res = res;
	}

	static std::string BuildKey(const std::string & path, int height)
	{
		return str_printf("%s, %d", path.c_str(), height);
	}
};

void ResourceMgr::LoadFontAsync(const std::string & path, int height, const ResourceCallback & callback)
{
	std::string key = FontIORequest::BuildKey(path, height);

	LoadResourceAsync(key, IORequestPtr(new FontIORequest(callback, path, height, this)), false);
}

boost::shared_ptr<Font> ResourceMgr::LoadFont(const std::string & path, int height)
{
	std::string key = FontIORequest::BuildKey(path, height);

	return LoadResource<Font>(key, IORequestPtr(new FontIORequest(ResourceCallback(), path, height, this)));
}

class EmitterIORequest : public IORequest
{
public:
	std::string m_path;

	ResourceMgr * m_arc;

	CachePtr m_cache;

public:
	EmitterIORequest(const ResourceCallback & callback, const std::string & path, ResourceMgr * arc)
		: m_path(path)
		, m_arc(arc)
	{
		if(callback)
		{
			m_callbacks.push_back(callback);
		}
	}

	virtual void DoLoad(void)
	{
		if(m_arc->CheckPath(m_path))
		{
			m_cache = m_arc->OpenIStream(m_path)->GetWholeCache();
			try
			{
				EmitterPtr res;
				membuf mb((char *)&(*m_cache)[0], m_cache->size());
				std::istream ims(&mb);
				boost::archive::xml_iarchive ia(ims);
				ia >> boost::serialization::make_nvp("Emitter", res);
				m_res = res;
			}
			catch (...)
			{
			}
		}
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(!m_res)
		{
			THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
		}
	}
};

void ResourceMgr::LoadEmitterAsync(const std::string & path, const ResourceCallback & callback)
{
	LoadResourceAsync(path, IORequestPtr(new EmitterIORequest(callback, path, this)), false);
}

boost::shared_ptr<Emitter> ResourceMgr::LoadEmitter(const std::string & path)
{
	return LoadResource<Emitter>(path, IORequestPtr(new EmitterIORequest(ResourceCallback(), path, this)));
}

void ResourceMgr::SaveEmitter(const std::string & path, boost::shared_ptr<Emitter> emitter)
{
	std::ofstream ofs(GetFullPath(path).c_str());
	boost::archive::xml_oarchive oa(ofs);
	oa << boost::serialization::make_nvp("Emitter", emitter);
}

void ResourceMgr::SaveMesh(const std::string & path, boost::shared_ptr<OgreMesh> mesh)
{
	std::ofstream ofs(GetFullPath(path).c_str());
	mesh->SaveMesh(ofs);
}

void ResourceMgr::SaveSimplyMesh(const std::string & path, boost::shared_ptr<OgreMesh> mesh, DWORD MinFaces)
{
	OgreMeshPtr mesh_sim(new OgreMesh());
	mesh_sim->Create(mesh->SimplifyMesh(&mesh->m_Adjacency[0], MinFaces, D3DXMESHSIMP_FACE).Detach());
	mesh_sim->m_aabb = mesh->m_aabb;
	mesh_sim->m_Adjacency = mesh->m_Adjacency;
	mesh_sim->m_MaterialNameList = mesh->m_MaterialNameList;
	mesh_sim->m_VertexElems = mesh->m_VertexElems;
	SaveMesh(path, mesh_sim);
}
