
#include "stdafx.h"
#include "myd3dlib.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

namespace my
{
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

	CachePtr ZipArchiveStream::GetWholeCache(void)
	{
		CachePtr cache(new Cache(m_zFileInfo.uncompressed_size));
		if(0 == cache->size())
		{
			THROW_CUSEXCEPTION("read zip file cache failed");
		}

		int ret = unzReadCurrentFile(m_zFile, &(*cache)[0], cache->size());
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
		size_t ret = fread(&(*cache)[0], 1, cache->size(), m_fp);
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

	ZipArchiveDir::ZipArchiveDir(const std::string & dir)
		: ResourceDir(dir)
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

	ArchiveStreamPtr ZipArchiveDir::OpenArchiveStream(const std::string & path, const std::string & password)
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

		if(!password.empty())
		{
			ret = unzOpenCurrentFilePassword(zFile, password.c_str());
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

	FileArchiveDir::FileArchiveDir(const std::string & dir)
		: ResourceDir(dir)
	{
	}

	bool FileArchiveDir::CheckArchivePath(const std::string & path)
	{
		return !GetFullPath(path).empty();
	}

	ArchiveStreamPtr FileArchiveDir::OpenArchiveStream(const std::string & path, const std::string & password)
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

	Singleton<ResourceMgr>::DrivedClassPtr ResourceMgr::s_ptr;

	void ResourceMgr::RegisterZipArchive(const std::string & zip_path)
	{
		m_dirList.push_back(ResourceDirPtr(new ZipArchiveDir(zip_path)));
	}

	void ResourceMgr::RegisterFileDir(const std::string & dir)
	{
		m_dirList.push_back(ResourceDirPtr(new FileArchiveDir(dir)));
	}

	ArchiveStreamPtr ResourceMgr::OpenArchiveStream(const std::string & path, const std::string & password /*= ""*/)
	{
		ResourceDirPtrList::iterator dir_iter = m_dirList.begin();
		for(; dir_iter != m_dirList.end(); dir_iter++)
		{
			if((*dir_iter)->CheckArchivePath(path))
			{
				return (*dir_iter)->OpenArchiveStream(path, password);
			}
		}

		THROW_CUSEXCEPTION(str_printf("cannot find specified file: %s", path.c_str()));
	}
};
