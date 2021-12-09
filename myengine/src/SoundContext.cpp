#include "SoundContext.h"

using namespace my;

bool SoundContext::Init(HWND hwnd)
{
	m_sound.CreateSound();

	m_sound.SetCooperativeLevel(hwnd, DSSCL_PRIORITY);

	DSBUFFERDESC             dsbd;
	LPDIRECTSOUNDBUFFER      lpdsbPrimary;  // Cannot be IDirectSoundBuffer8.

	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;

	m_sound.CreateSoundBuffer(&dsbd, &lpdsbPrimary);
	SoundBuffer buff;
	buff.Create(lpdsbPrimary);
	m_listener = buff.Get3DListener();

	return true;
}

void SoundContext::Shutdown(void)
{
	m_listener.reset();

	BufferEventPairList::iterator buff_event_iter = m_pool.begin();
	for (; buff_event_iter != m_pool.end(); buff_event_iter++)
	{
		buff_event_iter->second->m_sbuffer = NULL;
		buff_event_iter->second->m_3dbuffer.reset();
	}

	m_pool.clear();
}

void SoundContext::ReleaseIdleBuffer(float fElapsedTime)
{
	BufferEventPairList::iterator buff_event_iter = m_pool.begin();
	for (; buff_event_iter != m_pool.end(); )
	{
		DWORD status = buff_event_iter->first.GetStatus();
		if (!(status & DSBSTATUS_PLAYING) || (status & DSBSTATUS_TERMINATED))
		{
			buff_event_iter->second->m_sbuffer = NULL;
			buff_event_iter->second->m_3dbuffer.reset();
			buff_event_iter = m_pool.erase(buff_event_iter);
		}
		else
		{
			buff_event_iter++;
		}
	}
}

SoundContext::BufferEventPairList::iterator SoundContext::GetIdleBuffer(my::WavPtr wav, DWORD flags)
{
	m_pool.insert(m_pool.begin(), BufferEventPair());

	BufferEventPairList::iterator buff_event_iter = m_pool.begin();

	DSBUFFERDESC dsbd;
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = flags;
	dsbd.dwBufferBytes = wav->child.cksize;
	dsbd.dwReserved = 0;
	dsbd.lpwfxFormat = &wav->wavfmt;
	dsbd.guid3DAlgorithm = DS3DALG_DEFAULT;

	LPDIRECTSOUNDBUFFER pDsb = NULL;
	HRESULT hr = m_sound.m_ptr->CreateSoundBuffer(&dsbd, &pDsb, NULL);
	if (FAILED(hr))
	{
		THROW_DSOUNDEXCEPTION(hr);
	}
	buff_event_iter->first.Create(pDsb);

	unsigned char* buffer1, * buffer2;
	DWORD bytes1, bytes2;
	buff_event_iter->first.Lock(0, wav->child.cksize, (LPVOID*)&buffer1, &bytes1, (LPVOID*)&buffer2, &bytes2, DSBLOCK_ENTIREBUFFER);
	_ASSERT(bytes1 + bytes2 == wav->child.cksize);
	if (buffer1)
	{
		memcpy(buffer1, &wav->buffer[0], bytes1);
	}
	if (buffer2)
	{
		memcpy(buffer2, &wav->buffer[bytes1], bytes2);
	}
	buff_event_iter->first.Unlock(buffer1, bytes1, buffer2, bytes2);

	return buff_event_iter;
}

SoundEventPtr SoundContext::Play(my::WavPtr wav)
{
	BufferEventPairList::iterator buff_event_iter = GetIdleBuffer(wav, DSBCAPS_CTRLVOLUME | DSBCAPS_LOCDEFER);

	if (buff_event_iter != m_pool.end())
	{
		buff_event_iter->second.reset(new SoundEvent());
		buff_event_iter->second->m_sbuffer = &buff_event_iter->first;
		buff_event_iter->first.Play(0, DSBPLAY_TERMINATEBY_TIME);
		return buff_event_iter->second;
	}

	return SoundEventPtr();
}

SoundEventPtr SoundContext::Play(my::WavPtr wav, const my::Vector3 & pos, const my::Vector3 & vel, float min_dist, float max_dist)
{
	BufferEventPairList::iterator buff_event_iter = GetIdleBuffer(wav, DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_LOCDEFER);

	if (buff_event_iter != m_pool.end())
	{
		buff_event_iter->second.reset(new SoundEvent());
		buff_event_iter->second->m_sbuffer = &buff_event_iter->first;
		buff_event_iter->second->m_3dbuffer = buff_event_iter->first.Get3DBuffer();
		DS3DBUFFER ds3dbuff = buff_event_iter->second->m_3dbuffer->GetAllParameters();
		ds3dbuff.vPosition = (D3DVECTOR&)pos;
		ds3dbuff.vVelocity = (D3DVECTOR&)vel;
		ds3dbuff.flMinDistance = min_dist;
		ds3dbuff.flMaxDistance = max_dist;
		ds3dbuff.dwMode = DS3DMODE_NORMAL;
		buff_event_iter->second->m_3dbuffer->SetAllParameters(&ds3dbuff, DS3D_IMMEDIATE);
		buff_event_iter->first.Play(0, DSBPLAY_TERMINATEBY_TIME);
		return buff_event_iter->second;
	}

	return SoundEventPtr();
}
