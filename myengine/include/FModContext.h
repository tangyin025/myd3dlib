#pragma once

#include <fmod_event.hpp>
#include "mySingleton.h"

class FModContext
	: public my::SingleInstance<FModContext>
{
public:
	FMOD_RESULT result;

	FMOD::EventSystem *m_EventSystem;

public:
	FModContext(void)
		: m_EventSystem(NULL)
	{
	}

	bool Init(void);

	void Update(void);

	void Shutdown(void);

	void SetMediaPath(const char * path);

	void LoadEventFile(const char * file);

	void PlaySound(const char * name);
};
