#pragma once

#include "Console.h"

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

	virtual void SetTexture(IDirect3DBaseTexture9 * pTexture);

	virtual void SetTransform(const my::Matrix4 & World, const my::Matrix4 & View, const my::Matrix4 & Proj);

	virtual void DrawVertexList(void);
};

class GameStateBase
{
	friend class Game;

protected:
	bool m_DeviceObjectsCreated;

	bool m_DeviceObjectsReset;

	HRESULT hr;

public:
	GameStateBase(void)
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

typedef boost::shared_ptr<GameStateBase> GameStateBasePtr;

class Game
	: public my::DxutApp
	, public my::LoaderMgr
	, public my::TimerMgr
	, public my::DialogMgr
{
public:
	my::LuaContextPtr m_lua;

	my::UIRenderPtr m_UIRender;

	my::TexturePtr m_WhiteTex;

	my::FontPtr m_Font;

	my::DialogPtr m_Console;

	MessagePanelPtr m_Panel;

	my::InputPtr m_Input;

	my::KeyboardPtr m_Keyboard;

	my::MousePtr m_Mouse;

	my::SoundPtr m_Sound;

	typedef stdext::hash_map<std::string, GameStateBasePtr> GameStateBasePtrMap;

	GameStateBasePtrMap m_stateMap;

	GameStateBasePtrMap::const_iterator m_CurrentStateIter;

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

	IDirect3DDevice9 * GetD3D9Device(void)
	{
		return my::DxutApp::GetD3D9Device();
	}

	void AddLine(const std::wstring & str, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255))
	{
		m_Panel->AddLine(str, Color);
	}

	void puts(const std::wstring & str)
	{
		m_Panel->puts(str);
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

	void ExecuteCode(const char * code);

	void SetState(const std::string & key, GameStateBasePtr state);

	GameStateBasePtr GetState(const std::string & key) const;

	GameStateBasePtr GetCurrentState(void) const;

	std::string GetCurrentStateKey(void) const;

	void SafeCreateState(GameStateBasePtr state);

	void SafeResetState(GameStateBasePtr state);

	void SafeLostState(GameStateBasePtr state);

	void SafeDestroyState(GameStateBasePtr state);

	void SafeChangeState(GameStateBasePtr old_state, GameStateBasePtrMap::const_iterator new_state);

	void ChangeState(const std::string & key);

	void RemoveAllState(void);
};
