#pragma once

#include <boost/unordered_set.hpp>
#include "mySingleton.h"

namespace my
{
	class Log : public Singleton<Log>
	{
	public:
		enum LogType
		{
			LogTypeNone = 0,
			LogTypeWarning,
			LogTypeError,
		};

		class Listener
		{
		public:
			void OnOutput(LogType type, const std::basic_string<TCHAR> & text);
		};

		typedef boost::unordered_set<Listener *> ListenerPtrSet;

		ListenerPtrSet m_listenerSet;

	public:
		Log(void)
		{
		}

		virtual ~Log(void)
		{
		}

		void AddListener(Listener * listener)
		{
			m_listenerSet.insert(listener);
		}

		void RemoveListener(Listener * listener)
		{
			m_listenerSet.erase(listener);
		}

		void Output(LogType type, const std::basic_string<TCHAR> & text)
		{
			ListenerPtrSet::const_iterator listener_iter = m_listenerSet.begin();
			for(; listener_iter != m_listenerSet.end(); listener_iter++)
			{
				(*listener_iter)->OnOutput(type, text);
			}
		}
	};
}
