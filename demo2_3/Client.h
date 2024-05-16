#pragma once

#include "Console.h"
#include "LuaExtension.h"
#include "RenderPipeline.h"
#include "Actor.h"
#include "PhysxContext.h"
#include "SoundContext.h"
#include <boost/intrusive/list.hpp>
#include "SceneContext.h"
#include "../myd3dbox/DictionaryNode.h"

class Client
	: public my::DxutApp
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
	, public SoundContext
{
public:
	my::UIRenderPtr m_UIRender;

	my::FontPtr m_Font;

	DictionaryNode m_Dicts;

	ConsolePtr m_Console;

	float m_InitFov;

	unsigned int m_InitPhysxSceneFlags;

	my::Vector3 m_InitPhysxSceneGravity;

	int m_InitIOThreadNum;

	bool m_InitLoadShaderCache;

	std::string m_InitFont;

	int m_InitFontHeight;

	int m_InitFontFaceIndex;

	std::string m_InitDictFile;

	std::string m_InitUIEffect;

	my::Vector3 m_ViewedCenter;

	float m_ViewedDist;

	float m_ActorCullingThreshold;

	bool m_ShowNavigation;

	typedef boost::intrusive::list<Actor, boost::intrusive::base_hook<boost::intrusive::list_base_hook<boost::intrusive::tag<ViewedActorTag> > > > ViewedActorSet;

	ViewedActorSet m_ViewedActors;

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

	virtual void RemoveEntity(my::OctEntity * entity);

	ViewedActorSet::iterator RemoveViewedActorIter(ViewedActorSet::iterator actor_iter);

	void RemoveViewedActor(Actor* actor);

	virtual void OnControlSound(boost::shared_ptr<my::Wav> wav);

	virtual const std::wstring& OnControlTranslate(const std::wstring& wstr);

	virtual void OnPostCapture(double fTime, float fElapsedTime);

	virtual void OnPreUpdate(double fTime, float fElapsedTime);

	virtual void OnActorRequest(Actor* actor);

	virtual void OnActorRelease(Actor* actor);

	virtual void OnPostUIRender(my::UIRender* ui_render, double fTime, float fElapsedTime);

	template <typename T>
	void LoadSceneAsync(const char * path, const char * prefix, const T & callback, int Priority = 0)
	{
		std::string key = SceneContextRequest::BuildKey(path, prefix);
		IORequestPtr request(new SceneContextRequest(path, prefix, Priority));
		LoadIORequestAsync(key, request, callback);
	}

	boost::shared_ptr<SceneContext> LoadScene(const char * path, const char * prefix);

	void GetLoadSceneProgress(const char * path, const char * prefix, int & ActorListSize, int & ActorProgress, int & DialogListSize, int & DialogProgress);
};
