// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "myMath.h"

using namespace my;

Vector2 Vector2::RandomUnit(void)
{
	float theta = Random(0.0f, 2.0f * D3DX_PI);
	return PolarToCartesian(1.0f, theta);
}

Vector2 Vector2::RandomUnitCircle(void)
{
	return RandomUnit() * powf(Random(0.0f, 1.0f), 1.0f / 2.0f);
}

const Vector2 Vector2::zero(0, 0);

const Vector2 Vector2::one(1, 1);

const Vector2 Vector2::unitX(1, 0);

const Vector2 Vector2::unitY(0, 1);

Vector3 Vector3::RandomUnit(void)
{
	float y = Random(-1.0f, 1.0f);
	float theta = Random(0.0f, 2.0f * D3DX_PI);
	float r = sqrtf(1.0f - y * y);
	return Vector3(r * cosf(theta), y, r * sinf(theta));
}

Vector3 Vector3::RandomUnitSphere(void)
{
	return RandomUnit() * powf(Random(0.0f, 1.0f), 1.0f / 3.0f);
}

const Vector3 Vector3::zero(0, 0, 0);

const Vector3 Vector3::one(1, 1, 1);

const Vector3 Vector3::unitX(1, 0, 0);

const Vector3 Vector3::unitY(0, 1, 0);

const Vector3 Vector3::unitZ(0, 0, 1);

const Vector3 Vector3::Gravity(0.0f, -9.81f, 0.0f);

const Vector4 Vector4::zero(0, 0, 0, 0);

const Vector4 Vector4::one(1, 1, 1, 1);

const Vector4 Vector4::unitX(1, 0, 0, 0);

const Vector4 Vector4::unitY(0, 1, 0, 0);

const Vector4 Vector4::unitZ(0, 0, 1, 0);

const Vector4 Vector4::unitW(0, 0, 0, 1);

Quaternion Quaternion::RotationFromTo(const Vector3 & from, const Vector3 & to, const Vector3 & fallback_axis)
{
	// Copy, since cannot modify local
	_ASSERT(IS_NORMALIZED(from) && IS_NORMALIZED(to));
	float d = from.dot(to);
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
			Vector3 axis = Vector3::unitX.cross(from);
			if (axis.magnitudeSq() < 0.000001f)
			{
				// pick another if colinear
				axis = Vector3::unitY.cross(from);
			}
			return RotationAxis(axis.normalize(), D3DX_PI);
		}
	}

	float s = sqrtf((1 + d) * 2);
	float invs = 1 / s;
	Vector3 c = from.cross(to);
	return Quaternion(c.x * invs, c.y * invs, c.z * invs, s * 0.5f).normalize();
}

Quaternion Quaternion::RotationFromToSafe(const Vector3 & from, const Vector3 & to)
{
	float lensq0 = from.magnitudeSq();
	float lensq1 = to.magnitudeSq();
	if (lensq0 < EPSILON_E12 || lensq1 < EPSILON_E12)
	{
		return Identity();
	}

	return RotationFromTo(from * (1.0f / sqrtf(lensq0)), to * (1.0f / sqrtf(lensq1)), Vector3::zero);
}

Vector3 Quaternion::toEulerAngles(void) const
{
	Matrix4 mat = Matrix4::RotationQuaternion(*this);
	return mat.toEulerAngles();
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

static void _SanitizeEuler(Vector3& euler)
{
	const float negativeFlip = -0.0001f;
	const float positiveFlip = (D3DX_PI * 2.0f) - 0.0001f;

	if (euler.x < negativeFlip)
		euler.x += 2.0f * D3DX_PI;
	else if (euler.x > positiveFlip)
		euler.x -= 2.0f * D3DX_PI;

	if (euler.y < negativeFlip)
		euler.y += 2.0f * D3DX_PI;
	else if (euler.y > positiveFlip)
		euler.y -= 2.0f * D3DX_PI;

	if (euler.z < negativeFlip)
		euler.z += 2.0f * D3DX_PI;
	else if (euler.z > positiveFlip)
		euler.z -= 2.0f * D3DX_PI;
}

Vector3 Matrix4::toEulerAngles(void) const
{
	// from http://www.geometrictools.com/Documentation/EulerAngles.pdf
	// XYZ order
	Vector3 ret;
	if (_13 < 0.999f)
	{
		if (_13 > -0.999f)
		{
			//M = [
			//	[cos¦Â * cos¦Ã,						cos¦Â * sin¦Ã,						-sin¦Â		],
			//	[sin¦Á * sin¦Â * cos¦Ã - cos¦Á * sin¦Ã,	sin¦Á * sin¦Â * sin¦Ã + cos¦Á * cos¦Ã,	sin¦Á * cos¦Â	],
			//	[cos¦Á * sin¦Â * cos¦Ã + sin¦Á * sin¦Ã,	cos¦Á * sin¦Â * sin¦Ã - sin¦Á * cos¦Ã,	cos¦Á * cos¦Â	]]
			ret.x = atan2f(_23, _33);
			ret.y = asinf(-_13);
			ret.z = atan2f(_12, _11);
			_SanitizeEuler(ret);
		}
		else
		{
			//M_¦Â = ¦Ð / 2 = [
			//	[0,				0,				-1],
			//	[sin(¦Á - ¦Ã),	cos(¦Á - ¦Ã),		0],
			//	[cos(¦Á - ¦Ã),	-sin(¦Á - ¦Ã),	0]]
			ret.x = atan2f(_21, _22);
			ret.y = D3DX_PI * 0.5f;
			ret.z = 0.0f;
			_SanitizeEuler(ret);
		}
	}
	else
	{
		//M_¦Â = -¦Ð / 2 = [
		//	[0,				0,				1],
		//	[-sin(¦Á + ¦Ã),	cos(¦Á + ¦Ã),		0],
		//	[-cos(¦Á + ¦Ã),	-sin(¦Á + ¦Ã),	0]]
		ret.x = atan2f(-_21, _22);
		ret.y = -D3DX_PI * 0.5f;
		ret.z = 0.0f;
		_SanitizeEuler(ret);
	}
	return ret;
}

Quaternion Matrix4::toRotation(void) const
{
	// This algorithm comes from  "Quaternion Calculus and Fast Animation",
	// Ken Shoemake, 1987 SIGGRAPH course notes
	// https://www.cs.cmu.edu/~kiranb/animation/p245-shoemake.pdf
	_ASSERT(IS_UNITED(determinant()));
	Quaternion q;
	float t = (*this)[0][0] + (*this)[1][1] + (*this)[2][2];
	if (t > 0)
	{
		t = sqrtf(t + 1.0f);
		q.w = 0.5f * t;
		t = 0.5f / t;
		q.x = ((*this)[1][2] - (*this)[2][1]) * t;
		q.y = ((*this)[2][0] - (*this)[0][2]) * t;
		q.z = ((*this)[0][1] - (*this)[1][0]) * t;
	}
	else
	{
		int i = 0;
		if ((*this)[1][1] > (*this)[0][0])
			i = 1;
		if ((*this)[2][2] > (*this)[i][i])
			i = 2;
		int j = (i + 1) % 3;
		int k = (j + 1) % 3;

		t = sqrtf((*this)[i][i] - (*this)[j][j] - (*this)[k][k] + 1.0f);
		_ASSERT(t > EPSILON_E12);
		(&q.x)[i] = -0.5f * t;
		t = 0.5f / t;
		q.w = ((*this)[k][j] - (*this)[j][k]) * t;
		(&q.x)[j] = -((*this)[j][i] + (*this)[i][j]) * t;
		(&q.x)[k] = -((*this)[k][i] + (*this)[i][k]) * t;
	}
	return q;
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
	return NormalPosition((v1 - v0).cross(v2 - v0).normalize(Vector3(1, 0, 0)), v0);
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
