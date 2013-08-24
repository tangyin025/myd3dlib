#include "stdafx.h"
#include "myMath.h"
#include <crtdbg.h>

using namespace my;

float & Vector2::operator [](size_t i)
{
	_ASSERT(i < sizeof(*this) / sizeof(float)); return (&x)[i];
}

const float & Vector2::operator [](size_t i) const
{
	_ASSERT(i < sizeof(*this) / sizeof(float)); return (&x)[i];
}

const Vector2 Vector2::zero(0, 0);

const Vector2 Vector2::unitX(1, 0);

const Vector2 Vector2::unitY(0, 1);

float & Vector3::operator [](size_t i)
{
	_ASSERT(i < sizeof(*this) / sizeof(float)); return (&x)[i];
}

const float & Vector3::operator [](size_t i) const
{
	_ASSERT(i < sizeof(*this) / sizeof(float)); return (&x)[i];
}

const Vector3 Vector3::zero(0, 0, 0);

const Vector3 Vector3::unitX(1, 0, 0);

const Vector3 Vector3::unitY(0, 1, 0);

const Vector3 Vector3::unitZ(0, 0, 1);

float & Vector4::operator [](size_t i)
{
	_ASSERT(i < sizeof(*this) / sizeof(float)); return (&x)[i];
}

const float & Vector4::operator [](size_t i) const
{
	_ASSERT(i < sizeof(*this) / sizeof(float)); return (&x)[i];
}

const Vector4 Vector4::zero(0, 0, 0, 0);

const Vector4 Vector4::unitX(1, 0, 0, 0);

const Vector4 Vector4::unitY(0, 1, 0, 0);

const Vector4 Vector4::unitZ(0, 0, 1, 0);

const Vector4 Vector4::unitW(0, 0, 0, 1);

const Quaternion Quaternion::identity(Quaternion::Identity());

Vector4 & Matrix4::operator [](size_t i)
{
	_ASSERT(i < sizeof(*this) / sizeof(Vector4)); return ((Vector4 *)&_11)[i];
}

const Vector4 & Matrix4::operator [](size_t i) const
{
	_ASSERT(i < sizeof(*this) / sizeof(Vector4)); return ((Vector4 *)&_11)[i];
}

const Matrix4 Matrix4::identity(Matrix4::Identity());
