#pragma once

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
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
			_ASSERT(NULL != s_ptr);
			return dynamic_cast<DerivedClass *>(s_ptr);
		}

		static DerivedClass & getSingleton(void)
		{
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

	class DeviceRelatedObjectBase
	{
	public:
		virtual ~DeviceRelatedObjectBase(void)
		{
		}

		virtual void OnResetDevice(void) = 0;

		virtual void OnLostDevice(void) = 0;

		virtual void OnDestroyDevice(void) = 0;
	};

	typedef boost::shared_ptr<DeviceRelatedObjectBase> DeviceRelatedObjectBasePtr;

	template <class DerivedClass> 
	class DeviceRelatedObject
		: public DeviceRelatedObjectBase
	{
	public:
		HRESULT hr;

		DerivedClass * m_ptr;

	public:
		DeviceRelatedObject(void)
			: hr(S_OK)
			, m_ptr(NULL)
		{
		}

		virtual ~DeviceRelatedObject(void)
		{
			SAFE_RELEASE(m_ptr);
		}

		void OnResetDevice(void)
		{
		}

		void OnLostDevice(void)
		{
		}

		void OnDestroyDevice(void)
		{
			SAFE_RELEASE(m_ptr);
		}
	};
}
