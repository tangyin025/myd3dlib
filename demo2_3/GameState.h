#pragma once

#include "Game.h"
#include "Character.h"
#include "PhysxSample.h"

// ! Release build with Pch will suffer LNK2001, ref: http://thread.gmane.org/gmane.comp.lib.boost.user/23065
template< class Event > void boost::statechart::detail::no_context<Event>::no_function( const Event & ) {}

class GameEventInit : public boost::statechart::event<GameEventInit>
{
};

class GameStateMain;

class GameStateInit
	: public GameStateBase
	, public boost::statechart::simple_state<GameStateInit, GameStateMachine>
{
public:
	typedef boost::statechart::transition<GameEventInit, GameStateMain> reactions;

	GameStateInit(void)
	{
	}

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
};

class GameStateMain
	: public GameStateBase
	, public boost::statechart::simple_state<GameStateMain, GameStateMachine>
	, public my::EmitterMgr
	, public PhysxSample
{
public:
	typedef boost::statechart::transition<GameEventInit, GameStateMain> reactions;

	my::EffectPtr m_SimpleSample;

	my::EffectPtr m_ShadowMap;

	my::TexturePtr m_ShadowTextureRT;

	my::SurfacePtr m_ShadowTextureDS;

	boost::shared_ptr<my::Camera> m_Camera;

	typedef std::vector<my::OgreMeshPtr> OgreMeshPtrList;

	OgreMeshPtrList m_StaticMeshes;

	typedef std::vector<CharacterPtr> CharacterPtrList;

	CharacterPtrList m_Characters;

public:
	GameStateMain(void)
	{
	}

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

	void InsertStaticMesh(my::OgreMeshPtr mesh)
	{
		m_StaticMeshes.push_back(mesh);
	}

	void InsertCharacter(CharacterPtr character)
	{
		m_Characters.push_back(character);
	}
};
