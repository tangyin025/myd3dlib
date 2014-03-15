#include "stdafx.h"
#include "myResource.h"
#include "myDxutApp.h"
#include "libc.h"
#include <strstream>
#include <boost/bind.hpp>

using namespace my;

ZipStream::ZipStream(unzFile zFile)
	: m_zFile(zFile)
{
	_ASSERT(NULL != m_zFile);

	int ret = unzGetCurrentFileInfo(m_zFile, &m_zFileInfo, NULL, 0, NULL, 0, NULL, 0);
	if(UNZ_OK != ret)
	{
		unzClose(m_zFile);
		THROW_CUSEXCEPTION(_T("cannot get file info from zip file"));
	}
}

ZipStream::~ZipStream(void)
{
	unzCloseCurrentFile(m_zFile);
	unzClose(m_zFile);
}

int ZipStream::read(void * buff, unsigned read_size)
{
	return unzReadCurrentFile(m_zFile, buff, read_size);
}

CachePtr ZipStream::GetWholeCache(void)
{
	CachePtr cache(new Cache(m_zFileInfo.uncompressed_size));
	if(0 == cache->size())
	{
		THROW_CUSEXCEPTION(_T("read zip file cache failed"));
	}

	int ret = read(&(*cache)[0], cache->size());
	if(ret != cache->size())
	{
		THROW_CUSEXCEPTION(_T("read zip file cache failed"));
	}
	return cache;
}

FileStream::FileStream(FILE * fp)
	: m_fp(fp)
{
	_ASSERT(NULL != m_fp);
}

FileStream::~FileStream(void)
{
	fclose(m_fp);
}

int FileStream::read(void * buff, unsigned read_size)
{
	return fread(buff, 1, read_size, m_fp);
}

CachePtr FileStream::GetWholeCache(void)
{
	fseek(m_fp, 0, SEEK_END);
	long len = ftell(m_fp);
	CachePtr cache(new Cache(len));
	if(0 == cache->size())
	{
		THROW_CUSEXCEPTION(_T("read file cache failed"));
	}

	fseek(m_fp, 0, SEEK_SET);
	int ret = read(&(*cache)[0], cache->size());
	if(ret != cache->size())
	{
		THROW_CUSEXCEPTION(_T("read file cache failed"));
	}
	return cache;
}

std::string ZipStreamDir::ReplaceSlash(const std::string & path)
{
	size_t pos = 0;
	std::string ret = path;
	while(std::string::npos != (pos = ret.find('/', pos)))
	{
		ret.replace(pos++, 1, 1, '\\');
	}
	return ret;
}

std::string ZipStreamDir::ReplaceBackslash(const std::string & path)
{
	size_t pos = 0;
	std::string ret = path;
	while(std::string::npos != (pos = ret.find('\\', pos)))
	{
		ret.replace(pos++, 1, 1, '/');
	}
	return ret;
}

bool ZipStreamDir::CheckPath(const std::string & path)
{
	unzFile zFile = unzOpen(m_dir.c_str());
	if(NULL == zFile)
	{
		return false;
	}

	int ret = unzLocateFile(zFile, ReplaceBackslash(path).c_str(), 0);
	if(UNZ_OK != ret)
	{
		unzClose(zFile);
		return false;
	}

	unzClose(zFile);
	return true;
}

std::string ZipStreamDir::GetFullPath(const std::string & path)
{
	return std::string();
}

IOStreamPtr ZipStreamDir::OpenStream(const std::string & path)
{
	unzFile zFile = unzOpen(m_dir.c_str());
	if(NULL == zFile)
	{
		THROW_CUSEXCEPTION(str_printf(_T("cannot open zip archive: %s"), ms2ts(m_dir).c_str()));
	}

	int ret = unzLocateFile(zFile, ReplaceBackslash(path).c_str(), 0);
	if(UNZ_OK != ret)
	{
		unzClose(zFile);
		THROW_CUSEXCEPTION(str_printf(_T("cannot open zip file: %s"), ms2ts(path).c_str()));
	}

	if(m_UsePassword)
	{
		ret = unzOpenCurrentFilePassword(zFile, m_password.c_str());
	}
	else
	{
		ret = unzOpenCurrentFile(zFile);
	}
	if(UNZ_OK != ret)
	{
		unzClose(zFile);
		THROW_CUSEXCEPTION(str_printf(_T("cannot open zip file: %s"), ms2ts(path).c_str()));
	}
	return IOStreamPtr(new ZipStream(zFile));
}

bool FileStreamDir::CheckPath(const std::string & path)
{
	return !GetFullPath(path).empty();
}

std::string FileStreamDir::GetFullPath(const std::string & path)
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

IOStreamPtr FileStreamDir::OpenStream(const std::string & path)
{
	std::string fullPath = GetFullPath(path);
	if(fullPath.empty())
	{
		THROW_CUSEXCEPTION(str_printf(_T("cannot open file archive: %s"), ms2ts(path).c_str()));
	}

	FILE * fp;
	if(0 != fopen_s(&fp, fullPath.c_str(), "rb"))
	{
		THROW_CUSEXCEPTION(str_printf(_T("cannot open file archive: %s"), ms2ts(path).c_str()));
	}

	return IOStreamPtr(new FileStream(fp));
}

void StreamDirMgr::RegisterZipDir(const std::string & zip_path)
{
	CriticalSectionLock lock(m_DirListSection);
	m_DirList.push_back(ResourceDirPtr(new ZipStreamDir(zip_path)));
}

void StreamDirMgr::RegisterZipDir(const std::string & zip_path, const std::string & password)
{
	CriticalSectionLock lock(m_DirListSection);
	m_DirList.push_back(ResourceDirPtr(new ZipStreamDir(zip_path, password)));
}

void StreamDirMgr::RegisterFileDir(const std::string & dir)
{
	CriticalSectionLock lock(m_DirListSection);
	m_DirList.push_back(ResourceDirPtr(new FileStreamDir(dir)));
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
		ret = ZipStreamDir::ReplaceSlash(ret);
		std::string full_path;
		full_path.resize(MAX_PATH);
		GetFullPathNameA(ret.c_str(), full_path.size(), &full_path[0], NULL);
		return full_path;
	}

	return std::string();
}

IOStreamPtr StreamDirMgr::OpenStream(const std::string & path)
{
	if(!PathIsRelativeA(path.c_str()))
	{
		FILE * fp;
		if(0 != fopen_s(&fp, path.c_str(), "rb"))
		{
			THROW_CUSEXCEPTION(str_printf(_T("cannot open file archive: %s"), ms2ts(path).c_str()));
		}
		return IOStreamPtr(new FileStream(fp));
	}

	CriticalSectionLock lock(m_DirListSection);
	ResourceDirPtrList::iterator dir_iter = m_DirList.begin();
	for(; dir_iter != m_DirList.end(); dir_iter++)
	{
		if((*dir_iter)->CheckPath(path))
		{
			return (*dir_iter)->OpenStream(path);
		}
	}

	THROW_CUSEXCEPTION(str_printf(_T("cannot find specified file: %s"), ms2ts(path).c_str()));
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
			if(!req_iter->second->m_LoadEvent.WaitEvent(0))
			{
				break;
			}
		}
		if(req_iter != m_IORequestList.end())
		{
			// ! req_iter will be invalid after release mutex
			IORequestPtr request = req_iter->second;

			m_IORequestListMutex.Release();

			// ! havent handled exception
			request->DoLoad();

			// ! request will be decrease after set event, shared_ptr must be thread safe
			request->m_LoadEvent.SetEvent();

			m_IORequestListMutex.Wait();
		}
		else
		{
			m_IORequestListCondition.SleepMutex(m_IORequestListMutex, INFINITE);
		}
	}
	m_IORequestListMutex.Release();

	return 0;
}

AsynchronousIOMgr::IORequestPtrPairList::iterator AsynchronousIOMgr::PushIORequestResource(const std::string & key, my::IORequestPtr request)
{
	m_IORequestListMutex.Wait();

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
		req_iter->second->m_callbacks.insert(
			req_iter->second->m_callbacks.end(), request->m_callbacks.begin(), request->m_callbacks.end());
		m_IORequestListMutex.Release();
		return req_iter;
	}

	m_IORequestList.push_back(std::make_pair(key, request));
	m_IORequestListMutex.Release();
	m_IORequestListCondition.Wake();
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
	m_IORequestListMutex.Wait();
	m_bStopped = true;
	m_IORequestListMutex.Release();
	m_IORequestListCondition.Wake();
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

HRESULT AsynchronousResourceMgr::OnCreateDevice(
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

HRESULT AsynchronousResourceMgr::OnResetDevice(
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

void AsynchronousResourceMgr::OnLostDevice(void)
{
	DeviceRelatedResourceMgr::OnLostDevice();
}

void AsynchronousResourceMgr::OnDestroyDevice(void)
{
	StopIORequestProc();

	DeviceRelatedResourceMgr::OnDestroyDevice();

	m_EffectPool.Release();

	m_Thread.WaitForThreadStopped(INFINITE);

	m_Thread.CloseThread();

	m_IORequestList.clear();
}

HRESULT AsynchronousResourceMgr::Open(
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
			cache = OpenStream(path)->GetWholeCache();
			*ppData = &(*cache)[0];
			*pBytes = cache->size();
			_ASSERT(m_CacheSet.end() == m_CacheSet.find(*ppData));
			m_CacheSet[*ppData] = cache;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT AsynchronousResourceMgr::Close(
	LPCVOID pData)
{
	_ASSERT(m_CacheSet.end() != m_CacheSet.find(pData));
	m_CacheSet.erase(pData);
	return S_OK;
}

AsynchronousIOMgr::IORequestPtrPairList::iterator AsynchronousResourceMgr::LoadResourceAsync(const std::string & key, IORequestPtr request)
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

	return PushIORequestResource(key, request);
}

void AsynchronousResourceMgr::CheckRequests(void)
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
}

bool AsynchronousResourceMgr::CheckResource(const std::string & key, IORequestPtr request, DWORD timeout)
{
	if(request->m_LoadEvent.WaitEvent(timeout))
	{
		if(!request->m_res)
		{
			// ! havent handled exception
			try
			{
				request->BuildResource(D3DContext::getSingleton().GetD3D9Device());

				_ASSERT(m_ResourceWeakSet.end() == m_ResourceWeakSet.find(key));

				m_ResourceWeakSet[key] = request->m_res;

				IORequest::ResourceCallbackList::iterator callback_iter = request->m_callbacks.begin();
				for(; callback_iter != request->m_callbacks.end(); callback_iter++)
				{
					if(*callback_iter)
						(*callback_iter)(request->m_res);
				}
			}
			catch(const Exception & e)
			{
				OnResourceFailed(e.what());
			}
		}
		return true;
	}
	return false;
}

void AsynchronousResourceMgr::OnResourceFailed(const std::basic_string<TCHAR> & error_str)
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
			m_cache = m_arc->OpenStream(m_path)->GetWholeCache();
		}
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(!m_cache)
		{
			THROW_CUSEXCEPTION(str_printf(_T("failed open %s"), ms2ts(m_path).c_str()));
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
			THROW_CUSEXCEPTION(str_printf(_T("unsupported d3d texture format %u"), imif.ResourceType));
		}
	}
};

void AsynchronousResourceMgr::LoadTextureAsync(const std::string & path, const ResourceCallback & callback)
{
	LoadResourceAsync(path, IORequestPtr(new TextureIORequest(callback, path, this)));
}

BaseTexturePtr AsynchronousResourceMgr::LoadTexture(const std::string & path)
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
			m_cache = m_arc->OpenStream(m_path)->GetWholeCache();
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
			THROW_CUSEXCEPTION(str_printf(_T("failed open %s"), ms2ts(m_path).c_str()));
		}
		OgreMeshPtr res(new OgreMesh());
		res->CreateMeshFromOgreXml(pd3dDevice, &m_doc);
		m_res = res;
	}
};

void AsynchronousResourceMgr::LoadMeshAsync(const std::string & path, const ResourceCallback & callback)
{
	LoadResourceAsync(path, IORequestPtr(new MeshIORequest(callback, path, this)));
}

OgreMeshPtr AsynchronousResourceMgr::LoadMesh(const std::string & path)
{
	return LoadResource<OgreMesh>(path, IORequestPtr(new MeshIORequest(ResourceCallback(), path, this)));
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
			m_cache = m_arc->OpenStream(m_path)->GetWholeCache();
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
			THROW_CUSEXCEPTION(str_printf(_T("failed open %s"), ms2ts(m_path).c_str()));
		}
		OgreSkeletonAnimationPtr res(new OgreSkeletonAnimation());
		res->CreateOgreSkeletonAnimation(&m_doc);
		m_res = res;
	}
};

void AsynchronousResourceMgr::LoadSkeletonAsync(const std::string & path, const ResourceCallback & callback)
{
	LoadResourceAsync(path, IORequestPtr(new SkeletonIORequest(callback, path, this)));
}

OgreSkeletonAnimationPtr AsynchronousResourceMgr::LoadSkeleton(const std::string & path)
{
	return LoadResource<OgreSkeletonAnimation>(path, IORequestPtr(new SkeletonIORequest(ResourceCallback(), path, this)));
}

class AsynchronousResourceMgr::EffectIORequest : public IORequest
{
protected:
	std::string m_path;

	std::vector<D3DXMACRO> m_d3dmacros;

	AsynchronousResourceMgr * m_arc;

	CachePtr m_cache;

public:
	EffectIORequest(const ResourceCallback & callback, const std::string & path, const EffectMacroPairList & macros, AsynchronousResourceMgr * arc)
		: m_path(path)
		, m_arc(arc)
	{
		EffectMacroPairList::const_iterator macro_iter = macros.begin();
		for(; macro_iter != macros.end(); macro_iter++)
		{
			D3DXMACRO d3dmacro = {macro_iter->first.c_str(), macro_iter->second.c_str()};
			m_d3dmacros.push_back(d3dmacro);
		}
		D3DXMACRO end = {0};
		m_d3dmacros.push_back(end);

		if(callback)
		{
			m_callbacks.push_back(callback);
		}
	}

	virtual void DoLoad(void)
	{
		if(m_arc->CheckPath(m_path))
		{
			m_cache = m_arc->OpenStream(m_path)->GetWholeCache();
		}
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(!m_cache)
		{
			THROW_CUSEXCEPTION(str_printf(_T("failed open %s"), ms2ts(m_path).c_str()));
		}
		EffectPtr res(new Effect());
		m_arc->m_EffectInclude = ZipStreamDir::ReplaceSlash(m_path);
		PathRemoveFileSpecA(&m_arc->m_EffectInclude[0]);
		res->CreateEffect(pd3dDevice, &(*m_cache)[0], m_cache->size(), &m_d3dmacros[0], m_arc, 0, m_arc->m_EffectPool);
		m_res = res;
	}

	static std::string BuildKey(const std::string & path, const EffectMacroPairList & macros)
	{
		std::ostrstream ostr;
		ostr << path;
		EffectMacroPairList::const_iterator macro_iter = macros.begin();
		for(; macro_iter != macros.end(); macro_iter++)
		{
			ostr << ", " << macro_iter->first << ", " << macro_iter->second;
		}
		ostr << std::ends;
		return ostr.str();
	}
};

void AsynchronousResourceMgr::LoadEffectAsync(const std::string & path, const EffectMacroPairList & macros, const ResourceCallback & callback)
{
	std::string key = EffectIORequest::BuildKey(path, macros);

	LoadResourceAsync(key, IORequestPtr(new EffectIORequest(callback, path, macros, this)));
}

EffectPtr AsynchronousResourceMgr::LoadEffect(const std::string & path, const EffectMacroPairList & macros)
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
			m_cache = m_arc->OpenStream(m_path)->GetWholeCache();
		}
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(!m_cache)
		{
			THROW_CUSEXCEPTION(str_printf(_T("failed open %s"), ms2ts(m_path).c_str()));
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

void AsynchronousResourceMgr::LoadFontAsync(const std::string & path, int height, const ResourceCallback & callback)
{
	std::string key = FontIORequest::BuildKey(path, height);

	LoadResourceAsync(key, IORequestPtr(new FontIORequest(callback, path, height, this)));
}

FontPtr AsynchronousResourceMgr::LoadFont(const std::string & path, int height)
{
	std::string key = FontIORequest::BuildKey(path, height);

	return LoadResource<Font>(key, IORequestPtr(new FontIORequest(ResourceCallback(), path, height, this)));
}
