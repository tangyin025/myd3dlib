#pragma once

#include <myd3dlib.h>
#include "Console.h"
#include "SkyBox.h"
#pragma warning(disable: 4819)
#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#pragma warning(default: 4819)

class IGameStateBase
{
public:
	HRESULT hr;

	// 这里设计成直接使用构造函数创建，不再需要 OnCreateDevice
	//virtual HRESULT OnD3D9CreateDevice(
	//	IDirect3DDevice9 * pd3dDevice,
	//	const D3DSURFACE_DESC * pBackBufferSurfaceDesc) = 0;

	virtual HRESULT OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc) = 0;

	virtual void OnD3D9LostDevice(void) = 0;

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

class GameLoad;

class Game
	: public my::DxutApp
	, public boost::statechart::state_machine<Game, GameLoad>
{
public:
	HRESULT hr;

	CDXUTDialogResourceManager m_dlgResourceMgr;

	CD3DSettingsDlg m_settingsDlg;

	my::TexturePtr m_uiTex;

	my::FontPtr m_uiFnt;

	my::ControlSkinPtr m_defDlgSkin;

	typedef std::set<my::DialogPtr> DialogPtrSet;

	DialogPtrSet m_dlgSet;

	my::DialogPtr m_hudDlg;

	ConsolePtr m_console;

	my::InputPtr m_input;

	my::KeyboardPtr m_keyboard;

	my::MousePtr m_mouse;

	my::SoundPtr m_sound;

public:
	Game(void);

	virtual ~Game(void);

	static Game & getSingleton(void)
	{
		return *dynamic_cast<Game *>(getSingletonPtr());
	}

	virtual bool IsD3D9DeviceAcceptable(
		D3DCAPS9 * pCaps,
		D3DFORMAT AdapterFormat,
		D3DFORMAT BackBufferFormat,
		bool bWindowed);

	virtual bool ModifyDeviceSettings(
		DXUTDeviceSettings * pDeviceSettings);

	virtual void OnInit(void);

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

	void OnToggleFullScreen(my::ControlPtr ctrl);

	void OnToggleRef(my::ControlPtr ctrl);

	void OnChangeDevice(my::ControlPtr ctrl);

	IGameStateBase * CurrentState(void)
	{
		return const_cast<IGameStateBase *>(state_cast<const IGameStateBase *>());
	}
};

template <class DrivedClass>
class GameEvent : public boost::statechart::event<DrivedClass>
{
public:
	GameEvent(void)
	{
		// 当内部状态发生变化，新旧资源会被重新创建，
		// 所以就需要在切换状态时重新 Lost/Reset 一遍“相关”（目前还做不到只更新“相关”)资源
		Game::getSingleton().OnD3D9LostDevice();
	}

	virtual ~GameEvent(void)
	{
		Game::getSingleton().OnD3D9ResetDevice(DXUTGetD3D9Device(), DXUTGetD3D9BackBufferSurfaceDesc());
	}
};

class EvLoadOver : public GameEvent<EvLoadOver>
{
};

class GamePlay;

class GameLoad
	: public IGameStateBase
	, public boost::statechart::simple_state<GameLoad, Game>
{
public:
	typedef boost::statechart::transition<EvLoadOver, GamePlay> reactions;

	GameLoad(void);

	~GameLoad(void);

	virtual HRESULT OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	virtual void OnD3D9LostDevice(void);

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
};

class GamePlay
	: public IGameStateBase
	, public boost::statechart::simple_state<GamePlay, Game>
{
public:
	SkyBoxPtr m_skyBox;

public:
	GamePlay(void);

	~GamePlay(void);

	virtual HRESULT OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	virtual void OnD3D9LostDevice(void);

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
};
