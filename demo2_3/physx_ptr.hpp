#pragma once

#include <boost/smart_ptr/detail/atomic_count.hpp>

template<class T> class physx_ptr
{
public:
	physx_ptr() : px(0), pn(0)
	{
	}

	template<class Y>
	explicit physx_ptr(Y * p) : px(p), pn(0)
	{
		if(p)
		{
			pn = new boost::detail::atomic_count(1);
		}
	}

	~physx_ptr()
	{
		if(pn)
		{
			if(--*pn == 0)
			{
				px->release();
				delete pn;
			}
		}
	}

	physx_ptr(physx_ptr const & r) : px(r.px), pn(r.pn)
	{
		if(pn)
		{
			++*pn;
		}
	}

	template<class Y>
	physx_ptr(physx_ptr<Y> const & r) : px(r.px), pn(r.pn)
	{
		if(pn)
		{
			++*pn;
		}
	}

	//physx_ptr(physx_ptr & r) : px(0), pn(0)
	//{
	//	swap(r);
	//}

	//template<class Y>
	//physx_ptr(physx_ptr<Y> & r) : px(0), pn(0)
	//{
	//	swap(r);
	//}

	template<class Y>
	physx_ptr(physx_ptr<Y> const & r, T * p) : px(p), pn(r.pn)
	{
		if(pn)
		{
			++*pn;
		}
	}

	template<class Y>
	physx_ptr & operator=(physx_ptr<Y> const & r)
	{
		physx_ptr<T>(r).swap(*this);
		return *this;
	}

	void reset()
	{
		physx_ptr<T>().swap(*this);
	}

	template<class Y>
	void reset(Y * p = 0)
	{
		BOOST_ASSERT(p == 0 || p != px);
		physx_ptr<T>(p).swap(*this);
	}

	T & operator*() const
	{
		BOOST_ASSERT(px != 0);
		return *px;
	}

	T * operator->() const
	{
		BOOST_ASSERT(px != 0);
		return px;
	}

	T * get() const
	{
		return px;
	}

    operator bool () const
    {
        return px != 0;
    }

	long use_count() const
	{
		return *pn;
	}

	bool unique() const
	{
		return *pn == 1;
	}

	template<class Y>
	void swap(physx_ptr<Y> & other)
	{
		std::swap(px, other.px);
		std::swap(pn, other.pn);
	}

    boost::detail::atomic_count * pn;

	T * px;
};

template<class T, class U> inline bool operator==(physx_ptr<T> const & a, physx_ptr<U> const & b)
{
    return a.get() == b.get();
}

template<class T, class U> inline bool operator!=(physx_ptr<T> const & a, physx_ptr<U> const & b)
{
    return a.get() != b.get();
}

template<class T> inline bool operator<(physx_ptr<T> const & a, physx_ptr<T> const & b)
{
    return std::less<T*>()(a.get(), b.get());
}

template<class T> void swap(physx_ptr<T> & a, physx_ptr<T> & b)
{
    a.swap(b);
}

template<class T, class U> physx_ptr<T> static_pointer_cast( physx_ptr<U> const & r )
{
    return physx_ptr<T>(r, static_cast<T *>(r.get()));
}

template<class T, class U> physx_ptr<T> const_pointer_cast( physx_ptr<U> const & r )
{
    return physx_ptr<T>(r, const_cast<T *>(r.get()));
}

template<class T, class U> physx_ptr<T> dynamic_pointer_cast( physx_ptr<U> const & r )
{
    return physx_ptr<T>(r, dynamic_cast<T *>(r.get()));
}

template<class T, class U> physx_ptr<T> reinterpret_pointer_cast( physx_ptr<U> const & r )
{
    return physx_ptr<T>(r, reinterpret_cast<T *>(r.get()));
}

template<class T> inline T * get_pointer(physx_ptr<T> const & p)
{
    return p.get();
}
