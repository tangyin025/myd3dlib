#pragma once

#include <map>

namespace my
{
	template <typename StateType, typename EventType>
	class Fsm
	{
	public:
		typedef std::map<EventType, StateType *> EventStateMap;

		typedef std::map<StateType *, EventStateMap> StateMap;

		StateMap m_states;

		StateType * m_current;

	public:
		Fsm(void)
			: m_current(NULL)
		{
		}

		void AddState(StateType * state)
		{
			StateMap::const_iterator state_iter = m_states.find(state);
			if (state_iter != m_states.end())
			{
				_ASSERT(false);
				return;
			}
			else
			{
				_ASSERT(NULL == state->m_Owner);
				m_states.insert(std::make_pair(state, EventStateMap()));
				state->m_Owner = this;
				state->OnAdd();
			}
		}

		void AddTransition(StateType * src, const EventType & _event, StateType * dest)
		{
			StateMap::iterator src_iter = m_states.find(src);
			if (src_iter == m_states.end())
			{
				_ASSERT(false);
				return;
			}

			StateMap::const_iterator dest_iter = m_states.find(dest);
			if (dest_iter == m_states.end())
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
			if (m_current)
			{
				m_current->OnExit();
			}

			if (!state)
			{
				m_current = NULL;
				return;
			}

			StateMap::const_iterator state_iter = m_states.find(state);
			if (state_iter == m_states.end())
			{
				_ASSERT(false);
				return;
			}

			m_current = state_iter->first;
			if (m_current)
			{
				m_current->OnEnter();
			}
		}

		void ProcessEvent(const EventType & _event)
		{
			StateMap::const_iterator src_iter = m_states.find(m_current);
			if (src_iter == m_states.end())
			{
				_ASSERT(false);
				return;
			}

			EventStateMap::const_iterator event_iter = src_iter->second.find(_event);
			if (event_iter != src_iter->second.end())
			{
				SetState(event_iter->second);
			}
		}

		void ClearAllState(void)
		{
			SetState(NULL);
			StateMap::iterator state_iter = m_states.begin();
			for (; state_iter != m_states.end(); state_iter++)
			{
				_ASSERT(this == state_iter->first->m_Owner);
				state_iter->first->m_Owner = NULL;
			}
			m_states.clear();
		}

		size_t StatesCount(void)
		{
			return m_states.size();
		}
	};
}
