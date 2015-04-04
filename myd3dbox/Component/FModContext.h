#pragma once

#include <fmod.hpp>
#include <fmod_errors.h>

#define FMOD_ERRCHECK(result) if (result != FMOD_OK) { \
	throw my::CustomException(str_printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result)), __FILE__, __LINE__); }

class FModContext
{
public:
	FMOD_RESULT result;

	FMOD::System * m_FModSystem;

	unsigned int m_FModVersion;

public:
	FModContext(void)
		: m_FModSystem(NULL)
	{
	}

	bool OnInit(void);

	void OnShutdown(void);
};
