
#include "myResource.h"
#include "myException.h"
#include "libc.h"

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

	ZipArchiveDir::ZipArchiveDir(ZZIP_DIR * dir)
		: m_dir(dir)
	{
	}

	ZipArchiveDir::~ZipArchiveDir(void)
	{
		zzip_dir_close(m_dir);
	}

	bool ZipArchiveDir::CheckArchivePath(const std::basic_string<_TCHAR> & path)
	{
		ZZIP_FILE * fp = zzip_file_open(m_dir, tstringToMString(path).c_str(), ZZIP_CASEINSENSITIVE);
		if(NULL == fp)
		{
			return false;
		}

		zzip_file_close(fp);
		return true;
	}

	ArchiveStreamPtr ZipArchiveDir::OpenArchiveStream(const std::basic_string<_TCHAR> & path)
	{
		ZZIP_FILE * fp = zzip_file_open(m_dir, tstringToMString(path).c_str(), ZZIP_CASEINSENSITIVE);
		if(NULL == fp)
		{
			THROW_CUSEXCEPTION(str_printf(_T("cannot open zip file: %s"), path.c_str()));
		}

		return ArchiveStreamPtr(new ZipArchiveStream(fp));
	}

	Singleton<ResourceMgr>::DrivedClassPtr ResourceMgr::s_ptr;

	void ResourceMgr::RegisterZipArchive(const std::basic_string<_TCHAR> & path)
	{
		zzip_error_t rv;
		ZZIP_DIR * fd = zzip_dir_open(tstringToMString(path).c_str(), &rv);
		if(NULL == fd)
		{
			THROW_CUSEXCEPTION(str_printf(_T("cannot open zip archive: %s"), path.c_str()));
		}

		m_dirList.push_back(ResourceDirPtr(new ZipArchiveDir(fd)));
	}

	ArchiveStreamPtr ResourceMgr::OpenArchiveStream(const std::basic_string<_TCHAR> & path)
	{
		ResourceDirPtrList::iterator dir_iter = m_dirList.begin();
		for(; dir_iter != m_dirList.end(); dir_iter++)
		{
			if((*dir_iter)->CheckArchivePath(path))
			{
				return (*dir_iter)->OpenArchiveStream(path);
			}
		}

		THROW_CUSEXCEPTION(str_printf(_T("cannot find specified file: %s"), path.c_str()));
	}

	ArchiveCachePtr ReadWholeCacheFromStream(ArchiveStreamPtr stream)
	{
		stream->Seek(0, my::ArchiveStream::END);
		long len = stream->Tell();
		stream->Seek(0, my::ArchiveStream::SET);
		ArchiveCachePtr cache(new ArchiveCache(len));
		size_t ret_size = stream->Read(&(*cache)[0], sizeof(ArchiveCache::value_type), cache->size());
		_ASSERT(ret_size == cache->size());
		return cache;
	}
};
