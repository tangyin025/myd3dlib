#include "myEmitter.h"
#include "myDxutApp.h"
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

// ! must have virtual distructor or warning C4308, ref: http://stackoverflow.com/questions/26605497/boost-serialization-error-c4308-negative-integral-constant-converted-to-unsigne
//BOOST_CLASS_EXPORT(Emitter::Particle)
//

void Emitter::Spawn(const my::Vector3 & Position, const my::Vector3 & Velocity, const my::Vector4 & Color, const my::Vector2 & Size, float Angle, float Time)
{
	m_ParticleList.push_back(Particle(Position, Velocity, Color, Size, Angle, Time));
}

void Emitter::RemoveAllParticle(void)
{
	m_ParticleList.clear();
}

void Emitter::RemoveParticleBefore(float fTimeBefore)
{
	ParticleList::iterator part_iter = m_ParticleList.begin();
	for(; part_iter != m_ParticleList.end(); part_iter++)
	{
		if (part_iter->m_Time > fTimeBefore)
		{
			break;
		}
	}

	if (part_iter != m_ParticleList.begin())
	{
		m_ParticleList.rerase(m_ParticleList.begin(), part_iter);
	}
}
