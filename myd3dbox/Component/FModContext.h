#pragma once

class FModContext
{
public:
	FMOD_RESULT result;

	FMOD::EventSystem *m_EventSystem;

public:
	FModContext(void)
		: m_EventSystem(NULL)
	{
	}

	bool OnInit(void);

	void OnUpdate(void);

	void OnShutdown(void);

	void SetMediaPath(const char * path);

	void LoadEventFile(const char * file);

	void PlaySound(const char * name);
};
