#include "SoundContext.h"

using namespace my;

bool SoundContext::Init(void)
{
	m_sound.CreateSound();

	DSBUFFERDESC             dsbd;
	LPDIRECTSOUNDBUFFER      lpdsbPrimary;  // Cannot be IDirectSoundBuffer8.

	HRESULT hr;
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
