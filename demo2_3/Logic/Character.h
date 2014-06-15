#pragma once

#include "../physx_ptr.hpp"

class Character
	: my::Particle
{
public:
	physx_ptr<PxController> m_controller;

public:
	Character(void)
		: Particle(my::Vector3(0,0,0), my::Vector3(0,0,0), my::Vector3(0,-9.8f,0), my::Vector3(0,0,0), 1, 0.8f)
	{
	}

	virtual ~Character(void)
	{
		Destroy();
	}

	void Create(void);

	void Update(float fElapsedTime);

	void Destroy(void);
};

typedef boost::shared_ptr<Character> CharacterPtr;
