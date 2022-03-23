#pragma once

#include "Console.h"
#include "LuaExtension.h"
#include "RenderPipeline.h"
#include "Actor.h"
#include "PhysxContext.h"
#include "SoundContext.h"
#include "myStateChart.h"
#include <boost/intrusive/list.hpp>

class dtNavMesh;

class dtNavMeshQuery;

class SceneContext : public my::DeviceResourceBase
{
public:
	my::Vector3 m_SkyLightCamEuler;
	my::Vector4 m_SkyLightColor;
	my::Vector4 m_AmbientColor;
	my::Vector4 m_DofParams;
	float m_SsaoBias;
	float m_SsaoIntensity;
	float m_SsaoRadius;
	float m_SsaoScale;
	my::Vector4 m_FogColor;
	float m_FogStartDistance;
	float m_FogHeight;
	float m_FogFalloff;
	boost::shared_ptr<unsigned char> m_SerializeBuff;
	CollectionObjMap m_CollectionObjs;
	typedef std::vector<ActorPtr> ActorPtrList;
	ActorPtrList m_ActorList;
	typedef std::vector<my::DialogPtr> DialogPtrList;
	DialogPtrList m_DialogList;

public:
	SceneContext(void)
	{
	}
};

typedef boost::shared_ptr<SceneContext> SceneContextPtr;

class SceneContextRequest : public my::IORequest
{
protected:
	friend class Client;

	std::string m_path;

	std::string m_prefix;

	volatile LONG m_ActorProgress;

	volatile LONG m_DialogProgress;

public:
	SceneContextRequest(const char* path, const char* prefix, int Priority);

	virtual void SceneContextRequest::LoadResource(void);

	virtual void SceneContextRequest::CreateResource(LPDIRECT3DDEVICE9 pd3dDevice);

	static std::string SceneContextRequest::BuildKey(const char* path);
};

class StateBase
	: public my::StateChart<StateBase, std::string>
{
public:
	StateBase(void)
	{
	}

	virtual void OnAdd(void)
	{
	}

	virtual void OnEnter(void)
	{
	}

	virtual void OnExit(void)
	{
	}

	virtual void OnTick(float fElapsedTime)
	{
	}

	virtual void OnControlFocus(my::Control * control)
	{
	}

	virtual void OnActorRequestResource(Actor * actor)
	{
	}

	virtual void OnActorReleaseResource(Actor * actor)
	{
	}
};

typedef boost::shared_ptr<StateBase> StateBasePtr;

class Client
	: public my::DxutApp
	, public my::FontLibrary
	, public my::DialogMgr
	, public my::InputMgr
	, public my::ResourceMgr
	, public my::ParallelTaskManager
	, public my::DrawHelper
	, public my::OctRoot
	, public my::StateChart<StateBase, std::string>
	, public LuaContext
	, public RenderPipeline
	, public RenderPipeline::IRenderContext
	, public PhysxSdk
	, public PhysxScene
	, public SoundContext
{
public:
	my::UIRenderPtr m_UIRender;

	my::FontPtr m_Font;

	ConsolePtr m_Console;

	float m_InitFov;

	int m_InitIOThreadNum;

	bool m_InitLoadShaderCache;

	std::string m_InitFont;

	int m_InitFontHeight;

	int m_InitFontFaceIndex;

	std::string m_InitUIEffect;

	std::string m_InitScript;

	enum Key
	{
		KeyUIHorizontal,
		KeyUIVertical,
		KeyUIConfirm,
		KeyUICancel,
		KeyHorizontal,
		KeyVertical,
		KeyMouseX,
		KeyMouseY,
		KeyJump,
		KeyFire,
		KeyLock,
		KeyMenu,
		KeyWeapon1,
		KeyWeapon2,
		KeyWeapon3,
		KeyWeapon4,
		KeyCount,
	};

	boost::array<InputMgr::KeyPairList, KeyCount> m_InitBindKeys;

	my::Vector3 m_ViewedCenter;

	float m_ViewedDist;

	float m_ViewedDistDiff;

	typedef boost::intrusive::list<Actor, boost::intrusive::base_hook<boost::intrusive::list_base_hook<boost::intrusive::tag<ViewedActorTag> > > > ViewedActorSet;

	ViewedActorSet m_ViewedActors;

	bool m_Activated;

	typedef boost::signals2::signal<void(bool)> ActivateEvent;

	ActivateEvent m_ActivateEvent;

	int m_ShowCursorCount;

public:
	Client(void);

	virtual ~Client(void);

	static Client & getSingleton(void)
	{
		return *getSingletonPtr();
	}

	static Client * getSingletonPtr(void)
	{
		return static_cast<Client *>(DxutApp::getSingletonPtr());
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

	virtual void AddEntity(my::OctEntity * entity, const my::AABB & aabb, float minblock, float threshold);

	void AddEntity(my::OctEntity * entity);

	virtual void RemoveEntity(my::OctEntity * entity);

	virtual void OnControlSound(boost::shared_ptr<my::Wav> wav);

	virtual void OnControlFocus(my::Control * control);

	virtual std::wstring OnControlTranslate(const std::string& str);

	template <typename T>
	void LoadSceneAsync(const char * path, const char * prefix, const T & callback, int Priority = 0)
	{
		std::string key = SceneContextRequest::BuildKey(path);
		IORequestPtr request(new SceneContextRequest(path, prefix, Priority));
		LoadIORequestAsync(key, request, callback);
	}

	boost::shared_ptr<SceneContext> LoadScene(const char * path, const char * prefix);

	void GetLoadSceneProgress(const char * path, int & ActorProgress, int & DialogProgress);
	
	typedef boost::function<void(Actor *, Component *, unsigned int)> OverlapCallback;

	typedef boost::function<bool(Controller *)> ControllerFilterFunc;

	bool Overlap(
		const physx::PxGeometry & geometry,
		const my::Vector3 & Position,
		const my::Quaternion & Rotation,
		unsigned int filterWord0,
		const OverlapCallback & callback,
		unsigned int MaxNbTouches);
};
