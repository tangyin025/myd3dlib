#include "stdafx.h"
#include "Camera.h"
#include "Game.h"

using namespace my;

void Camera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	// ! 通过 Position，Orientation逆向计算 View非常费时，派生类应当跳过这一步直接获得 View
	m_View = (Matrix4::RotationQuaternion(m_Orientation) * Matrix4::Translation(m_Position)).inverse();

	m_Proj = Matrix4::PerspectiveFovLH(m_Fovy, m_Aspect, m_Nz, m_Fz);
}

void ModelViewerCamera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	Mouse * mouse = Game::getSingleton().m_Mouse.get();

	_ASSERT(mouse);

	if(!my::Dialog::m_ControlFocus.lock())
	{
		if(mouse->IsButtonDown(0))
		{
			m_Rotation.x += D3DXToRadian(mouse->GetY());
			m_Rotation.y += D3DXToRadian(mouse->GetX());
		}

		m_Distance -= mouse->GetZ() / 120.0f;
	}

	m_Orientation = Quaternion::RotationYawPitchRoll(m_Rotation.y, m_Rotation.x, 0);

	m_Position = Vector3(0,0,-m_Distance).transform(m_Orientation) + m_LookAt;

	m_View = Matrix4::Translation(-m_LookAt)
		* Matrix4::RotationY(-m_Rotation.y)
		* Matrix4::RotationX(-m_Rotation.x)
		* Matrix4::RotationZ(-m_Rotation.z)
		* Matrix4::Translation(Vector3(0,0,m_Distance));

	m_Proj = Matrix4::PerspectiveFovLH(m_Fovy, m_Aspect, m_Nz, m_Fz);
}

void FirstPersonCamera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	Mouse * mouse = Game::getSingleton().m_Mouse.get();

	Keyboard * keyboard = Game::getSingleton().m_Keyboard.get();

	_ASSERT(mouse && keyboard);

	const float Movement = 5.0f * fElapsedTime;
	Vector3 Velocity(0,0,0);
	if(!my::Dialog::m_ControlFocus.lock())
	{
		if(mouse->IsButtonDown(0))
		{
			m_Rotation.x += D3DXToRadian(mouse->GetY());
			m_Rotation.y += D3DXToRadian(mouse->GetX());
		}

		if(keyboard->IsKeyDown(DIK_UP))
		{
			m_Rotation.x -= D3DXToRadian(3);
		}

		if(keyboard->IsKeyDown(DIK_DOWN))
		{
			m_Rotation.x += D3DXToRadian(3);
		}

		if(keyboard->IsKeyDown(DIK_LEFT))
		{
			m_Rotation.y -= D3DXToRadian(3);
		}

		if(keyboard->IsKeyDown(DIK_RIGHT))
		{
			m_Rotation.y += D3DXToRadian(3);
		}

		if(keyboard->IsKeyDown(DIK_W))
		{
			Velocity.z = Movement;
		}

		if(keyboard->IsKeyDown(DIK_S))
		{
			Velocity.z = -Movement;
		}

		if(keyboard->IsKeyDown(DIK_A))
		{
			Velocity.x = -Movement;
		}

		if(keyboard->IsKeyDown(DIK_D))
		{
			Velocity.x = Movement;
		}

		if(keyboard->IsKeyDown(DIK_PGUP))
		{
			Velocity.y = Movement;
		}

		if(keyboard->IsKeyDown(DIK_PGDN))
		{
			Velocity.y = -Movement;
		}
	}

	m_Orientation = Quaternion::RotationYawPitchRoll(m_Rotation.y, m_Rotation.x, 0);

	Velocity = Velocity.transform(m_Orientation);

	m_Position += Velocity;

	m_View = Matrix4::Translation(-m_Position)
		* Matrix4::RotationY(-m_Rotation.y)
		* Matrix4::RotationX(-m_Rotation.x)
		* Matrix4::RotationZ(-m_Rotation.z);

	m_Proj = Matrix4::PerspectiveFovLH(m_Fovy, m_Aspect, m_Nz, m_Fz);
}
