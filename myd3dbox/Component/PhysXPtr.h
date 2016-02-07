#pragma once

template<class T>
struct PhysXDeleter
{
    typedef void result_type;

    typedef T * argument_type;

    void operator()(T * x) const
    {
		x->release();
    }
};

template<class T>
class PhysXPtr : public boost::shared_ptr<T>
{
public:
    PhysXPtr() BOOST_NOEXCEPT
		: shared_ptr()
    {
    }

    template<class Y>
    explicit PhysXPtr( Y * p ): 
		: shared_ptr<Y, PhysXDeleter<Y> >(p, PhysXDeleter<Y>())
    {
    }

	void reset() BOOST_NOEXCEPT
	{
		shared_ptr::reset();
	}

	template<class Y> void reset( Y * p )
    {
		shared_ptr::reset<Y, PhysXDeleter<Y> >(p, PhysXDeleter<Y>());
    }
};
//
//namespace boost {
//namespace serialization{
//
//template <class Archive, class T1, class T2>
//inline void serialize(Archive & ar, std::pair<T1, T2> & pair, const unsigned int version)
//{
//	ar & boost::serialization::make_nvp("first", pair.first);
//	ar & boost::serialization::make_nvp("second", pair.second);
//}
//
//template <class Archive>
//inline void serialize(Archive & ar, physx::PxTransform & trans, const unsigned int version)
//{
//	ar & boost::serialization::make_nvp("q", (my::Quaternion &)trans.q);
//	ar & boost::serialization::make_nvp("p", (my::Vector3 &)trans.p);
//}
//
//struct PxGeometryHelper : physx::PxGeometry
//{
//	using PxGeometry::mType;
//	PxGeometryHelper(physx::PxGeometryType::Enum type)
//		: PxGeometry(type)
//	{
//	}
//};
//
//template<class Archive>
//inline void save_construct_data(Archive & ar, const physx::PxGeometry * geometry, const unsigned int file_version)
//{
//    ar << boost::serialization::make_nvp("PxGeometry", static_cast<const PxGeometryHelper *>(geometry)->mType);
//}
//
//template<class Archive>
//inline void load_construct_data(Archive & ar, physx::PxGeometry * geometry, const unsigned int file_version)
//{
//	physx::PxGeometryType::Enum type;
//    ar >> boost::serialization::make_nvp("PxGeometry", type);
//    ::new(geometry) PxGeometryHelper(type);
//}
//
//template <class Archive>
//inline void serialize(Archive & ar, physx::PxGeometry & geometry, const unsigned int version)
//{
//	ar & boost::serialization::make_nvp("mType", static_cast<PxGeometryHelper &>(geometry).mType);
//}
//
//template <class Archive>
//inline void serialize(Archive & ar, physx::PxBoxGeometry & box, const unsigned int version)
//{
//	ar & boost::serialization::make_nvp("PxGeometry", boost::serialization::base_object<physx::PxGeometry>(*this));
//	ar & boost::serialization::make_nvp("hx", box.halfExtents.x);
//	ar & boost::serialization::make_nvp("hy", box.halfExtents.y);
//	ar & boost::serialization::make_nvp("hz", box.halfExtents.z);
//}
//
//}
//}
