#include "StdAfx.h"
#include "myEmitter.h"
#include "myDxutApp.h"
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
//
//BOOST_CLASS_EXPORT(Emitter)

template<>
void boost::serialization::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const my::Emitter::ParticleList &t, const unsigned int version)
{
	boost::serialization::stl::save_collection<boost::archive::polymorphic_oarchive, boost::circular_buffer<Emitter::Particle> >(ar, t);
}

template<>
void boost::serialization::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, my::Emitter::ParticleList &t, const unsigned int version)
{
	boost::serialization::item_version_type item_version(0);
	boost::serialization::collection_size_type count;
	ar >> BOOST_SERIALIZATION_NVP(count);
	ar >> BOOST_SERIALIZATION_NVP(item_version);
	t.resize(count);
	boost::serialization::stl::collection_load_impl(ar, t, count, item_version);
}

void Emitter::Spawn(const Vector3 & Position, const Vector3 & Velocity, const Vector4 & Color, const Vector2 & Size, float Angle)
{
	m_ParticleList.push_back(Particle(Position, Velocity, Color, Size, Angle, D3DContext::getSingleton().m_fTotalTime));
}

void Emitter::RemoveDeadParticle(float fParticleLifeTime)
{
	ParticleList::iterator part_iter = m_ParticleList.begin();
	for(; part_iter != m_ParticleList.end(); part_iter++)
	{
		if ((D3DContext::getSingleton().m_fTotalTime - part_iter->m_Time) < fParticleLifeTime)
		{
			break;
		}
	}

	if (part_iter != m_ParticleList.begin())
	{
		m_ParticleList.rerase(m_ParticleList.begin(), part_iter);
	}
}
