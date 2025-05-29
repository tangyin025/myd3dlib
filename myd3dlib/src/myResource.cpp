// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "myResource.h"
#include "myDxutApp.h"
#include "libc.h"
#include <strstream>
#include <boost/bind/bind.hpp>
#include <fcntl.h>
#include <io.h>
#include <fstream>
#include <SYS\Stat.h>
#include "zip.h"
#include "zipint.h"
#include "myMesh.h"
#include "mySkeleton.h"
#include "myEffect.h"
#include "myFont.h"
#include "mySound.h"
#include "rapidxml.hpp"
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/pool/pool.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

using namespace my;

ZipIStream::ZipIStream(boost::shared_ptr<zip_stat_t> stat, zip_file_t * zipf, CriticalSection & DirSec)
	: m_stat(stat)
	, m_zipf(zipf)
	, m_DirSec(DirSec)
{
	_ASSERT(NULL != m_zipf);
}

ZipIStream::~ZipIStream(void)
{
	CriticalSectionLock lock(m_DirSec);
	BOOST_VERIFY(0 == zip_fclose(m_zipf));
}

int ZipIStream::read(void * buff, unsigned int read_size)
{
	CriticalSectionLock lock(m_DirSec);
	return zip_fread(m_zipf, buff, read_size);
}

long ZipIStream::seek(long offset, int origin)
{
	CriticalSectionLock lock(m_DirSec);
	if (zip_file_is_seekable(m_zipf))
	{
		if (0 != zip_fseek(m_zipf, offset, origin))
		{
			return -1;
		}
	}
	else
	{
		zip_int64_t srcpos = zip_ftell(m_zipf);
		zip_int64_t dstpos;
		switch (origin)
		{
		default:
		case SEEK_SET:
			dstpos = offset;
			break;
		case SEEK_CUR:
			dstpos = srcpos + offset;
			break;
		case SEEK_END:
			dstpos = m_stat->size + offset;
			break;
		}

		zip_int64_t len = dstpos - srcpos;
		if (len < 0)
		{
			zip_file_t* zipf = zip_fopen_index(m_zipf->za, m_stat->index, ZIP_FL_ENC_GUESS | ZIP_FL_ENC_RAW);
			if (NULL == zipf)
			{
				return -1;
			}
			zip_fclose(m_zipf);
			m_zipf = zipf;
			len = dstpos;
		}

		std::vector<unsigned char> buff(len);
		if (len != zip_fread(m_zipf, buff.data(), len))
		{
			return -1;
		}
	}
	return zip_ftell(m_zipf);
}

long ZipIStream::tell(void)
{
	CriticalSectionLock lock(m_DirSec);
	return zip_ftell(m_zipf);
}

size_t ZipIStream::GetSize(void)
{
	return m_stat->size;
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
	errno_t err = _tsopen_s(&fp, pFilename, _O_RDONLY | _O_BINARY, _SH_DENYNO, 0);
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

size_t FileIStream::GetSize(void)
{
	return _filelength(m_fp);
}

ZipIStreamDir::ZipIStreamDir(const std::string & dir)
	: StreamDir(dir)
{
	int error;
	m_archive = zip_open(dir.c_str(), ZIP_RDONLY, &error);
	if (!m_archive)
	{
		THROW_CUSEXCEPTION(str_printf("cannot open zip archive: %s", m_dir.c_str()));
	}
}

ZipIStreamDir::~ZipIStreamDir(void)
{
	zip_close(m_archive);
}

bool ZipIStreamDir::CheckPath(const char * path)
{
	CriticalSectionLock lock(m_DirSec);

	_ASSERT(!strchr(path, '\\'));

	zip_int64_t idx = zip_name_locate(m_archive, path, ZIP_FL_ENC_GUESS | ZIP_FL_ENC_RAW);
	if (idx < 0)
	{
		return false;
	}
	return true;
}

std::basic_string<TCHAR> ZipIStreamDir::GetFullPath(const char * path)
{
	return std::basic_string<TCHAR>();
}

std::string ZipIStreamDir::GetRelativePath(const TCHAR * path)
{
	return std::string();
}

IStreamPtr ZipIStreamDir::OpenIStream(const char * path)
{
	CriticalSectionLock lock(m_DirSec);

	_ASSERT(!strchr(path, '\\'));

	boost::shared_ptr<zip_stat_t> stat(new zip_stat_t);
	if (0 != zip_stat(m_archive, path, ZIP_FL_ENC_GUESS | ZIP_FL_ENC_RAW, stat.get()))
	{
		THROW_CUSEXCEPTION(str_printf("cannot open zip file: %s", path));
	}

	zip_file_t* zipf = zip_fopen_index(m_archive, stat->index, ZIP_FL_ENC_GUESS | ZIP_FL_ENC_RAW);
	if (NULL == zipf)
	{
		THROW_CUSEXCEPTION(str_printf("cannot open zip file: %s", path));
	}

	return IStreamPtr(new ZipIStream(stat, zipf, m_DirSec));
}

bool FileIStreamDir::CheckPath(const char * path)
{
	std::string dummy_path = m_dir + "\\" + path;
	return PathFileExistsA(dummy_path.c_str());
}

std::basic_string<TCHAR> FileIStreamDir::GetFullPath(const char * path)
{
	std::basic_string<TCHAR> fullPath;
	LPTSTR lpFilePath;
	DWORD dwLen = MAX_PATH;
	do
	{
		fullPath.resize(dwLen);
		dwLen = SearchPath(ms2ts(m_dir.c_str()).c_str(), ms2ts(path).c_str(), NULL, fullPath.size(), &fullPath[0], &lpFilePath);
	}
	while(dwLen > fullPath.size());

	fullPath.resize(dwLen);
	return fullPath;
}

std::string FileIStreamDir::GetRelativePath(const TCHAR * path)
{
	TCHAR currentDir[MAX_PATH];
	::GetCurrentDirectory(_countof(currentDir), currentDir);
	::PathAppend(currentDir, ms2ts(m_dir.c_str()).c_str());
	TCHAR relativePath[MAX_PATH];
	if (::PathRelativePathTo(relativePath, currentDir, FILE_ATTRIBUTE_DIRECTORY, path, 0))
	{
		TCHAR canonicalizedPath[MAX_PATH];
		if (::PathCanonicalize(canonicalizedPath, relativePath))
		{
			std::string ret = ts2ms(canonicalizedPath);
			if (CheckPath(ret.c_str()))
			{
				return ret;
			}
		}
	}

	return std::string();
}

IStreamPtr FileIStreamDir::OpenIStream(const char * path)
{
	std::basic_string<TCHAR> fullPath = GetFullPath(path);
	if(fullPath.empty())
	{
		THROW_CUSEXCEPTION(str_printf("cannot open file archive: %s", path));
	}

	return FileIStream::Open(fullPath.c_str());
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
	if(PathIsRelativeA(path))
	{
		std::string dummy_path(path);

		boost::replace_all(dummy_path, "\\", "/");

		ResourceDirPtrList::iterator dir_iter = m_DirList.begin();
		for (; dir_iter != m_DirList.end(); dir_iter++)
		{
			if ((*dir_iter)->CheckPath(dummy_path.c_str()))
			{
				return true;
			}
		}
		return false;
	}

	return PathFileExistsA(path);
}

std::basic_string<TCHAR> StreamDirMgr::GetFullPath(const char * path)
{
	if (path[0] == '\0')
	{
		return std::basic_string<TCHAR>();
	}

	if(!PathIsRelativeA(path))
	{
		return ms2ts(path);
	}

	ResourceDirPtrList::iterator dir_iter = m_DirList.begin();
	for(; dir_iter != m_DirList.end(); dir_iter++)
	{
		std::basic_string<TCHAR> ret = (*dir_iter)->GetFullPath(path);
		if(!ret.empty())
		{
			return ret;
		}
	}

	// ! will return the default first combined path
	TCHAR currentDir[MAX_PATH];
	::GetCurrentDirectory(_countof(currentDir), currentDir);
	dir_iter = m_DirList.begin();
	for (; dir_iter != m_DirList.end(); dir_iter++)
	{
		FileIStreamDir* dir = dynamic_cast<FileIStreamDir*>(dir_iter->get());
		if (dir)
		{
			::PathAppend(currentDir, ms2ts(dir->m_dir.c_str()).c_str());
			break;
		}
	}
	::PathAppend(currentDir, ms2ts(path).c_str());
	return std::basic_string<TCHAR>(currentDir);
}

std::string StreamDirMgr::GetRelativePath(const TCHAR * path)
{
	if (path[0] == '\0')
	{
		return std::string();
	}

	if (PathIsRelative(path))
	{
		TCHAR canonicalizedPath[MAX_PATH];
		if (::PathCanonicalize(canonicalizedPath, path))
		{
			std::string ret = ts2ms(canonicalizedPath);
			if (CheckPath(ret.c_str()))
			{
				return ret;
			}
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
	if(PathIsRelativeA(path))
	{
		std::string dummy_path(path);

		boost::replace_all(dummy_path, "\\", "/");

		ResourceDirPtrList::iterator dir_iter = m_DirList.begin();
		for (; dir_iter != m_DirList.end(); dir_iter++)
		{
			if ((*dir_iter)->CheckPath(dummy_path.c_str()))
			{
				return (*dir_iter)->OpenIStream(dummy_path.c_str());
			}
		}
	}

	return FileIStream::Open(ms2ts(path).c_str());
}

bool AsynchronousIOMgr::IsMainThread(void)
{
	return GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId;
}

AsynchronousIOMgr::AsynchronousIOMgr(void)
	: m_bStopped(false)
{
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

			try
			{
				// ! HAVENT HANDLED EXCEPTION YET
				priority_req->LoadResource();
			}
			catch (const my::Exception& e)
			{
				D3DContext::getSingleton().m_EventLog(e.what().c_str());
			}
			catch (const std::exception& e)
			{
				D3DContext::getSingleton().m_EventLog(e.what());
			}

			// ! request list will be modified when set event, shared_ptr must be thread safe
			priority_req->m_PostLoadEvent.SetEvent();

			D3DContext::getSingleton().m_d3dDeviceSec.Enter();

			// ! discarded request may also destroy d3d object in no-main thread
			priority_req.reset();

			D3DContext::getSingleton().m_d3dDeviceSec.Leave();

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
//	D3DContext::getSingleton().m_d3dDeviceSec.Leave();
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
//	D3DContext::getSingleton().m_d3dDeviceSec.Enter();
//
//	m_IORequestList.clear();
//
//	m_IORequestListMutex.Release();
//}

bool AsynchronousIOMgr::FindIORequest(const std::string & key)
{
	MutexLock lock(m_IORequestListMutex);

	IORequestPtrPairList::iterator req_iter = m_IORequestList.find(key);
	if (req_iter != m_IORequestList.end())
	{
		return true;
	}
	return false;
}

void AsynchronousIOMgr::StartIORequestProc(LONG lMaximumCount)
{
	m_bStopped = false;

	D3DContext::getSingleton().m_d3dDeviceSec.Enter();

	for (int i = 0; i < lMaximumCount; i++)
	{
		ThreadPtr thread(new Thread(boost::bind(&AsynchronousIOMgr::IORequestProc, this)));
		thread->CreateThread(CREATE_SUSPENDED);
		thread->SetThreadPriority(THREAD_PRIORITY_LOWEST);
		thread->ResumeThread();
		m_Threads.push_back(thread);
	}
}

void AsynchronousIOMgr::StopIORequestProc(void)
{
	_ASSERT(IsMainThread());

	D3DContext::getSingleton().m_d3dDeviceSec.Leave();

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
	char path[MAX_PATH] = { '\0' };
	if (IncludeType == D3DXINC_LOCAL)
	{
		PathCombineA(path, m_LocalInclude.c_str(), pFileName);
		if (CheckPath(path))
		{
			goto open_stream;
		}
	}

	for (std::vector<std::string>::iterator dir_iter = m_SystemIncludes.begin(); dir_iter != m_SystemIncludes.end(); dir_iter++)
	{
		PathCombineA(path, dir_iter->c_str(), pFileName);
		if (CheckPath(path))
		{
			goto open_stream;
		}
	}
	return E_FAIL;

open_stream:
	CachePtr cache = OpenIStream(path)->GetWholeCache();
	*ppData = cache->data();
	*pBytes = (UINT)cache->size();
	BOOST_VERIFY(m_CacheSet.insert(std::make_pair(*ppData, cache)).second);
	return S_OK;
}

HRESULT ResourceMgr::Close(
	LPCVOID pData)
{
	CacheSet::iterator cache_iter = m_CacheSet.find(pData);
	_ASSERT(cache_iter != m_CacheSet.end());
	m_CacheSet.erase(cache_iter);
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

int ResourceMgr::CheckIORequests(DWORD dwMilliseconds)
{
	_ASSERT(IsMainThread());

	int max_req_priority = INT_MIN;
	IORequestPtrPairList::iterator req_iter = m_IORequestList.begin();
	for (; req_iter != m_IORequestList.end(); )
	{
		if (req_iter->second->m_PostLoadEvent.Wait(dwMilliseconds))
		{
			req_iter = OnIORequestIteratorReady(req_iter);
		}
		else
		{
			max_req_priority = my::Max(max_req_priority, req_iter->second->m_Priority);
			req_iter++;
		}
	}

	D3DContext::getSingleton().m_d3dDeviceSec.Enter();

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

	D3DContext::getSingleton().m_d3dDeviceSec.Leave();

	return max_req_priority;
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

			try
			{
				(*callback_iter)(request->m_res);
			}
			catch (const Exception& e)
			{
				D3DContext::getSingleton().m_EventLog(e.what().c_str());
			}
		}
	}
}

boost::shared_ptr<BaseTexture> ResourceMgr::LoadTexture(const char * path)
{
	SimpleResourceCallback cb;
	IORequestPtr request(new TextureIORequest(path, INT_MAX));
	LoadIORequestAndWait(path, request, boost::bind(&SimpleResourceCallback::OnResourceReady, &cb, boost::placeholders::_1));
	return boost::dynamic_pointer_cast<BaseTexture>(cb.m_res);
}

boost::shared_ptr<OgreMesh> ResourceMgr::LoadMesh(const char * path)
{
	SimpleResourceCallback cb;
	IORequestPtr request(new MeshIORequest(path, INT_MAX));
	LoadIORequestAndWait(path, request, boost::bind(&SimpleResourceCallback::OnResourceReady, &cb, boost::placeholders::_1));
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

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((unsigned)(ch0) | ((unsigned)(ch1) << 8) | ((unsigned)(ch2) << 16) | ((unsigned)(ch3) << 24))
#endif

#define FOURCC_DXT1 (MAKEFOURCC('D','X','T','1'))
#define FOURCC_DXT2 (MAKEFOURCC('D','X','T','2'))
#define FOURCC_DXT3 (MAKEFOURCC('D','X','T','3'))
#define FOURCC_DXT4 (MAKEFOURCC('D','X','T','4'))
#define FOURCC_DXT5 (MAKEFOURCC('D','X','T','5'))
#define FOURCC_DX10 (MAKEFOURCC('D','X','1','0'))

#define FOURCC_ETC1 (MAKEFOURCC('E', 'T', 'C', '1'))
#define FOURCC_ETC2 (MAKEFOURCC('E', 'T', 'C', '2'))
#define FOURCC_ETC2A (MAKEFOURCC('E', 'T', '2', 'A'))

static const unsigned DDSCAPS_COMPLEX = 0x00000008U;
static const unsigned DDSCAPS_TEXTURE = 0x00001000U;
static const unsigned DDSCAPS_MIPMAP = 0x00400000U;
static const unsigned DDSCAPS2_VOLUME = 0x00200000U;
static const unsigned DDSCAPS2_CUBEMAP = 0x00000200U;

static const unsigned DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400U;
static const unsigned DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800U;
static const unsigned DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000U;
static const unsigned DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000U;
static const unsigned DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000U;
static const unsigned DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000U;
static const unsigned DDSCAPS2_CUBEMAP_ALL_FACES = 0x0000FC00U;

// DX10 flags
static const unsigned DDS_DIMENSION_TEXTURE1D = 2;
static const unsigned DDS_DIMENSION_TEXTURE2D = 3;
static const unsigned DDS_DIMENSION_TEXTURE3D = 4;

static const unsigned DDS_RESOURCE_MISC_TEXTURECUBE = 0x4;

static const unsigned DDS_DXGI_FORMAT_R8G8B8A8_UNORM = 28;
static const unsigned DDS_DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 26;
static const unsigned DDS_DXGI_FORMAT_BC1_UNORM = 71;
static const unsigned DDS_DXGI_FORMAT_BC1_UNORM_SRGB = 72;
static const unsigned DDS_DXGI_FORMAT_BC2_UNORM = 74;
static const unsigned DDS_DXGI_FORMAT_BC2_UNORM_SRGB = 75;
static const unsigned DDS_DXGI_FORMAT_BC3_UNORM = 77;
static const unsigned DDS_DXGI_FORMAT_BC3_UNORM_SRGB = 78;

/// DirectDraw color key definition.
struct DDColorKey
{
	unsigned dwColorSpaceLowValue_;
	unsigned dwColorSpaceHighValue_;
};

/// DirectDraw pixel format definition.
struct DDPixelFormat
{
	unsigned dwSize_;
	unsigned dwFlags_;
	unsigned dwFourCC_;
	union
	{
		unsigned dwRGBBitCount_;
		unsigned dwYUVBitCount_;
		unsigned dwZBufferBitDepth_;
		unsigned dwAlphaBitDepth_;
		unsigned dwLuminanceBitCount_;
		unsigned dwBumpBitCount_;
		unsigned dwPrivateFormatBitCount_;
	};
	union
	{
		unsigned dwRBitMask_;
		unsigned dwYBitMask_;
		unsigned dwStencilBitDepth_;
		unsigned dwLuminanceBitMask_;
		unsigned dwBumpDuBitMask_;
		unsigned dwOperations_;
	};
	union
	{
		unsigned dwGBitMask_;
		unsigned dwUBitMask_;
		unsigned dwZBitMask_;
		unsigned dwBumpDvBitMask_;
		struct
		{
			unsigned short wFlipMSTypes_;
			unsigned short wBltMSTypes_;
		} multiSampleCaps_;
	};
	union
	{
		unsigned dwBBitMask_;
		unsigned dwVBitMask_;
		unsigned dwStencilBitMask_;
		unsigned dwBumpLuminanceBitMask_;
	};
	union
	{
		unsigned dwRGBAlphaBitMask_;
		unsigned dwYUVAlphaBitMask_;
		unsigned dwLuminanceAlphaBitMask_;
		unsigned dwRGBZBitMask_;
		unsigned dwYUVZBitMask_;
	};
};

/// DirectDraw surface capabilities.
struct DDSCaps2
{
	unsigned dwCaps_;
	unsigned dwCaps2_;
	unsigned dwCaps3_;
	union
	{
		unsigned dwCaps4_;
		unsigned dwVolumeDepth_;
	};
};

struct DDSHeader10
{
	unsigned dxgiFormat;
	unsigned resourceDimension;
	unsigned miscFlag;
	unsigned arraySize;
	unsigned reserved;
};

/// DirectDraw surface description.
struct DDSurfaceDesc2
{
	unsigned dwSize_;
	unsigned dwFlags_;
	unsigned dwHeight_;
	unsigned dwWidth_;
	union
	{
		unsigned lPitch_;
		unsigned dwLinearSize_;
	};
	union
	{
		unsigned dwBackBufferCount_;
		unsigned dwDepth_;
	};
	union
	{
		unsigned dwMipMapCount_;
		unsigned dwRefreshRate_;
		unsigned dwSrcVBHandle_;
	};
	unsigned dwAlphaBitDepth_;
	unsigned dwReserved_;
	unsigned lpSurface_; // Do not define as a void pointer, as it is 8 bytes in a 64bit build
	union
	{
		DDColorKey ddckCKDestOverlay_;
		unsigned dwEmptyFaceColor_;
	};
	DDColorKey ddckCKDestBlt_;
	DDColorKey ddckCKSrcOverlay_;
	DDColorKey ddckCKSrcBlt_;
	union
	{
		DDPixelFormat ddpfPixelFormat_;
		unsigned dwFVF_;
	};
	DDSCaps2 ddsCaps_;
	unsigned dwTextureStage_;
};

void TextureIORequest::LoadResource(void)
{
	if (ResourceMgr::getSingleton().CheckPath(m_path.c_str()))
	{
		// https://github.com/urho3d/urho3d/blob/master/Source/Urho3D/Resource/Image.cpp
		IStreamPtr ifs = ResourceMgr::getSingleton().OpenIStream(m_path.c_str());
		std::string header(4, '\0');
		ifs->read(&header[0], 4);
		if (header == "DDS ")
		{
			// DDS compressed format
			DDSurfaceDesc2 ddsd;        // NOLINT(hicpp-member-init)
			ifs->read(&ddsd, sizeof(ddsd));

			// DDS DX10+
			const bool hasDXGI = ddsd.ddpfPixelFormat_.dwFourCC_ == FOURCC_DX10;
			DDSHeader10 dxgiHeader;     // NOLINT(hicpp-member-init)
			if (hasDXGI)
				ifs->read(&dxgiHeader, sizeof(dxgiHeader));

			unsigned fourCC = ddsd.ddpfPixelFormat_.dwFourCC_;

			// If the DXGI header is available then remap formats and check sRGB
			bool sRGB_ = false;
			if (hasDXGI)
			{
				switch (dxgiHeader.dxgiFormat)
				{
				case DDS_DXGI_FORMAT_BC1_UNORM:
				case DDS_DXGI_FORMAT_BC1_UNORM_SRGB:
					fourCC = FOURCC_DXT1;
					break;
				case DDS_DXGI_FORMAT_BC2_UNORM:
				case DDS_DXGI_FORMAT_BC2_UNORM_SRGB:
					fourCC = FOURCC_DXT3;
					break;
				case DDS_DXGI_FORMAT_BC3_UNORM:
				case DDS_DXGI_FORMAT_BC3_UNORM_SRGB:
					fourCC = FOURCC_DXT5;
					break;
				case DDS_DXGI_FORMAT_R8G8B8A8_UNORM:
				case DDS_DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
					fourCC = 0;
					break;
				default:
					THROW_CUSEXCEPTION("Unrecognized DDS DXGI image format");
				}

				// Check the internal sRGB formats
				if (dxgiHeader.dxgiFormat == DDS_DXGI_FORMAT_BC1_UNORM_SRGB ||
					dxgiHeader.dxgiFormat == DDS_DXGI_FORMAT_BC2_UNORM_SRGB ||
					dxgiHeader.dxgiFormat == DDS_DXGI_FORMAT_BC3_UNORM_SRGB ||
					dxgiHeader.dxgiFormat == DDS_DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
				{
					sRGB_ = true;
				}
			}

			D3DFORMAT fmt = D3DFMT_UNKNOWN;
			switch (fourCC)
			{
			case FOURCC_DXT1:
				fmt = D3DFMT_DXT1;
				break;

			case FOURCC_DXT3:
				fmt = D3DFMT_DXT3;
				break;

			case FOURCC_DXT5:
				fmt = D3DFMT_DXT5;
				break;

			// https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds-pguide
			case 113:
				fmt = D3DFMT_A16B16G16R16F;
				break;

			case 0:
				if (ddsd.ddpfPixelFormat_.dwRGBBitCount_ == 32)
				{
					fmt = D3DFMT_A8R8G8B8;
				}
				else if (ddsd.ddpfPixelFormat_.dwRGBBitCount_ == 24)
				{
					fmt = D3DFMT_X8R8G8B8;
				}
				else if (ddsd.ddpfPixelFormat_.dwRGBBitCount_ == 8)
				{
					fmt = D3DFMT_L8;
				}
				else
				{
					THROW_CUSEXCEPTION("Unsupported DDS pixel byte size");
				}
				break;

			default:
				THROW_CUSEXCEPTION("Unrecognized DDS image format");
			}

			// Is it a cube map or texture array? If so determine the size of the image chain.
			bool cubemap_ = (ddsd.ddsCaps_.dwCaps2_ & DDSCAPS2_CUBEMAP_ALL_FACES) != 0 || (hasDXGI && (dxgiHeader.miscFlag & DDS_RESOURCE_MISC_TEXTURECUBE) != 0);
			unsigned imageChainCount = 1;
			bool array_ = false;
			if (cubemap_)
				imageChainCount = 6;
			else if (hasDXGI && dxgiHeader.arraySize > 1)
			{
				//imageChainCount = dxgiHeader.arraySize;
				//array_ = true;
				THROW_CUSEXCEPTION("hasDXGI && dxgiHeader.arraySize > 1");
			}

			BaseTexturePtr res;
			if (cubemap_)
			{
				res.reset(new CubeTexture());
				D3DContext::getSingleton().m_d3dDeviceSec.Enter();
				boost::static_pointer_cast<CubeTexture>(res)->CreateCubeTexture(ddsd.dwWidth_, Max(ddsd.dwMipMapCount_, 1U), 0, fmt, D3DPOOL_MANAGED);
				D3DContext::getSingleton().m_d3dDeviceSec.Leave();
			}
			else
			{
				res.reset(new Texture2D());
				D3DContext::getSingleton().m_d3dDeviceSec.Enter();
				boost::static_pointer_cast<Texture2D>(res)->CreateTexture(ddsd.dwWidth_, ddsd.dwHeight_, Max(ddsd.dwMipMapCount_, 1U), 0, fmt, D3DPOOL_MANAGED);
				D3DContext::getSingleton().m_d3dDeviceSec.Leave();
			}

			for (unsigned faceIndex = 0; faceIndex < imageChainCount; ++faceIndex)
			{
				for (unsigned level = 0; level < Max(ddsd.dwMipMapCount_, 1U); level++)
				{
					D3DLOCKED_RECT lrc;
					if (cubemap_)
					{
						D3DContext::getSingleton().m_d3dDeviceSec.Enter();
						lrc = boost::static_pointer_cast<CubeTexture>(res)->LockRect((D3DCUBEMAP_FACES)faceIndex, NULL, 0, level);
						D3DContext::getSingleton().m_d3dDeviceSec.Leave();
					}
					else
					{
						D3DContext::getSingleton().m_d3dDeviceSec.Enter();
						lrc = boost::static_pointer_cast<Texture2D>(res)->LockRect(NULL, 0, level);
						D3DContext::getSingleton().m_d3dDeviceSec.Leave();
					}

					// Calculate mip data size
					unsigned dataSize = 0;
					if (fmt == D3DFMT_DXT1 || fmt == D3DFMT_DXT3 || fmt == D3DFMT_DXT5)
					{
						//const unsigned blockSize = compressedFormat_ == CF_DXT1 ? 8 : 16; //DXT1/BC1 is 8 bytes, DXT3/BC2 and DXT5/BC3 are 16 bytes
						const unsigned blockSize = fmt == D3DFMT_DXT1 ? 8 : 16; //DXT1/BC1 is 8 bytes, DXT3/BC2 and DXT5/BC3 are 16 bytes
						// Add 3 to ensure valid block: ie 2x2 fits uses a whole 4x4 block
						unsigned blocksWide = (Max(ddsd.dwWidth_ >> level, 1U) + 3) / 4;
						unsigned blocksHeight = (Max(ddsd.dwHeight_ >> level, 1U) + 3) / 4;
						dataSize = blocksWide * blocksHeight * blockSize;
						if (level > 0)
							dataSize *= Max(ddsd.dwDepth_ >> level, 1U);
						ifs->read(lrc.pBits, dataSize);
					}
					else if (fmt == D3DFMT_A16B16G16R16F)
					{
						unsigned blocksWide = Max(ddsd.dwWidth_ >> level, 1U);
						unsigned blocksHeight = Max(ddsd.dwHeight_ >> level, 1U);
						dataSize = (64 / 8) * blocksWide * blocksHeight * Max(ddsd.dwDepth_ >> level, 1U);
						ifs->read(lrc.pBits, dataSize);
					}
					else if (fmt == D3DFMT_A8R8G8B8 || fmt == D3DFMT_L8)
					{
						unsigned blocksWide = Max(ddsd.dwWidth_ >> level, 1U);
						unsigned blocksHeight = Max(ddsd.dwHeight_ >> level, 1U);
						dataSize = (ddsd.ddpfPixelFormat_.dwRGBBitCount_ / 8) * blocksWide * blocksHeight * Max(ddsd.dwDepth_ >> level, 1U);
						ifs->read(lrc.pBits, dataSize);
					}
					else
					{
						_ASSERT(fmt == D3DFMT_X8R8G8B8 && ddsd.ddpfPixelFormat_.dwRGBBitCount_ == 24);
						unsigned blocksWide = Max(ddsd.dwWidth_ >> level, 1U);
						unsigned blocksHeight = Max(ddsd.dwHeight_ >> level, 1U);
						for (int i = 0; i < blocksHeight; i++)
						{
							for (int j = 0; j < blocksWide; j++)
							{
								unsigned char src[3];
								ifs->read(src, sizeof(src));
								unsigned char* dst = (unsigned char*)lrc.pBits + i * lrc.Pitch + j * 4;
								*(DWORD*)dst = D3DCOLOR_XRGB(src[2], src[1], src[0]);
							}
						}
					}

					if (cubemap_)
					{
						D3DContext::getSingleton().m_d3dDeviceSec.Enter();
						boost::static_pointer_cast<CubeTexture>(res)->UnlockRect((D3DCUBEMAP_FACES)faceIndex, level);
						D3DContext::getSingleton().m_d3dDeviceSec.Leave();
					}
					else
					{
						D3DContext::getSingleton().m_d3dDeviceSec.Enter();
						boost::static_pointer_cast<Texture2D>(res)->UnlockRect(level);
						D3DContext::getSingleton().m_d3dDeviceSec.Leave();
					}
				}
			}
			m_res = res;
		}
		else
		{
			int x, y, n;
			ifs->seek(SEEK_SET, 0);
			CachePtr cache = ifs->GetWholeCache();
			boost::shared_ptr<unsigned char> pixel(stbi_load_from_memory(cache->data(), cache->size(), &x, &y, &n, 0), stbi_image_free);
			if (!pixel)
			{
				THROW_CUSEXCEPTION("stbi_load_from_memory failed");
			}

			Texture2DPtr res(new Texture2D());
			if (n == 4)
			{
				D3DContext::getSingleton().m_d3dDeviceSec.Enter();
				res->CreateTexture(x, y, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED);
				D3DContext::getSingleton().m_d3dDeviceSec.Leave();
			}
			else if (n == 3)
			{
				D3DContext::getSingleton().m_d3dDeviceSec.Enter();
				res->CreateTexture(x, y, 0, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED);
				D3DContext::getSingleton().m_d3dDeviceSec.Leave();
			}
			else if (n == 1)
			{
				D3DContext::getSingleton().m_d3dDeviceSec.Enter();
				res->CreateTexture(x, y, 0, 0, D3DFMT_L8, D3DPOOL_MANAGED);
				D3DContext::getSingleton().m_d3dDeviceSec.Leave();
			}
			else
			{
				THROW_CUSEXCEPTION("stbi_load_from_memory n != 4 && n != 3 && n != 1");
			}

			unsigned dwMipMapCount = res->GetLevelCount();
			for (int level = 0; level < Max(dwMipMapCount, 1U); level++)
			{
				boost::shared_ptr<unsigned char[]> buff;
				unsigned blocksWide = Max(x >> level, 1);
				unsigned blocksHeight = Max(y >> level, 1);
				if (level > 0)
				{
					buff.reset(new unsigned char[blocksWide * blocksHeight * n]);
					if (0 == stbir_resize_uint8(pixel.get(), x, y, 0, buff.get(), blocksWide, blocksHeight, 0, n))
					{
						THROW_CUSEXCEPTION("stbir_resize_uint8 failed");
					}
				}

				D3DContext::getSingleton().m_d3dDeviceSec.Enter();
				D3DLOCKED_RECT lrc = res->LockRect(NULL, 0, level);
				D3DContext::getSingleton().m_d3dDeviceSec.Leave();

				for (int i = 0; i < blocksHeight; i++)
				{
					for (int j = 0; j < blocksWide; j++)
					{
						unsigned char* src = (level > 0 ? buff.get() : pixel.get()) + i * blocksWide * n + j * n;
						unsigned char* dst = (unsigned char*)lrc.pBits + i * lrc.Pitch + j * (n == 1 ? 1 : 4);
						switch (n)
						{
						case 4:
							*(DWORD*)dst = D3DCOLOR_ARGB(src[3], src[0], src[1], src[2]);
							break;
						case 3:
							*(DWORD*)dst = D3DCOLOR_XRGB(src[0], src[1], src[2]);
							break;
						case 1:
							*dst = src[0];
							break;
						default:
							_ASSERT(false);
						}
					}
				}

				D3DContext::getSingleton().m_d3dDeviceSec.Enter();
				res->UnlockRect(level);
				D3DContext::getSingleton().m_d3dDeviceSec.Leave();
			}
			m_res = res;
		}
	}
}

void TextureIORequest::CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
{
	if (!m_res)
	{
		THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
	}
}

MeshIORequest::MeshIORequest(const char * path, int Priority)
	: IORequest(Priority)
	, m_path(path)
{
}

class my_pool : public my::SingletonLocalThread<my_pool>
{
public:
	boost::pool<boost::default_user_allocator_malloc_free> pool;

	my_pool(void)
		: pool(RAPIDXML_DYNAMIC_POOL_SIZE + 64)
	{
	}

	static void* malloc(size_t size)
	{
		if (size <= getSingleton().pool.get_requested_size())
		{
			return getSingleton().pool.malloc();
		}
		return NULL;
	}

	static void free(void* p)
	{
		return getSingleton().pool.free(p);
	}
};

void MeshIORequest::LoadResource(void)
{
	if(ResourceMgr::getSingleton().CheckPath(m_path.c_str()))
	{
		CachePtr cache = ResourceMgr::getSingleton().OpenIStream(m_path.c_str())->GetWholeCache();
		cache->push_back(0);

		rapidxml::xml_document<char> doc;
		doc.set_allocator(&my_pool::malloc, &my_pool::free);
		doc.parse<0>((char *)&(*cache)[0]);

		OgreMeshPtr res(new OgreMesh());
		res->CreateMeshFromOgreXml(&doc, true, D3DXMESH_MANAGED);
		m_res = res;
	}

	my_pool::getSingleton().pool.purge_memory();
}

void MeshIORequest::CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
{
	if(!m_res)
	{
		THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
	}
}

void SkeletonIORequest::LoadResource(void)
{
	if(ResourceMgr::getSingleton().CheckPath(m_path.c_str()))
	{
		CachePtr cache = ResourceMgr::getSingleton().OpenIStream(m_path.c_str())->GetWholeCache();
		cache->push_back(0);

		rapidxml::xml_document<char> doc;
		doc.set_allocator(&my_pool::malloc, &my_pool::free);
		doc.parse<0>((char*)&(*cache)[0]);

		OgreSkeletonAnimationPtr res(new OgreSkeletonAnimation());
		res->CreateOgreSkeletonAnimation(&doc);
		m_res = res;
	}

	my_pool::getSingleton().pool.purge_memory();
}

void SkeletonIORequest::CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
{
	if(!m_res)
	{
		THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
	}
}

EffectIORequest::EffectIORequest(const char * path, const char * macros, int Priority)
	: IORequest(Priority)
	, m_path(path)
	, m_macros(macros)
{
	rapidxml::xml_document<char> doc;
	doc.parse<0>(&m_macros[0]);
	rapidxml::xml_node<char>* node = doc.first_node();
	for (; node != NULL; node = node->next_sibling())
	{
		D3DXMACRO m = { 0 };
		m.Name = node->name();
		if (node->value()[0])
		{
			m.Definition = node->value();
		}
		m_d3dmacros.push_back(m);
	}
	D3DXMACRO end = { 0 };
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
	ResourceMgr::getSingleton().m_LocalInclude = boost::replace_all_copy(m_path, "/", "\\");
	PathRemoveFileSpecA(&ResourceMgr::getSingleton().m_LocalInclude[0]);
	res->CreateEffect(&(*m_cache)[0], m_cache->size(), &m_d3dmacros[0], ResourceMgr::getSingletonPtr(), D3DXSHADER_PACKMATRIX_COLUMNMAJOR | D3DXSHADER_OPTIMIZATION_LEVEL3 | D3DXFX_LARGEADDRESSAWARE, ResourceMgr::getSingleton().m_EffectPool);
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
