#pragma once

template<class T> class PhysxPtr
{
private:

    // Borland 5.5.1 specific workaround
    typedef PhysxPtr<T> this_type;

public:
	static void Deletor(T * p) throw ()
	{
		p->release();
	}

    typedef T element_type;
    typedef T value_type;
    typedef T * pointer;
    typedef typename boost::detail::shared_ptr_traits<T>::reference reference;

    PhysxPtr(): px(0), pn() // never throws in 1.30+
    {
    }

    //template<class Y>
    //explicit PhysxPtr( Y * p ): px( p ), pn( p ) // Y must be complete
    //{
    //    boost::detail::sp_enable_shared_from_this( pn, p, p );
    //}

    //
    // Requirements: D's copy constructor must not throw
    //
    // PhysxPtr will release p by calling d(p)
    //

    template<class Y> PhysxPtr(Y * p): px(p), pn(p, Deletor)
    {
        boost::detail::sp_enable_shared_from_this( pn, p, p );
    }

    // As above, but with allocator. A's copy constructor shall not throw.

    template<class Y, class A> PhysxPtr( Y * p, A a ): px( p ), pn( p, Deletor, a )
    {
        boost::detail::sp_enable_shared_from_this( pn, p, p );
    }

//  generated copy constructor, assignment, destructor are fine...

//  except that Borland C++ has a bug, and g++ with -Wsynth warns
#if defined(__BORLANDC__) || defined(__GNUC__)

    PhysxPtr & operator=(PhysxPtr const & r) // never throws
    {
        px = r.px;
        pn = r.pn; // shared_count::op= doesn't throw
        return *this;
    }

#endif

    //template<class Y>
    //explicit PhysxPtr(weak_ptr<Y> const & r): pn(r.pn) // may throw
    //{
    //    // it is now safe to copy r.px, as pn(r.pn) did not throw
    //    px = r.px;
    //}

    template<class Y>
    PhysxPtr(PhysxPtr<Y> const & r): px(r.px), pn(r.pn) // never throws
    {
    }

    template<class Y>
    PhysxPtr(PhysxPtr<Y> const & r, boost::detail::static_cast_tag): px(static_cast<element_type *>(r.px)), pn(r.pn)
    {
    }

    template<class Y>
    PhysxPtr(PhysxPtr<Y> const & r, boost::detail::const_cast_tag): px(const_cast<element_type *>(r.px)), pn(r.pn)
    {
    }

    template<class Y>
    PhysxPtr(PhysxPtr<Y> const & r, boost::detail::dynamic_cast_tag): px(dynamic_cast<element_type *>(r.px)), pn(r.pn)
    {
        if(px == 0) // need to allocate new counter -- the cast failed
        {
            pn = boost::detail::shared_count();
        }
    }

    template<class Y>
    PhysxPtr(PhysxPtr<Y> const & r, boost::detail::polymorphic_cast_tag): px(dynamic_cast<element_type *>(r.px)), pn(r.pn)
    {
        if(px == 0)
        {
            boost::throw_exception(std::bad_cast());
        }
    }

#ifndef BOOST_NO_AUTO_PTR

    template<class Y>
    explicit PhysxPtr(std::auto_ptr<Y> & r): px(r.get()), pn()
    {
        Y * tmp = r.get();
        pn = boost::detail::shared_count(r);
        boost::detail::sp_enable_shared_from_this( pn, tmp, tmp );
    }

#if !defined( BOOST_NO_SFINAE ) && !defined( BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION )

    template<class Ap>
    explicit PhysxPtr( Ap r, typename boost::detail::sp_enable_if_auto_ptr<Ap, int>::type = 0 ): px( r.get() ), pn()
    {
        typename Ap::element_type * tmp = r.get();
        pn = boost::detail::shared_count( r );
        boost::detail::sp_enable_shared_from_this( pn, tmp, tmp );
    }


#endif // BOOST_NO_SFINAE, BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

#endif // BOOST_NO_AUTO_PTR

#if !defined(BOOST_MSVC) || (BOOST_MSVC >= 1300)

    template<class Y>
    PhysxPtr & operator=(PhysxPtr<Y> const & r) // never throws
    {
        px = r.px;
        pn = r.pn; // shared_count::op= doesn't throw
        return *this;
    }

#endif

#ifndef BOOST_NO_AUTO_PTR

    template<class Y>
    PhysxPtr & operator=( std::auto_ptr<Y> & r )
    {
        this_type(r).swap(*this);
        return *this;
    }

#if !defined( BOOST_NO_SFINAE ) && !defined( BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION )

    template<class Ap>
    typename boost::detail::sp_enable_if_auto_ptr< Ap, PhysxPtr & >::type operator=( Ap r )
    {
        this_type( r ).swap( *this );
        return *this;
    }


#endif // BOOST_NO_SFINAE, BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

#endif // BOOST_NO_AUTO_PTR

    void reset() // never throws in 1.30+
    {
        this_type().swap(*this);
    }

    //template<class Y> void reset(Y * p) // Y must be complete
    //{
    //    BOOST_ASSERT(p == 0 || p != px); // catch self-reset errors
    //    this_type(p).swap(*this);
    //}

    template<class Y> void reset( Y * p )
    {
        this_type( p ).swap( *this );
    }

    template<class Y, class A> void reset( Y * p, A a )
    {
        this_type( p, a ).swap( *this );
    }

    reference operator* () const // never throws
    {
        BOOST_ASSERT(px != 0);
        return *px;
    }

    T * operator-> () const // never throws
    {
        BOOST_ASSERT(px != 0);
        return px;
    }
    
    T * get() const // never throws
    {
        return px;
    }

    // implicit conversion to "bool"

#if defined(__SUNPRO_CC) && BOOST_WORKAROUND(__SUNPRO_CC, <= 0x580)

    operator bool () const
    {
        return px != 0;
    }

#elif defined( _MANAGED )

    static void unspecified_bool( this_type*** )
    {
    }

    typedef void (*unspecified_bool_type)( this_type*** );

    operator unspecified_bool_type() const // never throws
    {
        return px == 0? 0: unspecified_bool;
    }

#elif \
    ( defined(__MWERKS__) && BOOST_WORKAROUND(__MWERKS__, < 0x3200) ) || \
    ( defined(__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__ < 304) )

    typedef T * (this_type::*unspecified_bool_type)() const;
    
    operator unspecified_bool_type() const // never throws
    {
        return px == 0? 0: &this_type::get;
    }

#else 

    typedef T * this_type::*unspecified_bool_type;

    operator unspecified_bool_type() const // never throws
    {
        return px == 0? 0: &this_type::px;
    }

#endif

    // operator! is redundant, but some compilers need it

    bool operator! () const // never throws
    {
        return px == 0;
    }

    bool unique() const // never throws
    {
        return pn.unique();
    }

    long use_count() const // never throws
    {
        return pn.use_count();
    }

    void swap(PhysxPtr<T> & other) // never throws
    {
        std::swap(px, other.px);
        pn.swap(other.pn);
    }

    template<class Y> bool _internal_less(PhysxPtr<Y> const & rhs) const
    {
        return pn < rhs.pn;
    }

    void * _internal_get_deleter(std::type_info const & ti) const
    {
        return pn.get_deleter(ti);
    }

// Tasteless as this may seem, making all members public allows member templates
// to work in the absence of member template friends. (Matthew Langston)

#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS

private:

    template<class Y> friend class PhysxPtr;
    //template<class Y> friend class weak_ptr;


#endif

    T * px;                     // contained pointer
    boost::detail::shared_count pn;    // reference counter

};  // PhysxPtr

template<class T, class U> inline bool operator==(PhysxPtr<T> const & a, PhysxPtr<U> const & b)
{
    return a.get() == b.get();
}

template<class T, class U> inline bool operator!=(PhysxPtr<T> const & a, PhysxPtr<U> const & b)
{
    return a.get() != b.get();
}

#if __GNUC__ == 2 && __GNUC_MINOR__ <= 96

// Resolve the ambiguity between our op!= and the one in rel_ops

template<class T> inline bool operator!=(PhysxPtr<T> const & a, PhysxPtr<T> const & b)
{
    return a.get() != b.get();
}

#endif

template<class T, class U> inline bool operator<(PhysxPtr<T> const & a, PhysxPtr<U> const & b)
{
    return a._internal_less(b);
}

template<class T> inline void swap(PhysxPtr<T> & a, PhysxPtr<T> & b)
{
    a.swap(b);
}

template<class T, class U> PhysxPtr<T> static_pointer_cast(PhysxPtr<U> const & r)
{
    return PhysxPtr<T>(r, boost::detail::static_cast_tag());
}

template<class T, class U> PhysxPtr<T> const_pointer_cast(PhysxPtr<U> const & r)
{
    return PhysxPtr<T>(r, boost::detail::const_cast_tag());
}

template<class T, class U> PhysxPtr<T> dynamic_pointer_cast(PhysxPtr<U> const & r)
{
    return PhysxPtr<T>(r, boost::detail::dynamic_cast_tag());
}

// shared_*_cast names are deprecated. Use *_pointer_cast instead.

template<class T, class U> PhysxPtr<T> shared_static_cast(PhysxPtr<U> const & r)
{
    return PhysxPtr<T>(r, boost::detail::static_cast_tag());
}

template<class T, class U> PhysxPtr<T> shared_dynamic_cast(PhysxPtr<U> const & r)
{
    return PhysxPtr<T>(r, boost::detail::dynamic_cast_tag());
}

template<class T, class U> PhysxPtr<T> shared_polymorphic_cast(PhysxPtr<U> const & r)
{
    return PhysxPtr<T>(r, boost::detail::polymorphic_cast_tag());
}

template<class T, class U> PhysxPtr<T> shared_polymorphic_downcast(PhysxPtr<U> const & r)
{
    BOOST_ASSERT(dynamic_cast<T *>(r.get()) == r.get());
    return shared_static_cast<T>(r);
}

// get_pointer() enables boost::mem_fn to recognize PhysxPtr

template<class T> inline T * get_pointer(PhysxPtr<T> const & p)
{
    return p.get();
}
