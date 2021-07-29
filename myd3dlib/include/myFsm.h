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

		Fsm<StateType, EventType> * m_parent;

	public:
		Fsm(void)
			: m_current(NULL)
			, m_parent(NULL)
		{
		}

		virtual ~Fsm(void)
		{
			_ASSERT(NULL == m_parent);
		}

		typename StateMap::value_type * FindState(StateType * state)
		{
			StateMap::iterator state_iter = m_states.begin();
			for (; state_iter != m_states.end(); state_iter++)
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

		void AddState(StateType * state, StateType * parent = NULL)
		{
			_ASSERT(NULL == state->m_parent);

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
				parent_iter->first->AddState(state);
			}
			else
			{
				m_states.insert(std::make_pair(state, EventStateMap()));
				state->m_parent = this;
				state->OnAdd();

				if ((!m_parent || m_parent->m_current == this) && !m_current)
				{
					_ASSERT(m_states.size() == 1);
					SetState(state);
				}
			}
		}

		void AddTransition(StateType * src, const EventType & _event, StateType * dest)
		{
			StateMap::value_type * src_iter = FindState(src);
			_ASSERT(NULL != src_iter);
			_ASSERT(NULL != src_iter->first->m_parent);

			StateMap::const_iterator dest_iter = src_iter->first->m_parent->m_states.find(dest);
			if (dest_iter == src_iter->first->m_parent->m_states.end())
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
				m_current->SetState(NULL);

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

				if (!m_current->m_states.empty())
				{
					m_current->SetState(m_current->m_states.begin()->first);
				}
			}
		}

		void ProcessEvent(const EventType & _event)
		{
			if (m_current)
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
				else
				{
					src_iter->first->ProcessEvent(_event);
				}
			}
		}

		void ClearAllState(void)
		{
			SetState(NULL);
			StateMap::iterator state_iter = m_states.begin();
			for (; state_iter != m_states.end(); state_iter++)
			{
				_ASSERT(this == state_iter->first->m_parent);
				state_iter->first->ClearAllState();
				state_iter->first->m_parent = NULL;
			}
			m_states.clear();
		}
	};
}
