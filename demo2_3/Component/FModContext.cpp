#include "stdafx.h"
#include "FModContext.h"
#include <myResource.h>

FMOD_RESULT F_CALLBACK myopen(const char *name, int unicode, unsigned int *filesize, void **handle, void **userdata)
{
	if (name)
	{
		if (FModContext::GetResourceMgr()->CheckPath(name))
		{
			my::IStreamPtr istr = FModContext::GetResourceMgr()->OpenIStream(name);
			_ASSERT(istr);
			*filesize = istr->GetSize();
			*userdata = (void *)new my::IStreamPtr(istr);
			*handle = NULL;
			return FMOD_OK;
		}
	}
	return FMOD_ERR_FILE_NOTFOUND;
}

FMOD_RESULT F_CALLBACK myclose(void *handle, void *userdata)
{
	_ASSERT(NULL == handle);
	if (userdata)
	{
		delete (my::IStreamPtr *)userdata;
		return FMOD_OK;
	}
	return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK myread(void *handle, void *buffer, unsigned int sizebytes, unsigned int *bytesread, void *userdata)
{
	_ASSERT(NULL == handle);
	if (userdata)
	{
		my::IStreamPtr istr = *(my::IStreamPtr *)userdata;
		*bytesread = istr->read(buffer, sizebytes);
		if (*bytesread < sizebytes)
		{
			return FMOD_ERR_FILE_EOF;
		}
		return FMOD_OK;
	}
	return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK myseek(void *handle, unsigned int pos, void *userdata)
{
	_ASSERT(NULL == handle);
	if (userdata)
	{
		my::IStreamPtr istr = *(my::IStreamPtr *)userdata;
		istr->seek(pos);
		return FMOD_OK;
	}
    return FMOD_ERR_INVALID_PARAM;
}

bool FModContext::OnInit(void)
{
	result = FMOD::System_Create(&m_FModSystem);
	FMOD_ERRCHECK(result);

	result = m_FModSystem->getVersion(&m_FModVersion);
	FMOD_ERRCHECK(result);

	result = m_FModSystem->init(32, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, 0);
	FMOD_ERRCHECK(result);

    result = m_FModSystem->setFileSystem(myopen, myclose, myread, myseek, 0, 0, 2048);
    FMOD_ERRCHECK(result);

	return true;
}

void FModContext::OnShutdown(void)
{
	result = m_FModSystem->close();
	FMOD_ERRCHECK(result);

	result = m_FModSystem->release();
	FMOD_ERRCHECK(result);

	m_FModSystem = NULL;
}
