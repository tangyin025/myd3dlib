#pragma once

#include "../physx_ptr.hpp"

class Character
	: public my::Particle
	, public PxUserControllerHitReport
	, public PxControllerBehaviorCallback
{
public:
	physx_ptr<PxController> m_controller;

	float m_LookDir;

	enum MoveState
	{
		MoveStateFront	= 0x01,
		MoveStateBack	= 0x02,
		MoveStateLeft	= 0x04,
		MoveStateRight	= 0x08,
	};

	char m_MoveState;

public:
	Character(void);

	virtual ~Character(void);

	void Create(void);

	void Update(float fElapsedTime);

	void Destroy(void);

	void Jump(void);

	void AddMoveState(MoveState state);

	void RemoveMoveState(MoveState state);

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
	Player(void)
	{
	}
};

typedef boost::shared_ptr<Player> PlayerPtr;

class LocalPlayer : public Player
{
public:
	LocalPlayer(void)
	{
	}
};

typedef boost::shared_ptr<LocalPlayer> LocalPlayerPtr;
