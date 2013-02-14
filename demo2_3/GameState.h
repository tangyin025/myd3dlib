#pragma once

#include "Game.h"
#include "EffectMesh.h"
#include "Character.h"

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
{
public:
	typedef boost::statechart::transition<GameEventInit, GameStateMain> reactions;

	boost::shared_ptr<btDefaultCollisionConfiguration> m_collisionConfiguration;
	boost::shared_ptr<btCollisionDispatcher> m_dispatcher;
	boost::shared_ptr<btBroadphaseInterface> m_overlappingPairCache;
	boost::shared_ptr<btConstraintSolver> m_constraintSolver;
	boost::shared_ptr<btDiscreteDynamicsWorld> m_dynamicsWorld;
	btAlignedObjectArray<boost::shared_ptr<btCollisionShape> > m_collisionShapes;

	my::EffectPtr m_SimpleSample;

	my::EffectPtr m_ShadowMap;

	my::TexturePtr m_ShadowTextureRT;

	my::SurfacePtr m_ShadowTextureDS;

	boost::shared_ptr<my::Camera> m_Camera;

	typedef std::vector<EffectMeshPtr> EffectMeshPtrList;

	EffectMeshPtrList m_staticMeshes;

	typedef std::vector<CharacterPtr> CharacterPtrList;

	CharacterPtrList m_characters;

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

	void InsertStaticMesh(EffectMeshPtr effect_mesh)
	{
		m_staticMeshes.push_back(effect_mesh);
	}

	void InsertCharacter(CharacterPtr character)
	{
		m_characters.push_back(character);
	}
};
