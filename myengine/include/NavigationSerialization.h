#pragma once

#include <DetourNavMesh.h>
#include <boost/serialization/split_free.hpp>

BOOST_SERIALIZATION_SPLIT_FREE(dtNavMesh);

namespace boost {
	namespace serialization {

		template<class Archive>
		void save(Archive& ar, const dtNavMesh& mesh, unsigned int version);

		template<class Archive>
		void load(Archive& ar, dtNavMesh& mesh, unsigned int version);

	} // namespace serialization
} // namespace boost
