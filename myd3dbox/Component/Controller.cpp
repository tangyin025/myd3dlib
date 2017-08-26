#include "stdafx.h"
#include "Controller.h"
#include "Character.h"

using namespace my;

void CharacterController::Update(float fElapsedTime)
{
	Controller::Update(fElapsedTime);

	_ASSERT(m_Actor->m_Type == Component::ComponentTypeCharacter);
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

	float Delta = fmod(m_MoveOrientation - character->m_Orientation + D3DX_PI, 2 * D3DX_PI) - D3DX_PI;
	const float Rotation = m_RotationSpeed * fElapsedTime;
	if (Delta > 0)
	{
		character->m_Orientation += Min(Delta, Rotation);
		character->UpdateWorld();
	}
	else
	{
		character->m_Orientation += Max(Delta, -Rotation);
		character->UpdateWorld();
	}
}
