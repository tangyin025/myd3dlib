
#pragma once

namespace my
{
	template <class DrivedClass>
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

		virtual ~Singleton(void)
		{
			_ASSERT(NULL != s_ptr);
		}
	};

	template <class DrivedClass>
	class SingleInstance
	{
	public:
		static SingleInstance * s_ptr;

	public:
		static DrivedClass * getSingletonPtr(void)
		{
			_ASSERT(NULL != s_ptr);
			return dynamic_cast<DrivedClass *>(s_ptr);
		}

		static DrivedClass & getSingleton(void)
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
};
