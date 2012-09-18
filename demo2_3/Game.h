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
#include "EffectMesh.h"

class GameStateBase
{
protected:
	HRESULT hr;

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

public:
	GameStateBase(void)
	{
	}

	virtual ~GameStateBase(void)
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

	typedef std::set<boost::weak_ptr<my::DeviceRelatedObjectBase> > DeviceRelatedResourceSet;

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

class Game
	: public GameLoader
	, public boost::statechart::state_machine<Game, GameStateLoad>
{
public:
	GameStateBase * cs;

	my::LuaContextPtr m_lua;

	typedef std::vector<my::DialogPtr> DialogPtrSet;

	DialogPtrSet m_dlgSet;

	my::FontPtr m_font;

	my::DialogPtr m_console;

	MessagePanelPtr m_panel;

	my::EffectPtr m_SimpleSample;

	my::EffectPtr m_ShadowMap;

	my::TexturePtr m_ShadowTextureRT;

	my::SurfacePtr m_ShadowTextureDS;

	my::TexturePtr m_ScreenTextureRT;

	my::SurfacePtr m_ScreenTextureDS;

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

	void ToggleFullScreen(void);

	void ToggleRef(void);

	void ChangeDevice(void);

	void ExecuteCode(const char * code);

	void UpdateDlgViewProj(my::DialogPtr dlg);

	void InsertDlg(my::DialogPtr dlg)
	{
		UpdateDlgViewProj(dlg);

		m_dlgSet.push_back(dlg);
	}
};

class GameEventLoadOver : public boost::statechart::event<GameEventLoadOver>
{
};
