#pragma once

#include "myMath.h"

class Actor;

class Steering
{
public:
	const Actor * m_Actor;

	my::Vector3 m_Forward;

	float m_Speed;

	float m_BrakingRate;

	float m_MaxSpeed;

public:
	Steering(const Actor * _Actor, float BrakingRate, float MaxSpeed);

	~Steering(void);

	my::Vector3 SeekDir(my::Vector3 Force, float dtime);

	my::Vector3 SeekTarget(const my::Vector3 & Target, float dtime);
};
