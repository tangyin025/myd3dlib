// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "myEmitter.h"
#include "myDxutApp.h"

using namespace my;

// ! must have virtual distructor or warning C4308, ref: http://stackoverflow.com/questions/26605497/boost-serialization-error-c4308-negative-integral-constant-converted-to-unsigne
//BOOST_CLASS_EXPORT(Emitter::Particle)
//

void Emitter::Spawn(const my::Vector4 & Position, const my::Vector4 & Velocity, const my::Vector4 & Color, const my::Vector2 & Size, float Angle, float Time)
{
	m_ParticleList.push_back(Particle(Position, Velocity, Color, Size, Angle, Time));
}

void Emitter::RemoveAllParticle(void)
{
	m_ParticleList.clear();
}
