#pragma once

template<class T>
struct PhysXDeleter
{
    typedef void result_type;

    typedef T * argument_type;

    void operator()(T * x) const
    {
		_ASSERT(x); x->release();
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

	//PhysXPtr(PhysXPtr const & r)
	//	: shared_ptr(r)
	//{
	//}

    template<class Y>
    explicit PhysXPtr( Y * p )
		: shared_ptr<Y>(p, PhysXDeleter<Y>())
    {
    }

	//PhysXPtr & operator=(PhysXPtr const & r) // never throws
	//{
	//	shared_ptr::operator =(r);
	//	return *this;
	//}

	void reset() BOOST_NOEXCEPT
	{
		shared_ptr::reset();
	}

	template<class Y> void reset( Y * p )
    {
		shared_ptr::reset<Y, PhysXDeleter<Y> >(p, PhysXDeleter<Y>());
    }
};
