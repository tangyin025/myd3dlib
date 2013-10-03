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
	CriticalSectionLock lock(m_DirMapSection);
	m_DirMap[zip_path] = ResourceDirPtr(new ZipStreamDir(zip_path));
}

void StreamDirMgr::RegisterZipDir(const std::string & zip_path, const std::string & password)
{
	CriticalSectionLock lock(m_DirMapSection);
	m_DirMap[zip_path] = ResourceDirPtr(new ZipStreamDir(zip_path, password));
}

void StreamDirMgr::RegisterFileDir(const std::string & dir)
{
	CriticalSectionLock lock(m_DirMapSection);
	m_DirMap[dir] = ResourceDirPtr(new FileStreamDir(dir));
}

bool StreamDirMgr::CheckPath(const std::string & path)
{
	CriticalSectionLock lock(m_DirMapSection);
	ResourceDirPtrMap::iterator dir_iter = m_DirMap.begin();
	for(; dir_iter != m_DirMap.end(); dir_iter++)
	{
		if(dir_iter->second->CheckPath(path))
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

	CriticalSectionLock lock(m_DirMapSection);
	ResourceDirPtrMap::iterator dir_iter = m_DirMap.begin();
	for(; dir_iter != m_DirMap.end(); dir_iter++)
	{
		std::string ret = dir_iter->second->GetFullPath(path);
		if(!ret.empty())
		{
			return ret;
		}
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

	CriticalSectionLock lock(m_DirMapSection);
	ResourceDirPtrMap::iterator dir_iter = m_DirMap.begin();
	for(; dir_iter != m_DirMap.end(); dir_iter++)
	{
		if(dir_iter->second->CheckPath(path))
		{
			return dir_iter->second->OpenStream(path);
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
			m_IORequestListMutex.Release();
			// ! havent handled any exception yet
			req_iter->second->DoLoad();
			req_iter->second->m_LoadEvent.SetEvent();
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

IORequestPtr AsynchronousIOMgr::PushIORequestResource(const std::string & key, my::IORequestPtr request)
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
		return req_iter->second;
	}

	m_IORequestList.push_back(std::make_pair(key, request));
	m_IORequestListMutex.Release();
	m_IORequestListCondition.Wake();
	return request;
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

IORequestPtr AsynchronousResourceMgr::LoadResourceAsync(const std::string & key, IORequestPtr request)
{
	DeviceRelatedObjectBaseWeakPtrSet::iterator res_iter = m_ResourceWeakSet.find(key);
	if(res_iter != m_ResourceWeakSet.end())
	{
		DeviceRelatedObjectBasePtr res = res_iter->second.lock();
		if(res)
		{
			request->m_res = res;

			request->m_LoadEvent.SetEvent();
		}
	}

	return PushIORequestResource(key, request);
}

void AsynchronousResourceMgr::CheckResource(void)
{
	m_IORequestListMutex.Wait();
	IORequestPtrPairList::iterator req_iter = m_IORequestList.begin();
	for(; req_iter != m_IORequestList.end(); )
	{
		if(CheckRequest(req_iter->first, req_iter->second, 0))
		{
			req_iter = m_IORequestList.erase(req_iter);
		}
		else
		{
			break;
		}
	}
	m_IORequestListMutex.Release();
}

bool AsynchronousResourceMgr::CheckRequest(const std::string & key, IORequestPtr request, DWORD timeout)
{
	if(request->m_LoadEvent.WaitEvent(timeout))
	{
		if(!request->m_res)
		{
			request->BuildResource(D3DContext::getSingleton().GetD3D9Device());

			m_ResourceWeakSet[key] = request->m_res;
		}

		_ASSERT(m_ResourceWeakSet[key].lock() == request->m_res);

		IORequest::ResourceCallbackList::iterator callback_iter = request->m_callbacks.begin();
		for(; callback_iter != request->m_callbacks.end(); callback_iter++)
		{
			if(*callback_iter)
				(*callback_iter)(request->m_res);
		}
		return true;
	}
	return false;
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
		if(m_cache)
		{
			D3DXIMAGE_INFO imif;
			if(SUCCEEDED(D3DXGetImageInfoFromFileInMemory(&(*m_cache)[0], m_cache->size(), &imif)))
			{
				switch(imif.ResourceType)
				{
				case D3DRTYPE_TEXTURE:
					m_res.reset(new Texture2D());
					boost::static_pointer_cast<Texture2D>(m_res)->CreateTextureFromFileInMemory(pd3dDevice, &(*m_cache)[0], m_cache->size());
					break;
				case D3DRTYPE_CUBETEXTURE:
					m_res.reset(new CubeTexture());
					boost::static_pointer_cast<CubeTexture>(m_res)->CreateCubeTextureFromFileInMemory(pd3dDevice, &(*m_cache)[0], m_cache->size());
					break;
				}
			}
		}
	}
};

void AsynchronousResourceMgr::LoadTextureAsync(const std::string & path, const ResourceCallback & callback)
{
	LoadResourceAsync(path, IORequestPtr(new TextureIORequest(callback, path, this)));
}

BaseTexturePtr AsynchronousResourceMgr::LoadTexture(const std::string & path)
{
	IORequestPtr request = LoadResourceAsync(path, IORequestPtr(new TextureIORequest(ResourceCallback(), path, this)));

	CheckRequest(path, request, INFINITE);

	return boost::dynamic_pointer_cast<BaseTexture>(request->m_res);
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
			catch(rapidxml::parse_error & e)
			{
				THROW_CUSEXCEPTION(ms2ts(e.what()));
			}
		}
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(m_doc.first_node())
		{
			m_res.reset(new OgreMesh());
			boost::static_pointer_cast<OgreMesh>(m_res)->CreateMeshFromOgreXml(pd3dDevice, &m_doc);
		}
	}
};

void AsynchronousResourceMgr::LoadMeshAsync(const std::string & path, const ResourceCallback & callback)
{
	LoadResourceAsync(path, IORequestPtr(new MeshIORequest(callback, path, this)));
}

OgreMeshPtr AsynchronousResourceMgr::LoadMesh(const std::string & path)
{
	IORequestPtr request = LoadResourceAsync(path, IORequestPtr(new MeshIORequest(ResourceCallback(), path, this)));

	CheckRequest(path, request, INFINITE);

	return boost::dynamic_pointer_cast<OgreMesh>(request->m_res);
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
			catch(rapidxml::parse_error & e)
			{
				THROW_CUSEXCEPTION(ms2ts(e.what()));
			}
		}
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(m_doc.first_node())
		{
			m_res.reset(new OgreSkeletonAnimation());
			boost::static_pointer_cast<OgreSkeletonAnimation>(m_res)->CreateOgreSkeletonAnimation(&m_doc);
		}
	}
};

void AsynchronousResourceMgr::LoadSkeletonAsync(const std::string & path, const ResourceCallback & callback)
{
	LoadResourceAsync(path, IORequestPtr(new SkeletonIORequest(callback, path, this)));
}

OgreSkeletonAnimationPtr AsynchronousResourceMgr::LoadSkeleton(const std::string & path)
{
	IORequestPtr request = LoadResourceAsync(path, IORequestPtr(new SkeletonIORequest(ResourceCallback(), path, this)));

	CheckRequest(path, request, INFINITE);

	return boost::dynamic_pointer_cast<OgreSkeletonAnimation>(request->m_res);
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
		if(m_cache)
		{
			m_res.reset(new Effect());
			m_arc->m_EffectInclude = ZipStreamDir::ReplaceSlash(m_path);
			PathRemoveFileSpecA(&m_arc->m_EffectInclude[0]);
			boost::static_pointer_cast<Effect>(m_res)->CreateEffect(pd3dDevice, &(*m_cache)[0], m_cache->size(), &m_d3dmacros[0], m_arc, 0, m_arc->m_EffectPool);
		}
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

	IORequestPtr request = LoadResourceAsync(key, IORequestPtr(new EffectIORequest(ResourceCallback(), path, macros, this)));

	CheckRequest(key, request, INFINITE);

	return boost::dynamic_pointer_cast<Effect>(request->m_res);
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
		if(m_cache)
		{
			m_res.reset(new Font());
			boost::static_pointer_cast<Font>(m_res)->CreateFontFromFileInCache(pd3dDevice, m_cache, m_height);
		}
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

	IORequestPtr request = LoadResourceAsync(key, IORequestPtr(new FontIORequest(ResourceCallback(), path, height, this)));

	CheckRequest(key, request, INFINITE);

	return boost::dynamic_pointer_cast<Font>(request->m_res);
}
