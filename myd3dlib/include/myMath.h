
#pragma once

#include <d3dx9math.h>

namespace my
{
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
			return *this * rhs.Inverse();
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
		//void Decompose(Vector3 & outScale, Quaternion & outRotation, Vector3 & outTranslation)
		//{
		//	D3DXMatrixDecompose((D3DXVECTOR3 *)&outScale, (D3DXQUATERNION *)&outRotation, (D3DXVECTOR3 *)&outTranslation, (D3DXMATRIX *)this);
		//}

		float Determinant(void) const
		{
			return D3DXMatrixDeterminant((D3DXMATRIX *)this);
		}

		Matrix4 & SetIdentity(void)
		{
			D3DXMatrixIdentity((D3DXMATRIX *)this);
			return *this;
		}

		Matrix4 Inverse(void) const
		{
			Matrix4 ret;
			float det;
			D3DXMatrixInverse((D3DXMATRIX *)&ret, &det, (D3DXMATRIX *)this);
			return ret;
		}

		Matrix4 & SetLookAtLH(const Vector3 & eye, const Vector3 & at, const Vector3 & up)
		{
			D3DXMatrixLookAtLH((D3DXMATRIX *)this, (D3DXVECTOR3 *)&eye, (D3DXVECTOR3 *)&at, (D3DXVECTOR3 *)&up);
			return *this;
		}

		Matrix4 Multiply(const Matrix4 & rhs) const
		{
			return *this * rhs;
		}

		Matrix4 MultiplyTranspose(const Matrix4 & rhs) const
		{
			Matrix4 ret;
			D3DXMatrixMultiplyTranspose((D3DXMATRIX *)&ret, (D3DXMATRIX *)this, (D3DXMATRIX *)&rhs);
			return ret;
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

		Matrix4 Transpose(void)
		{
			Matrix4 ret;
			D3DXMatrixTranspose((D3DXMATRIX *)&ret, (D3DXMATRIX *)this);
			return ret;
		}

	public:
		Matrix4 Scaling(const Vector3 & scaling) const
		{
			return Matrix4(
				_11 * scaling.x, _12 * scaling.y, _13 * scaling.z, _14 * 1,
				_21 * scaling.x, _22 * scaling.y, _23 * scaling.z, _24 * 1,
				_31 * scaling.x, _32 * scaling.y, _33 * scaling.z, _34 * 1,
				_41 * scaling.x, _42 * scaling.y, _43 * scaling.z, _44 * 1);
		}

		Matrix4 & ScalingSelf(const Vector3 & scaling)
		{
			_11 = _11 * scaling.x; _12 = _12 * scaling.y; _13 = _13 * scaling.z; _14 = _14 * 1;
			_21 = _21 * scaling.x; _22 = _22 * scaling.y; _23 = _23 * scaling.z; _24 = _24 * 1;
			_31 = _31 * scaling.x; _32 = _32 * scaling.y; _33 = _33 * scaling.z; _34 = _34 * 1;
			_41 = _41 * scaling.x; _42 = _42 * scaling.y; _43 = _43 * scaling.z; _44 = _44 * 1;
			return *this;
		}

		Matrix4 RotationX(float angle) const
		{
			float cosangle = cos(angle);
			float sinangle = sin(angle);

			return Matrix4(
				_11, _12 * cosangle - _13 * sinangle, _12 * sinangle + _13 * cosangle, _14,
				_21, _22 * cosangle - _23 * sinangle, _22 * sinangle + _23 * cosangle, _24,
				_31, _32 * cosangle - _33 * sinangle, _32 * sinangle + _33 * cosangle, _34,
				_41, _42 * cosangle - _43 * sinangle, _42 * sinangle + _43 * cosangle, _44);
		}

		Matrix4 & RotationXSelf(float angle)
		{
			return *this = RotationX(angle);
		}

		Matrix4 RotationY(float angle) const
		{
			float cosangle = cos(angle);
			float sinangle = sin(angle);

			return Matrix4(
				_11 * cosangle + _13 * sinangle, _12, _13 * sinangle - _11 * cosangle, 14,
				_21 * cosangle + _23 * sinangle, _22, _23 * sinangle - _21 * cosangle, 14,
				_31 * cosangle + _33 * sinangle, _32, _33 * sinangle - _31 * cosangle, 14,
				_41 * cosangle + _43 * sinangle, _42, _43 * sinangle - _41 * cosangle, 14);
		}

		Matrix4 & RotationYSelf(float angle)
		{
			return *this = RotationY(angle);
		}

		Matrix4 RotationZ(float angle) const
		{
			float cosangle = cos(angle);
			float sinangle = sin(angle);

			return Matrix4(
				_11 * cosangle - _12 * sinangle, _11 * sinangle + _12 * cosangle, _13, _14,
				_21 * cosangle - _22 * sinangle, _21 * sinangle + _22 * cosangle, _23, _24,
				_31 * cosangle - _32 * sinangle, _31 * sinangle + _32 * cosangle, _33, _34,
				_41 * cosangle - _42 * sinangle, _41 * sinangle + _42 * cosangle, _43, _44);
		}

		Matrix4 & RotationZSelf(float angle)
		{
			return *this = RotationZ(angle);
		}

		Matrix4 Translation(const Vector3 & v) const
		{
			return Matrix4(
				_11 + _14 * v.x, _12 + _14 * v.y, _13 + _14 * v.z, _14,
				_21 + _24 * v.x, _22 + _24 * v.y, _23 + _24 * v.z, _24,
				_31 + _34 * v.x, _32 + _34 * v.y, _33 + _34 * v.z, _34,
				_41 + _44 * v.x, _42 + _44 * v.y, _43 + _44 * v.z, _44);
		}

		Matrix4 & TranslationSelf(const Vector3 & v)
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
};
