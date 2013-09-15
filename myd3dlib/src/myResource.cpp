#include "stdafx.h"
#include "myResource.h"
#include "myDxutApp.h"
#include "libc.h"
#include <strstream>
#include "myMesh.h"

using namespace my;

ZipArchiveStream::ZipArchiveStream(unzFile zFile)
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

ZipArchiveStream::~ZipArchiveStream(void)
{
	unzCloseCurrentFile(m_zFile);
	unzClose(m_zFile);
}

int ZipArchiveStream::read(void * buff, unsigned read_size)
{
	return unzReadCurrentFile(m_zFile, buff, read_size);
}

CachePtr ZipArchiveStream::GetWholeCache(void)
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

FileArchiveStream::FileArchiveStream(FILE * fp)
	: m_fp(fp)
{
	_ASSERT(NULL != m_fp);
}

FileArchiveStream::~FileArchiveStream(void)
{
	fclose(m_fp);
}

int FileArchiveStream::read(void * buff, unsigned read_size)
{
	return fread(buff, 1, read_size, m_fp);
}

CachePtr FileArchiveStream::GetWholeCache(void)
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

std::string ZipArchiveDir::ReplaceSlash(const std::string & path)
{
	size_t pos = 0;
	std::string ret = path;
	while(std::string::npos != (pos = ret.find('/', pos)))
	{
		ret.replace(pos++, 1, 1, '\\');
	}
	return ret;
}

std::string ZipArchiveDir::ReplaceBackslash(const std::string & path)
{
	size_t pos = 0;
	std::string ret = path;
	while(std::string::npos != (pos = ret.find('\\', pos)))
	{
		ret.replace(pos++, 1, 1, '/');
	}
	return ret;
}

bool ZipArchiveDir::CheckArchivePath(const std::string & path)
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

std::string ZipArchiveDir::GetFullPath(const std::string & path)
{
	return std::string();
}

ArchiveStreamPtr ZipArchiveDir::OpenArchiveStream(const std::string & path)
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
	return ArchiveStreamPtr(new ZipArchiveStream(zFile));
}

bool FileArchiveDir::CheckArchivePath(const std::string & path)
{
	return !GetFullPath(path).empty();
}

std::string FileArchiveDir::GetFullPath(const std::string & path)
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

ArchiveStreamPtr FileArchiveDir::OpenArchiveStream(const std::string & path)
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

	return ArchiveStreamPtr(new FileArchiveStream(fp));
}

void ArchiveDirMgr::RegisterZipArchive(const std::string & zip_path)
{
	CriticalSectionLock lock(m_DirMapSection);
	m_DirMap[zip_path] = ResourceDirPtr(new ZipArchiveDir(zip_path));
}

void ArchiveDirMgr::RegisterZipArchive(const std::string & zip_path, const std::string & password)
{
	CriticalSectionLock lock(m_DirMapSection);
	m_DirMap[zip_path] = ResourceDirPtr(new ZipArchiveDir(zip_path, password));
}

void ArchiveDirMgr::RegisterFileDir(const std::string & dir)
{
	CriticalSectionLock lock(m_DirMapSection);
	m_DirMap[dir] = ResourceDirPtr(new FileArchiveDir(dir));
}

bool ArchiveDirMgr::CheckArchivePath(const std::string & path)
{
	CriticalSectionLock lock(m_DirMapSection);
	ResourceDirPtrMap::iterator dir_iter = m_DirMap.begin();
	for(; dir_iter != m_DirMap.end(); dir_iter++)
	{
		if(dir_iter->second->CheckArchivePath(path))
		{
			return true;
		}
	}

	return false;
}

std::string ArchiveDirMgr::GetFullPath(const std::string & path)
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

ArchiveStreamPtr ArchiveDirMgr::OpenArchiveStream(const std::string & path)
{
	if(!PathIsRelativeA(path.c_str()))
	{
		FILE * fp;
		if(0 != fopen_s(&fp, path.c_str(), "rb"))
		{
			THROW_CUSEXCEPTION(str_printf(_T("cannot open file archive: %s"), ms2ts(path).c_str()));
		}

		return ArchiveStreamPtr(new FileArchiveStream(fp));
	}

	CriticalSectionLock lock(m_DirMapSection);
	ResourceDirPtrMap::iterator dir_iter = m_DirMap.begin();
	for(; dir_iter != m_DirMap.end(); dir_iter++)
	{
		if(dir_iter->second->CheckArchivePath(path))
		{
			return dir_iter->second->OpenArchiveStream(path);
		}
	}

	THROW_CUSEXCEPTION(str_printf(_T("cannot find specified file: %s"), ms2ts(path).c_str()));
}
//
//HRESULT DeviceRelatedResourceMgr::Open(
//	D3DXINCLUDE_TYPE IncludeType,
//	LPCSTR pFileName,
//	LPCVOID pParentData,
//	LPCVOID * ppData,
//	UINT * pBytes)
//{
//	CachePtr cache;
//	std::string loc_path;
//	loc_path.resize(MAX_PATH);
//	PathCombineA(&loc_path[0], m_EffectInclude.c_str(), pFileName);
//	switch(IncludeType)
//	{
//	case D3DXINC_SYSTEM:
//	case D3DXINC_LOCAL:
//		if(CheckArchivePath(loc_path))
//		{
//			cache = OpenArchiveStream(loc_path)->GetWholeCache();
//			*ppData = &(*cache)[0];
//			*pBytes = cache->size();
//			_ASSERT(m_cacheSet.end() == m_cacheSet.find(*ppData));
//			m_cacheSet[*ppData] = cache;
//			return S_OK;
//		}
//	}
//	return E_FAIL;
//}
//
//HRESULT DeviceRelatedResourceMgr::Close(
//	LPCVOID pData)
//{
//	_ASSERT(m_cacheSet.end() != m_cacheSet.find(pData));
//	m_cacheSet.erase(pData);
//	return S_OK;
//}
//
//HRESULT DeviceRelatedResourceMgr::OnCreateDevice(
//	IDirect3DDevice9 * pd3dDevice,
//	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
//{
//	HRESULT hr;
//	if(FAILED(hr = D3DXCreateEffectPool(&m_EffectPool)))
//	{
//		THROW_D3DEXCEPTION(hr);
//	}
//
//	return S_OK;
//}
//
//HRESULT DeviceRelatedResourceMgr::OnResetDevice(
//	IDirect3DDevice9 * pd3dDevice,
//	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
//{
//	DeviceRelatedObjectBaseWeakPtrSet::iterator res_iter = m_ResourceWeakSet.begin();
//	for(; res_iter != m_ResourceWeakSet.end();)
//	{
//		DeviceRelatedObjectBasePtr res = res_iter->second.lock();
//		if(res)
//		{
//			res->OnResetDevice();
//			res_iter++;
//		}
//		else
//		{
//			m_ResourceWeakSet.erase(res_iter++);
//		}
//	}
//
//	return S_OK;
//}
//
//void DeviceRelatedResourceMgr::OnLostDevice(void)
//{
//	DeviceRelatedObjectBaseWeakPtrSet::iterator res_iter = m_ResourceWeakSet.begin();
//	for(; res_iter != m_ResourceWeakSet.end();)
//	{
//		DeviceRelatedObjectBasePtr res = res_iter->second.lock();
//		if(res)
//		{
//			res->OnLostDevice();
//			res_iter++;
//		}
//		else
//		{
//			m_ResourceWeakSet.erase(res_iter++);
//		}
//	}
//}
//
//void DeviceRelatedResourceMgr::OnDestroyDevice(void)
//{
//	DeviceRelatedObjectBaseWeakPtrSet::iterator res_iter = m_ResourceWeakSet.begin();
//	for(; res_iter != m_ResourceWeakSet.end();)
//	{
//		DeviceRelatedObjectBasePtr res = res_iter->second.lock();
//		if(res)
//		{
//			res->OnDestroyDevice();
//			res_iter++;
//		}
//		else
//		{
//			m_ResourceWeakSet.erase(res_iter++);
//		}
//	}
//
//	m_ResourceWeakSet.clear();
//
//	m_EffectPool.Release();
//}
//
//TexturePtr DeviceRelatedResourceMgr::LoadTexture(const std::string & path, bool reload)
//{
//	TexturePtr ret = GetDeviceRelatedResource<Texture>(path, reload);
//	if(!ret->m_ptr)
//	{
//		std::string full_path = GetFullPath(path);
//		if(!full_path.empty())
//		{
//			ret->CreateTextureFromFile(D3DContext::getSingleton().GetD3D9Device(), ms2ts(full_path).c_str());
//		}
//		else
//		{
//			CachePtr cache = OpenArchiveStream(path)->GetWholeCache();
//			ret->CreateTextureFromFileInMemory(D3DContext::getSingleton().GetD3D9Device(), &(*cache)[0], cache->size());
//		}
//	}
//	return ret;
//}
//
//CubeTexturePtr DeviceRelatedResourceMgr::LoadCubeTexture(const std::string & path, bool reload)
//{
//	CubeTexturePtr ret = GetDeviceRelatedResource<CubeTexture>(path, reload);
//	if(!ret->m_ptr)
//	{
//		std::string full_path = GetFullPath(path);
//		if(!full_path.empty())
//		{
//			ret->CreateCubeTextureFromFile(D3DContext::getSingleton().GetD3D9Device(), ms2ts(full_path).c_str());
//		}
//		else
//		{
//			CachePtr cache = OpenArchiveStream(path)->GetWholeCache();
//			ret->CreateCubeTextureFromFileInMemory(D3DContext::getSingleton().GetD3D9Device(), &(*cache)[0], cache->size());
//		}
//	}
//	return ret;
//}
//
//OgreMeshPtr DeviceRelatedResourceMgr::LoadMesh(const std::string & path, bool reload)
//{
//	OgreMeshPtr ret = GetDeviceRelatedResource<OgreMesh>(path, reload);
//	if(!ret->m_ptr)
//	{
//		std::string full_path = GetFullPath(path);
//		if(!full_path.empty())
//		{
//			ret->CreateMeshFromOgreXmlInFile(D3DContext::getSingleton().GetD3D9Device(), ms2ts(full_path).c_str(), true);
//		}
//		else
//		{
//			CachePtr cache = OpenArchiveStream(path)->GetWholeCache();
//			cache->push_back(0);
//			ret->CreateMeshFromOgreXmlInMemory(D3DContext::getSingleton().GetD3D9Device(), (char *)&(*cache)[0], cache->size(), true);
//		}
//	}
//	return ret;
//}
//
//OgreSkeletonAnimationPtr DeviceRelatedResourceMgr::LoadSkeleton(const std::string & path, bool reload)
//{
//	OgreSkeletonAnimationPtr ret = GetDeviceRelatedResource<OgreSkeletonAnimation>(path, reload);
//	if(ret->m_boneHierarchy.empty())
//	{
//		std::string full_path = GetFullPath(path);
//		if(!full_path.empty())
//		{
//			ret->CreateOgreSkeletonAnimationFromFile(ms2ts(full_path).c_str());
//		}
//		else
//		{
//			CachePtr cache = OpenArchiveStream(path)->GetWholeCache();
//			cache->push_back(0);
//			ret->CreateOgreSkeletonAnimation((char *)&(*cache)[0], cache->size());
//		}
//	}
//	return ret;
//}
//
//EffectPtr DeviceRelatedResourceMgr::LoadEffect(const std::string & path, const string_pair_list & macros, bool reload)
//{
//	std::ostrstream ostr;
//	ostr << path;
//	std::vector<D3DXMACRO> d3dmacros;
//	string_pair_list::const_iterator macro_iter = macros.begin();
//	for(; macro_iter != macros.end(); macro_iter++)
//	{
//		D3DXMACRO d3dmacro = {macro_iter->first.c_str(), macro_iter->second.c_str()};
//		ostr << ", " << d3dmacro.Name << ", " << d3dmacro.Definition;
//		d3dmacros.push_back(d3dmacro);
//	}
//	D3DXMACRO end = {0};
//	d3dmacros.push_back(end);
//	EffectPtr ret = GetDeviceRelatedResource<Effect>(ostr.str(), reload);
//	if(!ret->m_ptr)
//	{
//		m_EffectInclude = ZipArchiveDir::ReplaceSlash(path);
//		PathRemoveFileSpecA(&m_EffectInclude[0]);
//		std::string full_path = GetFullPath(path);
//		if(!full_path.empty())
//		{
//			ret->CreateEffectFromFile(D3DContext::getSingleton().GetD3D9Device(), ms2ts(full_path).c_str(), &d3dmacros[0], NULL, 0, m_EffectPool);
//		}
//		else
//		{
//			CachePtr cache = OpenArchiveStream(path)->GetWholeCache();
//			ret->CreateEffect(D3DContext::getSingleton().GetD3D9Device(), &(*cache)[0], cache->size(), &d3dmacros[0], this, 0, m_EffectPool);
//		}
//	}
//	return ret;
//}
//
//FontPtr DeviceRelatedResourceMgr::LoadFont(const std::string & path, int height, bool reload)
//{
//	std::ostrstream ostr;
//	ostr << path << ", " << height;
//	FontPtr ret = GetDeviceRelatedResource<Font>(ostr.str(), reload);
//	if(!ret->m_face)
//	{
//		std::string full_path = GetFullPath(path);
//		if(!full_path.empty())
//		{
//			ret->CreateFontFromFile(D3DContext::getSingleton().GetD3D9Device(), full_path.c_str(), height);
//		}
//		else
//		{
//			CachePtr cache = OpenArchiveStream(path)->GetWholeCache();
//			ret->CreateFontFromFileInCache(D3DContext::getSingleton().GetD3D9Device(), cache, height);
//		}
//	}
//	return ret;
//}

void IORequest::CallbackAll(DeviceRelatedObjectBasePtr res)
{
	ResourceCallbackList::iterator callback_iter = m_callbacks.begin();
	for(; callback_iter != m_callbacks.end(); callback_iter++)
	{
		if(*callback_iter)
			(*callback_iter)(res);
	}
}

DWORD AsynchronousIOMgr::OnProc(void)
{
	m_IORequestListSection.Enter();
	while(!m_bStopped)
	{
		IORequestPtrPairList::iterator req_iter = m_IORequestList.begin();
		for(; req_iter != m_IORequestList.end(); req_iter++)
		{
			if(IORequest::IORequestStateNone == req_iter->second->m_state)
			{
				break;
			}
		}
		if(req_iter != m_IORequestList.end())
		{
			m_IORequestListSection.Leave();
			// ! havent handled any exception yet
			req_iter->second->DoLoad();
			m_IORequestListSection.Enter();
			req_iter->second->m_state = IORequest::IORequestStateLoaded;
		}
		else
		{
			m_IORequestListCondition.SleepCS(m_IORequestListSection, INFINITE);
		}
	}
	m_IORequestListSection.Leave();

	return 0;
}

void AsynchronousIOMgr::PushIORequestResource(const std::string & key, my::IORequestPtr request)
{
	m_IORequestListSection.Enter();
	IORequestPtrPairList::iterator req_iter = m_IORequestList.begin();
	for(; req_iter != m_IORequestList.end(); req_iter++)
	{
		if(IORequest::IORequestStateNone == req_iter->second->m_state)
		{
			break;
		}
	}
	if(req_iter != m_IORequestList.end())
	{
		req_iter->second->m_callbacks.insert(
			req_iter->second->m_callbacks.end(), request->m_callbacks.begin(), request->m_callbacks.end());
	}
	else
	{
		m_IORequestList.push_back(std::make_pair(key, request));
	}
	m_IORequestListSection.Leave();
	m_IORequestListCondition.Wake();
}

void AsynchronousIOMgr::StopIORequestProc(void)
{
	m_IORequestListSection.Enter();
	m_bStopped = true;
	m_IORequestListSection.Leave();
	m_IORequestListCondition.Wake();
}

HRESULT DeviceRelatedResourceMgr::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	CreateThread();

	ResumeThread();

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
			m_ResourceWeakSet.erase(res_iter++);
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
			m_ResourceWeakSet.erase(res_iter++);
		}
	}
}

void DeviceRelatedResourceMgr::OnDestroyDevice(void)
{
	StopIORequestProc();

	DeviceRelatedObjectBaseWeakPtrSet::iterator res_iter = m_ResourceWeakSet.begin();
	for(; res_iter != m_ResourceWeakSet.end();)
	{
		DeviceRelatedObjectBasePtr res = res_iter->second.lock();
		if(res)
		{
			res->OnDestroyDevice();
			res_iter++;
		}
		else
		{
			m_ResourceWeakSet.erase(res_iter++);
		}
	}

	m_ResourceWeakSet.clear();

	WaitForThreadStopped();
}

void DeviceRelatedResourceMgr::LoadResource(const std::string & key, IORequestPtr request)
{
	DeviceRelatedObjectBaseWeakPtrSet::iterator res_iter = m_ResourceWeakSet.find(key);
	if(res_iter != m_ResourceWeakSet.end())
	{
		DeviceRelatedObjectBasePtr res = res_iter->second.lock();
		if(res)
		{
			request->CallbackAll(res);
			return;
		}
	}

	PushIORequestResource(key, request);
}

void DeviceRelatedResourceMgr::CheckResource(void)
{
	m_IORequestListSection.Enter();
	IORequestPtrPairList::iterator req_iter = m_IORequestList.begin();
	for(; req_iter != m_IORequestList.end(); )
	{
		if(req_iter->second->m_state != IORequest::IORequestStateNone)
		{
			DeviceRelatedObjectBasePtr res = req_iter->second->GetResource(D3DContext::getSingleton().GetD3D9Device());

			m_ResourceWeakSet[req_iter->first] = res;

			req_iter->second->CallbackAll(res);

			req_iter = m_IORequestList.erase(req_iter);
		}
		else
		{
			break;
		}
	}
	m_IORequestListSection.Leave();
}

void DeviceRelatedResourceMgr::LoadTexture(const std::string & path, ResourceCallback callback)
{
	class TextureIOResource : public IORequest
	{
	protected:
		std::string m_path;

		ArchiveDirMgr * m_arc;

		CachePtr m_cache;

	public:
		TextureIOResource(const ResourceCallback & callback, const std::string & path, ArchiveDirMgr * arc)
			: m_path(path)
			, m_arc(arc)
		{
			m_callbacks.push_back(callback);
		}

		virtual void DoLoad(void)
		{
			if(m_arc->CheckArchivePath(m_path))
			{
				m_cache = m_arc->OpenArchiveStream(m_path)->GetWholeCache();
			}
		}

		virtual DeviceRelatedObjectBasePtr GetResource(LPDIRECT3DDEVICE9 pd3dDevice)
		{
			BaseTexturePtr ret;
			if(m_cache)
			{
				D3DXIMAGE_INFO imif;
				if(SUCCEEDED(D3DXGetImageInfoFromFileInMemory(&(*m_cache)[0], m_cache->size(), &imif)))
				{
					switch(imif.ResourceType)
					{
					case D3DRTYPE_TEXTURE:
						ret.reset(new Texture());
						boost::static_pointer_cast<Texture>(ret)->CreateTextureFromFileInMemory(pd3dDevice, &(*m_cache)[0], m_cache->size());
						break;
					case D3DRTYPE_CUBETEXTURE:
						ret.reset(new CubeTexture());
						boost::static_pointer_cast<CubeTexture>(ret)->CreateCubeTextureFromFileInMemory(pd3dDevice, &(*m_cache)[0], m_cache->size());
						break;
					}
				}
			}
			return ret;
		}
	};

	LoadResource(path, IORequestPtr(new TextureIOResource(callback, path, this)));
}

void DeviceRelatedResourceMgr::LoadMesh(const std::string & path, ResourceCallback callback)
{
	class MeshIOResource : public IORequest
	{
	protected:
		std::string m_path;

		ArchiveDirMgr * m_arc;

		CachePtr m_cache;

		rapidxml::xml_document<char> m_doc;

	public:
		MeshIOResource(const ResourceCallback & callback, const std::string & path, ArchiveDirMgr * arc)
			: m_path(path)
			, m_arc(arc)
		{
			m_callbacks.push_back(callback);
		}

		virtual void DoLoad(void)
		{
			if(m_arc->CheckArchivePath(m_path))
			{
				m_cache = m_arc->OpenArchiveStream(m_path)->GetWholeCache();
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

		virtual DeviceRelatedObjectBasePtr GetResource(LPDIRECT3DDEVICE9 pd3dDevice)
		{
			OgreMeshPtr ret;
			if(m_doc.first_node())
			{
				ret.reset(new OgreMesh());
				ret->CreateMeshFromOgreXml(pd3dDevice, &m_doc);
			}
			return ret;
		}
	};

	LoadResource(path, IORequestPtr(new MeshIOResource(callback, path, this)));
}
