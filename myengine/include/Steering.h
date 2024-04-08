#pragma once

#include "Component.h"
#include "DetourPathCorridor.h"
#include "DetourLocalBoundary.h"
#include "DetourObstacleAvoidance.h"

class Navigation;

/// The maximum number of corners a crowd agent will look ahead in the path.
/// This value is used for sizing the crowd agent corner buffers.
/// Due to the behavior of the crowd manager, the actual number of useful
/// corners will be one less than this number.
/// @ingroup crowd
static const int DT_CROWDAGENT_MAX_CORNERS = 4;

class Steering : public Component
{
public:
	enum { TypeID = ComponentTypeSteering };

	/// The type of navigation mesh polygon the agent is currently traversing.
	/// @ingroup crowd
	enum CrowdAgentState
	{
		DT_CROWDAGENT_STATE_INVALID,		///< The agent is not in a valid state.
		DT_CROWDAGENT_STATE_WALKING,		///< The agent is traversing a normal navigation mesh polygon.
		DT_CROWDAGENT_STATE_OFFMESH,		///< The agent is traversing an off-mesh connection.
	};

	my::Vector3 m_Forward;

	float m_Speed;

	float m_MaxSpeed;

	float m_BrakingSpeed;

	float m_MaxAdjustedSpeed;

	Navigation* m_navi;

	/// The path corridor the agent is using.
	dtPathCorridor m_corridor;

	/// The local boundary data for the agent.
	dtLocalBoundary m_boundary;

	my::Vector3 m_agentPos;

	my::Vector3 m_targetPos;

	dtPolyRef m_targetRef;

	my::Vector3 m_targetRefPos;

	float m_targetReplanTime;				/// <Time since the agent's target was replanned.

	/// The local path corridor corners for the agent. (Staight path.) [(x, y, z) * #ncorners]
	float m_cornerVerts[DT_CROWDAGENT_MAX_CORNERS * 3];

	/// The local path corridor corner flags. (See: #dtStraightPathFlags) [(flags) * #ncorners]
	unsigned char m_cornerFlags[DT_CROWDAGENT_MAX_CORNERS];

	/// The reference id of the polygon being entered at the corner. [(polyRef) * #ncorners]
	dtPolyRef m_cornerPolys[DT_CROWDAGENT_MAX_CORNERS];

	/// The number of corners.
	int m_ncorners;

public:
	Steering(const char * Name, float MaxSpeed, float BrakingSpeed, float MaxAdjustedSpeed, Navigation * navi);

	~Steering(void);

	virtual DWORD GetComponentType(void) const
	{
		return TypeID;
	}

	my::Vector3 SeekDir(my::Vector3 Force, float dtime);

	CrowdAgentState SeekTarget(const my::Vector3 & Target, const dtQueryFilter & filter, float dtime, unsigned int filterWord0, my::Vector3 & desiredVel, my::Vector3 & startPos, my::Vector3 & endPos);
};
