#pragma once

#include <myd3dlib.h>
#include <LuaContext.h>
#include "Console.h"
#pragma warning(disable: 4819)
#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#pragma warning(default: 4819)

class DrawHelper
{
protected:
	HRESULT hr;

public:
	static void DrawLine(
		IDirect3DDevice9 * pd3dDevice,
		const my::Vector3 & v0,
		const my::Vector3 & v1,
		D3DCOLOR Color,
		const my::Matrix4 & world = my::Matrix4::identity);

	static void DrawSphere(
		IDirect3DDevice9 * pd3dDevice,
		float radius,
		D3DCOLOR Color,
		const my::Matrix4 & world = my::Matrix4::identity);

	static void DrawBox(
		IDirect3DDevice9 * pd3dDevice,
		const my::Vector3 & halfSize,
		D3DCOLOR Color,
		const my::Matrix4 & world = my::Matrix4::identity);

	static void DrawTriangle(
		IDirect3DDevice9 * pd3dDevice,
		const my::Vector3 & v0,
		const my::Vector3 & v1,
		const my::Vector3 & v2,
		D3DCOLOR Color,
		const my::Matrix4 & world = my::Matrix4::identity);

	static void DrawSpereStage(
		IDirect3DDevice9 * pd3dDevice,
		float radius,
		int VSTAGE_BEGIN,
		int VSTAGE_END,
		float offsetY,
		D3DCOLOR Color,
		const my::Matrix4 & world = my::Matrix4::identity);

	static void DrawCylinderStage(
		IDirect3DDevice9 * pd3dDevice,
		float radius,
		float y0,
		float y1,
		D3DCOLOR Color,
		const my::Matrix4 & world = my::Matrix4::identity);

	static void DrawCapsule(
		IDirect3DDevice9 * pd3dDevice,
		float radius,
		float height,
		D3DCOLOR Color,
		const my::Matrix4 & world = my::Matrix4::identity);
};

class GameStateBase
	: public DrawHelper
{
	friend class Game;

protected:
	bool m_DeviceObjectsCreated;

	bool m_DeviceObjectsReset;

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

class GameStateLoad;

typedef boost::statechart::event_base GameEventBase;

// ! Release build with Pch will suffer LNK2001, ref: http://thread.gmane.org/gmane.comp.lib.boost.user/23065
template< class Event > void boost::statechart::detail::no_context<Event>::no_function( const Event & ) {}

class GameLoader
	: public my::ResourceMgr
	, public ID3DXInclude
	, public my::DxutApp
{
protected:
	std::map<LPCVOID, my::CachePtr> m_cacheSet;

	CComPtr<ID3DXEffectPool> m_EffectPool;

	typedef std::map<std::string, boost::weak_ptr<my::DeviceRelatedObjectBase> > DeviceRelatedResourceSet;

	DeviceRelatedResourceSet m_resourceSet;

public:
	GameLoader(void);

	virtual ~GameLoader(void);

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

	// ! luabind cannt convert boost::shared_ptr<Derived Class> to base ptr
	boost::shared_ptr<my::BaseTexture> LoadTexture(const std::string & path);

	boost::shared_ptr<my::BaseTexture> LoadCubeTexture(const std::string & path);

	my::OgreMeshPtr LoadMesh(const std::string & path);

	my::OgreSkeletonAnimationPtr LoadSkeleton(const std::string & path);

	my::EffectPtr LoadEffect(const std::string & path);

	my::FontPtr LoadFont(const std::string & path, int height);
};

typedef boost::function<void (void)> TimerEvent;

class Timer
{
public:
	float m_Interval;

	float m_RemainingTime;

	TimerEvent m_EventTimer;

public:
	Timer(float Interval)
		: m_Interval(Interval)
		, m_RemainingTime(-Interval)
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
	}

	void RemoveTimer(TimerPtr timer)
	{
		m_timerSet.erase(timer);
	}

	void ClearAllTimer(void)
	{
		m_timerSet.clear();
	}

	void OnFrameMove(
		double fTime,
		float fElapsedTime);
};

class Game
	: public GameLoader
	, public boost::statechart::state_machine<Game, GameStateLoad>
	, public TimerMgr
{
public:
	GameStateBase * cs;

	my::LuaContextPtr m_lua;

	typedef std::vector<my::DialogPtr> DialogPtrSet;

	DialogPtrSet m_dlgSet;

	my::FontPtr m_font;

	my::DialogPtr m_console;

	MessagePanelPtr m_panel;

	my::InputPtr m_input;

	my::KeyboardPtr m_keyboard;

	my::MousePtr m_mouse;

	my::SoundPtr m_sound;

public:
	Game(void);

	virtual ~Game(void);

	GameStateBase * CurrentState(void)
	{
		return const_cast<GameStateBase *>(state_cast<const GameStateBase *>());
	}

	void SafeCreateCurrentState(void)
	{
		if((cs = CurrentState()) && !cs->m_DeviceObjectsCreated)
		{
			cs->OnCreateDevice(GetD3D9Device(), &m_BackBufferSurfaceDesc);
			cs->m_DeviceObjectsCreated = true;
		}
	}

	void SafeResetCurrentState(void)
	{
		if((cs = CurrentState()) && !cs->m_DeviceObjectsReset)
		{
			cs->OnResetDevice(GetD3D9Device(), &m_BackBufferSurfaceDesc);
			cs->m_DeviceObjectsReset = true;
		}
	}

	void SafeLostCurrentState(void)
	{
		if((cs = CurrentState()) && cs->m_DeviceObjectsReset)
		{
			cs->OnLostDevice();
			cs->m_DeviceObjectsReset = false;
		}
	}

	void SafeDestroyCurrentState(void)
	{
		if((cs = CurrentState()) && cs->m_DeviceObjectsCreated)
		{
			cs->OnDestroyDevice();
			cs->m_DeviceObjectsCreated = false;
		}
	}

	void process_event(const boost::statechart::event_base & evt)
	{
		SafeLostCurrentState();

		SafeDestroyCurrentState();

		try
		{
			state_machine::process_event(evt);

			// ! 当状态切换时发生异常 正确的做法应该是让状态切换失败，而不是继续下去
			SafeCreateCurrentState();

			SafeResetCurrentState();
		}
		catch(const my::Exception & e)
		{
			SafeLostCurrentState();

			SafeDestroyCurrentState();

			terminate();

			THROW_CUSEXCEPTION(e.GetDescription());
		}
	}

	my::ControlPtr GetPanel(void) const
	{
		return m_panel;
	}

	void SetPanel(my::ControlPtr panel)
	{
		m_panel = boost::dynamic_pointer_cast<MessagePanel>(panel);
	}

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

	void ToggleRef(void);

	void ExecuteCode(const char * code);

	void UpdateDlgViewProj(my::DialogPtr dlg);

	void InsertDlg(my::DialogPtr dlg)
	{
		UpdateDlgViewProj(dlg);

		m_dlgSet.push_back(dlg);
	}

	void ClearAllDlg(void)
	{
		m_dlgSet.clear();
	}
};

class GameEventLoadOver : public boost::statechart::event<GameEventLoadOver>
{
};
