#include "myResource.h"
#include "myDxutApp.h"
#include "libc.h"
#include <strstream>
#include <boost/bind/bind.hpp>
#include <zzip/file.h>
#include <fstream>
#include <SYS\Stat.h>
#include "myMesh.h"
#include "mySkeleton.h"
#include "myEffect.h"
#include "myFont.h"
#include "mySound.h"
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

int ZipIStream::read(void * buff, unsigned int read_size)
{
	CriticalSectionLock lock(m_DirSec);
	return zzip_file_read(m_fp, buff, read_size);
}

long ZipIStream::seek(long offset, int origin)
{
	CriticalSectionLock lock(m_DirSec);
	return zzip_seek(m_fp, offset, origin);
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

int FileIStream::read(void * buff, unsigned int read_size)
{
	return _read(m_fp, buff, read_size);
}

long FileIStream::seek(long offset, int origin)
{
	return _lseek(m_fp, offset, origin);
}

long FileIStream::tell(void)
{
	return _tell(m_fp);
}

unsigned long FileIStream::GetSize(void)
{
	return _filelength(m_fp);
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

std::string ZipIStreamDir::GetRelativePath(const char * path)
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

std::string FileIStreamDir::GetRelativePath(const char * path)
{
	char currentDir[MAX_PATH];
	::GetCurrentDirectoryA(_countof(currentDir), currentDir);
	::PathAppendA(currentDir, m_dir.c_str());
	char relativePath[MAX_PATH];
	if (::PathRelativePathToA(relativePath, currentDir, FILE_ATTRIBUTE_DIRECTORY, path, 0))
	{
		char canonicalizedPath[MAX_PATH];
		if (::PathCanonicalizeA(canonicalizedPath, relativePath) && CheckPath(canonicalizedPath))
		{
			return std::string(canonicalizedPath);
		}
	}

	return std::string();
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
	if (path[0] == '\0')
	{
		return std::string();
	}

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
	char currentDir[MAX_PATH];
	::GetCurrentDirectoryA(_countof(currentDir), currentDir);
	if(!m_DirList.empty())
	{
		::PathAppendA(currentDir, m_DirList.front()->m_dir.c_str());
	}
	::PathAppendA(currentDir, path);
	return std::string(currentDir);
}

std::string StreamDirMgr::GetRelativePath(const char * path)
{
	if (path[0] == '\0')
	{
		return std::string();
	}

	if (PathIsRelativeA(path))
	{
		char canonicalizedPath[MAX_PATH];
		if (::PathCanonicalizeA(canonicalizedPath, path) && CheckPath(canonicalizedPath))
		{
			return std::string(canonicalizedPath);
		}
		return std::string();
	}

	ResourceDirPtrList::iterator dir_iter = m_DirList.begin();
	for (; dir_iter != m_DirList.end(); dir_iter++)
	{
		std::string ret = (*dir_iter)->GetRelativePath(path);
		if (!ret.empty())
		{
			return ret;
		}
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

bool AsynchronousIOMgr::IsMainThread(void) const
{
	return GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId;
}

AsynchronousIOMgr::AsynchronousIOMgr(void)
	: m_bStopped(false)
{
}

void AsynchronousIOMgr::EnterDeviceSection(void)
{
	D3DContext::getSingleton().m_d3dDeviceSec.Enter();
}

void AsynchronousIOMgr::LeaveDeviceSection(void)
{
	D3DContext::getSingleton().m_d3dDeviceSec.Leave();
}

DWORD AsynchronousIOMgr::IORequestProc(void)
{
	m_IORequestListMutex.Wait(INFINITE);
	while(!m_bStopped)
	{
		int Priority = INT_MIN;
		IORequestPtr priority_req;
		IORequestPtrPairList::iterator req_iter = m_IORequestList.begin();
		for(; req_iter != m_IORequestList.end(); req_iter++)
		{
			if(!req_iter->second->m_PreLoadEvent.Wait(0) && req_iter->second->m_Priority > Priority)
			{
				_ASSERT(!req_iter->second->m_callbacks.empty());
				Priority = req_iter->second->m_Priority;
				priority_req = req_iter->second;
			}
		}

		if(priority_req)
		{
			// ! req_iter will be invalid after release mutex
			priority_req->m_PreLoadEvent.SetEvent();

			m_IORequestListMutex.Release();

			// ! HAVENT HANDLED EXCEPTION YET
			priority_req->LoadResource();

			// ! request list will be modified when set event, shared_ptr must be thread safe
			priority_req->m_PostLoadEvent.SetEvent();

			EnterDeviceSection();

			// ! discarded request may also destroy d3d object in no-main thread
			priority_req.reset();

			LeaveDeviceSection();

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
//
//void AsynchronousIOMgr::RemoveAllIORequest(void)
//{
//	_ASSERT(IsMainThread());
//
//	m_IORequestListMutex.Wait(INFINITE);
//
//	LeaveDeviceSection();
//
//	IORequestPtrPairList::iterator req_iter = m_IORequestList.begin();
//	for (; req_iter != m_IORequestList.end(); req_iter++)
//	{
//		if (req_iter->second->m_PreLoadEvent.Wait(0))
//		{
//			req_iter->second->m_PostLoadEvent.Wait(INFINITE);
//		}
//	}
//
//	EnterDeviceSection();
//
//	m_IORequestList.clear();
//
//	m_IORequestListMutex.Release();
//}

void AsynchronousIOMgr::StartIORequestProc(LONG lMaximumCount)
{
	m_bStopped = false;

	EnterDeviceSection();

	for (int i = 0; i < lMaximumCount; i++)
	{
		ThreadPtr thread(new Thread(boost::bind(&AsynchronousIOMgr::IORequestProc, this)));
		thread->CreateThread(0);
		m_Threads.push_back(thread);
	}
}

void AsynchronousIOMgr::StopIORequestProc(void)
{
	_ASSERT(IsMainThread());

	LeaveDeviceSection();

	m_IORequestListMutex.Wait(INFINITE);
	m_bStopped = true;
	m_IORequestListMutex.Release();
	m_IORequestListCondition.Wake(m_Threads.size());

	for (unsigned int i = 0; i < m_Threads.size(); i++)
	{
		m_Threads[i]->WaitForThreadStopped(INFINITE);
		m_Threads[i]->CloseThread();
	}
	m_Threads.clear();
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

	m_ResourceSet.clear();

	m_EffectPool.Release();

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
	_ASSERT(IsMainThread());

	DeviceResourceBasePtrSet::iterator res_iter = m_ResourceSet.find(key);
	if(res_iter != m_ResourceSet.end())
	{
		return res_iter->second;
	}
	return DeviceResourceBasePtr();
}

DeviceResourceBasePtr ResourceMgr::AddResource(const std::string & key, DeviceResourceBasePtr res)
{
	_ASSERT(IsMainThread());

	_ASSERT(!GetResource(key));

	std::pair<DeviceResourceBasePtrSet::iterator, bool> result = m_ResourceSet.insert(DeviceResourceBasePtrSet::value_type(key, res));

	_ASSERT(result.second);

	res->m_Key = result.first->first.c_str();

	std::string logs("Loaded: ");
	logs.append(res->m_Key);
	D3DContext::getSingleton().m_EventLog(logs.c_str());

	return result.first->second;
}

bool ResourceMgr::CheckIORequests(DWORD dwMilliseconds)
{
	_ASSERT(IsMainThread());

	IORequestPtrPairList::iterator req_iter = m_IORequestList.begin();
	for (; req_iter != m_IORequestList.end(); )
	{
		if (req_iter->second->m_PostLoadEvent.Wait(dwMilliseconds))
		{
			req_iter = OnIORequestIteratorReady(req_iter);
		}
		else
		{
			req_iter++;
		}
	}

	EnterDeviceSection();

	DeviceResourceBasePtrSet::iterator res_iter = m_ResourceSet.begin();
	for (; res_iter != m_ResourceSet.end(); )
	{
		if (res_iter->second.use_count() > 1)
		{
			res_iter++;
		}
		else
		{
			std::string logs("Release: ");
			logs.append(res_iter->second->m_Key);
			D3DContext::getSingleton().m_EventLog(logs.c_str());

			res_iter = m_ResourceSet.erase(res_iter);
		}
	}

	LeaveDeviceSection();

	return !m_IORequestList.empty();
}

ResourceMgr::IORequestPtrPairList::iterator ResourceMgr::OnIORequestIteratorReady(IORequestPtrPairList::iterator req_iter)
{
	_ASSERT(IsMainThread());

	IORequestPtrPairList::value_type pair = *req_iter;
	m_IORequestListMutex.Wait(INFINITE);
	IORequestPtrPairList::iterator ret = m_IORequestList.erase(req_iter);
	m_IORequestListMutex.Release();

	OnIORequestReady(pair.first, pair.second);

	return ret;
}

void ResourceMgr::OnIORequestReady(const std::string & key, IORequestPtr request)
{
	_ASSERT(request->m_PostLoadEvent.Wait(0));

	//if(!request->m_res)
	//{
		// ! havent handled exception
		try
		{
			_ASSERT(D3DContext::getSingleton().m_DeviceObjectsCreated);

			CriticalSectionLock lock(D3DContext::getSingleton().m_d3dDeviceSec);
			request->CreateResource(D3DContext::getSingleton().m_d3dDevice);
			lock.Unlock();

			if (request->m_res && D3DContext::getSingleton().m_DeviceObjectsReset)
			{

				CriticalSectionLock lock(D3DContext::getSingleton().m_d3dDeviceSec);
				request->m_res->OnResetDevice();
				lock.Unlock();
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
	//}

	OnIORequestCallback(request);
}

void ResourceMgr::OnIORequestCallback(IORequestPtr request)
{
	if (request->m_res)
	{
		IORequest::ResourceCallbackSet::iterator callback_iter = request->m_callbacks.begin();
		for (; callback_iter != request->m_callbacks.end(); callback_iter++)
		{
			_ASSERT(*callback_iter);

			(*callback_iter)(request->m_res);
		}
	}
}

class SimpleResourceCallback
{
public:
	DeviceResourceBasePtr m_res;

	void OnResourceReady(DeviceResourceBasePtr res)
	{
		m_res = res;
	}
};

boost::shared_ptr<BaseTexture> ResourceMgr::LoadTexture(const char * path)
{
	SimpleResourceCallback cb;
	IORequestPtr request(new TextureIORequest(path, INT_MAX));
	LoadIORequestAndWait(path, request, boost::bind(&SimpleResourceCallback::OnResourceReady, &cb, boost::placeholders::_1));
	return boost::dynamic_pointer_cast<BaseTexture>(cb.m_res);
}

boost::shared_ptr<OgreMesh> ResourceMgr::LoadMesh(const char * path, const char * sub_mesh_name)
{
	std::string key = MeshIORequest::BuildKey(path, sub_mesh_name);
	SimpleResourceCallback cb;
	IORequestPtr request(new MeshIORequest(path, sub_mesh_name, INT_MAX));
	LoadIORequestAndWait(key, request, boost::bind(&SimpleResourceCallback::OnResourceReady, &cb, boost::placeholders::_1));
	return boost::dynamic_pointer_cast<OgreMesh>(cb.m_res);
}

boost::shared_ptr<OgreSkeletonAnimation> ResourceMgr::LoadSkeleton(const char * path)
{
	SimpleResourceCallback cb;
	IORequestPtr request(new SkeletonIORequest(path, INT_MAX));
	LoadIORequestAndWait(path, request, boost::bind(&SimpleResourceCallback::OnResourceReady, &cb, boost::placeholders::_1));
	return boost::dynamic_pointer_cast<OgreSkeletonAnimation>(cb.m_res);
}

boost::shared_ptr<Effect> ResourceMgr::LoadEffect(const char * path, const char * macros)
{
	std::string key = EffectIORequest::BuildKey(path, macros);
	SimpleResourceCallback cb;
	IORequestPtr request(new EffectIORequest(path, macros, INT_MAX));
	LoadIORequestAndWait(key, request, boost::bind(&SimpleResourceCallback::OnResourceReady, &cb, boost::placeholders::_1));
	return boost::dynamic_pointer_cast<Effect>(cb.m_res);
}

boost::shared_ptr<Font> ResourceMgr::LoadFont(const char * path, int height, int face_index)
{
	std::string key = FontIORequest::BuildKey(path, height, face_index);
	SimpleResourceCallback cb;
	IORequestPtr request(new FontIORequest(path, height, face_index, INT_MAX));
	LoadIORequestAndWait(key, request, boost::bind(&SimpleResourceCallback::OnResourceReady, &cb, boost::placeholders::_1));
	return boost::dynamic_pointer_cast<Font>(cb.m_res);
}

boost::shared_ptr<Wav> ResourceMgr::LoadWav(const char * path)
{
	SimpleResourceCallback cb;
	IORequestPtr request(new WavIORequest(path, INT_MAX));
	LoadIORequestAndWait(path, request, boost::bind(&SimpleResourceCallback::OnResourceReady, &cb, boost::placeholders::_1));
	return boost::dynamic_pointer_cast<Wav>(cb.m_res);
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

MeshIORequest::MeshIORequest(const char * path, const char * sub_mesh_name, int Priority)
	: IORequest(Priority)
	, m_path(path)
	, m_sub_mesh_name(sub_mesh_name)
{
}

void MeshIORequest::LoadResource(void)
{
	if(ResourceMgr::getSingleton().CheckPath(m_path.c_str()))
	{
		CachePtr cache = ResourceMgr::getSingleton().OpenIStream(m_path.c_str())->GetWholeCache();
		cache->push_back(0);
		try
		{
			rapidxml::xml_document<char> doc;
			doc.parse<0>((char *)&(*cache)[0]);

			OgreMeshPtr res(new OgreMesh());
			res->CreateMeshFromOgreXml(&doc, m_sub_mesh_name);
			m_res = res;
		}
		catch(rapidxml::parse_error &)
		{
		}
	}
}

void MeshIORequest::CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
{
	if(!m_res)
	{
		THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
	}
}

std::string MeshIORequest::BuildKey(const char * path, const char * sub_mesh_name)
{
	return str_printf("%s %s", path, sub_mesh_name);
}

void SkeletonIORequest::LoadResource(void)
{
	if(ResourceMgr::getSingleton().CheckPath(m_path.c_str()))
	{
		CachePtr cache = ResourceMgr::getSingleton().OpenIStream(m_path.c_str())->GetWholeCache();
		cache->push_back(0);
		try
		{
			rapidxml::xml_document<char> doc;
			doc.parse<0>((char *)&(*cache)[0]);

			OgreSkeletonAnimationPtr res(new OgreSkeletonAnimation());
			res->CreateOgreSkeletonAnimation(&doc);
			m_res = res;
		}
		catch(rapidxml::parse_error &)
		{
		}
	}
}

void SkeletonIORequest::CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
{
	if(!m_res)
	{
		THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
	}
}

EffectIORequest::EffectIORequest(const char * path, std::string macros, int Priority)
	: IORequest(Priority)
	, m_path(path)
{
	boost::trim_if(macros, boost::algorithm::is_any_of(" \t,"));
	if (!macros.empty())
	{
		boost::algorithm::split(m_macros, macros, boost::algorithm::is_any_of(" \t,"), boost::algorithm::token_compress_on);
	}

	std::vector<std::string>::const_iterator macro_iter = m_macros.begin();
	for(; macro_iter != m_macros.end(); macro_iter++)
	{
		D3DXMACRO d3dmacro;
		d3dmacro.Name = macro_iter->c_str();
		d3dmacro.Definition = NULL;
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
	res->CreateFontFromFileInCache(m_cache, m_height, m_face_index);
	m_res = res;
}

std::string FontIORequest::BuildKey(const char * path, int height, int face_index)
{
	return str_printf("%s %d %d", path, height, face_index);
}

void WavIORequest::LoadResource(void)
{
	if (ResourceMgr::getSingleton().CheckPath(m_path.c_str()))
	{
		WavPtr res(new Wav());
		res->CreateWavFromFileInStream(ResourceMgr::getSingleton().OpenIStream(m_path.c_str()));
		m_res = res;
	}
}

void WavIORequest::CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
{
	if (!m_res)
	{
		THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
	}
}
