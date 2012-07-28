
#include "stdafx.h"
#include "myResource.h"
#include "libc.h"
#include "myDxutApp.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

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

ResourceDir::ResourceDir(const std::string & dir)
	: m_dir(dir)
{
}

ResourceDir::~ResourceDir(void)
{
}

ZipArchiveDir::ZipArchiveDir(const std::string & dir, const std::string & password)
	: ResourceDir(dir)
	, m_password(password)
{
}

bool ZipArchiveDir::CheckArchivePath(const std::string & path)
{
	unzFile zFile = unzOpen(m_dir.c_str());
	if(NULL == zFile)
	{
		return false;
	}

	int ret = unzLocateFile(zFile, path.c_str(), 0);
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

	int ret = unzLocateFile(zFile, path.c_str(), 0);
	if(UNZ_OK != ret)
	{
		unzClose(zFile);
		THROW_CUSEXCEPTION(str_printf("cannot open zip file: %s", path.c_str()));
	}

	if(!m_password.empty())
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

FileArchiveDir::FileArchiveDir(const std::string & dir)
	: ResourceDir(dir)
{
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

ResourceMgrBase::ResourceMgrBase(void)
{
}

ResourceMgrBase::~ResourceMgrBase(void)
{
}

void ResourceMgrBase::RegisterZipArchive(const std::string & zip_path, const std::string & password)
{
	m_dirMap[zip_path] = ResourceDirPtr(new ZipArchiveDir(zip_path, password));
}

void ResourceMgrBase::RegisterFileDir(const std::string & dir)
{
	m_dirMap[dir] = ResourceDirPtr(new FileArchiveDir(dir));
}

bool ResourceMgrBase::CheckArchivePath(const std::string & path)
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

std::string ResourceMgrBase::GetFullPath(const std::string & path)
{
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

ArchiveStreamPtr ResourceMgrBase::OpenArchiveStream(const std::string & path)
{
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

ResourceMgr::DrivedClassPtr Singleton<ResourceMgr>::s_ptr;

ResourceMgr::ResourceMgr(void)
{
	FT_Error err = FT_Init_FreeType(&m_library);
	if(err)
	{
		THROW_CUSEXCEPTION("FT_Init_FreeType failed");
	}
}

ResourceMgr::~ResourceMgr(void)
{
	FT_Error err = FT_Done_FreeType(m_library);
}

void ResourceMgr::OnResetDevice(void)
{
	LPDIRECT3DDEVICE9 pd3dDevice = DxutApp::getSingleton().GetD3D9Device();

	_ASSERT(NULL != pd3dDevice);

	HRESULT hres = pd3dDevice->CreateStateBlock(D3DSBT_ALL, &m_stateBlock);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	WeakDeviceRelatedObjectBasePtrSet::iterator obj_iter = m_DeviceRelatedObjs.begin();
	for(; obj_iter != m_DeviceRelatedObjs.end(); )
	{
		DeviceRelatedObjectBasePtr obj_ptr = obj_iter->lock();
		if(obj_ptr)
		{
			obj_ptr->OnResetDevice();
			obj_iter++;
		}
		else
		{
			m_DeviceRelatedObjs.erase(obj_iter++);
		}
	}
}

void ResourceMgr::OnLostDevice(void)
{
	m_stateBlock.Release();

	WeakDeviceRelatedObjectBasePtrSet::iterator obj_iter = m_DeviceRelatedObjs.begin();
	for(; obj_iter != m_DeviceRelatedObjs.end(); )
	{
		DeviceRelatedObjectBasePtr obj_ptr = obj_iter->lock();
		if(obj_ptr)
		{
			obj_ptr->OnLostDevice();
			obj_iter++;
		}
		else
		{
			m_DeviceRelatedObjs.erase(obj_iter++);
		}
	}
}

void ResourceMgr::OnDestroyDevice(void)
{
	//m_ControlFocus.reset();

	WeakDeviceRelatedObjectBasePtrSet::iterator obj_iter = m_DeviceRelatedObjs.begin();
	for(; obj_iter != m_DeviceRelatedObjs.end(); )
	{
		DeviceRelatedObjectBasePtr obj_ptr = obj_iter->lock();
		if(obj_ptr)
		{
			obj_ptr->OnDestroyDevice();
			obj_iter++;
		}
		else
		{
			m_DeviceRelatedObjs.erase(obj_iter++);
		}
	}
}

HRESULT IncludeFromResource::Open(
	D3DXINCLUDE_TYPE IncludeType,
	LPCSTR pFileName,
	LPCVOID pParentData,
	LPCVOID * ppData,
	UINT * pBytes)
{
	switch(IncludeType)
	{
	case D3DXINC_SYSTEM:
		if(CheckArchivePath(pFileName))
		{
			m_cache = OpenArchiveStream(pFileName)->GetWholeCache();
			*ppData = &(*m_cache)[0];
			*pBytes = m_cache->size();
			return S_OK;
		}

	case D3DXINC_LOCAL:
		if(ResourceMgr::getSingleton().CheckArchivePath(pFileName))
		{
			m_cache = ResourceMgr::getSingleton().OpenArchiveStream(pFileName)->GetWholeCache();
			*ppData = &(*m_cache)[0];
			*pBytes = m_cache->size();
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT IncludeFromResource::Close(
	LPCVOID pData)
{
	_ASSERT(m_cache && pData == &(*m_cache)[0]);
	m_cache.reset();
	return S_OK;
}
