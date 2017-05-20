#pragma once

#include "Console.h"
#include "../myd3dbox/Component/RenderPipeline.h"
#include "../myd3dbox/Component/Component.h"
#include "../myd3dbox/Component/Actor.h"
#include "../myd3dbox/Component/PhysXContext.h"
#include "../myd3dbox/Component/FModContext.h"
#include "../myd3dbox/Component/LuaExtension.h"

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
	typedef boost::tuple<RenderPipeline::MeshType, bool, std::string> ShaderCacheKey;

	typedef boost::unordered_map<ShaderCacheKey, my::EffectPtr> ShaderCacheMap;

	ShaderCacheMap m_ShaderCache;

	typedef std::map<int, boost::array<wchar_t, 256> > ScrInfoType;

	ScrInfoType m_ScrInfos;

	std::string m_LastErrorStr;

	my::UIRenderPtr m_UIRender;

	my::FontPtr m_Font;

	ConsolePtr m_Console;

	Octree m_Root;

	typedef boost::unordered_set<Actor *> OctActorSet;

	OctActorSet m_ViewedActors;

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

	virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line);

	void AddLine(const std::wstring & str, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255));

	void puts(const std::wstring & str);

	bool ExecuteCode(const char * code) throw();

	virtual my::Effect * QueryShader(RenderPipeline::MeshType mesh_type, bool bInstance, const Material * material, unsigned int PassID);

	virtual void QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	void ResetViewedActors(const my::Vector3 & ViewPos);

	void SaveDialog(my::DialogPtr dlg, const char * path);

	my::DialogPtr LoadDialog(const char * path);

	void Game::SaveMaterial(MaterialPtr mat, const std::string & path);

	MaterialPtr LoadMaterial(const char * path);

	void SaveComponent(ComponentPtr cmp, const char * path);

	ComponentPtr LoadComponent(const char * path);

	void ImportScene(const char * path);

	void ImportStaticCollision(const char * path);
};
