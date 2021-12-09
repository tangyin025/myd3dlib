#pragma once

#include "mySingleton.h"
#include "mySound.h"
#include <boost/array.hpp>

class SoundEvent;

typedef boost::shared_ptr<SoundEvent> SoundEventPtr;

class SoundContext
	: public my::SingleInstance<SoundContext>
{
public:
	my::Sound m_sound;

	my::Sound3DListenerPtr m_listener;

	typedef std::pair<my::SoundBuffer, SoundEventPtr> BufferEventPair;

	typedef boost::array<BufferEventPair, 32> BufferEventPairArray;

	BufferEventPairArray m_pool;

public:
	SoundContext(void)
	{
	}

	bool Init(HWND hwnd);

	void Shutdown(void);

	BufferEventPairArray::iterator GetIdleBuffer(my::WavPtr wav, DWORD flags);

	SoundEventPtr Play(my::WavPtr wav);

	SoundEventPtr Play(my::WavPtr wav, const my::Vector3 & pos, const my::Vector3 & vel = my::Vector3(0, 0, 0), float min_dist = 1.0f, float max_dist = 1.0e9f);
};

class SoundEvent
{
protected:
	friend SoundContext;

	my::SoundBuffer * m_sbuffer;

	my::Sound3DBufferPtr m_3dbuffer;

public:
	SoundEvent(void)
		: m_sbuffer(NULL)
	{
	}
};
