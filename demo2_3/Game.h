#pragma once

#include <myd3dlib.h>
#include <LuaContext.h>
#include "Console.h"

class Game
	: public my::DxutApp
{
public:
	HRESULT hr;

	CDXUTDialogResourceManager m_dlgResourceMgr;

	CD3DSettingsDlg m_settingsDlg;

	my::LuaContextPtr m_lua;

	typedef std::set<my::DialogPtr> DialogPtrSet;

	DialogPtrSet m_dlgSet;

	my::FontPtr m_uiFnt;

	my::DialogPtr m_console;

	MessagePanelPtr m_panel;

	my::DialogPtr m_hudDlg;

	my::InputPtr m_input;

	my::KeyboardPtr m_keyboard;

	my::MousePtr m_mouse;

	my::SoundPtr m_sound;

public:
	Game(void);

	virtual ~Game(void);

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

	void ToggleFullScreen(void);

	void OnToggleRef(my::ControlPtr control);

	void ChangeDevice(void);

	bool ExecuteCode(const char * code);

	void UpdateDlgPerspective(my::DialogPtr dlg);

	void InsertDlg(my::DialogPtr dlg);

	void AddLine(LPCWSTR pString, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255));

	void puts(const std::wstring & str, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255));
};
