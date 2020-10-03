#include "myMath.h"

using namespace my;

template <>
int my::Round<int>(int v, int min, int max)
{
	return v >= max ? min + (v - max) % (max - min) : (v < min ? max - (min - v) % (max - min) : v);
}

template <>
float my::Round<float>(float v, float min, float max)
{
	return v >= max ? min + fmodf(v - max, max - min) : (v < min ? max - fmodf(min - v, max - min) : v);
}

template <>
int my::Random<int>(int range)
{
	return rand() % range;
}

template <>
float my::Random<float>(float range)
{
	return range * ((float)rand() / RAND_MAX);
}

template <>
int my::Random<int>(int min, int max)
{
	return min + rand() % (max - min);
}

template <>
float my::Random<float>(float min, float max)
{
	return min + (max - min) * ((float)rand() / RAND_MAX);
}

Vector4 Vector2::transform(const Matrix4 & m) const
{
	return Vector4(x, y, 0, 1).transform(m);
}

Vector4 Vector2::transformTranspose(const Matrix4 & m) const
{
	return Vector4(x, y, 0, 1).transformTranspose(m);
}

Vector2 Vector2::transformCoord(const Matrix4 & m) const
{
	Vector4 ret = transform(m);

	return ret.xy / ret.w;
}

Vector2 Vector2::transformCoordTranspose(const Matrix4 & m) const
{
	Vector4 ret = transformTranspose(m);

	return ret.xy / ret.w;
}

Vector2 Vector2::transformNormal(const Matrix4 & m) const
{
	return Vector4(x, y, 0, 0).transform(m).xy;
}

Vector2 Vector2::transformNormalTranspose(const Matrix4 & m) const
{
	return Vector4(x, y, 0, 0).transformTranspose(m).xy;
}

const Vector2 Vector2::zero(0, 0);

const Vector2 Vector2::one(1, 1);

const Vector2 Vector2::unitX(1, 0);

const Vector2 Vector2::unitY(0, 1);

Vector4 Vector3::transform(const Matrix4 & m) const
{
	return Vector4(x, y, z, 1).transform(m);
}

Vector4 Vector3::transformTranspose(const Matrix4 & m) const
{
	return Vector4(x, y, z, 1).transformTranspose(m);
}

Vector3 Vector3::transformCoord(const Matrix4 & m) const
{
	Vector4 ret = transform(m);

	return ret.xyz / ret.w;
}

Vector3 Vector3::transformCoordTranspose(const Matrix4 & m) const
{
	Vector4 ret = transformTranspose(m);

	return ret.xyz / ret.w;
}

Vector3 Vector3::transformNormal(const Matrix4 & m) const
{
	return Vector4(x, y, z, 0).transform(m).xyz;
}

Vector3 Vector3::transformNormalTranspose(const Matrix4 & m) const
{
	return Vector4(x, y, z, 0).transformTranspose(m).xyz;
}

Vector3 Vector3::transform(const Quaternion & q) const
{
	Quaternion ret(q.conjugate() * Quaternion(x, y, z, 0) * q);

	return Vector3(ret.x, ret.y, ret.z);
}

const Vector3 Vector3::zero(0, 0, 0);

const Vector3 Vector3::one(1, 1, 1);

const Vector3 Vector3::unitX(1, 0, 0);

const Vector3 Vector3::unitY(0, 1, 0);

const Vector3 Vector3::unitZ(0, 0, 1);

const Vector3 Vector3::Gravity(0.0f, -9.81f, 0.0f);

Vector4 Vector4::transform(const Matrix4 & m) const
{
	return Vector4(
		x * m._11 + y * m._21 + z * m._31 + w * m._41,
		x * m._12 + y * m._22 + z * m._32 + w * m._42,
		x * m._13 + y * m._23 + z * m._33 + w * m._43,
		x * m._14 + y * m._24 + z * m._34 + w * m._44);
}

Vector4 Vector4::transformTranspose(const Matrix4 & m) const
{
	return Vector4(
		x * m._11 + y * m._12 + z * m._13 + w * m._14,
		x * m._21 + y * m._22 + z * m._23 + w * m._24,
		x * m._31 + y * m._32 + z * m._33 + w * m._34,
		x * m._41 + y * m._42 + z * m._43 + w * m._44);
}

const Vector4 Vector4::zero(0, 0, 0, 0);

const Vector4 Vector4::one(1, 1, 1, 1);

const Vector4 Vector4::unitX(1, 0, 0, 0);

const Vector4 Vector4::unitY(0, 1, 0, 0);

const Vector4 Vector4::unitZ(0, 0, 1, 0);

const Vector4 Vector4::unitW(0, 0, 0, 1);

Quaternion Quaternion::RotationFromTo(const Vector3 & from, const Vector3 & to, const Vector3 & fallback_axis)
{
	// Copy, since cannot modify local
	Vector3 v0 = from.normalize();
	Vector3 v1 = to.normalize();
	float d = v0.dot(v1);
	// If dot == 1, vectors are the same
	if (d >= 1.0f)
	{
		return Identity();
	}

	if (d < (EPSILON_E6 - 1.0f))
	{
		if (fallback_axis != Vector3::zero)
		{
			// rotate 180 degrees about the fallback axis
			return RotationAxis(fallback_axis, D3DX_PI);
		}
		else
		{
			// Generate an axis
			Vector3 axis = Vector3::unitX.cross(v0);
			if (axis.magnitudeSq() < 0.000001f)
			{
				// pick another if colinear
				axis = Vector3::unitY.cross(v0);
			}
			return RotationAxis(axis.normalize(), D3DX_PI);
		}
	}

	float s = sqrtf((1 + d) * 2);
	float invs = 1 / s;
	Vector3 c = v0.cross(v1);
	return Quaternion(c.x * invs, c.y * invs, c.z * invs, s * 0.5f).normalize();
}

const Quaternion Quaternion::identity(Quaternion::Identity());

const Matrix4 Matrix4::identity(Matrix4::Identity());

// convert unit dual quaternion to rotation matrix
Matrix4 Matrix4::UDQtoRM(const Matrix4 & dual)
{
	Matrix4 m ;
	float length = dual[0].magnitudeSq();
	float x = dual[0].x, y = dual[0].y, z = dual[0].z, w = dual[0].w;
	float t1 = dual[1].x, t2 = dual[1].y, t3 = dual[1].z, t0 = dual[1].w;

	m[0][0] = w*w + x*x - y*y - z*z; 
	m[1][0] = 2*x*y - 2*w*z; 
	m[2][0] = 2*x*z + 2*w*y;
	m[0][1] = 2*x*y + 2*w*z; 
	m[1][1] = w*w + y*y - x*x - z*z; 
	m[2][1] = 2*y*z - 2*w*x; 
	m[0][2] = 2*x*z - 2*w*y; 
	m[1][2] = 2*y*z + 2*w*x; 
	m[2][2] = w*w + z*z - x*x - y*y;

	m[3][0] = -2*t0*x + 2*t1*w - 2*t2*z + 2*t3*y ;
	m[3][1] = -2*t0*y + 2*t1*z + 2*t2*w - 2*t3*x ;
	m[3][2] = -2*t0*z - 2*t1*y + 2*t2*x + 2*t3*w ;

	m[0][3] = 0.0 ;
	m[1][3] = 0.0 ;
	m[2][3] = 0.0 ;
	m[3][3] = 1.0 ;            

	m /= length ;

	return m ;      
} 

Matrix4 TransformList::BuildSkinnedDualQuaternion(DWORD indices, const Vector4 & weights) const
{
	Matrix4 m = operator[](((unsigned char*)&indices)[0]);
	Vector4 dq0 = m[0];
	Matrix4 dual = m * weights.x;
	m = operator[](((unsigned char*)&indices)[1]);
	Vector4 dq = m[0];
	if (dq0.dot(dq) < 0)
		dual -= m * weights.y;
	else
		dual += m * weights.y;
	m = operator[](((unsigned char*)&indices)[2]);
	dq = m[0];
	if (dq0.dot(dq) < 0)
		dual -= m * weights.z;
	else
		dual += m * weights.z;
	m = operator[](((unsigned char*)&indices)[3]);
	dq = m[0];
	if (dq0.dot(dq) < 0)
		dual -= m * weights.w;
	else
		dual += m * weights.w;
	float length = dual[0].magnitude();
	dual = dual / length;
	return dual;
}

Vector3 TransformList::TransformVertexWithDualQuaternion(const Vector3 & position, const Matrix4 & dual)
{
	Vector3 outPosition = position + dual[0].xyz.cross(dual[0].xyz.cross(position) + position * dual[0].w) * 2;
	Vector3 translation = (dual[1].xyz * dual[0].w - dual[0].xyz * dual[1].w + dual[0].xyz.cross(dual[1].xyz)) * 2;
	outPosition += translation;
	return outPosition;
}

Vector3 TransformList::TransformVertexWithDualQuaternionList(const Vector3 & position, DWORD indices, const Vector4 & weights) const
{
	Matrix4 dual = BuildSkinnedDualQuaternion(indices, weights);
	return TransformVertexWithDualQuaternion(position, dual);
}

Plane Plane::NormalDistance(const Vector3 & normal, float distance)
{
	_ASSERT(IS_NORMALIZED(normal));

	return Plane(normal.x, normal.y, normal.z, -distance);
}

Plane Plane::NormalPosition(const Vector3 & normal, const Vector3 & pos)
{
	_ASSERT(IS_NORMALIZED(normal));

	return NormalDistance(normal, pos.dot(normal));
}

Plane Plane::FromTriangle(const Vector3 & v0, const Vector3 & v1, const Vector3 & v2)
{
	return NormalPosition((v1 - v0).cross(v2 - v0).normalize(), v0);
}

Plane Plane::transform(const Matrix4 & InverseTranspose) const
{
	Plane ret;
	D3DXPlaneTransform((D3DXPLANE *)&ret, (D3DXPLANE *)this, (D3DXMATRIX *)&InverseTranspose);
	return ret;
}

Plane & Plane::transformSelf(const Matrix4 & InverseTranspose)
{
	return *this = transform(InverseTranspose);
}

Ray Ray::transform(const Matrix4 & m) const
{
	return Ray(p.transformCoord(m), d.transformNormal(m).normalize());
}

Ray & Ray::transformSelf(const Matrix4 & m)
{
	p = p.transformCoord(m);
	d = d.transformNormal(m);
	return *this;
}

Frustum Frustum::transform(const Matrix4 & InverseTranspose) const
{
	return Frustum(
		Up.transform(InverseTranspose),
		Down.transform(InverseTranspose),
		Left.transform(InverseTranspose),
		Right.transform(InverseTranspose),
		Near.transform(InverseTranspose),
		Far.transform(InverseTranspose));
}

Frustum & Frustum::transformSelf(const Matrix4 & InverseTranspose)
{
	Up.transformSelf(InverseTranspose);
	Down.transformSelf(InverseTranspose);
	Left.transformSelf(InverseTranspose);
	Right.transformSelf(InverseTranspose);
	Near.transformSelf(InverseTranspose);
	Far.transformSelf(InverseTranspose);
	return *this;
}

const AABB AABB::invalid(FLT_MAX, -FLT_MAX);

template <>
AABB AABB::Slice<AABB::QuadrantPxPyPz>(const Vector3 & cente)
{
	return AABB(cente.x, cente.y, cente.z, m_max.x, m_max.y, m_max.z);
}

template <>
AABB AABB::Slice<AABB::QuadrantPxPyNz>(const Vector3 & cente)
{
	return AABB(cente.x, cente.y, m_min.z, m_max.x, m_max.y, cente.z);
}

template <>
AABB AABB::Slice<AABB::QuadrantPxNyPz>(const Vector3 & cente)
{
	return AABB(cente.x, m_min.y, cente.z, m_max.x, cente.y, m_max.z);
}

template <>
AABB AABB::Slice<AABB::QuadrantPxNyNz>(const Vector3 & cente)
{
	return AABB(cente.x, m_min.y, m_min.z, m_max.x, cente.y, cente.z);
}

template <>
AABB AABB::Slice<AABB::QuadrantNxPyPz>(const Vector3 & cente)
{
	return AABB(m_min.x, cente.y, cente.z, cente.x, m_max.y, m_max.z);
}

template <>
AABB AABB::Slice<AABB::QuadrantNxPyNz>(const Vector3 & cente)
{
	return AABB(m_min.x, cente.y, m_min.z, cente.x, m_max.y, cente.z);
}

template <>
AABB AABB::Slice<AABB::QuadrantNxNyPz>(const Vector3 & cente)
{
	return AABB(m_min.x, m_min.y, cente.z, cente.x, cente.y, m_max.z);
}

template <>
AABB AABB::Slice<AABB::QuadrantNxNyNz>(const Vector3 & cente)
{
	return AABB(m_min.x, m_min.y, m_min.z, cente.x, cente.y, cente.z);
}

AABB AABB::transform(const Matrix4 & m) const
{
	Vector3 v[] =
	{
		Vector3(m_min.x,m_min.y,m_min.z).transformCoord(m),
		Vector3(m_min.x,m_min.y,m_max.z).transformCoord(m),
		Vector3(m_min.x,m_max.y,m_min.z).transformCoord(m),
		Vector3(m_min.x,m_max.y,m_max.z).transformCoord(m),
		Vector3(m_max.x,m_min.y,m_min.z).transformCoord(m),
		Vector3(m_max.x,m_min.y,m_max.z).transformCoord(m),
		Vector3(m_max.x,m_max.y,m_min.z).transformCoord(m),
		Vector3(m_max.x,m_max.y,m_max.z).transformCoord(m)
	};

	AABB ret(v[0], v[0]);
	for (unsigned int i = 1; i < _countof(v); i++)
	{
		if (v[i].x < ret.m_min.x)
		{
			ret.m_min.x = v[i].x;
		}
		else if (v[i].x > ret.m_max.x)
		{
			ret.m_max.x = v[i].x;
		}

		if (v[i].y < ret.m_min.y)
		{
			ret.m_min.y = v[i].y;
		}
		else if (v[i].y > ret.m_max.y)
		{
			ret.m_max.y = v[i].y;
		}

		if (v[i].z < ret.m_min.z)
		{
			ret.m_min.z = v[i].z;
		}
		else if (v[i].z > ret.m_max.z)
		{
			ret.m_max.z = v[i].z;
		}
	}
	return ret;
}

AABB & AABB::transformSelf(const Matrix4 & m)
{
	return *this = transform(m);
}
