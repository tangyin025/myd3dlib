#pragma once

#include "../physx_ptr.hpp"

class LocalPlayer
	: my::Particle
{
public:
	physx_ptr<PxController> m_controller;

public:
	LocalPlayer(void)
		: Particle(my::Vector3(0,0,0), my::Vector3(0,0,0), my::Vector3(0,-9.8f,0), my::Vector3(0,0,0), 1, 0.8f)
	{
	}

	~LocalPlayer(void)
	{
		Destroy();
	}

	void Create(void);

	void Update(float fElapsedTime);

	void Destroy(void);
};

typedef boost::shared_ptr<LocalPlayer> LocalPlayerPtr;
