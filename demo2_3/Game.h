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
protected:
	HRESULT hr;

public:
	GameStateBase(void)
	{
	}

	virtual ~GameStateBase(void)
	{
	}

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

typedef boost::statechart::event_base GameEventBase;

// ! Release build with Pch will suffer LNK2001, ref: http://thread.gmane.org/gmane.comp.lib.boost.user/23065
template< class Event > void boost::statechart::detail::no_context<Event>::no_function( const Event & ) {}

class GameLoader
	: public my::DxutApp
{
public:
	my::TexturePtr LoadTexture(const std::string & path);

	my::MeshPtr LoadMesh(const std::string & path);

	my::OgreSkeletonAnimationPtr LoadSkeletonAnimation(const std::string & path);

	my::EffectPtr LoadEffect(const std::string & path);

	my::FontPtr LoadFont(const std::string & path, int height);
};

class Game
	: public GameLoader
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

	my::DialogPtr m_console;

	MessagePanelPtr m_panel;

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

	my::ControlPtr GetPanel(void) const
	{
		return m_panel;
	}

	void SetPanel(my::ControlPtr panel)
	{
		m_panel = boost::dynamic_pointer_cast<MessagePanel>(panel);
	}

	void AddLine(const std::wstring & str, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255))
	{
		m_panel->AddLine(str, Color);
	}

	void puts(const std::wstring & str)
	{
		m_panel->puts(str);
	}

	static Game & getSingleton(void)
	{
		return *getSingletonPtr();
	}

	static Game * getSingletonPtr(void)
	{
		return dynamic_cast<Game *>(DxutApp::getSingletonPtr());
	}

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

class GameEventLoadOver : public boost::statechart::event<GameEventLoadOver>
{
};
