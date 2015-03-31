#include "StdAfx.h"
#include "myEmitter.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

BOOST_CLASS_EXPORT(Emitter)

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

void Emitter::Spawn(const Vector3 & Position, const Vector3 & Velocity)
{
	if(m_ParticleList.size() < PARTICLE_INSTANCE_MAX)
	{
		m_ParticleList.push_back(std::make_pair(0.0f, Particle(Position, Velocity)));
	}
}

void Emitter::Update(float fElapsedTime)
{
	ParticlePairList::reverse_iterator part_iter = m_ParticleList.rbegin();
	for(; part_iter != m_ParticleList.rend(); part_iter++)
	{
		if((part_iter->first += fElapsedTime) < m_ParticleLifeTime)
		{
			UpdateParticle(part_iter->second, part_iter->first, fElapsedTime);
		}
		else
		{
			m_ParticleList.erase(m_ParticleList.begin(), m_ParticleList.begin() + (m_ParticleList.rend() - part_iter));
			break;
		}
	}
}

void Emitter::UpdateParticle(Particle & particle, float time, float fElapsedTime)
{
	particle.m_Position += particle.m_Velocity * fElapsedTime;

	particle.m_Color = D3DCOLOR_ARGB(
		(int)m_ParticleColorA.Interpolate(time, 255),
		(int)m_ParticleColorR.Interpolate(time, 255),
		(int)m_ParticleColorG.Interpolate(time, 255),
		(int)m_ParticleColorB.Interpolate(time, 255));

	particle.m_Texcoord1 = Vector4(
		m_ParticleSizeX.Interpolate(time, 1), m_ParticleSizeY.Interpolate(time, 1), m_ParticleAngle.Interpolate(time, 0), 1);

	const unsigned int AnimFrame = (unsigned int)(time * m_ParticleAnimFPS) % ((unsigned int)m_ParticleAnimColumn * m_ParticleAnimRow);
	particle.m_Texcoord2 = Vector4((float)(AnimFrame / m_ParticleAnimRow), (float)(AnimFrame % m_ParticleAnimColumn), 0, 0);
}

BOOST_CLASS_EXPORT(SphericalEmitter)

void SphericalEmitter::Update(float fElapsedTime)
{
	Emitter::Update(fElapsedTime);

	m_Time += fElapsedTime;

	m_RemainingSpawnTime += fElapsedTime;

	_ASSERT(m_SpawnInterval > 0);

	while(m_RemainingSpawnTime >= 0)
	{
		Spawn(
			Vector3(
				Random(m_Position.x - m_HalfSpawnArea.x, m_Position.x + m_HalfSpawnArea.x),
				Random(m_Position.y - m_HalfSpawnArea.y, m_Position.y + m_HalfSpawnArea.y),
				Random(m_Position.z - m_HalfSpawnArea.z, m_Position.z + m_HalfSpawnArea.z)),

			Vector3::SphericalToCartesian(
				m_SpawnSpeed,
				m_SpawnInclination.Interpolate(fmod(m_Time, m_SpawnLoopTime), 0),
				m_SpawnAzimuth.Interpolate(fmod(m_Time, m_SpawnLoopTime), 0)).transform(m_Orientation));

		m_RemainingSpawnTime -= m_SpawnInterval;
	}
}
