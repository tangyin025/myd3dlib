#include "myEmitter.h"
#include "myDxutApp.h"
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
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

template<class Archive>
void Emitter::ParticleList::save(Archive & ar, const unsigned int version) const
{
	capacity_type buffer_capacity = capacity();
	ar << BOOST_SERIALIZATION_NVP(buffer_capacity);
	boost::serialization::stl::save_collection<Archive, ParticleList>(ar, *this);
}

template<class Archive>
void Emitter::ParticleList::load(Archive & ar, const unsigned int version)
{
	capacity_type buffer_capacity;
	ar >> BOOST_SERIALIZATION_NVP(buffer_capacity);
	set_capacity(buffer_capacity);
	boost::serialization::item_version_type item_version(0);
	boost::serialization::collection_size_type count;
	ar >> BOOST_SERIALIZATION_NVP(count);
	ar >> BOOST_SERIALIZATION_NVP(item_version);
	resize(count);
	boost::serialization::stl::collection_load_impl<Archive, ParticleList>(ar, *this, count, item_version);
}

template
void Emitter::ParticleList::save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive & ar, const unsigned int version) const;

template
void Emitter::ParticleList::save<boost::archive::text_oarchive>(boost::archive::text_oarchive & ar, const unsigned int version) const;

template
void Emitter::ParticleList::save<boost::archive::binary_oarchive>(boost::archive::binary_oarchive & ar, const unsigned int version) const;

template
void Emitter::ParticleList::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const;

template
void Emitter::ParticleList::load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive & ar, const unsigned int version);

template
void Emitter::ParticleList::load<boost::archive::text_iarchive>(boost::archive::text_iarchive & ar, const unsigned int version);

template
void Emitter::ParticleList::load<boost::archive::binary_iarchive>(boost::archive::binary_iarchive & ar, const unsigned int version);

template
void Emitter::ParticleList::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version);

void Emitter::RemoveAllParticle(void)
{
	m_ParticleList.clear();
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
