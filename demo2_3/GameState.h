#pragma once

#include "Game.h"
#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include "Camera.h"
#include "EffectMesh.h"
#include "Character.h"

class GameStateMain;

class GameStateLoad
	: public GameStateBase
	, public boost::statechart::simple_state<GameStateLoad, Game>
{
public:
	typedef boost::statechart::transition<GameEventLoadOver, GameStateMain> reactions;

	GameStateLoad(void);

	~GameStateLoad(void);

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
	, public boost::statechart::simple_state<GameStateMain, Game>
{
public:
	boost::shared_ptr<btDefaultCollisionConfiguration> m_collisionConfiguration;
	boost::shared_ptr<btCollisionDispatcher> m_dispatcher;
	boost::shared_ptr<btBroadphaseInterface> m_overlappingPairCache;
	boost::shared_ptr<btConstraintSolver> m_constraintSolver;
	boost::shared_ptr<btDiscreteDynamicsWorld> m_dynamicsWorld;
	btAlignedObjectArray<boost::shared_ptr<btCollisionShape> > m_collisionShapes;

	boost::shared_ptr<ModuleViewCamera> m_Camera;

	typedef std::vector<EffectMeshPtr> EffectMeshPtrList;

	EffectMeshPtrList m_staticMeshes;

	typedef std::vector<CharacterPtr> CharacterPtrList;

	CharacterPtrList m_characters;

public:
	GameStateMain(void);

	~GameStateMain(void);

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
