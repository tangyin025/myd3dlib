#pragma once

#include "Console.h"
#include "PhysXContext.h"

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

	virtual void SetTexture(const my::BaseTexturePtr & Texture);

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

	virtual void SetTexture(const my::BaseTexturePtr & Texture);

	virtual void SetDirection(const my::Vector3 & Dir, const my::Vector3 & Up, const my::Vector3 & Right);

	virtual void SetAnimationColumnRow(unsigned char Column, unsigned char Row);

	virtual void DrawInstance(DWORD NumInstances);
};

class Game
	: public my::DxutApp
	, public my::TimerMgr
	, public my::DialogMgr
	, public my::EmitterMgr
	, public my::ResourceMgr
	, public PhysXContext
	, public PhysXSceneContext
	, public my::ParallelTaskManager
{
public:
	my::LuaContextPtr m_lua;

	my::UIRenderPtr m_UIRender;

	my::EmitterInstancePtr m_EmitterInst;

	my::Texture2DPtr m_ShadowRT;

	my::SurfacePtr m_ShadowDS;

	my::BaseTexturePtr m_WhiteTex;

	my::FontPtr m_Font;

	ConsolePtr m_Console;

	my::InputPtr m_Input;

	my::KeyboardPtr m_Keyboard;

	my::MousePtr m_Mouse;

	my::SoundPtr m_Sound;

	my::CameraPtr m_Camera;

	typedef std::vector<my::Component *> RenderObjList;

	RenderObjList m_RenderObjList;

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

	virtual void OnFrameTick(
		double fTime,
		float fElapsedTime);

	virtual LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing);

	virtual void OnResourceFailed(const std::basic_string<TCHAR> & error_str);

	void AddLine(const std::wstring & str, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255));

	void puts(const std::wstring & str);

	bool ExecuteCode(const char * code) throw();
};
