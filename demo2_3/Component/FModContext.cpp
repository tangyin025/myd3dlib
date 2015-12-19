#include "stdafx.h"
#include "FModContext.h"

#define ERRCHECK(result) if ((result) != FMOD_OK) { \
	throw my::CustomException(str_printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result)), __FILE__, __LINE__); }

static FMOD_RESULT F_CALLBACK myopen(const char *name, int unicode, unsigned int *filesize, void **handle, void **userdata)
{
	if (name)
	{
		if (my::ResourceMgr::getSingleton().CheckPath(name))
		{
			my::IStreamPtr istr = my::ResourceMgr::getSingleton().OpenIStream(name);
			_ASSERT(istr);
			*filesize = istr->GetSize();
			*userdata = (void *)new my::IStreamPtr(istr);
			*handle = NULL;
			return FMOD_OK;
		}
	}
	return FMOD_ERR_FILE_NOTFOUND;
}

static FMOD_RESULT F_CALLBACK myclose(void *handle, void *userdata)
{
	_ASSERT(NULL == handle);
	if (userdata)
	{
		delete (my::IStreamPtr *)userdata;
		return FMOD_OK;
	}
	return FMOD_ERR_INVALID_PARAM;
}

static FMOD_RESULT F_CALLBACK myread(void *handle, void *buffer, unsigned int sizebytes, unsigned int *bytesread, void *userdata)
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

static FMOD_RESULT F_CALLBACK myseek(void *handle, unsigned int pos, void *userdata)
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
    ERRCHECK(result = FMOD::EventSystem_Create(&m_EventSystem));
    ERRCHECK(result = m_EventSystem->init(64, FMOD_INIT_NORMAL, 0, FMOD_EVENT_INIT_NORMAL));
	FMOD::System * system;
	ERRCHECK(result = m_EventSystem->getSystemObject(&system));
    ERRCHECK(result = system->setFileSystem(myopen, myclose, myread, myseek, 0, 0, 2048));
	return true;
}

void FModContext::OnUpdate(void)
{
	ERRCHECK(m_EventSystem->update());
}

void FModContext::OnShutdown(void)
{
	ERRCHECK(result = m_EventSystem->release());
	m_EventSystem = NULL;
}

void FModContext::SetMediaPath(const char * path)
{
	ERRCHECK(result = m_EventSystem->setMediaPath(path));
}

void FModContext::LoadEventFile(const char * file)
{
	ERRCHECK(result = m_EventSystem->load(file, 0, 0));
}

void FModContext::PlaySound(const char * name)
{
	FMOD::Event       *event;
    ERRCHECK(result = m_EventSystem->getEvent(name, FMOD_EVENT_DEFAULT, &event));
	ERRCHECK(result = event->start());
}
