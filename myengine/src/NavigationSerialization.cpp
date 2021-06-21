#include "NavigationSerialization.h"
#include "mySingleton.h"
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/binary_object.hpp>

static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';

static const int NAVMESHSET_VERSION = 1;

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
				THROW_CUSEXCEPTION("cannot init dtNavMesh");
			}

			for (int i = 0; i < header.numTiles; ++i)
			{
				NavMeshTileHeader tileHeader;
				ar >> BOOST_SERIALIZATION_NVP(tileHeader.tileRef);
				ar >> BOOST_SERIALIZATION_NVP(tileHeader.dataSize);
				std::vector<unsigned char> data(tileHeader.dataSize);
				ar >> boost::serialization::make_nvp("tile.data", boost::serialization::binary_object(&data[0], data.size()));

				mesh.addTile(&data[0], data.size(), DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
			}
		}

		template void save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive& ar, const dtNavMesh& navMesh, unsigned int version);

		template void load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive& ar, dtNavMesh& navMesh, unsigned int version);

	} // namespace serialization
} // namespace boost
