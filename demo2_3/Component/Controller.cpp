#include "stdafx.h"
#include "Controller.h"
#include "Character.h"

using namespace my;

CharacterController::CharacterController(Character * character)
	: Controller(character)
	, m_Character(character)
{
}

void CharacterController::Update(float fElapsedTime)
{
}
