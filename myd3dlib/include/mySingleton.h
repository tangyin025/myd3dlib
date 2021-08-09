#pragma once

#include <boost/scoped_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/function.hpp>
#include <boost/signals2.hpp>
#include <boost/serialization/nvp.hpp>
#include <Windows.h>
#include "myException.h"

namespace my
{
	template <class DerivedClass>
	class Singleton
	{
	public:
		static DerivedClass * getSingletonPtr(void)
		{
			static boost::scoped_ptr<DerivedClass> ptr(new DerivedClass);
			return ptr.get();
		}

		static DerivedClass & getSingleton(void)
		{
			return *getSingletonPtr();
		}

	public:
		virtual ~Singleton(void)
		{
		}
	};

	template <class DerivedClass>
	class SingleInstance
	{
	public:
		static SingleInstance * s_ptr;

	public:
		static DerivedClass * getSingletonPtr(void)
		{
			return dynamic_cast<DerivedClass *>(s_ptr);
		}

		static DerivedClass & getSingleton(void)
		{
			_ASSERT(NULL != s_ptr);
			return *getSingletonPtr();
		}

	public:
		SingleInstance(void)
		{
			_ASSERT(NULL == s_ptr);
			s_ptr = this;
		}

		virtual ~SingleInstance(void)
		{
			_ASSERT(this == s_ptr);
			s_ptr = NULL;
		}
	};

	template <class DerivedClass>
	SingleInstance<DerivedClass> * SingleInstance<DerivedClass>::s_ptr(0);

	class DeviceResourceBase : public boost::intrusive_ref_counter<DeviceResourceBase>
	{
	public:
		const char * m_Key;

		DeviceResourceBase(void);

		virtual ~DeviceResourceBase(void);

		virtual void OnResetDevice(void)
		{
		}

		virtual void OnLostDevice(void)
		{
		}

		virtual void OnDestroyDevice(void)
		{
		}
	};

	typedef boost::intrusive_ptr<DeviceResourceBase> DeviceResourceBasePtr;

	template <class DerivedClass> 
	class D3DDeviceResource
		: public DeviceResourceBase
	{
	public:
		HRESULT hr;

		DerivedClass * m_ptr;

	public:
		D3DDeviceResource(void)
			: hr(S_OK)
			, m_ptr(NULL)
		{
		}

		virtual ~D3DDeviceResource(void)
		{
			// ! need to check if D3DContext.m_d3dDeviceSec is entered
			SAFE_RELEASE(m_ptr);
		}

		virtual void OnResetDevice(void)
		{
		}

		virtual void OnLostDevice(void)
		{
		}

		virtual void OnDestroyDevice(void)
		{
			SAFE_RELEASE(m_ptr);
		}
	};

	typedef std::vector<unsigned char> Cache;

	typedef boost::shared_ptr<Cache> CachePtr;

	class IStream
	{
	public:
		virtual ~IStream(void)
		{
		}

		virtual int read(void* buff, unsigned read_size) = 0;

		virtual long seek(long offset) = 0;

		virtual long tell(void) = 0;

		virtual unsigned long GetSize(void) = 0;

		virtual CachePtr GetWholeCache(void);
	};

	typedef boost::shared_ptr<IStream> IStreamPtr;

	class NamedObjectSerializationContext
	{
	public:
		std::string prefix;

		NamedObjectSerializationContext(const char * _prefix)
			: prefix(_prefix)
		{
		}
	};

	class NamedObject
	{
	protected:
		static unsigned int UniqueNameIndex;

		const char * m_Name;

		NamedObject(void)
			: m_Name(NULL)
		{
		}

	public:
		NamedObject(const char * Name);

		virtual ~NamedObject(void);

		static std::string MakeUniqueName(const char * Prefix);

		static void ResetUniqueNameIndex(void);

		void SetName(const char * Name);

		const char * GetName(void) const
		{
			return m_Name;
		}

		friend class boost::serialization::access;

		template<class Archive>
		void save(Archive & ar, const unsigned int version) const;

		template<class Archive>
		void load(Archive & ar, const unsigned int version);

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			boost::serialization::split_member(ar, *this, version);
		}
	};

	struct EventArg
	{
	public:
		EventArg(void)
		{
		}

		virtual ~EventArg(void)
		{
		}
	};

	typedef boost::function<void(EventArg *)> EventFunction;

	typedef boost::signals2::signal<void(EventArg *)> EventSignal;
}
