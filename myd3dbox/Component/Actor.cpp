#include "stdafx.h"
#include "Actor.h"
#include "Animator.h"

using namespace my;

void Actor::Attacher::UpdateWorld(void)
{
	switch (m_Type)
	{
	case AttachTypeWorld:
		{
			m_World = m_Offset * m_Owner->m_World;
		}
		break;
	case AttachTypeSlot:
		{
			my::Matrix4 Slot = BoneList::UDQtoRM(m_Owner->m_Animator->m_DualQuats[m_SlotId]);
			m_World = m_Offset * Slot * m_Owner->m_World;
		}
		break;
	case AttachTypeAnimation:
		{
			m_World = m_Owner->m_World;
		}
		break;
	}
}
