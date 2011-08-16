
#pragma once

#include <d3dx9math.h>

namespace my
{
	class Vector4;

	class Matrix4;

	class Vector2
	{
	public:
		float x, y;

	public:
		Vector2(void)
			: x(0)
			, y(0)
		{
		}

		Vector2(float _x, float _y)
			: x(_x)
			, y(_y)
		{
		}

	public:
		Vector2 operator + (const Vector2 & rhs) const
		{
			return Vector2(x + rhs.x, y + rhs.y);
		}

		Vector2 operator + (float scaler) const
		{
			return Vector2(x + scaler, y + scaler);
		}

		Vector2 operator - (const Vector2 & rhs) const
		{
			return Vector2(x - rhs.x, y - rhs.y);
		}

		Vector2 operator - (float scaler) const
		{
			return Vector2(x - scaler, y - scaler);
		}

		Vector2 operator * (const Vector2 & rhs) const
		{
			return Vector2(x * rhs.x, y * rhs.y);
		}

		Vector2 operator * (float scaler) const
		{
			return Vector2(x * scaler, y * scaler);
		}

		Vector2 operator / (const Vector2 & rhs) const
		{
			return Vector2(x / rhs.x, y / rhs.y);
		}

		Vector2 operator / (float scaler) const
		{
			return Vector2(x / scaler, y / scaler);
		}

		Vector2 & operator += (const Vector2 & rhs)
		{
			x += rhs.x;
			y += rhs.y;
			return *this;
		}

		Vector2 & operator += (float scaler)
		{
			x += scaler;
			y += scaler;
			return *this;
		}

		Vector2 & operator -= (const Vector2 & rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}

		Vector2 & operator -= (float scaler)
		{
			x -= scaler;
			y -= scaler;
			return *this;
		}

		Vector2 & operator *= (const Vector2 & rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			return *this;
		}

		Vector2 & operator *= (float scaler)
		{
			x *= scaler;
			y *= scaler;
			return *this;
		}

		Vector2 & operator /= (const Vector2 & rhs)
		{
			x /= rhs.x;
			y /= rhs.y;
			return *this;
		}

		Vector2 & operator /= (float scaler)
		{
			x /= scaler;
			y /= scaler;
			return *this;
		}

	public:
		float dot(const Vector2 & rhs) const
		{
			return x * rhs.x + y * rhs.y;
		}

		float length(void) const
		{
			return sqrt(lengthSq());
		}

		float lengthSq(void) const
		{
			return x * x + y * y;
		}

		Vector2 lerp(const Vector2 & rhs, float s) const
		{
			return Vector2(
				x + s * (rhs.x - x),
				y + s * (rhs.y - y));
		}

		Vector2 normalize(void) const
		{
			float l = length();

			return Vector2(x / l, y / l);
		}

		Vector2 & normalizeSelf(void)
		{
			float l = length();
			x = x / l;
			y = y / l;
			return *this;
		}

		Vector4 transform(const Matrix4 & m) const;

	public:
		static const Vector2 zero;

		static const Vector2 unitX;

		static const Vector2 unitY;
	};

	class Vector3
	{
	public:
		float x, y, z;

	public:
		Vector3(void)
			: x(0)
			, y(0)
			, z(0)
		{
		}

		Vector3(float _x, float _y, float _z)
			: x(_x)
			, y(_y)
			, z(_z)
		{
		}

	public:
		Vector3 operator + (const Vector3 & rhs) const
		{
			return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
		}

		Vector3 operator + (float scaler) const
		{
			return Vector3(x + scaler, y + scaler, z + scaler);
		}

		Vector3 operator - (const Vector3 & rhs) const
		{
			return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
		}

		Vector3 operator - (float scaler) const
		{
			return Vector3(x - scaler, y - scaler, z - scaler);
		}

		Vector3 operator * (const Vector3 & rhs) const
		{
			return Vector3(x * rhs.x, y * rhs.y, z * rhs.z);
		}

		Vector3 operator * (float scaler) const
		{
			return Vector3(x * scaler, y * scaler, z * scaler);
		}

		Vector3 operator / (const Vector3 & rhs) const
		{
			return Vector3(x / rhs.x, y / rhs.y, z / rhs.z);
		}

		Vector3 operator / (float scaler) const
		{
			return Vector3(x / scaler, y / scaler, z / scaler);
		}

		Vector3 & operator += (const Vector3 & rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			return *this;
		}

		Vector3 & operator += (float scaler)
		{
			x += scaler;
			y += scaler;
			z += scaler;
			return *this;
		}

		Vector3 & operator -= (const Vector3 & rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			return *this;
		}

		Vector3 & operator -= (float scaler)
		{
			x -= scaler;
			y -= scaler;
			z -= scaler;
			return *this;
		}

		Vector3 & operator *= (const Vector3 & rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;
			return *this;
		}

		Vector3 & operator *= (float scaler)
		{
			x *= scaler;
			y *= scaler;
			z *= scaler;
			return *this;
		}
 
		Vector3 & operator /= (const Vector3 & rhs)
		{
			x /= rhs.x;
			y /= rhs.y;
			z /= rhs.z;
			return *this;
		}

		Vector3 & operator /= (float scaler)
		{
			x /= scaler;
			y /= scaler;
			z /= scaler;
			return *this;
		}

	public:
		Vector3 cross(const Vector3 & rhs) const
		{
			return Vector3(
				y * rhs.z - z * rhs.y,
				z * rhs.x - x * rhs.z,
				x * rhs.y - y * rhs.x);
		}

		float dot(const Vector3 & rhs) const
		{
			return x * rhs.x + y * rhs.y + z * rhs.z;
		}

		float length(void) const
		{
			return sqrt(lengthSq());
		}

		float lengthSq(void) const
		{
			return x * x + y * y + z * z;
		}

		Vector3 lerp(const Vector3 & rhs, float s)
		{
			return Vector3(
				x + s * (rhs.x - x),
				y + s * (rhs.y - y),
				z + s * (rhs.z - z));
		}

		Vector3 normalize(void) const
		{
			float l = length();

			return Vector3(x / l, y / l, z / l);
		}

		Vector3 & normalizeSelf(void)
		{
			float l = length();
			x = x / l;
			y = y / l;
			z = z / l;
			return *this;
		}

		Vector4 transform(const Matrix4 & m) const;

	public:
		static const Vector3 zero;

		static const Vector3 unitX;

		static const Vector3 unitY;

		static const Vector3 unitZ;
	};

	class Vector4
	{
	public:
		float x, y, z, w;

	public:
		Vector4(void)
			: x(0)
			, y(0)
			, z(0)
			, w(0)
		{
		}

		Vector4(float _x, float _y, float _z)
			: x(_x)
			, y(_y)
			, z(_z)
			, w(1)
		{
		}

		Vector4(float _x, float _y, float _z, float _w)
			: x(_x)
			, y(_y)
			, z(_z)
			, w(_w)
		{
		}

	public:
		Vector4 operator + (const Vector4 & rhs) const
		{
			return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
		}

		Vector4 operator + (float scaler) const
		{
			return Vector4(x + scaler, y + scaler, z + scaler, w + scaler);
		}

		Vector4 operator - (const Vector4 & rhs) const
		{
			return Vector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
		}

		Vector4 operator - (float scaler) const
		{
			return Vector4(x - scaler, y - scaler, z - scaler, w - scaler);
		}

		Vector4 operator * (const Vector4 & rhs) const
		{
			return Vector4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w);
		}

		Vector4 operator * (float scaler) const
		{
			return Vector4(x * scaler, y * scaler, z * scaler, w * scaler);
		}

		Vector4 operator / (const Vector4 & rhs) const
		{
			return Vector4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w);
		}

		Vector4 operator / (float scaler) const
		{
			return Vector4(x / scaler, y / scaler, z / scaler, w / scaler);
		}

		Vector4 & operator += (const Vector4 & rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			w += rhs.w;
			return *this;
		}

		Vector4 & operator += (float scaler)
		{
			x += scaler;
			y += scaler;
			z += scaler;
			w += scaler;
			return *this;
		}

		Vector4 & operator -= (const Vector4 & rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			w -= rhs.w;
			return *this;
		}

		Vector4 & operator -= (float scaler)
		{
			x -= scaler;
			y -= scaler;
			z -= scaler;
			w -= scaler;
			return *this;
		}

		Vector4 & operator *= (const Vector4 & rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;
			w *= rhs.w;
			return *this;
		}

		Vector4 & operator *= (float scaler)
		{
			x *= scaler;
			y *= scaler;
			z *= scaler;
			w *= scaler;
			return *this;
		}

		Vector4 & operator /= (const Vector4 & rhs)
		{
			x /= rhs.x;
			y /= rhs.y;
			z /= rhs.z;
			w /= rhs.w;
			return *this;
		}

		Vector4 & operator /= (float scaler)
		{
			x /= scaler;
			y /= scaler;
			z /= scaler;
			w /= scaler;
			return *this;
		}

	public:
		float dot(const Vector4 & rhs) const
		{
			return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
		}

		float length(void) const
		{
			return sqrt(lengthSq());
		}

		float lengthSq(void) const
		{
			return x * x + y * y + z * z + w * w;
		}

		Vector4 lerp(const Vector4 & rhs, float s)
		{
			return Vector4(
				x + s * (rhs.x - x),
				y + s * (rhs.y - y),
				z + s * (rhs.z - z),
				w + s * (rhs.w - w));
		}

		Vector4 normalize(void) const
		{
			float l = length();

			return Vector4(x / l, y / l, z / l, w / l);
		}

		Vector4 & normalizeSelf(void)
		{
			float l = length();
			x = x / l;
			y = y / l;
			z = z / l;
			w = w / l;
			return *this;
		}

		Vector4 transform(const Matrix4 & m) const;

	public:
		static const Vector4 zero;

		static const Vector4 unitX;

		static const Vector4 unitY;

		static const Vector4 unitZ;

		static const Vector4 unitW;
	};

	class Matrix4
	{
	public:
		union
		{
			struct
			{
				float _11, _12, _13, _14;
				float _21, _22, _23, _24;
				float _31, _32, _33, _34;
				float _41, _42, _43, _44;
			};

			float m[4][4];
		};

	public:
		Matrix4(void)
			: _11(0), _12(0), _13(0), _14(0)
			, _21(0), _22(0), _23(0), _24(0)
			, _31(0), _32(0), _33(0), _34(0)
			, _41(0), _42(0), _43(0), _44(0)
		{
		}

		Matrix4(float m11, float m12, float m13, float m14,
				float m21, float m22, float m23, float m24,
				float m31, float m32, float m33, float m34,
				float m41, float m42, float m43, float m44)
			: _11(m11), _12(m12), _13(m13), _14(m14)
			, _21(m21), _22(m22), _23(m23), _24(m24)
			, _31(m31), _32(m32), _33(m33), _34(m34)
			, _41(m41), _42(m42), _43(m43), _44(m44)
		{
		}

	public:
		Matrix4 operator + (const Matrix4 & rhs) const
		{
			return Matrix4(
				_11 + rhs._11, _12 + rhs._12, _13 + rhs._13, _14 + rhs._14,
				_21 + rhs._21, _22 + rhs._22, _23 + rhs._23, _24 + rhs._24,
				_31 + rhs._31, _32 + rhs._32, _33 + rhs._33, _34 + rhs._34,
				_41 + rhs._41, _42 + rhs._42, _43 + rhs._43, _44 + rhs._44);
		}

		Matrix4 operator + (float scaler) const
		{
			return Matrix4(
				_11 + scaler, _12 + scaler, _13 + scaler, _14 + scaler,
				_21 + scaler, _22 + scaler, _23 + scaler, _24 + scaler,
				_31 + scaler, _32 + scaler, _33 + scaler, _34 + scaler,
				_41 + scaler, _42 + scaler, _43 + scaler, _44 + scaler);
		}

		Matrix4 operator - (const Matrix4 & rhs) const
		{
			return Matrix4(
				_11 - rhs._11, _12 - rhs._12, _13 - rhs._13, _14 - rhs._14,
				_21 - rhs._21, _22 - rhs._22, _23 - rhs._23, _24 - rhs._24,
				_31 - rhs._31, _32 - rhs._32, _33 - rhs._33, _34 - rhs._34,
				_41 - rhs._41, _42 - rhs._42, _43 - rhs._43, _44 - rhs._44);
		}

		Matrix4 operator - (float scaler) const
		{
			return Matrix4(
				_11 - scaler, _12 - scaler, _13 - scaler, _14 - scaler,
				_21 - scaler, _22 - scaler, _23 - scaler, _24 - scaler,
				_31 - scaler, _32 - scaler, _33 - scaler, _34 - scaler,
				_41 - scaler, _42 - scaler, _43 - scaler, _44 - scaler);
		}

		Matrix4 operator * (const Matrix4 & rhs) const
		{
			Matrix4 ret;
			D3DXMatrixMultiply((D3DXMATRIX *)&ret, (D3DXMATRIX *)this, (D3DXMATRIX *)&rhs);
			return ret;
		}

		Matrix4 operator * (float scaler) const
		{
			return Matrix4(
				_11 * scaler, _12 * scaler, _13 * scaler, _14 * scaler,
				_21 * scaler, _22 * scaler, _23 * scaler, _24 * scaler,
				_31 * scaler, _32 * scaler, _33 * scaler, _34 * scaler,
				_41 * scaler, _42 * scaler, _43 * scaler, _44 * scaler);
		}

		Matrix4 operator / (const Matrix4 & rhs) const
		{
			Matrix4 ret;
			return *this * rhs.inverse();
		}

		Matrix4 operator / (float scaler) const
		{
			return Matrix4(
				_11 / scaler, _12 / scaler, _13 / scaler, _14 / scaler,
				_21 / scaler, _22 / scaler, _23 / scaler, _24 / scaler,
				_31 / scaler, _32 / scaler, _33 / scaler, _34 / scaler,
				_41 / scaler, _42 / scaler, _43 / scaler, _44 / scaler);
		}

		Matrix4 & operator += (const Matrix4 & rhs)
		{
			_11 += rhs._11; _12 += rhs._12; _13 += rhs._13; _14 += rhs._14;
			_21 += rhs._21; _22 += rhs._22; _23 += rhs._23; _24 += rhs._24;
			_31 += rhs._31; _32 += rhs._32; _33 += rhs._33; _34 += rhs._34;
			_41 += rhs._41; _42 += rhs._42; _43 += rhs._43; _44 += rhs._44;
			return *this;
		}

		Matrix4 & operator += (float scaler)
		{
			_11 += scaler; _12 += scaler; _13 += scaler; _14 += scaler;
			_21 += scaler; _22 += scaler; _23 += scaler; _24 += scaler;
			_31 += scaler; _32 += scaler; _33 += scaler; _34 += scaler;
			_41 += scaler; _42 += scaler; _43 += scaler; _44 += scaler;
			return *this;
		}

		Matrix4 & operator -= (const Matrix4 & rhs)
		{
			_11 -= rhs._11; _12 -= rhs._12; _13 -= rhs._13; _14 -= rhs._14;
			_21 -= rhs._21; _22 -= rhs._22; _23 -= rhs._23; _24 -= rhs._24;
			_31 -= rhs._31; _32 -= rhs._32; _33 -= rhs._33; _34 -= rhs._34;
			_41 -= rhs._41; _42 -= rhs._42; _43 -= rhs._43; _44 -= rhs._44;
			return *this;
		}

		Matrix4 & operator -= (float scaler)
		{
			_11 -= scaler; _12 -= scaler; _13 -= scaler; _14 -= scaler;
			_21 -= scaler; _22 -= scaler; _23 -= scaler; _24 -= scaler;
			_31 -= scaler; _32 -= scaler; _33 -= scaler; _34 -= scaler;
			_41 -= scaler; _42 -= scaler; _43 -= scaler; _44 -= scaler;
			return *this;
		}

		Matrix4 & operator *= (const Matrix4 & rhs)
		{
			return *this = *this * rhs;
		}

		Matrix4 & operator *= (float scaler)
		{
			_11 *= scaler; _12 *= scaler; _13 *= scaler; _14 *= scaler;
			_21 *= scaler; _22 *= scaler; _23 *= scaler; _24 *= scaler;
			_31 *= scaler; _32 *= scaler; _33 *= scaler; _34 *= scaler;
			_41 *= scaler; _42 *= scaler; _43 *= scaler; _44 *= scaler;
			return *this;
		}

		Matrix4 & operator /= (const Matrix4 & rhs)
		{
			return *this = *this / rhs;
		}

		Matrix4 & operator /= (float scaler)
		{
			_11 /= scaler; _12 /= scaler; _13 /= scaler; _14 /= scaler;
			_21 /= scaler; _22 /= scaler; _23 /= scaler; _24 /= scaler;
			_31 /= scaler; _32 /= scaler; _33 /= scaler; _34 /= scaler;
			_41 /= scaler; _42 /= scaler; _43 /= scaler; _44 /= scaler;
			return *this;
		}

	public:
		float a11(void) const
		{
			return _22 * _33 * _44
				 + _23 * _34 * _42
				 + _24 * _32 * _43
				 - _22 * _34 * _43
				 - _23 * _32 * _44
				 - _24 * _33 * _42;
		}

		float a12(void) const
		{
			return _23 * _34 * _41
				 + _24 * _31 * _43
				 + _21 * _33 * _44
				 - _23 * _31 * _44
				 - _24 * _33 * _41
				 - _21 * _34 * _43;
		}

		float a13(void) const
		{
			return _24 * _31 * _42
				 + _21 * _32 * _44
				 + _22 * _34 * _41
				 - _24 * _32 * _41
				 - _21 * _34 * _42
				 - _22 * _31 * _44;
		}

		float a14(void) const
		{
			return _21 * _32 * _43
				 + _22 * _33 * _41
				 + _23 * _31 * _42
				 - _21 * _33 * _42
				 - _22 * _31 * _43
				 - _23 * _32 * _41;
		}

		float a21(void) const
		{
			return _32 * _43 * _14
				 + _33 * _44 * _12
				 + _34 * _42 * _13
				 - _32 * _44 * _13
				 - _33 * _42 * _14
				 - _34 * _43 * _12;
		}

		float a22(void) const
		{
			return _33 * _44 * _11
				 + _34 * _41 * _13
				 + _31 * _43 * _14
				 - _33 * _41 * _14
				 - _34 * _43 * _11
				 - _31 * _44 * _13;
		}

		float a23(void) const
		{
			return _34 * _41 * _12
				 + _31 * _42 * _14
				 + _32 * _44 * _11
				 - _34 * _42 * _11
				 - _31 * _44 * _12
				 - _32 * _41 * _14;
		}

		float a24(void) const
		{
			return _31 * _42 * _13
				 + _32 * _43 * _11
				 + _33 * _41 * _12
				 - _31 * _43 * _12
				 - _32 * _41 * _13
				 - _33 * _42 * _11;
		}

		float a31(void) const
		{
			return _42 * _13 * _24
				 + _43 * _14 * _22
				 + _44 * _12 * _23
				 - _42 * _14 * _23
				 - _43 * _12 * _24
				 - _44 * _13 * _22;
		}

		float a32(void) const
		{
			return _43 * _14 * _21
				 + _44 * _11 * _23
				 + _41 * _13 * _24
				 - _43 * _11 * _24
				 - _44 * _13 * _21
				 - _41 * _14 * _23;
		}

		float a33(void) const
		{
			return _44 * _11 * _22
				 + _41 * _12 * _24
				 + _42 * _14 * _21
				 - _44 * _12 * _21
				 - _41 * _14 * _22
				 - _42 * _11 * _24;
		}

		float a34(void) const
		{
			return _41 * _12 * _23
				 + _42 * _13 * _21
				 + _43 * _11 * _22
				 - _41 * _13 * _22
				 - _42 * _11 * _23
				 - _43 * _12 * _21;
		}

		float a41(void) const
		{
			return _12 * _23 * _34
				 + _13 * _24 * _32
				 + _14 * _22 * _33
				 - _12 * _24 * _33
				 - _13 * _22 * _34
				 - _14 * _23 * _32;
		}

		float a42(void) const
		{
			return _13 * _24 * _31
				 + _14 * _21 * _33
				 + _11 * _23 * _34
				 - _13 * _21 * _34
				 - _14 * _23 * _31
				 - _11 * _24 * _33;
		}

		float a43(void) const
		{
			return _14 * _21 * _32
				 + _11 * _22 * _34
				 + _12 * _24 * _31
				 - _14 * _22 * _31
				 - _11 * _24 * _32
				 - _12 * _21 * _34;
		}

		float a44(void) const
		{
			return _11 * _22 * _33
				 + _12 * _23 * _31
				 + _13 * _21 * _32
				 - _11 * _23 * _32
				 - _12 * _21 * _33
				 - _13 * _22 * _31;
		}

		Matrix4 adjoint(void) const
		{
			return Matrix4(
				 a11(), -a21(),  a31(), -a41(),
				-a12(),  a22(), -a32(),  a42(),
				 a13(), -a23(),  a33(), -a43(),
				-a14(),  a24(), -a34(),  a44());
		}

		//void Decompose(Vector3 & outScale, Quaternion & outRotation, Vector3 & outTranslation)
		//{
		//	D3DXMatrixDecompose((D3DXVECTOR3 *)&outScale, (D3DXVECTOR3 *)&outRotation, (D3DXVECTOR3 *)&outTranslation, (D3DXMATRIX *)this);
		//}

		float determinant(void) const
		{
			return _11 * a11() - _12 * a12() + _13 * a13() - _14 * a14();
		}

		Matrix4 & SetIdentity(void)
		{
			D3DXMatrixIdentity((D3DXMATRIX *)this);
			return *this;
		}

		Matrix4 inverse(void) const
		{
			return adjoint() / determinant();
		}

		Matrix4 & SetLookAtLH(const Vector3 & eye, const Vector3 & at, const Vector3 & up)
		{
			D3DXMatrixLookAtLH((D3DXMATRIX *)this, (D3DXVECTOR3 *)&eye, (D3DXVECTOR3 *)&at, (D3DXVECTOR3 *)&up);
			return *this;
		}

		Matrix4 multiply(const Matrix4 & rhs) const
		{
			return *this * rhs;
		}

		Matrix4 multiplyTranspose(const Matrix4 & rhs) const
		{
			return multiply(rhs).transpose();
		}

		Matrix4 & SetLookAtRH(const Vector3 & eye, const Vector3 & at, const Vector3 & up)
		{
			D3DXMatrixLookAtRH((D3DXMATRIX *)this, (D3DXVECTOR3 *)&eye, (D3DXVECTOR3 *)&at, (D3DXVECTOR3 *)&up);
			return *this;
		}

		Matrix4 & SetOrthoLH(float w, float h, float zn, float zf)
		{
			D3DXMatrixOrthoLH((D3DXMATRIX *)this, w, h, zn, zf);
			return *this;
		}

		Matrix4 & SetOrthoRH(float w, float h, float zn, float zf)
		{
			D3DXMatrixOrthoRH((D3DXMATRIX *)this, w, h, zn, zf);
			return *this;
		}

		Matrix4 & SetOrthoOffCenterLH(float l, float r, float b, float t, float zn, float zf)
		{
			D3DXMatrixOrthoOffCenterLH((D3DXMATRIX *)this, l, r, b, t, zn, zf);
			return *this;
		}

		Matrix4 & SetOrthoOffCenterRH(float l, float r, float b, float t, float zn, float zf)
		{
			D3DXMatrixOrthoOffCenterRH((D3DXMATRIX *)this, l, r, b, t, zn, zf);
			return *this;
		}

		Matrix4 & SetPerspectiveFovLH(float fovy, float aspect, float zn, float zf)
		{
			D3DXMatrixPerspectiveFovLH((D3DXMATRIX *)this, fovy, aspect, zn, zf);
			return *this;
		}

		Matrix4 & SetPerspectiveFovRH(float fovy, float aspect, float zn, float zf)
		{
			D3DXMatrixPerspectiveFovRH((D3DXMATRIX *)this, fovy, aspect, zn, zf);
			return *this;
		}

		Matrix4 & SetPerspectiveLH(float w, float h, float zn, float zf)
		{
			D3DXMatrixPerspectiveLH((D3DXMATRIX *)this, w, h, zn, zf);
			return *this;
		}

		Matrix4 & SetPerspectiveRH(float w, float h, float zn, float zf)
		{
			D3DXMatrixPerspectiveRH((D3DXMATRIX *)this, w, h, zn, zf);
			return *this;
		}

		Matrix4 & SetPerspectiveOffCenterLH(float l, float r, float b, float t, float zn, float zf)
		{
			D3DXMatrixPerspectiveOffCenterLH((D3DXMATRIX *)this, l, r, b, t, zn, zf);
			return *this;
		}

		Matrix4 & SetRotationAxis(const Vector3 & v, float angle)
		{
			D3DXMatrixRotationAxis((D3DXMATRIX *)this, (D3DXVECTOR3 *)&v, angle);
			return *this;
		}

		//Matrix4 & SetRotationQuaternion(const Quaternion & q)
		//{
		//	D3DXMatrixQuaternion((D3DXMATRIX *)this, (D3DXQUATERNION *)&q);
		//	return *this;
		//}

		Matrix4 & SetRotationX(float angle)
		{
			D3DXMatrixRotationX((D3DXMATRIX *)this, angle);
			return *this;
		}

		Matrix4 & SetRotationY(float angle)
		{
			D3DXMatrixRotationY((D3DXMATRIX *)this, angle);
			return *this;
		}

		Matrix4 & SetRotationZ(float angle)
		{
			D3DXMatrixRotationZ((D3DXMATRIX *)this, angle);
			return *this;
		}

		Matrix4 & SetRotationYawPitchRoll(float yaw, float pitch, float roll)
		{
			D3DXMatrixRotationYawPitchRoll((D3DXMATRIX *)this, yaw, pitch, roll);
			return *this;
		}

		Matrix4 & SetScaling(float sx, float sy, float sz)
		{
			D3DXMatrixScaling((D3DXMATRIX *)this, sx, sy, sz);
			return *this;
		}

		//Matrix4 & SetTransformation(
		//	const Vector3 & scalingCenter,
		//	const Quaternion & scalingRotation,
		//	const Vector3 & scaling,
		//	const Vector3 & rotationCenter,
		//	const Vector3 & rotation,
		//	const Vector3 & translation)
		//{
		//	D3DXMatrixTransformation(
		//		(D3DXMATRIX *)this,
		//		(D3DXVECTOR3 *)&scalingCenter,
		//		(D3DXQUATERNION *)&scalingRotation,
		//		(D3DXVECTOR3 *)&scaling,
		//		(D3DXVECTOR3 *)&rotationCenter,
		//		(D3DXVECTOR3 *)&rotation,
		//		(D3DXVECTOR3 *)&translation);
		//	return *this;
		//}

		Matrix4 & SetTransformation2D(
			const Vector2 & scalingCenter,
			float scalingRotation,
			const Vector2 & scaling,
			const Vector2 & rotationCenter,
			float rotation,
			const Vector2 & translation)
		{
			D3DXMatrixTransformation2D(
				(D3DXMATRIX *)this,
				(D3DXVECTOR2 *)&scalingCenter,
				scalingRotation,
				(D3DXVECTOR2 *)&scaling,
				(D3DXVECTOR2 *)&rotationCenter,
				rotation,
				(D3DXVECTOR2 *)&translation);
			return *this;
		}

		Matrix4 & SetTranslation(float x, float y, float z)
		{
			D3DXMatrixTranslation((D3DXMATRIX *)this, x, y, z);
			return *this;
		}

		Matrix4 transpose(void)
		{
			Matrix4 ret;
			D3DXMatrixTranspose((D3DXMATRIX *)&ret, (D3DXMATRIX *)this);
			return ret;
		}

	public:
		Matrix4 scaling(const Vector3 & scaling) const
		{
			return Matrix4(
				_11 * scaling.x, _12 * scaling.y, _13 * scaling.z, _14 * 1,
				_21 * scaling.x, _22 * scaling.y, _23 * scaling.z, _24 * 1,
				_31 * scaling.x, _32 * scaling.y, _33 * scaling.z, _34 * 1,
				_41 * scaling.x, _42 * scaling.y, _43 * scaling.z, _44 * 1);
		}

		Matrix4 & scalingSelf(const Vector3 & scaling)
		{
			_11 = _11 * scaling.x; _12 = _12 * scaling.y; _13 = _13 * scaling.z; _14 = _14 * 1;
			_21 = _21 * scaling.x; _22 = _22 * scaling.y; _23 = _23 * scaling.z; _24 = _24 * 1;
			_31 = _31 * scaling.x; _32 = _32 * scaling.y; _33 = _33 * scaling.z; _34 = _34 * 1;
			_41 = _41 * scaling.x; _42 = _42 * scaling.y; _43 = _43 * scaling.z; _44 = _44 * 1;
			return *this;
		}

		Matrix4 rotationX(float angle) const
		{
			float c = cos(angle);
			float s = sin(angle);

			return Matrix4(
				_11, _12 * c - _13 * s, _12 * s + _13 * c, _14,
				_21, _22 * c - _23 * s, _22 * s + _23 * c, _24,
				_31, _32 * c - _33 * s, _32 * s + _33 * c, _34,
				_41, _42 * c - _43 * s, _42 * s + _43 * c, _44);
		}

		Matrix4 & rotationXSelf(float angle)
		{
			return *this = rotationX(angle);
		}

		Matrix4 rotationY(float angle) const
		{
			float c = cos(angle);
			float s = sin(angle);

			return Matrix4(
				_11 * c + _13 * s, _12, _13 * s - _11 * c, 14,
				_21 * c + _23 * s, _22, _23 * s - _21 * c, 14,
				_31 * c + _33 * s, _32, _33 * s - _31 * c, 14,
				_41 * c + _43 * s, _42, _43 * s - _41 * c, 14);
		}

		Matrix4 & rotationYSelf(float angle)
		{
			return *this = rotationY(angle);
		}

		Matrix4 rotationZ(float angle) const
		{
			float c = cos(angle);
			float s = sin(angle);

			return Matrix4(
				_11 * c - _12 * s, _11 * s + _12 * c, _13, _14,
				_21 * c - _22 * s, _21 * s + _22 * c, _23, _24,
				_31 * c - _32 * s, _31 * s + _32 * c, _33, _34,
				_41 * c - _42 * s, _41 * s + _42 * c, _43, _44);
		}

		Matrix4 & rotationZSelf(float angle)
		{
			return *this = rotationZ(angle);
		}

		Matrix4 translation(const Vector3 & v) const
		{
			return Matrix4(
				_11 + _14 * v.x, _12 + _14 * v.y, _13 + _14 * v.z, _14,
				_21 + _24 * v.x, _22 + _24 * v.y, _23 + _24 * v.z, _24,
				_31 + _34 * v.x, _32 + _34 * v.y, _33 + _34 * v.z, _34,
				_41 + _44 * v.x, _42 + _44 * v.y, _43 + _44 * v.z, _44);
		}

		Matrix4 & translationSelf(const Vector3 & v)
		{
			_11 = _11 + _14 * v.x; _12 = _12 + 14 * v.y; _13 = _13 + _14 * v.z;
			_21 = _21 + _24 * v.x; _22 = _22 + 14 * v.y; _23 = _23 + _24 * v.z;
			_31 = _31 + _34 * v.x; _32 = _32 + 14 * v.y; _33 = _33 + _34 * v.z;
			_41 = _41 + _44 * v.x; _42 = _42 + 14 * v.y; _43 = _43 + _44 * v.z;
			return *this;
		}

	public:
		static const Matrix4 identity;
	};

	inline Vector4 Vector2::transform(const Matrix4 & m) const
	{
		return Vector4(x, y, 0, 1).transform(m);
	}

	inline Vector4 Vector3::transform(const Matrix4 & m) const
	{
		return Vector4(x, y, z, 1).transform(m);
	}

	inline Vector4 Vector4::transform(const Matrix4 & m) const
	{
		return Vector4(
			x * m._11 + y * m._21 + z * m._31 + w * m._41,
			x * m._12 + y * m._22 + z * m._32 + w * m._42,
			x * m._13 + y * m._23 + z * m._33 + w * m._43,
			x * m._14 + y * m._24 + z * m._34 + w * m._44);
	}
};
