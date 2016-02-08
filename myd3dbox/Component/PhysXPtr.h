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
    explicit PhysXPtr( Y * p )
		: shared_ptr<Y>(p, PhysXDeleter<Y>())
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
