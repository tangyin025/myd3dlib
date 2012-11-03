#pragma once

#include <myd3dlib.h>

class BaseCamera
{
public:
	float m_Fovy;

	float m_Aspect;

	float m_Nz;

	float m_Fz;

	my::Matrix4 m_View;

	my::Matrix4 m_Proj;

	my::ControlEvent EventAlign;

public:
	BaseCamera(float Fovy, float Aspect, float Nz, float Fz)
		: m_Fovy(Fovy)
		, m_Aspect(Aspect)
		, m_Nz(Nz)
		, m_Fz(Fz)
		, m_View(my::Matrix4::identity)
		, m_Proj(my::Matrix4::identity)
	{
	}

	virtual ~BaseCamera(void)
	{
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

public:
	Camera(float Fovy = D3DXToRadian(75.0f), float Aspect = 1.333333f, float Nz = 0.1f, float Fz = 3000.0f)
		: BaseCamera(Fovy, Aspect, Nz, Fz)
		, m_Position(my::Vector3::zero)
		, m_Orientation(my::Quaternion::identity)
	{
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime);
};

class ModelViewerCamera
	: public Camera
{
public:
	my::Vector3 m_LookAt;

	my::Vector3 m_Rotation;

	float m_Distance;

public:
	ModelViewerCamera(float Fovy = D3DXToRadian(75.0f), float Aspect = 1.333333f, float Nz = 0.1f, float Fz = 3000.0f)
		: Camera(Fovy, Aspect, Nz, Fz)
		, m_LookAt(my::Vector3::zero)
		, m_Rotation(my::Vector3::zero)
		, m_Distance(0)
	{
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime);
};

class FirstPersonCamera
	: public Camera
{
public:
	my::Vector3 m_Rotation;

public:
	FirstPersonCamera(float Fovy = D3DXToRadian(75.0f), float Aspect = 1.333333f, float Nz = 0.1f, float Fz = 3000.0f)
		: Camera(Fovy, Aspect, Nz, Fz)
		, m_Rotation(0,0,0)
	{
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime);
};
