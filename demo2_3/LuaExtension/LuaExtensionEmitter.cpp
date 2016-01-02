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
			.def_readwrite("ParticleLifeTime", &my::Emitter::m_ParticleLifeTime)
			.def("Spawn", &my::Emitter::Spawn)

		, class_<my::SphericalEmitter, my::Emitter, boost::shared_ptr<my::Emitter> >("SphericalEmitter")
			.def(constructor<>())
			.def_readwrite("Time", &my::SphericalEmitter::m_Time)
			.def_readwrite("SpawnInterval", &my::SphericalEmitter::m_SpawnInterval)
			.def_readwrite("RemainingSpawnTime", &my::SphericalEmitter::m_RemainingSpawnTime)
			.def_readwrite("HalfSpawnArea", &my::SphericalEmitter::m_HalfSpawnArea)
			.def_readwrite("SpawnSpeed", &my::SphericalEmitter::m_SpawnSpeed)
			.def_readwrite("SpawnInclination", &my::SphericalEmitter::m_SpawnInclination)
			.def_readwrite("SpawnAzimuth", &my::SphericalEmitter::m_SpawnAzimuth)
			.def_readwrite("SpawnColorA", &my::SphericalEmitter::m_SpawnColorA)
			.def_readwrite("SpawnColorR", &my::SphericalEmitter::m_SpawnColorR)
			.def_readwrite("SpawnColorG", &my::SphericalEmitter::m_SpawnColorG)
			.def_readwrite("SpawnColorB", &my::SphericalEmitter::m_SpawnColorB)
			.def_readwrite("SpawnSizeX", &my::SphericalEmitter::m_SpawnSizeX)
			.def_readwrite("SpawnSizeY", &my::SphericalEmitter::m_SpawnSizeY)
			.def_readwrite("SpawnAngle", &my::SphericalEmitter::m_SpawnAngle)
			.def_readwrite("SpawnLoopTime", &my::SphericalEmitter::m_SpawnLoopTime)
	];
}
