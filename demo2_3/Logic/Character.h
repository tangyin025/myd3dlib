#pragma once

#include "../../myd3dbox/Component/PhysXPtr.h"
#include "../../myd3dbox/Component/Component.h"

class Character
	: public my::Particle
	, public PxUserControllerHitReport
	, public PxControllerBehaviorCallback
{
public:
	PhysXPtr<PxMaterial> m_PxMaterial;

	PhysXPtr<PxController> m_controller;

	enum HitState
	{
		HitStateGround	= 0x01,
	};

	unsigned int m_HitState;

public:
	Character(void);

	virtual ~Character(void);

	void CreateController(void);

	virtual void Update(float fElapsedTime);

	virtual void OnPxThreadSubstep(float dtime);

	virtual void onShapeHit(const PxControllerShapeHit& hit);

	virtual void onControllerHit(const PxControllersHit& hit);

	virtual void onObstacleHit(const PxControllerObstacleHit& hit);

	virtual PxU32 getBehaviorFlags(const PxShape& shape);

	virtual PxU32 getBehaviorFlags(const PxController& controller);

	virtual PxU32 getBehaviorFlags(const PxObstacle& obstacle);
};

typedef boost::shared_ptr<Character> CharacterPtr;

class Player : public Character
{
public:
	Player(void);
};

typedef boost::shared_ptr<Player> PlayerPtr;

class LocalPlayer : public Player
{
public:
	my::Vector3 m_LookAngles;

	my::Vector3 m_LookDir;

	float m_LookDist;

	float m_FaceAngle;

	float m_FaceAngleInerp;

	MeshComponentPtr m_MeshCmp;

	int m_InputLtRt;

	int m_InputUpDn;

public:
	LocalPlayer(void);

	~LocalPlayer(void);

	void CreateMeshComponent(void);

	virtual void Update(float fElapsedTime);

	void OnMouseMove(my::InputEventArg * arg);

	void OnMouseBtnDown(my::InputEventArg * arg);

	void OnMouseBtnUp(my::InputEventArg * arg);

	void OnKeyDown(my::InputEventArg * arg);

	void OnKeyUp(my::InputEventArg * arg);

	void OnJoystickAxisMove(my::InputEventArg * arg);

	void OnJoystickPovMove(my::InputEventArg * arg);

	void OnJoystickBtnDown(my::InputEventArg * arg);

	void OnJoystickBtnUp(my::InputEventArg * arg);
};

typedef boost::shared_ptr<LocalPlayer> LocalPlayerPtr;
