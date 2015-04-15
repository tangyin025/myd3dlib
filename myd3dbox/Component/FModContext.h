#pragma once

#define FMOD_ERRCHECK(result) if (result != FMOD_OK) { \
	throw my::CustomException(str_printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result)), __FILE__, __LINE__); }

namespace my
{
	class ResourceMgr;
}

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

	static my::ResourceMgr * GetResourceMgr(void);

	bool OnInit(void);

	void OnShutdown(void);
};
