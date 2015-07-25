#include "stdafx.h"
#include "myMath.h"
#include <crtdbg.h>

using namespace my;

template <>
int my::Round<int>(int v, int min, int max)
{
	return v > max ? min + (max - v) % (max - min) : (v < min ? max - (min - v) % (max - min) : v);
}

template <>
float my::Round<float>(float v, float min, float max)
{
	return v > max ? min + fmod(max - v, max - min) : (v < min ? max - fmod(min - v, max - min) : v);
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

const Vector3 Vector3::unitX(1, 0, 0);

const Vector3 Vector3::unitY(0, 1, 0);

const Vector3 Vector3::unitZ(0, 0, 1);

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

const Vector4 Vector4::unitX(1, 0, 0, 0);

const Vector4 Vector4::unitY(0, 1, 0, 0);

const Vector4 Vector4::unitZ(0, 0, 1, 0);

const Vector4 Vector4::unitW(0, 0, 0, 1);

const Quaternion Quaternion::identity(Quaternion::Identity());

const Matrix4 Matrix4::identity(Matrix4::Identity());

Plane Plane::NormalDistance(const Vector3 & normal, float distance)
{
	_ASSERT(IS_NORMALIZED(normal));

	return Plane(normal.x, normal.y, normal.z, -distance);
}

Ray Ray::transform(const Matrix4 & m) const
{
	return Ray(p.transformCoord(m), d.transformNormal(m));
}

Ray & Ray::transformSelf(const Matrix4 & m)
{
	p = p.transformCoord(m);
	d = d.transformNormal(m);
	return *this;
}
