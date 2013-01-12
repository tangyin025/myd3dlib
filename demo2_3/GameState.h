#pragma once

#include "Game.h"
#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include "Camera.h"
#include "EffectMesh.h"
#include "Character.h"
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

class GameStateLoad
	: public GameStateBase
{
public:
	GameStateLoad(void);

	~GameStateLoad(void);

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
};

class GameStateMain
	: public GameStateBase
{
public:
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

	boost::shared_ptr<Camera> m_Camera;

	typedef std::vector<EffectMeshPtr> EffectMeshPtrList;

	EffectMeshPtrList m_staticMeshes;

	typedef std::vector<CharacterPtr> CharacterPtrList;

	CharacterPtrList m_characters;

public:
	GameStateMain(void);

	~GameStateMain(void);

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

	void InsertStaticMesh(EffectMeshPtr effect_mesh)
	{
		m_staticMeshes.push_back(effect_mesh);
	}

	void InsertCharacter(CharacterPtr character)
	{
		m_characters.push_back(character);
	}
};
