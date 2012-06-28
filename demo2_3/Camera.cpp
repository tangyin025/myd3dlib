#include "stdafx.h"
#include "Camera.h"

void Camera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_View = (my::Matrix4::RotationQuaternion(m_Orientation) * my::Matrix4::Translation(m_Position)).inverse();

	m_Proj = my::Matrix4::PerspectiveFovLH(m_Fovy, m_Aspect, m_Nz, m_Fz);
}
