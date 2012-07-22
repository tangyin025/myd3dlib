#pragma once

#include <myd3dlib.h>
#include <LuaContext.h>
#include "Console.h"
#pragma warning(disable: 4819)
#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#pragma warning(default: 4819)

class GameStateBase
{
public:
	HRESULT hr;

	virtual HRESULT OnD3D9CreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc) = 0;

	virtual HRESULT OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc) = 0;

	virtual void OnD3D9LostDevice(void) = 0;

	virtual void OnD3D9DestroyDevice(void) = 0;

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime) = 0;

	virtual void OnD3D9FrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime) = 0;

	virtual LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing) = 0;
};

class GameStateLoad;

class Game
	: public my::DxutApp
	, public boost::statechart::state_machine<Game, GameStateLoad>
{
public:
	HRESULT hr;

	GameStateBase * cs;

	CDXUTDialogResourceManager m_dlgResourceMgr;

	CD3DSettingsDlg m_settingsDlg;

	my::LuaContextPtr m_lua;

	typedef std::vector<my::DialogPtr> DialogPtrSet;

	DialogPtrSet m_dlgSet;

	my::FontPtr m_font;

	ConsolePtr m_console;

	my::InputPtr m_input;

	my::KeyboardPtr m_keyboard;

	my::MousePtr m_mouse;

	my::SoundPtr m_sound;

public:
	Game(void);

	virtual ~Game(void);

	GameStateBase * CurrentState(void)
	{
		return const_cast<GameStateBase *>(state_cast<const GameStateBase *>());
	}

	void process_event(const event_base_type & evt)
	{
		if(cs = CurrentState())
		{
			cs->OnD3D9LostDevice();
			cs->OnD3D9DestroyDevice();
		}

		state_machine::process_event(evt);

		if(cs = CurrentState())
		{
			cs->OnD3D9CreateDevice(GetD3D9Device(), DXUTGetD3D9BackBufferSurfaceDesc());
			cs->OnD3D9ResetDevice(GetD3D9Device(), DXUTGetD3D9BackBufferSurfaceDesc());
		}
	}

	void AddLine(const std::wstring & str, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255))
	{
		m_console->m_panel->AddLine(str, Color);
	}

	void puts(const std::wstring & str)
	{
		m_console->m_panel->puts(str);
	}

	static Game & getSingleton(void)
	{
		return *getSingletonPtr();
	}

	static Game * getSingletonPtr(void)
	{
		return dynamic_cast<Game *>(DxutApp::getSingletonPtr());
	}

	static my::TexturePtr LoadTexture(const char * path);

	static my::FontPtr LoadFont(const char * path, int height);

	virtual bool IsD3D9DeviceAcceptable(
		D3DCAPS9 * pCaps,
		D3DFORMAT AdapterFormat,
		D3DFORMAT BackBufferFormat,
		bool bWindowed);

	virtual bool ModifyDeviceSettings(
		DXUTDeviceSettings * pDeviceSettings);

	virtual HRESULT OnD3D9CreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	virtual HRESULT OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	virtual void OnD3D9LostDevice(void);

	virtual void OnD3D9DestroyDevice(void);

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime);

	virtual void OnD3D9FrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime);

	virtual LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing);

	void ToggleFullScreen(void);

	void ToggleRef(void);

	void ChangeDevice(void);

	void ExecuteCode(const char * code);

	void UpdateDlgViewProj(my::DialogPtr dlg);

	void InsertDlg(my::DialogPtr dlg)
	{
		UpdateDlgViewProj(dlg);

		m_dlgSet.push_back(dlg);
	}
};
