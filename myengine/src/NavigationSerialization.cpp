#include "NavigationSerialization.h"
#include "mySingleton.h"
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <boost/serialization/split_free.hpp>
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>

static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';

static const int NAVMESHSET_VERSION = 1;

BOOST_CLASS_EXPORT(Navigation)

struct NavMeshSetHeader
{
	int magic;
	int version;
	int numTiles;
	dtNavMeshParams params;
};

struct NavMeshTileHeader
{
	dtTileRef tileRef;
	int dataSize;
};

BOOST_SERIALIZATION_SPLIT_FREE(dtNavMesh);

namespace boost {
	namespace serialization {

		template<class Archive>
		void save(Archive& ar, const dtNavMesh& mesh, unsigned int version)
		{
			NavMeshSetHeader header;
			header.magic = NAVMESHSET_MAGIC;
			header.version = NAVMESHSET_VERSION;
			header.numTiles = 0;
			for (int i = 0; i < mesh.getMaxTiles(); ++i)
			{
				const dtMeshTile* tile = mesh.getTile(i);
				if (!tile || !tile->header || !tile->dataSize) continue;
				header.numTiles++;
			}
			ar << BOOST_SERIALIZATION_NVP(header.magic);
			ar << BOOST_SERIALIZATION_NVP(header.version);
			ar << BOOST_SERIALIZATION_NVP(header.numTiles);
			ar << boost::serialization::make_nvp("header.params", boost::serialization::binary_object(mesh.getParams(), sizeof(dtNavMeshParams)));

			for (int i = 0; i < mesh.getMaxTiles(); ++i)
			{
				const dtMeshTile* tile = mesh.getTile(i);
				if (!tile || !tile->header || !tile->dataSize) continue;

				NavMeshTileHeader tileHeader;
				tileHeader.tileRef = mesh.getTileRef(tile);
				tileHeader.dataSize = tile->dataSize;
				ar << BOOST_SERIALIZATION_NVP(tileHeader.tileRef);
				ar << BOOST_SERIALIZATION_NVP(tileHeader.dataSize);
				ar << boost::serialization::make_nvp("tile.data", boost::serialization::binary_object(tile->data, tile->dataSize));
			}
		}

		template<class Archive>
		void load(Archive& ar, dtNavMesh& mesh, unsigned int version)
		{
			NavMeshSetHeader header;
			ar >> BOOST_SERIALIZATION_NVP(header.magic);
			ar >> BOOST_SERIALIZATION_NVP(header.version);
			ar >> BOOST_SERIALIZATION_NVP(header.numTiles);
			ar >> boost::serialization::make_nvp("header.params", boost::serialization::binary_object(&header.params, sizeof(dtNavMeshParams)));

			dtStatus status = mesh.init(&header.params);
			if (dtStatusFailed(status))
			{
				THROW_CUSEXCEPTION("init dtNavMesh failed");
			}

			for (int i = 0; i < header.numTiles; ++i)
			{
				NavMeshTileHeader tileHeader;
				ar >> BOOST_SERIALIZATION_NVP(tileHeader.tileRef);
				ar >> BOOST_SERIALIZATION_NVP(tileHeader.dataSize);
				unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
				if (!data)
				{
					THROW_CUSEXCEPTION("alloc tile data failed");
				}
				ar >> boost::serialization::make_nvp("tile.data", boost::serialization::binary_object(data, tileHeader.dataSize));

				mesh.addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
			}
		}

		template void save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive& ar, const dtNavMesh& navMesh, unsigned int version);

		template void load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive& ar, dtNavMesh& navMesh, unsigned int version);

	} // namespace serialization
} // namespace boost

Navigation::Navigation(void)
{

}

Navigation::Navigation(const char* Name)
	: Component(Name)
{

}

Navigation::~Navigation(void)
{

}

template<class Archive>
void Navigation::save(Archive& ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar << BOOST_SERIALIZATION_NVP(m_navMesh);
}

template<class Archive>
void Navigation::load(Archive& ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar >> BOOST_SERIALIZATION_NVP(m_navMesh);

	if (m_navMesh)
	{
		m_navQuery.reset(new dtNavMeshQuery);
		m_navQuery->init(m_navMesh.get(), 1024);
	}
}
