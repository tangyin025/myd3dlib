#include "stdafx.h"
#include "myResource.h"
#include "libc.h"

using namespace my;

ZipArchiveStream::ZipArchiveStream(unzFile zFile)
	: m_zFile(zFile)
{
	_ASSERT(NULL != m_zFile);

	int ret = unzGetCurrentFileInfo(m_zFile, &m_zFileInfo, NULL, 0, NULL, 0, NULL, 0);
	if(UNZ_OK != ret)
	{
		unzClose(m_zFile);
		THROW_CUSEXCEPTION("cannot get file info from zip file");
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
		THROW_CUSEXCEPTION("read zip file cache failed");
	}

	int ret = read(&(*cache)[0], cache->size());
	if(ret != cache->size())
	{
		THROW_CUSEXCEPTION("read zip file cache failed");
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
		THROW_CUSEXCEPTION("read file cache failed");
	}

	fseek(m_fp, 0, SEEK_SET);
	int ret = read(&(*cache)[0], cache->size());
	if(ret != cache->size())
	{
		THROW_CUSEXCEPTION("read file cache failed");
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
		THROW_CUSEXCEPTION(str_printf("cannot open zip archive: %s", m_dir.c_str()));
	}

	int ret = unzLocateFile(zFile, ReplaceBackslash(path).c_str(), 0);
	if(UNZ_OK != ret)
	{
		unzClose(zFile);
		THROW_CUSEXCEPTION(str_printf("cannot open zip file: %s", path.c_str()));
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
		THROW_CUSEXCEPTION(str_printf("cannot open zip file: %s", path.c_str()));
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
		THROW_CUSEXCEPTION(str_printf("cannot open file archive: %s", path.c_str()));
	}

	FILE * fp;
	if(0 != fopen_s(&fp, fullPath.c_str(), "rb"))
	{
		THROW_CUSEXCEPTION(str_printf("cannot open file archive: %s", path.c_str()));
	}

	return ArchiveStreamPtr(new FileArchiveStream(fp));
}

ArchiveDirMgr::ArchiveDirMgr(void)
{
}

ArchiveDirMgr::~ArchiveDirMgr(void)
{
}

void ArchiveDirMgr::RegisterZipArchive(const std::string & zip_path)
{
	m_dirMap[zip_path] = ResourceDirPtr(new ZipArchiveDir(zip_path));
}

void ArchiveDirMgr::RegisterZipArchive(const std::string & zip_path, const std::string & password)
{
	m_dirMap[zip_path] = ResourceDirPtr(new ZipArchiveDir(zip_path, password));
}

void ArchiveDirMgr::RegisterFileDir(const std::string & dir)
{
	m_dirMap[dir] = ResourceDirPtr(new FileArchiveDir(dir));
}

bool ArchiveDirMgr::CheckArchivePath(const std::string & path)
{
	ResourceDirPtrMap::iterator dir_iter = m_dirMap.begin();
	for(; dir_iter != m_dirMap.end(); dir_iter++)
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

	ResourceDirPtrMap::iterator dir_iter = m_dirMap.begin();
	for(; dir_iter != m_dirMap.end(); dir_iter++)
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
			THROW_CUSEXCEPTION(str_printf("cannot open file archive: %s", path.c_str()));
		}

		return ArchiveStreamPtr(new FileArchiveStream(fp));
	}

	ResourceDirPtrMap::iterator dir_iter = m_dirMap.begin();
	for(; dir_iter != m_dirMap.end(); dir_iter++)
	{
		if(dir_iter->second->CheckArchivePath(path))
		{
			return dir_iter->second->OpenArchiveStream(path);
		}
	}

	THROW_CUSEXCEPTION(str_printf("cannot find specified file: %s", path.c_str()));
}

HRESULT ResourceMgr::Open(
	D3DXINCLUDE_TYPE IncludeType,
	LPCSTR pFileName,
	LPCVOID pParentData,
	LPCVOID * ppData,
	UINT * pBytes)
{
	CachePtr cache;
	std::string loc_path;
	loc_path.resize(MAX_PATH);
	PathCombineA(&loc_path[0], m_EffectInclude.c_str(), pFileName);
	switch(IncludeType)
	{
	case D3DXINC_SYSTEM:
	case D3DXINC_LOCAL:
		if(CheckArchivePath(loc_path))
		{
			cache = OpenArchiveStream(loc_path)->GetWholeCache();
			*ppData = &(*cache)[0];
			*pBytes = cache->size();
			_ASSERT(m_cacheSet.end() == m_cacheSet.find(*ppData));
			m_cacheSet[*ppData] = cache;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT ResourceMgr::Close(
	LPCVOID pData)
{
	_ASSERT(m_cacheSet.end() != m_cacheSet.find(pData));
	m_cacheSet.erase(m_cacheSet.find(pData));
	return S_OK;
}

HRESULT ResourceMgr::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	m_Device = pd3dDevice;

	HRESULT hr;
	if(FAILED(hr = D3DXCreateEffectPool(&m_EffectPool)))
	{
		THROW_D3DEXCEPTION(hr);
	}

	return S_OK;
}

HRESULT ResourceMgr::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	DeviceRelatedResourceSet::iterator res_iter = m_resourceSet.begin();
	for(; res_iter != m_resourceSet.end();)
	{
		boost::shared_ptr<DeviceRelatedObjectBase> res = res_iter->second.lock();
		if(res)
		{
			res->OnResetDevice();
			res_iter++;
		}
		else
		{
			m_resourceSet.erase(res_iter++);
		}
	}

	return S_OK;
}

void ResourceMgr::OnLostDevice(void)
{
	DeviceRelatedResourceSet::iterator res_iter = m_resourceSet.begin();
	for(; res_iter != m_resourceSet.end();)
	{
		boost::shared_ptr<DeviceRelatedObjectBase> res = res_iter->second.lock();
		if(res)
		{
			res->OnLostDevice();
			res_iter++;
		}
		else
		{
			m_resourceSet.erase(res_iter++);
		}
	}
}

void ResourceMgr::OnDestroyDevice(void)
{
	DeviceRelatedResourceSet::iterator res_iter = m_resourceSet.begin();
	for(; res_iter != m_resourceSet.end();)
	{
		boost::shared_ptr<DeviceRelatedObjectBase> res = res_iter->second.lock();
		if(res)
		{
			res->OnDestroyDevice();
			res_iter++;
		}
		else
		{
			m_resourceSet.erase(res_iter++);
		}
	}

	m_resourceSet.clear();

	m_EffectPool.Release();

	m_Device.Release();
}

TexturePtr ResourceMgr::LoadTexture(const std::string & path, bool reload)
{
	TexturePtr ret = GetDeviceRelatedResource<Texture>(path, reload);
	if(!ret->m_ptr)
	{
		std::string full_path = GetFullPath(path);
		if(!full_path.empty())
		{
			ret->CreateTextureFromFile(m_Device, ms2ts(full_path.c_str()).c_str());
		}
		else
		{
			CachePtr cache = OpenArchiveStream(path)->GetWholeCache();
			ret->CreateTextureFromFileInMemory(m_Device, &(*cache)[0], cache->size());
		}
	}
	return ret;
}

CubeTexturePtr ResourceMgr::LoadCubeTexture(const std::string & path, bool reload)
{
	CubeTexturePtr ret = GetDeviceRelatedResource<CubeTexture>(path, reload);
	if(!ret->m_ptr)
	{
		std::string full_path = GetFullPath(path);
		if(!full_path.empty())
		{
			ret->CreateCubeTextureFromFile(m_Device, ms2ts(full_path.c_str()).c_str());
		}
		else
		{
			CachePtr cache = OpenArchiveStream(path)->GetWholeCache();
			ret->CreateCubeTextureFromFileInMemory(m_Device, &(*cache)[0], cache->size());
		}
	}
	return ret;
}

OgreMeshPtr ResourceMgr::LoadMesh(const std::string & path, bool reload)
{
	OgreMeshPtr ret = GetDeviceRelatedResource<OgreMesh>(path, reload);
	if(!ret->m_ptr)
	{
		std::string full_path = GetFullPath(path);
		if(!full_path.empty())
		{
			ret->CreateMeshFromOgreXml(m_Device, full_path.c_str(), true);
		}
		else
		{
			CachePtr cache = OpenArchiveStream(path)->GetWholeCache();
			ret->CreateMeshFromOgreXmlInMemory(m_Device, (char *)&(*cache)[0], cache->size(), true);
		}
	}
	return ret;
}

OgreSkeletonAnimationPtr ResourceMgr::LoadSkeleton(const std::string & path, bool reload)
{
	OgreSkeletonAnimationPtr ret = GetDeviceRelatedResource<OgreSkeletonAnimation>(path, reload);
	if(ret->m_boneHierarchy.empty())
	{
		std::string full_path = GetFullPath(path);
		if(!full_path.empty())
		{
			ret->CreateOgreSkeletonAnimationFromFile(ms2ts(full_path.c_str()).c_str());
		}
		else
		{
			CachePtr cache = OpenArchiveStream(path)->GetWholeCache();
			ret->CreateOgreSkeletonAnimation((char *)&(*cache)[0], cache->size());
		}
	}
	return ret;
}

EffectPtr ResourceMgr::LoadEffect(const std::string & path, bool reload)
{
	EffectPtr ret = GetDeviceRelatedResource<Effect>(path, reload);
	if(!ret->m_ptr)
	{
		m_EffectInclude = ZipArchiveDir::ReplaceSlash(path);
		PathRemoveFileSpecA(&m_EffectInclude[0]);
		std::string full_path = GetFullPath(path);
		if(!full_path.empty())
		{
			ret->CreateEffectFromFile(m_Device, ms2ts(full_path.c_str()).c_str(), NULL, NULL, 0, m_EffectPool);
		}
		else
		{
			CachePtr cache = OpenArchiveStream(path)->GetWholeCache();
			ret->CreateEffect(m_Device, &(*cache)[0], cache->size(), NULL, this, 0, m_EffectPool);
		}
	}
	return ret;
}

FontPtr ResourceMgr::LoadFont(const std::string & path, int height, bool reload)
{
	FontPtr ret = GetDeviceRelatedResource<Font>(str_printf("%s, %d", path.c_str(), height), reload);
	if(!ret->m_face)
	{
		std::string full_path = GetFullPath(path);
		if(!full_path.empty())
		{
			ret->CreateFontFromFile(m_Device, full_path.c_str(), height);
		}
		else
		{
			CachePtr cache = OpenArchiveStream(path)->GetWholeCache();
			ret->CreateFontFromFileInCache(m_Device, cache, height);
		}
	}
	return ret;
}
