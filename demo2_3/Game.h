#pragma once

#include <myd3dlib.h>
#include "Console.h"

class Game : public my::DxutApp
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
};
