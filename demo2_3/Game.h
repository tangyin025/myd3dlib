#pragma once

#include "Console.h"
#include "LuaExtension.h"
#include "RenderPipeline.h"
#include "Character.h"
#include "PhysxContext.h"
#include "FModContext.h"

namespace boost
{
	namespace archive
	{
		class polymorphic_iarchive;
	};
};

class Game
	: public my::DxutApp
	, public my::TimerMgr
	, public my::FontLibrary
	, public my::DialogMgr
	, public my::InputMgr
	, public my::ResourceMgr
	, public my::ParallelTaskManager
	, public my::DrawHelper
	, public my::OctRoot
	, public LuaContext
	, public RenderPipeline
	, public RenderPipeline::IRenderContext
	, public PhysxContext
	, public PhysxSceneContext
	, public FModContext
	, public my::Thread
{
public:
	my::UIRenderPtr m_UIRender;

	my::FontPtr m_Font;

	ConsolePtr m_Console;

	float m_InitFov;

	bool m_InitLoadShaderCache;

	std::string m_InitFont;

	int m_InitFontHeight;

	int m_InitFontFaceIndex;

	std::string m_InitUIEffect;

	std::string m_InitSound;

	std::string m_InitScript;

	my::Vector3 m_ViewedCenter;

	typedef std::set<Actor *> ViewedActorSet;

	ViewedActorSet m_ViewedActors;

	typedef std::set<ActorPtr> ActorPtrSet;

	ActorPtrSet m_ActorList;

	boost::shared_ptr<my::IStreamBuff> m_LoadSceneBuff;

	boost::shared_ptr<std::istream> m_LoadSceneStream;

	boost::shared_ptr<boost::archive::polymorphic_iarchive> m_LoadSceneArchive;

	my::EventFunction m_EventLoadScene;

	bool m_Activated;

	typedef boost::signals2::signal<void(bool)> ActivateEvent;

	ActivateEvent m_ActivateEvent;

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

	void CheckViewedActor(const my::AABB & In, const my::AABB & Out);

	void AddEntity(my::OctEntity * entity, const my::AABB & aabb);

	bool RemoveEntity(my::OctEntity * entity);

	virtual void OnControlSound(const char * name);

	virtual void OnControlFocus(bool bFocus);

	void LoadScene(const char * path);

	DWORD LoadSceneProc(void);

	void LoadSceneCheck(DWORD dwMilliseconds);
};
