#pragma once

#include <myd3dlib.h>
#include <LuaContext.h>
#include "Console.h"
#include "Scene.h"

class AlignEventArgs : public my::EventArgs
{
public:
	my::Vector2 vp;

	AlignEventArgs(const my::Vector2 & _vp)
		: vp(_vp)
	{
	}
};

class Game
	: public my::DxutApp
{
public:
	HRESULT hr;

	CDXUTDialogResourceManager m_dlgResourceMgr;

	CD3DSettingsDlg m_settingsDlg;

	my::LuaContextPtr m_lua;

	typedef std::vector<my::DialogPtr> DialogPtrSet;

	DialogPtrSet m_dlgSet;

	typedef std::vector<BaseScenePtr> BaseScenePtrSet;

	BaseScenePtrSet m_sceneSet;

	my::FontPtr m_font;

	//MessagePanelPtr m_panel;

	ConsolePtr m_console;

	my::InputPtr m_input;

	my::KeyboardPtr m_keyboard;

	my::MousePtr m_mouse;

	my::SoundPtr m_sound;

public:
	//my::ControlPtr GetPanel(void) const
	//{
	//	return m_panel;
	//}

	//void SetPanel(my::ControlPtr panel)
	//{
	//	m_panel = boost::dynamic_pointer_cast<MessagePanel>(panel);
	//}

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

	void InsertScene(BaseScenePtr scene)
	{
		m_sceneSet.push_back(scene);
	}
};
