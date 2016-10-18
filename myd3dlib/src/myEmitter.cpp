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

BOOST_CLASS_EXPORT(DynamicEmitter)

BOOST_CLASS_EXPORT(SphericalEmitter)

void Emitter::Spawn(const Vector3 & Position, const Vector3 & Velocity, const Vector4 & Color, const Vector2 & Size, float Angle)
{
	m_ParticleList.push_back(Particle(Position, Velocity, Color, Size, Angle, m_Time));
}

void Emitter::Reset(void)
{
}

void Emitter::Update(float fElapsedTime)
{
	m_Time += fElapsedTime;
}

void DynamicEmitter::Reset(void)
{
	m_ParticleList.clear();
}

void DynamicEmitter::Update(float fElapsedTime)
{
	Emitter::Update(fElapsedTime);

	ParticleList::iterator part_iter = m_ParticleList.begin();
	for(; part_iter != m_ParticleList.end(); part_iter++)
	{
		if((m_Time - part_iter->m_Time) < m_ParticleLifeTime)
		{
			break;
		}
	}

	if (part_iter != m_ParticleList.begin())
	{
		m_ParticleList.rerase(m_ParticleList.begin(), part_iter);
	}
}

void SphericalEmitter::Reset(void)
{
	DynamicEmitter::Reset();
	m_Time = 0;
	m_RemainingSpawnTime = 0;
}

void SphericalEmitter::Update(float fElapsedTime)
{
	DynamicEmitter::Update(fElapsedTime);

	m_RemainingSpawnTime += fElapsedTime;

	_ASSERT(m_SpawnInterval > 0);

	float SpawnTime = fmod(m_Time, m_SpawnLoopTime);

	while(m_RemainingSpawnTime >= 0)
	{
		Spawn(
			Vector3(
				Random(m_HalfSpawnArea.x, m_HalfSpawnArea.x),
				Random(m_HalfSpawnArea.y, m_HalfSpawnArea.y),
				Random(m_HalfSpawnArea.z, m_HalfSpawnArea.z)),
			Vector3::SphericalToCartesian(
				m_SpawnSpeed,
				m_SpawnInclination.Interpolate(SpawnTime, 0),
				m_SpawnAzimuth.Interpolate(SpawnTime, 0)),
			Vector4(
				m_SpawnColorR.Interpolate(SpawnTime, 1),
				m_SpawnColorG.Interpolate(SpawnTime, 1),
				m_SpawnColorB.Interpolate(SpawnTime, 1),
				m_SpawnColorA.Interpolate(SpawnTime, 1)),
			Vector2(
				m_SpawnSizeX.Interpolate(SpawnTime, 1)),
			m_SpawnAngle.Interpolate(SpawnTime, 0));

		m_RemainingSpawnTime -= m_SpawnInterval;
	}
}
