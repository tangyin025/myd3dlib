
#include "stdafx.h"
#include "myd3dlib.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

namespace my
{
	int ZipArchiveStream::ToZipSeekType(SeekType origin)
	{
		switch(origin)
		{
		case CUR:
			return SEEK_CUR;
		case END:
			return SEEK_END;
		case SET:
			return SEEK_SET;
		}

		_ASSERT("unkwon seek type"); return 0;
	}

	ZipArchiveStream::ZipArchiveStream(ZZIP_FILE * fp)
		: m_fp(fp)
	{
		_ASSERT(NULL != m_fp);
	}

	ZipArchiveStream::~ZipArchiveStream(void)
	{
		zzip_file_close(m_fp);
	}

	size_t ZipArchiveStream::Read(void * buffer, size_t size, size_t count)
	{
		return zzip_file_read(m_fp, buffer, size * count);
	}

	size_t ZipArchiveStream::Write(void * buffer, size_t size, size_t count)
	{
		_ASSERT("unsupport writing zip stream"); return 0;
	}

	long ZipArchiveStream::Seek(long offset, SeekType origin)
	{
		return zzip_seek(m_fp, offset, ToZipSeekType(origin));
	}

	long ZipArchiveStream::Tell(void)
	{
		return zzip_tell(m_fp);
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

	size_t FileArchiveStream::Read(void * buffer, size_t size, size_t count)
	{
		return fread(buffer, size, count, m_fp);
	}

	size_t FileArchiveStream::Write(void * buffer, size_t size, size_t count)
	{
		return fwrite(buffer, size, count, m_fp);
	}

	long FileArchiveStream::Seek(long offset, SeekType origin)
	{
		return fseek(m_fp, offset, ZipArchiveStream::ToZipSeekType(origin));
	}

	long FileArchiveStream::Tell(void)
	{
		return ftell(m_fp);
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
		zzip_error_t rv;
		ZZIP_DIR * zdir = zzip_dir_open(m_dir.c_str(), &rv);
		if(NULL == zdir)
		{
			return false;
		}

		ZZIP_FILE * zfile = zzip_file_open(zdir, path.c_str(), ZZIP_CASEINSENSITIVE);
		if(NULL == zfile)
		{
			zzip_dir_close(zdir);
			return false;
		}

		zzip_dir_close(zdir);
		zzip_file_close(zfile);
		return true;
	}

	ArchiveStreamPtr ZipArchiveDir::OpenArchiveStream(const std::string & path)
	{
		zzip_error_t rv;
		ZZIP_DIR * zdir = zzip_dir_open(m_dir.c_str(), &rv);
		if(NULL == zdir)
		{
			THROW_CUSEXCEPTION(str_printf("cannot open zip archive: %s", m_dir.c_str()));
		}

		ZZIP_FILE * zfile = zzip_file_open(zdir, path.c_str(), ZZIP_CASEINSENSITIVE);
		if(NULL == zfile)
		{
			zzip_dir_close(zdir);
			THROW_CUSEXCEPTION(str_printf("cannot open zip file: %s", path.c_str()));
		}

		zzip_dir_close(zdir);
		return ArchiveStreamPtr(new ZipArchiveStream(zfile));
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

	Singleton<ResourceMgr>::DrivedClassPtr ResourceMgr::s_ptr;

	void ResourceMgr::RegisterZipArchive(const std::string & zip_path)
	{
		m_dirList.push_back(ResourceDirPtr(new ZipArchiveDir(zip_path)));
	}

	void ResourceMgr::RegisterFileDir(const std::string & dir)
	{
		m_dirList.push_back(ResourceDirPtr(new FileArchiveDir(dir)));
	}

	ArchiveStreamPtr ResourceMgr::OpenArchiveStream(const std::string & path)
	{
		ResourceDirPtrList::iterator dir_iter = m_dirList.begin();
		for(; dir_iter != m_dirList.end(); dir_iter++)
		{
			if((*dir_iter)->CheckArchivePath(path))
			{
				return (*dir_iter)->OpenArchiveStream(path);
			}
		}

		THROW_CUSEXCEPTION(str_printf("cannot find specified file: %s", path.c_str()));
	}

	CachePtr ReadWholeCacheFromStream(ArchiveStreamPtr stream)
	{
		stream->Seek(0, my::ArchiveStream::END);
		long len = stream->Tell();
		stream->Seek(0, my::ArchiveStream::SET);
		CachePtr cache(new Cache(len));
		size_t ret_size = stream->Read(&(*cache)[0], sizeof(Cache::value_type), cache->size());
		_ASSERT(ret_size == cache->size());
		return cache;
	}
};
