#include "stdafx.h"
#include "mySound.h"
#include "myException.h"

using namespace my;

Sound::Sound(void)
	: m_ptr(NULL)
{
}

Sound::~Sound(void)
{
	SAFE_RELEASE(m_ptr);
}

void Sound::Create(IDirectSound8 * ptr)
{
	_ASSERT(!m_ptr);

	m_ptr = ptr;
}

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

void Sound::CreateSoundBuffer(
	LPCDSBUFFERDESC pcDSBufferDesc,
	LPDIRECTSOUNDBUFFER * ppDSBuffer)
{
	V(m_ptr->CreateSoundBuffer(pcDSBufferDesc, ppDSBuffer, NULL));
}

DSCAPS Sound::GetCaps(void)
{
	DSCAPS res;
	V(m_ptr->GetCaps(&res));
	return res;
}

void Sound::SetCooperativeLevel(
	HWND hwnd,
	DWORD dwLevel)
{
	V(m_ptr->SetCooperativeLevel(hwnd, dwLevel));
}

SoundBuffer::SoundBuffer(void)
	: m_ptr(NULL)
{
}

SoundBuffer::~SoundBuffer(void)
{
	SAFE_RELEASE(m_ptr);
}

void SoundBuffer::Create(IDirectSoundBuffer * ptr)
{
	_ASSERT(!m_ptr);

	m_ptr = ptr;
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
		THROW_CUSEXCEPTION("mmioDescend parent failed");
	}

	child.fccType = mmioFOURCC('f', 'm', 't', ' ');
	if(MMSYSERR_NOERROR != mmioDescend(hmmio, &child, &parent, 0))
	{
		mmioClose(hmmio, 0);
		THROW_CUSEXCEPTION("mmioDescend child failed");
	}

	if(sizeof(wavfmt) != mmioRead(hmmio, (HPSTR)&wavfmt, sizeof(wavfmt)))
	{
		mmioClose(hmmio, 0);
		THROW_CUSEXCEPTION("mmioRead wav format failed");
	}

	if(WAVE_FORMAT_PCM != wavfmt.wFormatTag)
	{
		mmioClose(hmmio, 0);
		THROW_CUSEXCEPTION("not wave format pcm");
	}

	if(MMSYSERR_NOERROR != mmioAscend(hmmio, &child, 0))
	{
		mmioClose(hmmio, 0);
		THROW_CUSEXCEPTION("mmioAscend child failed");
	}

	child.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if(MMSYSERR_NOERROR != mmioDescend(hmmio, &child, &parent, 0))
	{
		mmioClose(hmmio, 0);
		THROW_CUSEXCEPTION("mmioDescend child failed");
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
		THROW_CUSEXCEPTION("mmioRead wav buffer failed");
	}

	if(buffer2 != NULL && (LONG)bytes2 != mmioRead(hmmio, (HPSTR)buffer2, bytes2))
	{
		mmioClose(hmmio, 0);
		THROW_CUSEXCEPTION("mmioRead wav buffer failed");
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
		THROW_CUSEXCEPTION("open wave file failed");
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
		THROW_CUSEXCEPTION("open wave file failed");
	}

	CreateSoundBufferFromMmio(pDSound, hmmio, flags);
}

void SoundBuffer::GetCurrentPosition(
	LPDWORD pdwCurrentPlayCursor,
	LPDWORD pdwCurrentWriteCursor)
{
	V(m_ptr->GetCurrentPosition(pdwCurrentPlayCursor, pdwCurrentWriteCursor));
}

void SoundBuffer::Lock(
	DWORD dwOffset,
	DWORD dwBytes,
	LPVOID * ppvAudioPtr1,
	LPDWORD  pdwAudioBytes1,
	LPVOID * ppvAudioPtr2,
	LPDWORD pdwAudioBytes2,
	DWORD dwFlags)
{
	V(m_ptr->Lock(dwOffset, dwBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags));
}

void SoundBuffer::Play(
	DWORD dwPriority,
	DWORD dwFlags)
{
	V(m_ptr->Play(0, dwPriority, dwFlags));
}

void SoundBuffer::SetCurrentPosition(
	DWORD dwNewPosition)
{
	V(m_ptr->SetCurrentPosition(dwNewPosition));
}

void SoundBuffer::Stop(void)
{
	V(m_ptr->Stop());
}

void SoundBuffer::Unlock(
	LPVOID pvAudioPtr1,
	DWORD dwAudioBytes1,
	LPVOID pvAudioPtr2,
	DWORD dwAudioBytes2)
{
	V(m_ptr->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2));
}

DWORD SoundBuffer::GetFrequency(void)
{
	DWORD res;
	V(m_ptr->GetFrequency(&res));
	return res;
}

LONG SoundBuffer::GetPan(void)
{
	LONG res;
	V(m_ptr->GetPan(&res));
	return res;
}

LONG SoundBuffer::GetVolume(void)
{
	LONG res;
	V(m_ptr->GetVolume(&res));
	return res;
}

void SoundBuffer::SetFrequency(
	DWORD dwFrequency)
{
	V(m_ptr->SetFrequency(dwFrequency));
}

void SoundBuffer::SetPan(
	LONG lPan)
{
	V(m_ptr->SetPan(lPan));
}

void SoundBuffer::SetVolume(
	LONG lVolume)
{
	V(m_ptr->SetVolume(lVolume));
}

DSBCAPS SoundBuffer::GetCaps(void)
{
	DSBCAPS res;
	V(m_ptr->GetCaps(&res));
	return res;
}

void SoundBuffer::GetFormat(
	LPWAVEFORMATEX pwfxFormat,
	DWORD dwSizeAllocated,
	LPDWORD pdwSizeWritten)
{
	V(m_ptr->GetFormat(pwfxFormat, dwSizeAllocated, pdwSizeWritten));
}

DWORD SoundBuffer::GetStatus(void)
{
	DWORD res;
	V(m_ptr->GetStatus(&res));
	return res;
}

void SoundBuffer::SetFormat(
	LPCWAVEFORMATEX pcfxFormat)
{
	V(m_ptr->SetFormat(pcfxFormat));
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

Sound3DBuffer::Sound3DBuffer(IDirectSound3DBuffer * ptr)
	: m_ptr(ptr)
{
}

Sound3DBuffer::~Sound3DBuffer(void)
{
	SAFE_RELEASE(m_ptr);
}

DWORD Sound3DBuffer::GetMode(void)
{
	DWORD res;
	V(m_ptr->GetMode(&res));
	return res;
}

void Sound3DBuffer::SetMode(
	DWORD dwMode,
	DWORD dwApply)
{
	V(m_ptr->SetMode(dwMode, dwApply));
}

DS3DBUFFER Sound3DBuffer::GetAllParameters(void)
{
	DS3DBUFFER res;
	V(m_ptr->GetAllParameters(&res));
	return res;
}

void Sound3DBuffer::SetAllParameters(
	LPCDS3DBUFFER pcDs3dBuffer,
	DWORD dwApply)
{
	V(m_ptr->SetAllParameters(pcDs3dBuffer, dwApply));
}

D3DVALUE Sound3DBuffer::GetMaxDistance(void)
{
	D3DVALUE res;
	V(m_ptr->GetMaxDistance(&res));
	return res;
}

D3DVALUE Sound3DBuffer::GetMinDistance(void)
{
	D3DVALUE res;
	V(m_ptr->GetMinDistance(&res));
	return res;
}

void Sound3DBuffer::SetMaxDistance(
	D3DVALUE flMaxDistance,
	DWORD dwApply)
{
	V(m_ptr->SetMaxDistance(flMaxDistance, dwApply));
}

void Sound3DBuffer::SetMinDistance(
	D3DVALUE flMinDistance,
	DWORD dwApply)
{
	V(m_ptr->SetMinDistance(flMinDistance, dwApply));
}

Vector3 Sound3DBuffer::GetPosition(void)
{
	Vector3 res;
	V(m_ptr->GetPosition((D3DVECTOR *)&res));
	return res;
}

void Sound3DBuffer::SetPosition(
	const Vector3 & pos,
	DWORD dwApply)
{
	V(m_ptr->SetPosition(pos.x, pos.y, pos.z, dwApply));
}

void Sound3DBuffer::GetConeAngles(
	LPDWORD pdwInsideConeAngle,
	LPDWORD pdwOutsideConeAngle)
{
	V(m_ptr->GetConeAngles(pdwInsideConeAngle, pdwOutsideConeAngle));
}

Vector3 Sound3DBuffer::GetConeOrientation(void)
{
	Vector3 res;
	V(m_ptr->GetConeOrientation((D3DVECTOR *)&res));
	return res;
}

LONG Sound3DBuffer::GetConeOutsideVolume(void)
{
	LONG res;
	V(m_ptr->GetConeOutsideVolume(&res));
	return res;
}

void Sound3DBuffer::SetConeAngles(
	DWORD dwInsideConeAngle,
	DWORD dwOutsideConeAngle,
	DWORD dwApply)
{
	V(m_ptr->SetConeAngles(dwInsideConeAngle, dwOutsideConeAngle, dwApply));
}

void Sound3DBuffer::SetConeOrientation(
	const Vector3 & ori,
	DWORD dwApply)
{
	V(m_ptr->SetConeOrientation(ori.x, ori.y, ori.z, dwApply));
}

void Sound3DBuffer::SetConeOutsideVolume(
	LONG lConeOutsideVolume,
	DWORD dwApply)
{
	V(m_ptr->SetConeOutsideVolume(lConeOutsideVolume, dwApply));
}

Vector3 Sound3DBuffer::GetVelocity(void)
{
	Vector3 res;
	V(m_ptr->GetVelocity((D3DVECTOR *)&res));
	return res;
}

void Sound3DBuffer::SetVelocity(
	const Vector3 & vel,
	DWORD dwApply)
{
	V(m_ptr->SetVelocity(vel.x, vel.y, vel.z, dwApply));
}

Sound3DListener::Sound3DListener(IDirectSound3DListener * ptr)
	: m_ptr(ptr)
{
}

Sound3DListener::~Sound3DListener(void)
{
	SAFE_RELEASE(m_ptr);
}

void Sound3DListener::CommitDeferredSettings(void)
{
	V(m_ptr->CommitDeferredSettings());
}

DS3DLISTENER Sound3DListener::GetAllParameters(void)
{
	DS3DLISTENER res;
	V(m_ptr->GetAllParameters(&res));
	return res;
}

void Sound3DListener::SetAllParameters(
	LPCDS3DLISTENER pcListener,
	DWORD dwApply)
{
	V(m_ptr->SetAllParameters(pcListener, dwApply));
}

D3DVALUE Sound3DListener::GetDistanceFactor(void)
{
	D3DVALUE res;
	V(m_ptr->GetDistanceFactor(&res));
	return res;
}

D3DVALUE Sound3DListener::GetDopplerFactor(void)
{
	D3DVALUE res;
	V(m_ptr->GetDopplerFactor(&res));
	return res;
}

D3DVALUE Sound3DListener::GetRolloffFactor(void)
{
	D3DVALUE res;
	V(m_ptr->GetRolloffFactor(&res));
	return res;
}

void Sound3DListener::SetDistanceFactor(
	D3DVALUE flDistanceFactor,
	DWORD dwApply)
{
	V(m_ptr->SetDistanceFactor(flDistanceFactor, dwApply));
}

void Sound3DListener::SetDopplerFactor(
	D3DVALUE flDopplerFactor,
	DWORD dwApply)
{
	V(m_ptr->SetDopplerFactor(flDopplerFactor, dwApply));
}

void Sound3DListener::SetRolloffFactor(
	D3DVALUE flRolloffFactor,
	DWORD dwApply)
{
	V(m_ptr->SetRolloffFactor(flRolloffFactor, dwApply));
}

void Sound3DListener::GetOrientation(
	D3DVECTOR * pvOrientFront,
	D3DVECTOR * pvOrientTop)
{
	V(m_ptr->GetOrientation(pvOrientFront, pvOrientTop));
}

Vector3 Sound3DListener::GetPosition(void)
{
	Vector3 res;
	V(m_ptr->GetPosition((D3DVECTOR *)&res));
	return res;
}

Vector3 Sound3DListener::GetVelocity(void)
{
	Vector3 res;
	V(m_ptr->GetVelocity((D3DVECTOR *)&res));
	return res;
}

void Sound3DListener::SetOrientation(
	const Vector3 & Front,
	const Vector3 & Top,
	DWORD dwApply)
{
	V(m_ptr->SetOrientation(Front.x, Front.y, Front.z, Top.x, Top.y, Top.z, dwApply));
}

void Sound3DListener::SetPosition(
	const Vector3 & pos,
	DWORD dwApply)
{
	V(m_ptr->SetPosition(pos.x, pos.y, pos.z, dwApply));
}

void Sound3DListener::SetVelocity(
	const Vector3 & vel,
	DWORD dwApply)
{
	V(m_ptr->SetVelocity(vel.x, vel.y, vel.z, dwApply));
}
