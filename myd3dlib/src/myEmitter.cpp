#include "StdAfx.h"
#include "myEmitter.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

// ! must have virtual distructor or warning C4308, ref: http://stackoverflow.com/questions/26605497/boost-serialization-error-c4308-negative-integral-constant-converted-to-unsigne
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Emitter)

BOOST_CLASS_EXPORT(SphericalEmitter)

Emitter::~Emitter(void)
{
}

void Emitter::OnResetDevice(void)
{
}

void Emitter::OnLostDevice(void)
{
}

void Emitter::OnDestroyDevice(void)
{
}

void Emitter::Reset(void)
{
	m_ParticleList.clear();
}

void Emitter::Spawn(const Vector3 & Position, const Vector3 & Velocity, D3DCOLOR Color, const Vector2 & Size, float Angle)
{
	if(m_ParticleList.size() < PARTICLE_INSTANCE_MAX)
	{
		m_ParticleList.push_back(std::make_pair(0.0f, Particle(Position, Velocity, Color, Size, Angle)));
	}
}

void Emitter::Update(float fElapsedTime)
{
	ParticlePairList::reverse_iterator part_iter = m_ParticleList.rbegin();
	for(; part_iter != m_ParticleList.rend(); part_iter++)
	{
		if((part_iter->first += fElapsedTime) > m_ParticleLifeTime)
		{
			m_ParticleList.erase(m_ParticleList.begin(), m_ParticleList.begin() + (m_ParticleList.rend() - part_iter));
			break;
		}
	}
}

void SphericalEmitter::Update(float fElapsedTime)
{
	Emitter::Update(fElapsedTime);

	m_Time += fElapsedTime;

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
			D3DCOLOR_ARGB(
				(unsigned char)m_SpawnColorA.Interpolate(SpawnTime, 255),
				(unsigned char)m_SpawnColorR.Interpolate(SpawnTime, 255),
				(unsigned char)m_SpawnColorG.Interpolate(SpawnTime, 255),
				(unsigned char)m_SpawnColorB.Interpolate(SpawnTime, 255)),
			Vector2(
				m_SpawnSizeX.Interpolate(SpawnTime, 1)),
			m_SpawnAngle.Interpolate(SpawnTime, 0));

		m_RemainingSpawnTime -= m_SpawnInterval;
	}
}
