// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include "mySingleton.h"
#include "mySound.h"
#include "myThread.h"

class SoundEvent;

typedef boost::shared_ptr<SoundEvent> SoundEventPtr;

class SoundContext
	: public my::SingletonInstance<SoundContext>
{
public:
	my::Sound m_sound;

	my::CriticalSection m_soundsec;

	my::Sound3DListenerPtr m_listener;

	typedef std::pair<my::SoundBuffer, SoundEventPtr> BufferEventPair;

	typedef std::list<BufferEventPair> BufferEventPairList;

	BufferEventPairList m_pool;

	LONG m_Volume;

public:
	SoundContext(void)
		: m_Volume(DSBVOLUME_MAX)
	{
	}

	bool Init(HWND hwnd);

	void Shutdown(void);

	LONG GetVolume(void);

	void SetVolume(LONG lVolume);

	void ReleaseIdleBuffer(float fElapsedTime);

	BufferEventPairList::iterator GetIdleBuffer(my::WavPtr wav, size_t startbyte, DWORD bytes, DWORD flags);

	SoundEventPtr Play(my::WavPtr wav, float StartSec, float EndSec, bool Loop);

	SoundEventPtr Play(my::WavPtr wav, float StartSec, float EndSec, bool Loop, const my::Vector3 & pos, const my::Vector3 & vel = my::Vector3(0, 0, 0), float min_dist = 1.0f, float max_dist = 1.0e9f);
};

class SoundEvent
{
public:
	my::SoundBuffer * m_sbuffer;

	my::Sound3DBufferPtr m_3dbuffer;

public:
	SoundEvent(void)
		: m_sbuffer(NULL)
	{
	}
};

class Mp3 : public my::Thread
{
public:
	static const DWORD BLOCK_COUNT = 3;

	WAVEFORMATEX m_wavfmt;

	my::SoundBufferPtr m_dsbuffer;

	my::CriticalSection m_buffersec;

	my::SoundNotifyPtr m_dsnotify;

	DSBPOSITIONNOTIFY m_dsnp[BLOCK_COUNT];

	my::Event m_events[BLOCK_COUNT + 1];

	std::string m_Mp3Path;

	bool m_Loop;

	volatile LONG m_Volume;

	bool PlayOnceByThread(void);

public:
	Mp3(void);

	virtual ~Mp3(void);

	void Play(const char * path, bool Loop);

	bool IsPlaying(void);

	LONG GetVolume(void);

	void SetVolume(LONG lVolume);

	void StopAsync(void);

	void Stop(void);

	DWORD OnThreadProc(void);
};

typedef boost::shared_ptr<Mp3> Mp3Ptr;
