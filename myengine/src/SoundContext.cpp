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
}

SoundEventPtr SoundContext::Play(my::WavPtr wav)
{
	BufferEventPairArray::iterator buff_event_iter = m_pool.begin();
	for (; buff_event_iter != m_pool.end(); buff_event_iter++)
	{
		if (!buff_event_iter->first.m_ptr)
		{
			_ASSERT(!buff_event_iter->second);
			DSBUFFERDESC dsbd;
			dsbd.dwSize = sizeof(dsbd);
			dsbd.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_STATIC | DSBCAPS_LOCSOFTWARE;
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
			break;
		}
	}

	if (buff_event_iter != m_pool.end())
	{
		buff_event_iter->second.reset(new SoundEvent());
		buff_event_iter->second->m_3dbuffer = buff_event_iter->first.Get3DBuffer();
		buff_event_iter->first.Play(0, 0);
		return buff_event_iter->second;
	}
	return SoundEventPtr();
}
