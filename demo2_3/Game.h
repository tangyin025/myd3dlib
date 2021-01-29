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

struct ActorEventArg : public my::EventArg
{
public:
	Actor* self;

	ActorEventArg(Actor* _self)
		: self(_self)
	{
	}
};

struct TriggerEventArg : public ActorEventArg
{
public:
	Actor* other;

	TriggerEventArg(Actor* _self, Actor* _other)
		: ActorEventArg(_self)
		, other(_other)
	{
	}
};
//
//struct ShapeHitEventArg : public ActorEventArg
//{
//public:
//	my::Vector3 worldPos;		//!< Contact position in world space
//	my::Vector3 worldNormal;	//!< Contact normal in world space
//	my::Vector3 dir;			//!< Motion direction
//	float length;				//!< Motion length
//	Component* cmp;			//!< Touched shape
//	Actor* other;				//!< Touched actor
//	unsigned int triangleIndex;	//!< touched triangle index (only for meshes/heightfields)
//
//	ShapeHitEventArg::ShapeHitEventArg(Character* _self)
//		: ActorEventArg(_self)
//		, worldPos(0, 0, 0)
//		, worldNormal(1, 0, 0)
//		, dir(1, 0, 0)
//		, length(0)
//		, cmp(NULL)
//		, other(NULL)
//		, triangleIndex(0)
//	{
//	}
//};

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
	, public PhysxSdk
	, public PhysxScene
	, public FModContext
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

	my::EventFunction m_EventEnterView;

	my::EventFunction m_EventLeaveView;

	my::EventFunction m_EventEnterTrigger;

	my::EventFunction m_EventLeaveTrigger;

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

	void AddEntity(my::OctEntity * entity, const my::AABB & aabb, float minblock, float threshold);

	bool RemoveEntity(my::OctEntity * entity);

	virtual void OnControlSound(const char * name);

	virtual void OnControlFocus(bool bFocus);

	void LoadScene(const char * path);
};
