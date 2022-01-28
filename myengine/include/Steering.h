#pragma once

#include "myMath.h"
#include "DetourPathCorridor.h"
#include "DetourObstacleAvoidance.h"
#include "mySingleton.h"

class Actor;

/// The maximum number of corners a crowd agent will look ahead in the path.
/// This value is used for sizing the crowd agent corner buffers.
/// Due to the behavior of the crowd manager, the actual number of useful
/// corners will be one less than this number.
/// @ingroup crowd
static const int DT_CROWDAGENT_MAX_CORNERS = 4;

class ObstacleAvoidanceContext
	: public dtObstacleAvoidanceQuery
	, public my::SingleInstance<ObstacleAvoidanceContext>
{
public:
	ObstacleAvoidanceContext(void);

	virtual ~ObstacleAvoidanceContext(void);
};

class Steering
{
public:
	const Actor * m_Actor;

	my::Vector3 m_Forward;

	float m_Speed;

	float m_BrakingRate;

	float m_MaxSpeed;

	/// The path corridor the agent is using.
	dtPathCorridor m_corridor;

	my::Vector3 m_agentPos;

	my::Vector3 m_targetPos;

	dtPolyRef m_targetRef;

	/// The local path corridor corners for the agent. (Staight path.) [(x, y, z) * #ncorners]
	float m_cornerVerts[DT_CROWDAGENT_MAX_CORNERS * 3];

	/// The local path corridor corner flags. (See: #dtStraightPathFlags) [(flags) * #ncorners]
	unsigned char m_cornerFlags[DT_CROWDAGENT_MAX_CORNERS];

	/// The reference id of the polygon being entered at the corner. [(polyRef) * #ncorners]
	dtPolyRef m_cornerPolys[DT_CROWDAGENT_MAX_CORNERS];

	/// The number of corners.
	int m_ncorners;

public:
	Steering(const Actor * _Actor, float BrakingRate, float MaxSpeed);

	~Steering(void);

	my::Vector3 SeekDir(my::Vector3 Force, float dtime);

	my::Vector3 SeekTarget(const my::Vector3 & Target, float dtime);
};
