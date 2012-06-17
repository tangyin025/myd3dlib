#include "Camera.h"

void Camera::UpdateViewProj(void)
{
	m_view = (my::Matrix4::Translation(m_pos) * my::Matrix4::RotationQuaternion(m_ori)).inverse();

	m_proj = my::Matrix4::PerspectiveFovLH(m_fovy, m_aspect, m_nz, m_fz);
}
