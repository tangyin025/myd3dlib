#pragma once

#include <myd3dlib.h>

class BaseCamera
{
public:
	float m_Aspect;

	float m_Nz;

	float m_Fz;

	my::Matrix4 m_View;

	my::Matrix4 m_Proj;

public:
	BaseCamera(float Aspect, float Nz, float Fz)
		: m_Aspect(Aspect)
		, m_Nz(Nz)
		, m_Fz(Fz)
		, m_View(my::Matrix4::identity)
		, m_Proj(my::Matrix4::identity)
	{
	}

	virtual ~BaseCamera(void)
	{
	}

	void UpdateAspect(float Aspect)
	{
		m_Aspect = Aspect;
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime) = 0;
};

typedef boost::shared_ptr<BaseCamera> BaseCameraPtr;

class Camera
	: public BaseCamera
{
public:
	my::Vector3 m_Position;

	my::Quaternion m_Orientation;

	float m_Fovy;

public:
	Camera(float Fovy = D3DXToRadian(75.0f), float Aspect = 1.333333f, float Nz = 0.1f, float Fz = 3000.0f)
		: BaseCamera(Aspect, Nz, Fz)
		, m_Position(my::Vector3::zero)
		, m_Orientation(my::Quaternion::identity)
		, m_Fovy(Fovy)
	{
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime);
};

class ModuleViewCamera
	: public BaseCamera
{
public:
	my::Vector3 m_LookAt;

	my::Vector3 m_Rotation;

	float m_Fovy;

	float m_Distance;

public:
	ModuleViewCamera(float Fovy = D3DXToRadian(75.0f), float Aspect = 1.333333f, float Nz = 0.1f, float Fz = 3000.0f)
		: BaseCamera(Aspect, Nz, Fz)
		, m_LookAt(my::Vector3::zero)
		, m_Rotation(my::Vector3::zero)
		, m_Fovy(Fovy)
		, m_Distance(0)
	{
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime);
};
