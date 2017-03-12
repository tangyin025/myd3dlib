#include "StdAfx.h"
#include "myEmitter.h"
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

// ! must have virtual distructor or warning C4308, ref: http://stackoverflow.com/questions/26605497/boost-serialization-error-c4308-negative-integral-constant-converted-to-unsigne
//BOOST_CLASS_EXPORT(Emitter::Particle)
//
//BOOST_CLASS_EXPORT(Emitter::ParticlePair)

BOOST_CLASS_EXPORT(Emitter)

void Emitter::Spawn(const Vector3 & Position, const Vector3 & Velocity, const Vector4 & Color, const Vector2 & Size, float Angle)
{
	m_ParticleList.push_back(Particle(Position, Velocity, Color, Size, Angle, m_Time));
}

void Emitter::RemoveDeadParticle(float fParticleLifeTime)
{
	ParticleList::iterator part_iter = m_ParticleList.begin();
	for(; part_iter != m_ParticleList.end(); part_iter++)
	{
		if((m_Time - part_iter->m_Time) < fParticleLifeTime)
		{
			break;
		}
	}

	if (part_iter != m_ParticleList.begin())
	{
		m_ParticleList.rerase(m_ParticleList.begin(), part_iter);
	}
}

void Emitter::Update(float fElapsedTime)
{
	m_Time += fElapsedTime;
}
