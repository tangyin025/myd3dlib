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

public:
	SoundContext(void)
	{
	}

	bool Init(HWND hwnd);

	void Shutdown(void);

	void ReleaseIdleBuffer(float fElapsedTime);

	BufferEventPairList::iterator GetIdleBuffer(my::WavPtr wav, DWORD flags);

	SoundEventPtr Play(my::WavPtr wav, bool Loop);

	SoundEventPtr Play(my::WavPtr wav, bool Loop, const my::Vector3 & pos, const my::Vector3 & vel = my::Vector3(0, 0, 0), float min_dist = 1.0f, float max_dist = 1.0e9f);
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
protected:
	static const DWORD MPEG_BUFSZ = 40000;

	//static const DWORD MAX_RESAMPLEFACTOR = 6;

	//static const DWORD MAX_NSAMPLES = 1152 * MAX_RESAMPLEFACTOR;

	static const DWORD BLOCK_COUNT = 3;

protected:
	WAVEFORMATEX m_wavfmt;

	my::SoundBufferPtr m_dsbuffer;

	my::SoundNotifyPtr m_dsnotify;

	DSBPOSITIONNOTIFY m_dsnp[BLOCK_COUNT];

	my::Event m_events[BLOCK_COUNT + 1];

	typedef std::vector<unsigned char> FileBuffer;

	FileBuffer m_buffer;

	my::IStreamPtr m_stream;

	bool m_Loop;

	//my::CriticalSection m_LoopLock;

	bool PlayOnceByThread(void);

public:
	Mp3(void);

	virtual ~Mp3(void);

	void SetLoop(bool Loop)
	{
		//my::CriticalSectionLock lock(m_LoopLock);
		m_Loop = Loop;
	}

	bool GetLoop(void)
	{
		//my::CriticalSectionLock lock(m_LoopLock);
		return m_Loop;
	}

	void Play(my::IStreamPtr istr, bool Loop);

	void Play(const char * path, bool Loop);

	void StopAsync(void);

	void Stop(void);

	DWORD OnThreadProc(void);
};

typedef boost::shared_ptr<Mp3> Mp3Ptr;
