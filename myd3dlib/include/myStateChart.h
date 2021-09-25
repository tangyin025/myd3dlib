#pragma once

#include <map>

namespace my
{
	template <typename StateType, typename EventType>
	class StateChart
	{
	public:
		typedef std::map<EventType, StateType *> EventStateMap;

		typedef std::map<StateType *, EventStateMap> StateMap;

		StateMap m_States;

		StateType * m_Current;

		StateChart<StateType, EventType> * m_Parent;

	public:
		StateChart(void)
			: m_Current(NULL)
			, m_Parent(NULL)
		{
		}

		virtual ~StateChart(void)
		{
			_ASSERT(NULL == m_Parent);
		}

		typename StateMap::value_type * FindState(StateType * state)
		{
			StateMap::iterator state_iter = m_States.begin();
			for (; state_iter != m_States.end(); state_iter++)
			{
				StateMap::value_type * res = state_iter->first->FindState(state);
				if (res)
				{
					return res;
				}

				if (state_iter->first == state)
				{
					return &(*state_iter);
				}
			}
			return NULL;
		}

		void AddState(StateType * state, StateType * parent)
		{
			_ASSERT(NULL == state->m_Parent);

			StateMap::value_type * state_iter = FindState(state);
			if (state_iter)
			{
				_ASSERT(false);
				return;
			}

			if (parent)
			{
				StateMap::value_type * parent_iter = FindState(parent);
				_ASSERT(NULL != parent_iter);
				parent_iter->first->AddState(state, NULL);
			}
			else
			{
				m_States.insert(std::make_pair(state, EventStateMap()));
				state->m_Parent = this;
				state->OnAdd();

				if ((!m_Parent || m_Parent->m_Current == this) && !m_Current)
				{
					_ASSERT(m_States.size() == 1);
					SetState(state);
				}
			}
		}

		void AddTransition(StateType * src, const EventType & _event, StateType * dest)
		{
			StateMap::value_type * src_iter = FindState(src);
			_ASSERT(NULL != src_iter);
			_ASSERT(NULL != src_iter->first->m_Parent);

			StateMap::const_iterator dest_iter = src_iter->first->m_Parent->m_States.find(dest);
			if (dest_iter == src_iter->first->m_Parent->m_States.end())
			{
				_ASSERT(false);
				return;
			}

			EventStateMap::const_iterator event_iter = src_iter->second.find(_event);
			if (event_iter != src_iter->second.end())
			{
				_ASSERT(false);
				return;
			}

			src_iter->second.insert(EventStateMap::value_type(_event, dest));
		}

		void SetState(StateType * state)
		{
			if (m_Current)
			{
				m_Current->SetState(NULL);

				m_Current->OnExit();
			}

			if (!state)
			{
				m_Current = NULL;
				return;
			}

			StateMap::const_iterator state_iter = m_States.find(state);
			if (state_iter == m_States.end())
			{
				_ASSERT(false);
				return;
			}

			m_Current = state_iter->first;
			if (m_Current)
			{
				m_Current->OnEnter();

				if (!m_Current->m_States.empty())
				{
					m_Current->SetState(m_Current->m_States.begin()->first);
				}
			}
		}

		void ProcessEvent(const EventType & _event)
		{
			if (m_Current)
			{
				StateMap::const_iterator src_iter = m_States.find(m_Current);
				if (src_iter == m_States.end())
				{
					_ASSERT(false);
					return;
				}

				EventStateMap::const_iterator event_iter = src_iter->second.find(_event);
				if (event_iter != src_iter->second.end())
				{
					SetState(event_iter->second);
				}
				else
				{
					src_iter->first->ProcessEvent(_event);
				}
			}
		}

		void ClearAllState(void)
		{
			SetState(NULL);
			StateMap::iterator state_iter = m_States.begin();
			for (; state_iter != m_States.end(); state_iter++)
			{
				_ASSERT(this == state_iter->first->m_Parent);
				state_iter->first->ClearAllState();
				state_iter->first->m_Parent = NULL;
			}
			m_States.clear();
		}
	};
}
