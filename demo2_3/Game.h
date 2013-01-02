#pragma once

#include <myd3dlib.h>
#include <LuaContext.h>
#include "Console.h"
//
//class DrawHelper
//{
//protected:
//	HRESULT hr;
//
//public:
//	static void DrawLine(
//		IDirect3DDevice9 * pd3dDevice,
//		const my::Vector3 & v0,
//		const my::Vector3 & v1,
//		D3DCOLOR Color,
//		const my::Matrix4 & world = my::Matrix4::identity);
//
//	static void DrawSphere(
//		IDirect3DDevice9 * pd3dDevice,
//		float radius,
//		D3DCOLOR Color,
//		const my::Matrix4 & world = my::Matrix4::identity);
//
//	static void DrawBox(
//		IDirect3DDevice9 * pd3dDevice,
//		const my::Vector3 & halfSize,
//		D3DCOLOR Color,
//		const my::Matrix4 & world = my::Matrix4::identity);
//
//	static void DrawTriangle(
//		IDirect3DDevice9 * pd3dDevice,
//		const my::Vector3 & v0,
//		const my::Vector3 & v1,
//		const my::Vector3 & v2,
//		D3DCOLOR Color,
//		const my::Matrix4 & world = my::Matrix4::identity);
//
//	static void DrawSpereStage(
//		IDirect3DDevice9 * pd3dDevice,
//		float radius,
//		int VSTAGE_BEGIN,
//		int VSTAGE_END,
//		float offsetY,
//		D3DCOLOR Color,
//		const my::Matrix4 & world = my::Matrix4::identity);
//
//	static void DrawCylinderStage(
//		IDirect3DDevice9 * pd3dDevice,
//		float radius,
//		float y0,
//		float y1,
//		D3DCOLOR Color,
//		const my::Matrix4 & world = my::Matrix4::identity);
//
//	static void DrawCapsule(
//		IDirect3DDevice9 * pd3dDevice,
//		float radius,
//		float height,
//		D3DCOLOR Color,
//		const my::Matrix4 & world = my::Matrix4::identity);
//};

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

class LoaderMgr
	: public my::ResourceMgr
	, public ID3DXInclude
	, virtual public my::DxutApp
{
protected:
	std::map<LPCVOID, my::CachePtr> m_cacheSet;

	CComPtr<ID3DXEffectPool> m_EffectPool;

	typedef std::map<std::string, boost::weak_ptr<my::DeviceRelatedObjectBase> > DeviceRelatedResourceSet;

	DeviceRelatedResourceSet m_resourceSet;

	typedef std::map<std::string, boost::weak_ptr<my::OgreSkeletonAnimation> > OgreSkeletonAnimationSet;

	OgreSkeletonAnimationSet m_skeletonSet;

public:
	LoaderMgr(void);

	virtual ~LoaderMgr(void);

	virtual __declspec(nothrow) HRESULT __stdcall Open(
		D3DXINCLUDE_TYPE IncludeType,
		LPCSTR pFileName,
		LPCVOID pParentData,
		LPCVOID * ppData,
		UINT * pBytes);

	virtual __declspec(nothrow) HRESULT __stdcall Close(
		LPCVOID pData);

	virtual HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	virtual void OnLostDevice(void);

	virtual void OnDestroyDevice(void);

	template <class ResourceType>
	boost::shared_ptr<ResourceType> GetDeviceRelatedResource(const std::string & key, bool reload)
	{
		DeviceRelatedResourceSet::const_iterator res_iter = m_resourceSet.find(key);
		if(m_resourceSet.end() != res_iter)
		{
			boost::shared_ptr<my::DeviceRelatedObjectBase> res = res_iter->second.lock();
			if(res)
			{
				if(reload)
					res->OnDestroyDevice();

				return boost::dynamic_pointer_cast<ResourceType>(res);
			}
		}

		boost::shared_ptr<ResourceType> res(new ResourceType());
		m_resourceSet[key] = res;
		return res;
	}

	my::TexturePtr LoadTexture(const std::string & path, bool reload = false);

	my::CubeTexturePtr LoadCubeTexture(const std::string & path, bool reload = false);

	my::OgreMeshPtr LoadMesh(const std::string & path, bool reload = false);

	my::OgreSkeletonAnimationPtr LoadSkeleton(const std::string & path, bool reload = false);

	my::EffectPtr LoadEffect(const std::string & path, bool reload = false);

	my::FontPtr LoadFont(const std::string & path, int height, bool reload = false);
};

class DialogMgr
	: virtual public my::DxutApp
{
public:
	typedef std::vector<my::DialogPtr> DialogPtrSet;

	DialogPtrSet m_dlgSet;

public:
	DialogMgr(void)
	{
	}

	void OnAlign(void);

	void UpdateDlgViewProj(my::DialogPtr dlg);

	void Draw(
		my::UIRender * ui_render,
		double fTime,
		float fElapsedTime);

	bool MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam);

	void InsertDlg(my::DialogPtr dlg)
	{
		UpdateDlgViewProj(dlg);

		m_dlgSet.push_back(dlg);
	}

	void RemoveDlg(my::DialogPtr dlg)
	{
		DialogPtrSet::iterator dlg_iter = std::find(m_dlgSet.begin(), m_dlgSet.end(), dlg);
		if(dlg_iter != m_dlgSet.end())
		{
			m_dlgSet.erase(dlg_iter);
		}
	}

	void RemoveAllDlg(void)
	{
		m_dlgSet.clear();
	}
};

class Timer
{
public:
	float m_Interval;

	float m_RemainingTime;

	const unsigned int m_MaxIter;

	bool m_Running;

	my::ControlEvent m_EventTimer;

	my::EventArgsPtr m_DefaultArgs;

public:
	Timer(float Interval)
		: m_Interval(Interval)
		, m_RemainingTime(-Interval)
		, m_MaxIter(4)
		, m_Running(false)
		, m_DefaultArgs(new my::EventArgs())
	{
	}

	void OnFrameMove(
		double fTime,
		float fElapsedTime);
};

typedef boost::shared_ptr<Timer> TimerPtr;

class TimerMgr
{
protected:
	typedef std::set<TimerPtr> TimerPtrSet;

	TimerPtrSet m_timerSet;

public:
	TimerMgr(void)
	{
	}

	void InsertTimer(TimerPtr timer)
	{
		m_timerSet.insert(timer);

		timer->m_Running = true;
	}

	void RemoveTimer(TimerPtr timer)
	{
		m_timerSet.erase(timer);

		timer->m_Running = false;
	}

	void RemoveAllTimer(void)
	{
		m_timerSet.clear();
	}

	void OnFrameMove(
		double fTime,
		float fElapsedTime);
};

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

	virtual void SetTexture(my::TexturePtr texture);

	virtual void SetTransform(const my::Matrix4 & world, const my::Matrix4 & view, const my::Matrix4 & proj);

	virtual void PushVertex(float x, float y, float u, float v, D3DCOLOR color);

	virtual void DrawVertexList(void);
};

class Game
	: public LoaderMgr
	, public DialogMgr
	, public TimerMgr
{
public:
	my::LuaContextPtr m_lua;

	my::FontPtr m_font;

	my::UIRenderPtr m_uiRender;

	my::TexturePtr m_whiteTexture;

	my::DialogPtr m_console;

	MessagePanelPtr m_panel;

	my::InputPtr m_input;

	my::KeyboardPtr m_keyboard;

	my::MousePtr m_mouse;

	my::SoundPtr m_sound;

	typedef stdext::hash_map<std::string, GameStateBasePtr> GameStateBasePtrMap;

	GameStateBasePtrMap m_stateMap;

	GameStateBasePtrMap::const_iterator m_CurrentStateIter;

public:
	Game(void);

	virtual ~Game(void);

	void AddLine(const std::wstring & str, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255))
	{
		m_panel->AddLine(str, Color);
	}

	void puts(const std::wstring & str)
	{
		m_panel->puts(str);
	}

	static Game & getSingleton(void)
	{
		return *getSingletonPtr();
	}

	static Game * getSingletonPtr(void)
	{
		return dynamic_cast<Game *>(DxutApp::getSingletonPtr());
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
