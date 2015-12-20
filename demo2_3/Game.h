#pragma once

#include "Console.h"
#include "Component/FModContext.h"
#include "Component/RenderPipeline.h"
#include "Component/Component.h"
#include "Component/Actor.h"
#include "Logic/Logic.h"

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

	virtual void Flush(void);
};

class Game
	: public my::DxutApp
	, public my::TimerMgr
	, public my::LuaContext
	, public my::DialogMgr
	, public my::InputMgr
	, public my::ResourceMgr
	, public ComponentContext
	, public RenderPipeline
	, public RenderPipeline::IRenderContext
	, public FModContext
	, public my::ParallelTaskManager
	, public my::DrawHelper
{
public:
	typedef boost::tuple<RenderPipeline::MeshType, bool, std::string> ShaderCacheKey;

	typedef boost::unordered_map<ShaderCacheKey, my::EffectPtr> ShaderCacheMap;

	ShaderCacheMap m_ShaderCache;

	typedef std::map<int, boost::array<wchar_t, 256> > ScrInfoType;

	ScrInfoType m_ScrInfos;

	std::string m_LastErrorStr;

	my::UIRenderPtr m_UIRender;

	my::Texture2DPtr m_NormalRT;

	my::Texture2DPtr m_PositionRT;

	my::Texture2DPtr m_LightRT;

	my::Texture2DPtr m_OpaqueRT;

	my::Texture2DPtr m_DownFilterRT[2];

	CComPtr<IDirect3DSurface9> m_OldRT;

	CComPtr<IDirect3DSurface9> m_OldDS;

	my::FontPtr m_Font;

	ConsolePtr m_Console;

	LogicPtr m_Logic;

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

	virtual void OnPxThreadSubstep(float dtime);

	virtual void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime);

	virtual void OnUIRender(
		my::UIRender * ui_render,
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

	virtual void OnResourceFailed(const std::string & error_str);

	void AddLine(const std::wstring & str, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255));

	void puts(const std::wstring & str);

	bool ExecuteCode(const char * code) throw();

	virtual IDirect3DSurface9 * GetScreenSurface(void);

	virtual IDirect3DSurface9 * GetScreenDepthStencilSurface(void);

	virtual IDirect3DSurface9 * GetNormalSurface(void);

	virtual my::Texture2D * GetNormalTexture(void);

	virtual IDirect3DSurface9 * GetPositionSurface(void);

	virtual my::Texture2D * GetPositionTexture(void);

	virtual IDirect3DSurface9 * GetLightSurface(void);

	virtual my::Texture2D * GetLightTexture(void);

	virtual IDirect3DSurface9 * GetOpaqueSurface(void);

	virtual my::Texture2D * GetOpaqueTexture(void);

	virtual IDirect3DSurface9 * GetDownFilterSurface(unsigned int id);

	virtual my::Texture2D * GetDownFilterTexture(unsigned int id);

	virtual my::Effect * QueryShader(RenderPipeline::MeshType mesh_type, bool bInstance, const Material * material, unsigned int PassID);

	virtual void QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);
};
