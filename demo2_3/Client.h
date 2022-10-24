#pragma once

#include "Console.h"
#include "LuaExtension.h"
#include "RenderPipeline.h"
#include "Actor.h"
#include "PhysxContext.h"
#include "SoundContext.h"
#include "myStateChart.h"
#include <boost/intrusive/list.hpp>
#include "SceneContext.h"

class dtNavMesh;

class dtNavMeshQuery;

struct PlayerData : public my::DeviceResourceBase
{
	double logintime;

	double gametime;

	int sceneid;

	my::Vector3 pos;

	float angle;

	int attrs[1024];

	int quests[1024];

	int items[1024];

	unsigned int itemstatus[_countof(items)];

	PlayerData(void);

	virtual ~PlayerData(void);

	template<class Archive>
	void save(Archive& ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive& ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	void OnResetDevice(void)
	{
	}

	void OnLostDevice(void)
	{
	}

	void OnDestroyDevice(void)
	{
	}

	void setattr(int i, int value)
	{
		my::Subscribe(attrs, i) = value;
	}

	int getattr(int i) const
	{
		return my::Subscribe(attrs, i);
	}

	void setquest(int i, int value)
	{
		my::Subscribe(quests, i) = value;
	}

	int getquest(int i) const
	{
		return my::Subscribe(quests, i);
	}

	void setitem(int i, int value)
	{
		my::Subscribe(items, i) = value;
	}

	int getitem(int i) const
	{
		return my::Subscribe(items, i);
	}

	void setitemstatus(int i, unsigned int value)
	{
		my::Subscribe(itemstatus, i) = value;
	}

	unsigned int getitemstatus(int i) const
	{
		return my::Subscribe(itemstatus, i);
	}
};

typedef boost::shared_ptr<PlayerData> PlayerDataPtr;

class PlayerDataRequest : public my::IORequest
{
protected:
	std::string m_path;

public:
	PlayerDataRequest(const PlayerData* data, const char* path, int Priority);

	virtual void LoadResource(void);

	virtual void CreateResource(LPDIRECT3DDEVICE9 pd3dDevice);
};

class StateBase
	: public my::StateChart<StateBase, std::string>
{
public:
	StateBase(void)
	{
	}

	virtual ~StateBase(void)
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

	virtual void OnUpdate(float fElapsedTime)
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

	virtual void OnGUI(my::UIRender * ui_render, float fElapsedTime, const my::Vector2 & Viewport)
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

	unsigned int m_InitPhysxSceneFlags;

	my::Vector3 m_InitPhysxSceneGravity;

	int m_InitIOThreadNum;

	bool m_InitLoadShaderCache;

	std::string m_InitFont;

	int m_InitFontHeight;

	int m_InitFontFaceIndex;

	std::string m_InitUIEffect;

	std::string m_InitScript;

	std::string m_InitLanguageId;

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
		KeyCheck,
		KeyLock,
		KeyMenu,
		KeyWeapon1,
		KeyWeapon2,
		KeyWeapon3,
		KeyWeapon4,
		KeyCount,
	};

	boost::array<InputMgr::KeyPairList, KeyCount> m_InitBindKeys;

	typedef boost::unordered_map<std::string, std::wstring> TranslationMap;

	TranslationMap m_TranslationMap;

	my::Vector3 m_ViewedCenter;

	float m_ViewedDist;

	float m_ActorCullingThreshold;

	bool m_ShowNavigation;

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

	virtual void OnControlSound(boost::shared_ptr<my::Wav> wav);

	virtual void OnControlFocus(my::Control * control);

	virtual std::wstring OnControlTranslate(const std::string& str);

	void SetTranslation(const std::string& key, const std::wstring& text);

	template <typename T>
	void LoadSceneAsync(const char * path, const char * prefix, const T & callback, int Priority = 0)
	{
		std::string key = SceneContextRequest::BuildKey(path, prefix);
		IORequestPtr request(new SceneContextRequest(path, prefix, Priority));
		LoadIORequestAsync(key, request, callback);
	}

	boost::shared_ptr<SceneContext> LoadScene(const char * path, const char * prefix);

	void GetLoadSceneProgress(const char * path, const char * prefix, int & ActorListSize, int & ActorProgress, int & DialogListSize, int & DialogProgress);

	boost::shared_ptr<PlayerData> LoadPlayerData(const char * path);

	template <typename T>
	void SavePlayerDataAsync(const PlayerData * data, const char * path, const T & callback, int Priority = 0)
	{
		if (FindIORequest(path))
		{
			THROW_CUSEXCEPTION("PlayerData is in writing!");
		}
		IORequestPtr request(new PlayerDataRequest(data, path, Priority));
		LoadIORequestAsync(path, request, callback);
	}

	void SavePlayerData(const PlayerData * data, const char * path);
};
