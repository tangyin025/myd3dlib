#include "stdafx.h"
#include "Controller.h"
#include "Character.h"

using namespace my;

void CharacterController::Update(float fElapsedTime)
{
	Character * character = dynamic_cast<Character *>(m_Actor);
	if (m_MoveAcceleration > 0)
	{
		character->m_Acceleration.x = -m_MoveAcceleration * sinf(m_MoveOrientation);
		character->m_Acceleration.y = -9.8f;
		character->m_Acceleration.z = -m_MoveAcceleration * cosf(m_MoveOrientation);
	}
	else
	{
		character->m_Acceleration.x = 0.0f;
		character->m_Acceleration.y = -9.8f;
		character->m_Acceleration.z = 0.0f;
	}
}
