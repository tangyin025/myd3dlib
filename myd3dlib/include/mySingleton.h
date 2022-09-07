#pragma once

#include <boost/scoped_ptr.hpp>
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
	class SingletonLocalThread
	{
	public:
		static DerivedClass* getSingletonPtr(void)
		{
			__declspec(thread) static boost::scoped_ptr<DerivedClass> ptr(new DerivedClass);
			return ptr.get();
		}

		static DerivedClass& getSingleton(void)
		{
			return *getSingletonPtr();
		}

	public:
		virtual ~SingletonLocalThread(void)
		{
		}
	};

	template <class DerivedClass>
	class SingletonInstance
	{
	public:
		static SingletonInstance * s_ptr;

	public:
		static DerivedClass * getSingletonPtr(void)
		{
			return static_cast<DerivedClass *>(s_ptr);
		}

		static DerivedClass & getSingleton(void)
		{
			_ASSERT(NULL != s_ptr);
			return *getSingletonPtr();
		}

	public:
		SingletonInstance(void)
		{
			_ASSERT(NULL == s_ptr);
			s_ptr = this;
		}

		virtual ~SingletonInstance(void)
		{
			_ASSERT(this == s_ptr);
			s_ptr = NULL;
		}
	};

	template <class DerivedClass>
	SingletonInstance<DerivedClass> * SingletonInstance<DerivedClass>::s_ptr(0);

	class DeviceResourceBase
	{
	public:
		const char * m_Key;

		DeviceResourceBase(void);

		virtual ~DeviceResourceBase(void);

		virtual void OnResetDevice(void) = 0;

		virtual void OnLostDevice(void) = 0;

		virtual void OnDestroyDevice(void) = 0;
	};

	typedef boost::shared_ptr<DeviceResourceBase> DeviceResourceBasePtr;

	template <class DerivedClass> 
	class D3DDeviceResource
		: public DeviceResourceBase
	{
	public:
		__declspec(thread) static HRESULT hr;

		DerivedClass * m_ptr;

	public:
		D3DDeviceResource(void)
			: m_ptr(NULL)
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

	template <class DerivedClass>
	HRESULT D3DDeviceResource<DerivedClass>::hr(S_OK);

	class ShaderResourceBase
	{
	public:
		ShaderResourceBase(void);

		virtual ~ShaderResourceBase(void);

		virtual void OnResetShader(void) = 0;
	};

	typedef std::vector<unsigned char> Cache;

	typedef boost::shared_ptr<Cache> CachePtr;

	class IStream
	{
	public:
		virtual ~IStream(void)
		{
		}

		virtual int read(void* buff, unsigned int read_size) = 0;

		virtual long seek(long offset, int origin) = 0;

		virtual long tell(void) = 0;

		virtual size_t GetSize(void) = 0;

		virtual CachePtr GetWholeCache(void);
	};

	typedef boost::shared_ptr<IStream> IStreamPtr;

	class NamedObjectSerializationContext
	{
	public:
		std::string prefix;

		bool make_unique;

		NamedObjectSerializationContext(void)
			: make_unique(false)
		{
		}
	};

	class NamedObject
	{
	protected:
		friend class D3DContext;

		const char * m_Name;

		NamedObject(void);

	public:
		NamedObject(const char * Name);

		virtual ~NamedObject(void);

		static std::string MakeUniqueName(const char * Prefix);

		void SetName(const char * Name);

		const char * GetName(void) const;

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

	typedef boost::function<bool(my::EventArg*)> EventCallback;

	typedef boost::signals2::signal<void(EventArg *)> EventSignal;
}
