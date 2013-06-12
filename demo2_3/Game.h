#pragma once

#include "Console.h"
#pragma warning(disable: 4819)
#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#pragma warning(default: 4819)
#include "PhysxSample.h"

class EffectUIRender
	: public my::UIRender
{
public:
	my::EffectPtr m_UIEffect;

	UINT m_Passes;

public:
	EffectUIRender(IDirect3DDevice9 * pd3dDevice, my::EffectPtr effect)
		: UIRender(pd3dDevice)
		, m_UIEffect(effect)
		, m_Passes(0)
	{
		_ASSERT(m_UIEffect);
	}

	virtual void Begin(void);

	virtual void End(void);

	virtual void SetWorld(const my::Matrix4 & World);

	virtual void SetViewProj(const my::Matrix4 & ViewProj);

	virtual void SetTexture(IDirect3DBaseTexture9 * pTexture);

	virtual void DrawVertexList(void);
};

class EffectEmitterInstance
	: public my::EmitterInstance
{
public:
	my::EffectPtr m_ParticleEffect;

	UINT m_Passes;

public:
	EffectEmitterInstance(my::EffectPtr effect)
		: m_ParticleEffect(effect)
		, m_Passes(0)
	{
	}

	virtual void Begin(void);

	virtual void End(void);

	virtual void SetWorld(const my::Matrix4 & World);

	virtual void SetViewProj(const my::Matrix4 & ViewProj);

	virtual void SetTexture(IDirect3DBaseTexture9 * pTexture);

	virtual void SetDirection(const my::Vector3 & Dir, const my::Vector3 & Up, const my::Vector3 & Right);

	virtual void SetAnimationColumnRow(unsigned char Column, unsigned char Row);

	virtual void DrawInstance(DWORD NumInstances);
};

class GameStateBase
{
private:
	friend class GameStateMachine;

	bool m_DeviceObjectsCreated;

	bool m_DeviceObjectsReset;

public:
	HRESULT hr;

	GameStateBase(void) throw ()
		: m_DeviceObjectsCreated(false)
		, m_DeviceObjectsReset(false)
	{
	}

	virtual ~GameStateBase(void)
	{
	}

	virtual HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		return S_OK;
	}

	virtual HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		return S_OK;
	}

	virtual void OnLostDevice(void)
	{
	}

	virtual void OnDestroyDevice(void)
	{
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime)
	{
	}

	virtual void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
	}

	virtual LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing)
	{
		return 0;
	}
};

class GameStateInit;

class GameStateMachine
	: public boost::statechart::state_machine<GameStateMachine, GameStateInit>
{
public:
	GameStateBase * CurrentState(void)
	{
		return const_cast<GameStateBase *>(state_cast<const GameStateBase *>());
	}

	void process_event(const boost::statechart::event_base & evt)
	{
		SafeLostCurrentState();
		SafeDestroyCurrentState();
		state_machine::process_event(evt);
		SafeCreateCurrentState(my::DxutApp::getSingleton().GetD3D9Device(), &my::DxutApp::getSingleton().GetD3D9BackBufferSurfaceDesc());
		SafeResetCurrentState(my::DxutApp::getSingleton().GetD3D9Device(), &my::DxutApp::getSingleton().GetD3D9BackBufferSurfaceDesc());
	}

	void SafeCreateCurrentState(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		GameStateBase * cs = CurrentState();
		if(cs && !cs->m_DeviceObjectsCreated)
		{
			_ASSERT(!cs->m_DeviceObjectsReset);
			if(SUCCEEDED(cs->OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
				cs->m_DeviceObjectsCreated = true;
		}
	}

	void SafeResetCurrentState(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		GameStateBase * cs = CurrentState();
		if(cs && cs->m_DeviceObjectsCreated && !cs->m_DeviceObjectsReset)
		{
			if(SUCCEEDED(cs->OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
				cs->m_DeviceObjectsReset = true;
		}
	}

	void SafeLostCurrentState(void)
	{
		GameStateBase * cs = CurrentState();
		if(cs && cs->m_DeviceObjectsCreated && cs->m_DeviceObjectsReset)
		{
			cs->OnLostDevice();
			cs->m_DeviceObjectsReset = false;
		}
	}

	void SafeDestroyCurrentState(void)
	{
		GameStateBase * cs = CurrentState();
		if(cs && cs->m_DeviceObjectsCreated)
		{
			_ASSERT(!cs->m_DeviceObjectsReset);
			cs->OnDestroyDevice();
			cs->m_DeviceObjectsCreated = false;
		}
	}

	void SafeFrameMoveCurrentState(
		double fTime,
		float fElapsedTime)
	{
		GameStateBase * cs = CurrentState();
		if(cs && cs->m_DeviceObjectsReset)
		{
			cs->OnFrameMove(fTime, fElapsedTime);
		}
	}

	void SafeFrameRenderCurrentState(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		GameStateBase * cs = CurrentState();
		if(cs && cs->m_DeviceObjectsReset)
		{
			cs->OnFrameRender(pd3dDevice, fTime, fElapsedTime);
		}
	}

	LRESULT SafeMsgProcCurrentState(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing)
	{
		GameStateBase * cs = CurrentState();
		if(cs && cs->m_DeviceObjectsReset)
		{
			return cs->MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing);
		}
		return 0;
	}
};

class Game
	: public my::DxutApp
	, public my::ResourceMgrEx
	, public my::TimerMgr
	, public my::DialogMgr
	, public my::MaterialMgr
	, public PhysxSample
	, public GameStateMachine
{
public:
	my::LuaContextPtr m_lua;

	my::UIRenderPtr m_UIRender;

	my::EmitterInstancePtr m_EmitterInst;

	my::TexturePtr m_WhiteTex;

	my::FontPtr m_Font;

	ConsolePtr m_Console;

	my::InputPtr m_Input;

	my::KeyboardPtr m_Keyboard;

	my::MousePtr m_Mouse;

	my::SoundPtr m_Sound;

public:
	Game(void);

	virtual ~Game(void);

	static Game & getSingleton(void)
	{
		return *getSingletonPtr();
	}

	static Game * getSingletonPtr(void)
	{
		return static_cast<Game *>(DxutApp::getSingletonPtr());
	}

	void AddLine(const std::wstring & str, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255))
	{
		m_Console->m_Panel->AddLine(str, Color);
	}

	void puts(const std::wstring & str)
	{
		m_Console->m_Panel->puts(str);
	}

	virtual bool IsDeviceAcceptable(
		D3DCAPS9 * pCaps,
		D3DFORMAT AdapterFormat,
		D3DFORMAT BackBufferFormat,
		bool bWindowed);

	virtual bool ModifyDeviceSettings(
		DXUTD3D9DeviceSettings * pDeviceSettings);

	virtual HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	virtual HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	virtual void OnLostDevice(void);

	virtual void OnDestroyDevice(void);

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime);

	virtual void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime);

	virtual LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing);

	bool ExecuteCode(const char * code) throw();
};
