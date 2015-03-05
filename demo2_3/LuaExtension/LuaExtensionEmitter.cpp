#include "stdafx.h"
#include "LuaExtension.h"

using namespace luabind;

void ExportEmitter2Lua(lua_State * L)
{
	module(L)
	[
		class_<my::Spline, boost::shared_ptr<my::Spline> >("Spline")
			.def(constructor<>())
			.def("AddNode", (void (my::Spline::*)(float, float, float, float))&my::Spline::AddNode)
			.def("Interpolate", (float (my::Spline::*)(float, float) const)&my::Spline::Interpolate)

		, class_<my::Emitter, boost::shared_ptr<my::Emitter> >("Emitter")
			.def(constructor<>())
			.def_readwrite("Position", &my::SphericalEmitter::m_Position)
			.def_readwrite("Orientation", &my::SphericalEmitter::m_Orientation)
			.def_readwrite("ParticleLifeTime", &my::Emitter::m_ParticleLifeTime)
			.def_readonly("ParticleColorA", &my::Emitter::m_ParticleColorA)
			.def_readonly("ParticleColorR", &my::Emitter::m_ParticleColorR)
			.def_readonly("ParticleColorG", &my::Emitter::m_ParticleColorG)
			.def_readonly("ParticleColorB", &my::Emitter::m_ParticleColorB)
			.def_readonly("ParticleSizeX", &my::Emitter::m_ParticleSizeX)
			.def_readonly("ParticleSizeY", &my::Emitter::m_ParticleSizeY)
			.def_readonly("ParticleAngle", &my::Emitter::m_ParticleAngle)
			.def_readwrite("ParticleAnimFPS", &my::Emitter::m_ParticleAnimFPS)
			.def_readwrite("ParticleAnimColumn", &my::Emitter::m_ParticleAnimColumn)
			.def_readwrite("ParticleAnimRow", &my::Emitter::m_ParticleAnimRow)

		, class_<my::SphericalEmitter, my::Emitter, boost::shared_ptr<my::Emitter> >("SphericalEmitter")
			.def(constructor<>())
			.def_readonly("Time", &my::SphericalEmitter::m_Time)
			.def_readwrite("SpawnInterval", &my::SphericalEmitter::m_SpawnInterval)
			.def_readwrite("RemainingSpawnTime", &my::SphericalEmitter::m_RemainingSpawnTime)
			.def_readwrite("HalfSpawnArea", &my::SphericalEmitter::m_HalfSpawnArea)
			.def_readwrite("SpawnSpeed", &my::SphericalEmitter::m_SpawnSpeed)
			.def_readwrite("SpawnInclination", &my::SphericalEmitter::m_SpawnInclination)
			.def_readwrite("SpawnAzimuth", &my::SphericalEmitter::m_SpawnAzimuth)
			.def_readwrite("SpawnLoopTime", &my::SphericalEmitter::m_SpawnLoopTime)
	];
}
