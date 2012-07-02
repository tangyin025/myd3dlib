#pragma once

#include <myd3dlib.h>
#include "Camera.h"

class BaseScene
{
public:
	static void DrawLine(
		IDirect3DDevice9 * pd3dDevice,
		const my::Vector3 & v0,
		const my::Vector3 & v1,
		D3DCOLOR Color,
		const my::Matrix4 & world = my::Matrix4::identity);

	static void DrawSphere(
		IDirect3DDevice9 * pd3dDevice,
		float radius,
		D3DCOLOR Color,
		const my::Matrix4 & world = my::Matrix4::identity);

	static void DrawBox(
		IDirect3DDevice9 * pd3dDevice,
		const my::Vector3 & halfSize,
		D3DCOLOR Color,
		const my::Matrix4 & world = my::Matrix4::identity);

	static void DrawTriangle(
		IDirect3DDevice9 * pd3dDevice,
		const my::Vector3 & v0,
		const my::Vector3 & v1,
		const my::Vector3 & v2,
		D3DCOLOR Color,
		const my::Matrix4 & world = my::Matrix4::identity);

public:
	virtual ~BaseScene(void)
	{
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime) = 0;

	virtual void OnRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime) = 0;
};

typedef boost::shared_ptr<BaseScene> BaseScenePtr;

class RigidBody
	: public my::RigidBody
{
protected:
	typedef std::vector<my::CollisionPrimitivePtr> CollisionPrimitivePtrSet;

	CollisionPrimitivePtrSet m_shapes;

public:
	static my::Matrix4 CalculateInertiaTensor(float Ixx, float Iyy, float Izz, float Ixy = 0, float Ixz = 0, float Iyz = 0)
	{
		return my::Matrix4(
			 Ixx,			-Ixy,			-Ixz,			0,
			-Ixy,			 Iyy,			-Iyz,			0,
			-Ixz,			-Iyz,			 Izz,			0,
			0,				0,				0,				1);
	}

	static my::Matrix4 CalculateSphereInertiaTensor(float radius, float mass)
	{
		float coeff = 0.4f * mass * radius * radius;

		return CalculateInertiaTensor(coeff, coeff, coeff);
	}

	static my::Matrix4 CalculateBlockInertiaTensor(const my::Vector3 & halfSizes, float mass)
	{
		my::Vector3 squares = halfSizes * halfSizes;

		return CalculateInertiaTensor(
			0.3f * mass * (squares.y + squares.z),
			0.3f * mass * (squares.x + squares.z),
			0.3f * mass * (squares.x + squares.y));
	}

public:
	RigidBody(void)
	{
	}

	void InsertShape(my::CollisionPrimitivePtr shape)
	{
		shape->setRigidBody(this);

		m_shapes.push_back(shape);
	}

	void UpdateShapes(void)
	{
		CollisionPrimitivePtrSet::iterator lhs_iter = m_shapes.begin();
		for(; lhs_iter != m_shapes.end(); lhs_iter++)
		{
			(*lhs_iter)->calculateInternals();
		}
	}

	unsigned collide(const RigidBody * rhs, my::Contact * contacts, unsigned limits)
	{
		unsigned used = 0;
		CollisionPrimitivePtrSet::const_iterator lhs_iter = m_shapes.begin();
		for(; lhs_iter != m_shapes.end(); lhs_iter++)
		{
			CollisionPrimitivePtrSet::const_iterator rhs_iter = rhs->m_shapes.begin();
			for(; rhs_iter != rhs->m_shapes.end(); rhs_iter++)
			{
				if(limits > used)
					used += (*lhs_iter)->collide((*rhs_iter).get(), contacts + used, limits - used);
				else
					return used;
			}
		}
		return used;
	}

	unsigned collideHalfSpace(
		const my::Vector3 & planeNormal,
		float planeDistance,
		float planeFriction,
		float planeRestitution,
		my::Contact * contacts,
		unsigned limits)
	{
		unsigned used = 0;
		CollisionPrimitivePtrSet::const_iterator lhs_iter = m_shapes.begin();
		for(; lhs_iter != m_shapes.end(); lhs_iter++)
		{
			if(limits > used)
				used += (*lhs_iter)->collideHalfSpace(planeNormal, planeDistance, planeFriction, planeRestitution, contacts + used, limits - used);
			else
				return used;
		}
		return used;
	}

	void DrawShapes(IDirect3DDevice9 * pd3dDevice, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255), D3DCOLOR SleepColor = D3DCOLOR_ARGB(255,213,213,213))
	{
		CollisionPrimitivePtrSet::const_iterator lhs_iter = m_shapes.begin();
		for(; lhs_iter != m_shapes.end(); lhs_iter++)
		{
			switch((*lhs_iter)->getPrimitiveType())
			{
			case my::CollisionPrimitive::PrimitiveTypeShere:
				{
					my::CollisionSphere * sphere = static_cast<my::CollisionSphere *>((*lhs_iter).get());
					BaseScene::DrawSphere(pd3dDevice, sphere->getRadius(), getAwake() ? Color : SleepColor, sphere->getTransform());
				}
				break;
			case my::CollisionPrimitive::PrimitiveTypeBox:
				{
					my::CollisionBox * box = static_cast<my::CollisionBox *>((*lhs_iter).get());
					BaseScene::DrawBox(pd3dDevice, box->getHalfSize(), getAwake() ? Color : SleepColor, box->getTransform());
				}
				break;
			}
		}
	}
};

typedef boost::shared_ptr<RigidBody> RigidBodyPtr;

class Scene
	: public BaseScene
	, public my::World
{
public:
	BaseCameraPtr m_Camera;

	unsigned used;

public:
	Scene(void)
		: my::World(1024,4,4)
	{
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime);

	virtual void OnRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime);

	void InsertBody(RigidBodyPtr body)
	{
		body->calculateDerivedData();

		bodyList.push_back(body);
	}

	unsigned generateContacts(my::Contact * contacts, unsigned limits);
};
