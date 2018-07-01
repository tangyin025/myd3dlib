#pragma once

#include "Console.h"
#include "LuaExtension.h"
#include "Component/RenderPipeline.h"
#include "Component/Character.h"
#include "Component/Controller.h"
#include "Component/PhysXContext.h"
#include "Component/FModContext.h"

class Game
	: public my::DxutApp
	, public my::TimerMgr
	, public my::DialogMgr
	, public my::InputMgr
	, public my::ResourceMgr
	, public my::ParallelTaskManager
	, public my::DrawHelper
	, public LuaContext
	, public RenderPipeline
	, public RenderPipeline::IRenderContext
	, public PhysXContext
	, public PhysXSceneContext
	, public FModContext
{
public:
	typedef std::map<int, boost::array<wchar_t, 256> > ScrInfoType;

	ScrInfoType m_ScrInfos;

	std::string m_LastErrorStr;

	my::UIRenderPtr m_UIRender;

	my::FontPtr m_Font;

	ConsolePtr m_Console;

	my::OctRoot m_Root;

	typedef std::map<Actor *, boost::weak_ptr<Actor> > WeakActorMap;

	WeakActorMap m_ViewedActors;

	std::string m_InitFont;

	std::string m_InitUIEffect;

	std::string m_InitSound;

	std::string m_InitScene;

	std::string m_InitScript;

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

	virtual void OnFrameTick(
		double fTime,
		float fElapsedTime);

	virtual void OnRender(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
		IRenderContext * pRC,
		double fTime,
		float fElapsedTime);

	virtual void OnUIRender(
		my::UIRender * ui_render,
		double fTime,
		float fElapsedTime);

	virtual LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing);

	void puts(const std::wstring & str);

	bool ExecuteCode(const char * code) throw();

	virtual void QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	void SaveDialog(my::DialogPtr dlg, const char * path);

	my::DialogPtr LoadDialog(const char * path);

	void Game::SaveMaterial(MaterialPtr mat, const char * path);

	MaterialPtr LoadMaterial(const char * path);

	void SaveComponent(ComponentPtr cmp, const char * path);

	ComponentPtr LoadComponent(const char * path);

	void LoadScene(const char * path);
};
