
#pragma once

#include <boost/shared_ptr.hpp>
#include <crtdbg.h>

namespace my
{
	template <typename DrivedClass>
	class Singleton
	{
	public:
		typedef boost::shared_ptr<DrivedClass> DrivedClassPtr;

		static DrivedClassPtr s_ptr;

	public:
		static DrivedClassPtr getSingletonPtr(void)
		{
			if(NULL == s_ptr)
			{
				s_ptr = DrivedClassPtr(new DrivedClass());
			}
			return s_ptr;
		}

		static DrivedClass & getSingleton(void)
		{
			return *getSingletonPtr();
		}

	public:
		Singleton(void)
		{
			_ASSERT(NULL == s_ptr);
		}

		~Singleton(void)
		{
			_ASSERT(NULL != s_ptr);
		}
	};
};
