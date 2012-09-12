#pragma once

#include <boost/shared_ptr.hpp>
#include <Windows.h>
#include "myException.h"

namespace my
{
	template <class DerivedClass>
	class Singleton
	{
	public:
		typedef boost::shared_ptr<DerivedClass> DrivedClassPtr;

		static DrivedClassPtr s_ptr;

	public:
		static DerivedClass * getSingletonPtr(void)
		{
			if(NULL == s_ptr)
			{
				s_ptr.reset(new DerivedClass());
			}
			return s_ptr.get();
		}

		static DerivedClass & getSingleton(void)
		{
			return *getSingletonPtr();
		}

	public:
		Singleton(void)
		{
			_ASSERT(NULL == s_ptr);
		}

		virtual ~Singleton(void)
		{
			_ASSERT(NULL != s_ptr); s_ptr.reset();
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

	class DeviceRelatedObjectBase
	{
	public:
		DeviceRelatedObjectBase(void);

		virtual ~DeviceRelatedObjectBase(void);

		virtual void OnResetDevice(void) = 0;

		virtual void OnLostDevice(void) = 0;

		virtual void OnDestroyDevice(void) = 0;
	};

	template <class DerivedClass> 
	class DeviceRelatedObject
		: public DeviceRelatedObjectBase
	{
	protected:
		HRESULT hr;

	public:
		DerivedClass * m_ptr;

	public:
		DeviceRelatedObject(void)
			: m_ptr(NULL)
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
};
