#pragma once

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
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

	class DeviceResourceBase
	{
	public:
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

	typedef boost::shared_ptr<DeviceResourceBase> DeviceResourceBasePtr;

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

	class IResourceCallback
	{
	public:
		friend class AsynchronousIOMgr;

		friend class ResourceMgr;

		IResourceCallback(void)
		{
		}

		virtual ~IResourceCallback(void);

		bool IsRequested(void) const;

		virtual void OnReady(DeviceResourceBasePtr res) = 0;
	};

	class Control;

	class ControlEventArgs
	{
	public:
		Control * sender;

		ControlEventArgs(Control * _sender)
			: sender(_sender)
		{
		}

		virtual ~ControlEventArgs(void)
		{
		}
	};

	typedef boost::function<void(ControlEventArgs *)> ControlEvent;
}
