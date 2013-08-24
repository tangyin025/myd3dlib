#include "stdafx.h"
#include "mySound.h"
#include "myException.h"

using namespace my;

void Sound::CreateSound(void)
{
	LPDIRECTSOUND8 lpsound;
	HRESULT hr;
	if(FAILED(hr = DirectSoundCreate8(NULL, &lpsound, NULL)))
	{
		THROW_DSOUNDEXCEPTION(hr);
	}

	Create(lpsound);
}

void SoundBuffer::CreateSoundBufferFromMmio(
	LPDIRECTSOUND8 pDSound,
	HMMIO hmmio,
	DWORD flags)
{
	MMCKINFO parent;
	MMCKINFO child;
	WAVEFORMATEX wavfmt;

	parent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	if(MMSYSERR_NOERROR != mmioDescend(hmmio, &parent, NULL, MMIO_FINDRIFF))
	{
		mmioClose(hmmio, 0);
		THROW_CUSEXCEPTION(_T("mmioDescend parent failed"));
	}

	child.fccType = mmioFOURCC('f', 'm', 't', ' ');
	if(MMSYSERR_NOERROR != mmioDescend(hmmio, &child, &parent, 0))
	{
		mmioClose(hmmio, 0);
		THROW_CUSEXCEPTION(_T("mmioDescend child failed"));
	}

	if(sizeof(wavfmt) != mmioRead(hmmio, (HPSTR)&wavfmt, sizeof(wavfmt)))
	{
		mmioClose(hmmio, 0);
		THROW_CUSEXCEPTION(_T("mmioRead wav format failed"));
	}

	if(WAVE_FORMAT_PCM != wavfmt.wFormatTag)
	{
		mmioClose(hmmio, 0);
		THROW_CUSEXCEPTION(_T("not wave format pcm"));
	}

	if(MMSYSERR_NOERROR != mmioAscend(hmmio, &child, 0))
	{
		mmioClose(hmmio, 0);
		THROW_CUSEXCEPTION(_T("mmioAscend child failed"));
	}

	child.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if(MMSYSERR_NOERROR != mmioDescend(hmmio, &child, &parent, 0))
	{
		mmioClose(hmmio, 0);
		THROW_CUSEXCEPTION(_T("mmioDescend child failed"));
	}

	DSBUFFERDESC dsbd;
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = flags;
	dsbd.dwBufferBytes = child.cksize;
	dsbd.dwReserved = 0;
	dsbd.lpwfxFormat = &wavfmt;
	dsbd.guid3DAlgorithm = DS3DALG_DEFAULT;

	HRESULT hr;
	LPDIRECTSOUNDBUFFER lpdsbuffer;
	if(FAILED(hr = pDSound->CreateSoundBuffer(&dsbd, &lpdsbuffer, NULL)))
	{
		THROW_DSOUNDEXCEPTION(hr);
	}

	Create(lpdsbuffer);

	unsigned char * buffer1;
	DWORD bytes1;
	unsigned char * buffer2;
	DWORD bytes2;
	Lock(0, child.cksize, (LPVOID *)&buffer1, &bytes1, (LPVOID *)&buffer2, &bytes2, DSBLOCK_ENTIREBUFFER);

	_ASSERT(bytes1 + bytes2 == child.cksize);

	if(buffer1 != NULL && (LONG)bytes1 != mmioRead(hmmio, (HPSTR)buffer1, bytes1))
	{
		mmioClose(hmmio, 0);
		THROW_CUSEXCEPTION(_T("mmioRead wav buffer failed"));
	}

	if(buffer2 != NULL && (LONG)bytes2 != mmioRead(hmmio, (HPSTR)buffer2, bytes2))
	{
		mmioClose(hmmio, 0);
		THROW_CUSEXCEPTION(_T("mmioRead wav buffer failed"));
	}

	Unlock(buffer1, bytes1, buffer2, bytes2);

	mmioClose(hmmio, 0);
}

void SoundBuffer::CreateSoundBufferFromFile(
	LPDIRECTSOUND8 pDSound,
	LPCTSTR pSrcFile,
	DWORD flags)
{
	HMMIO hmmio;
	if(NULL == (hmmio = mmioOpen(const_cast<TCHAR *>(pSrcFile), NULL, MMIO_READ | MMIO_ALLOCBUF)))
	{
		THROW_CUSEXCEPTION(_T("open wave file failed"));
	}

	CreateSoundBufferFromMmio(pDSound, hmmio, flags);
}

void SoundBuffer::CreateSoundBufferFromFileInMemory(
	LPDIRECTSOUND8 pDSound,
	LPCVOID pSrcData,
	UINT SrcDataLen,
	DWORD flags)
{
	struct IOProc
	{
		static LRESULT CALLBACK MMIOProc(
			LPSTR lpmmioinfo,  
			UINT uMsg,         
			LONG lParam1,      
			LONG lParam2)
		{
			MMIOINFO * pinfo = (MMIOINFO *)lpmmioinfo;
			switch(uMsg)
			{
			case MMIOM_OPEN:
			case MMIOM_CLOSE:
				return 0;

			case MMIOM_READ:
				memcpy((void *)lParam1, pinfo->pchBuffer + pinfo->lDiskOffset, lParam2);
				pinfo->lDiskOffset += lParam2;
				return lParam2;
			}
			return -1;
		}
	};

	MMIOINFO mmioinfo = {0};
	mmioinfo.dwFlags = MMIO_READ;
	mmioinfo.pIOProc = IOProc::MMIOProc;
	mmioinfo.cchBuffer = SrcDataLen;
	mmioinfo.pchBuffer = (HPSTR)pSrcData;

	HMMIO hmmio;
	if(NULL == (hmmio = mmioOpenA(NULL, &mmioinfo, MMIO_READ)))
	{
		THROW_CUSEXCEPTION(_T("open wave file failed"));
	}

	CreateSoundBufferFromMmio(pDSound, hmmio, flags);
}

Sound3DBufferPtr SoundBuffer::Get3DBuffer(void)
{
	LPDIRECTSOUND3DBUFFER lp3dbuffer = NULL;
	if(FAILED(hr = m_ptr->QueryInterface(IID_IDirectSound3DBuffer, (LPVOID *)&lp3dbuffer)))
	{
		THROW_DSOUNDEXCEPTION(hr);
	}
	return Sound3DBufferPtr(new Sound3DBuffer(lp3dbuffer));
}

Sound3DListenerPtr SoundBuffer::Get3DListener(void)
{
	LPDIRECTSOUND3DLISTENER lp3dlistener = NULL;
	if(FAILED(hr = m_ptr->QueryInterface(IID_IDirectSound3DListener, (LPVOID *)&lp3dlistener)))
	{
		THROW_DSOUNDEXCEPTION(hr);
	}
	return Sound3DListenerPtr(new Sound3DListener(lp3dlistener));
}
