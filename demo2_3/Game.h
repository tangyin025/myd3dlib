#pragma once

#include <myd3dlib.h>
#include <LuaContext.h>
#include "Console.h"

class LoaderMgr
	: public my::ResourceMgr
	, public ID3DXInclude
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

class Timer
{
public:
	const float m_Interval;

	float m_RemainingTime;

	my::ControlEvent m_EventTimer;

public:
	Timer(float Interval, float RemainingTime = 0)
		: m_Interval(Interval)
		, m_RemainingTime(Interval)
	{
	}
};

typedef boost::shared_ptr<Timer> TimerPtr;

class TimerMgr
{
protected:
	typedef std::map<TimerPtr, bool> TimerPtrSet;

	TimerPtrSet m_timerSet;

	const float m_MinRemainingTime;

	my::EventArgsPtr m_DefaultArgs;

public:
	TimerMgr(void)
		: m_MinRemainingTime(-1/10.0f)
		, m_DefaultArgs(new my::EventArgs())
	{
	}

	TimerPtr AddTimer(float Interval, my::ControlEvent EventTimer)
	{
		TimerPtr timer(new Timer(Interval, Interval));
		timer->m_EventTimer = EventTimer;
		InsertTimer(timer);
		return timer;
	}

	void InsertTimer(TimerPtr timer)
	{
		m_timerSet.insert(std::make_pair(timer, true));
	}

	void RemoveTimer(TimerPtr timer)
	{
		TimerPtrSet::iterator timer_iter = m_timerSet.find(timer);
		if(timer_iter != m_timerSet.end())
		{
			timer_iter->second = false;
		}
	}

	void RemoveAllTimer(void)
	{
		m_timerSet.clear();
	}

	void OnFrameMove(
		double fTime,
		float fElapsedTime);
};

class DialogMgr
{
public:
	typedef std::vector<my::DialogPtr> DialogPtrSet;

	DialogPtrSet m_dlgSet;

public:
	DialogMgr(void)
	{
	}

	static void UpdateDlgViewProj(my::DialogPtr dlg, const my::Vector2 & vp);

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
		const D3DSURFACE_DESC & desc = my::DxutApp::getSingleton().GetD3D9BackBufferSurfaceDesc();

		UpdateDlgViewProj(dlg, my::Vector2((float)desc.Width, (float)desc.Height));

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

	virtual void SetTexture(IDirect3DBaseTexture9 * pTexture);

	virtual void SetTransform(const my::Matrix4 & World, const my::Matrix4 & View, const my::Matrix4 & Proj);

	virtual void PushVertex(float x, float y, float u, float v, D3DCOLOR color);

	virtual void DrawVertexList(void);
};

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

class Game
	: public my::DxutApp
	, public LoaderMgr
	, public TimerMgr
	, public DialogMgr
{
public:
	my::LuaContextPtr m_lua;

	Timer m_Timer;

	my::UIRenderPtr m_UIRender;

	my::TexturePtr m_WhiteTexture;

	my::FontPtr m_Font;

	my::DialogPtr m_Console;

	MessagePanelPtr m_Panel;

	my::InputPtr m_Input;

	my::KeyboardPtr m_Keyboard;

	my::MousePtr m_Mouse;

	my::SoundPtr m_Sound;

	typedef stdext::hash_map<std::string, GameStateBasePtr> GameStateBasePtrMap;

	GameStateBasePtrMap m_stateMap;

	GameStateBasePtrMap::const_iterator m_CurrentStateIter;

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

	void AddLine(const std::wstring & str, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255))
	{
		m_Panel->AddLine(str, Color);
	}

	void puts(const std::wstring & str)
	{
		m_Panel->puts(str);
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
