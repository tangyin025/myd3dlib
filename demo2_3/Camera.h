#pragma once

#include <myd3dlib.h>

class BaseCamera
{
public:
	float m_aspect;

	float m_nz;

	float m_fz;

	my::Matrix4 m_view;

	my::Matrix4 m_proj;

public:
	BaseCamera(float aspect, float nz, float fz)
		: m_aspect(aspect)
		, m_nz(nz)
		, m_fz(fz)
		, m_view(my::Matrix4::identity)
		, m_proj(my::Matrix4::identity)
	{
	}

	virtual ~BaseCamera(void)
	{
	}

	void UpdateAspect(float aspect)
	{
		m_aspect = aspect;
	}

	virtual void UpdateViewProj(void) = 0;
};

typedef boost::shared_ptr<BaseCamera> BaseCameraPtr;

class Camera
	: public BaseCamera
{
public:
	my::Vector3 m_pos;

	my::Quaternion m_ori;

	float m_fovy;

public:
	Camera(float fovy = D3DXToRadian(75.0f), float aspect = 1.333333f, float nz = 0.1f, float fz = 3000.0f)
		: BaseCamera(aspect, nz, fz)
		, m_pos(my::Vector3::zero)
		, m_ori(my::Quaternion::identity)
		, m_fovy(fovy)
	{
	}

	virtual void UpdateViewProj(void);
};
