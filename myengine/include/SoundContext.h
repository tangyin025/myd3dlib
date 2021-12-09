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

	typedef std::pair<my::SoundBufferPtr, SoundEventPtr> BufferEventPair;

	typedef boost::array<BufferEventPair, 32> BufferEventPairArray;

public:
	SoundContext(void)
	{
	}

	bool Init(void);

	void Shutdown(void);

	SoundEventPtr Play(my::WavPtr wav);
};

class SoundEvent
{
public:
	SoundEvent(void)
	{
	}
};
