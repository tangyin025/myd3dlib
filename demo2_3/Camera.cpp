#include "stdafx.h"
#include "Camera.h"
#include "Game.h"

using namespace my;

void Camera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_View = (Matrix4::RotationQuaternion(m_Orientation) * Matrix4::Translation(m_Position)).inverse();

	m_Proj = Matrix4::PerspectiveFovLH(m_Fovy, m_Aspect, m_Nz, m_Fz);
}

void ModuleViewCamera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	Mouse * mouse = Game::getSingleton().m_mouse.get();
	_ASSERT(mouse);

	if(!my::ResourceMgr::getSingleton().m_ControlFocus.lock() && mouse->IsButtonDown(0))
	{
		m_Rotation.x -= D3DXToRadian(mouse->GetY());
		m_Rotation.y -= D3DXToRadian(mouse->GetX());
	}

	m_View = Matrix4::Translation(-m_LookAt)
		* Matrix4::RotationY(m_Rotation.y)
		* Matrix4::RotationX(m_Rotation.x)
		* Matrix4::RotationZ(m_Rotation.z)
		* Matrix4::Translation(Vector3(0, 0, m_Distance));

	m_Proj = Matrix4::PerspectiveFovLH(m_Fovy, m_Aspect, m_Nz, m_Fz);
}
