#include "stdafx.h"
#include "FModContext.h"
#include "ComponentResMgr.h"
//
//FMOD_RESULT F_CALLBACK myopen(const char *name, int unicode, unsigned int *filesize, void **handle, void **userdata)
//{
//	if (name)
//	{
//		if (ComponentResMgr::getSingleton().CheckPath(name))
//		{
//			my::IStreamPtr istr = ComponentResMgr::getSingleton().OpenIStream(name);
//			_ASSERT(istr);
//			*filesize = istr->GetSize();
//			*userdata = (void *)new my::IStreamPtr(istr);
//			*handle = NULL;
//			return FMOD_OK;
//		}
//	}
//	return FMOD_ERR_FILE_NOTFOUND;
//}
//
//FMOD_RESULT F_CALLBACK myclose(void *handle, void *userdata)
//{
//	_ASSERT(NULL == handle);
//	if (userdata)
//	{
//		delete (my::IStreamPtr *)userdata;
//		return FMOD_OK;
//	}
//	return FMOD_ERR_INVALID_PARAM;
//}
//
//FMOD_RESULT F_CALLBACK myread(void *handle, void *buffer, unsigned int sizebytes, unsigned int *bytesread, void *userdata)
//{
//	_ASSERT(NULL == handle);
//	if (userdata)
//	{
//		my::IStreamPtr istr = *(my::IStreamPtr *)userdata;
//		*bytesread = istr->read(buffer, sizebytes);
//		if (*bytesread < sizebytes)
//		{
//			return FMOD_ERR_FILE_EOF;
//		}
//		return FMOD_OK;
//	}
//	return FMOD_ERR_INVALID_PARAM;
//}
//
//FMOD_RESULT F_CALLBACK myseek(void *handle, unsigned int pos, void *userdata)
//{
//    //printf("************ seek to %d\n", pos);
//    //if (!handle)
//    //{
//    //    return FMOD_ERR_INVALID_PARAM;
//    //}
//   
//    //fseek((FILE *)handle, pos, SEEK_SET);
//
//	_ASSERT(false);
//	//my::IStreamPtr istr = *(my::IStreamPtr *)userdata;
//	//my::ZipIStream * str = (my::ZipIStream *)istr.get();
//	//unzSetOffset(str->m_zFile, pos);
//
//    return FMOD_OK;
//}

bool FModContext::OnInit(void)
{
	result = FMOD::System_Create(&m_FModSystem);
	FMOD_ERRCHECK(result);

	result = m_FModSystem->getVersion(&m_FModVersion);
	FMOD_ERRCHECK(result);

	result = m_FModSystem->init(32, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, 0);
	FMOD_ERRCHECK(result);

    //result = m_FModSystem->setFileSystem(myopen, myclose, myread, myseek, 0, 0, 2048);
    //FMOD_ERRCHECK(result);

	//FMOD::Sound      *sound1, *sound2, *sound3;
	//FMOD::Channel    *channel = 0;
	//result = m_FModSystem->createSound("../demo2_3/Media/sound/drumloop.wav", FMOD_HARDWARE, 0, &sound1);
	//FMOD_ERRCHECK(result);
	//result = sound1->setMode(FMOD_LOOP_NORMAL);    /* drumloop.wav has embedded loop points which automatically makes looping turn on, */
	//FMOD_ERRCHECK(result);                           /* so turn it off here.  We could have also just put FMOD_LOOP_OFF in the above CreateSound call. */
	//result = m_FModSystem->playSound(FMOD_CHANNEL_FREE, sound1, false, &channel);
	//FMOD_ERRCHECK(result);
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
