#include "stdafx.h"
#include "PlayerAgent.h"
#include "MainApp.h"

PlayerAgent::~PlayerAgent(void)
{
	_ASSERT(!IsRequested());
}

DWORD PlayerAgent::GetComponentType(void) const
{
	return ComponentTypeScript;
}

void PlayerAgent::RequestResource(void)
{
	Component::RequestResource();

	theApp.m_mouse->Unacquire();
	theApp.m_mouse->SetCooperativeLevel(AfxGetMainWnd()->GetSafeHwnd(), DISCL_FOREGROUND | DISCL_EXCLUSIVE);
}

void PlayerAgent::ReleaseResource(void)
{
	Component::ReleaseResource();

	theApp.m_mouse->Unacquire();
	theApp.m_mouse->SetCooperativeLevel(AfxGetMainWnd()->GetSafeHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
}

void PlayerAgent::Update(float fElapsedTime)
{
	if (theApp.m_keyboard->IsKeyPress(my::KeyCode::KC_ESCAPE))
	{
		AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_TOOLS_PLAYING);
	}
}

void PlayerAgent::OnPxThreadSubstep(float dtime)
{
}
