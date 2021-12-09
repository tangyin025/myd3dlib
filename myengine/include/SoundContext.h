#pragma once

#include "mySingleton.h"
#include "mySound.h"

class SoundContext
	: public my::SingleInstance<SoundContext>
{
public:
	my::Sound m_sound;

	my::Sound3DListenerPtr m_listener;

	SoundContext(void)
	{
	}

	bool Init(void);

	void Shutdown(void);
};
