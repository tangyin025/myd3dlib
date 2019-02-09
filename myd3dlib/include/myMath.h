#pragma once

#include <d3dx9math.h>
#include <set>
#include <map>
#include <vector>
#include <boost/serialization/nvp.hpp>

#define EPSILON_E3			(1.0e-3)
#define EPSILON_E4			(1.0e-4)
#define EPSILON_E5			(1.0e-5)
#define EPSILON_E6			(1.0e-6)
#define EPSILON_E12			(1.0e-12)

#define cot(x)	tan(D3DX_PI / 2 - (x))

#define IS_UNITED(v) (abs((float)(v) - 1) < EPSILON_E6)

#define IS_NORMALIZED(v) (IS_UNITED((v).magnitude()))

namespace my
{
	//inline int _ftoi(double dval)
	//{
	//	// seee,eeee,eeee,mmmm,mmmm,mmmm,mmmm,mmmm,mmmm,mmmm,mmmm,mmmm,mmmm,mmmm,mmmm,mmmm
	//	return *(int *)&(dval += 1.5 * (1LL << 52));
	//}

	//inline int _ftoi(float fval)
	//{
	//	// seee,eeee,emmm,mmmm,mmmm,mmmm,mmmm,mmmm
	//	return *(int *)&(fval += 1.5 * (1LL << 23)) << 10 >> 10;
	//}

	template <typename T>
	T Min(T a, T b)
	{
		return a < b ? a : b;
	}

	template <typename T>
	T Min(T a, T b, T c)
	{
		return a < b ? (a < c ? a : c) : (b < c ? b : c);
	}

	template <typename T>
	T Max(T a, T b)
	{
		return a > b ? a : b;
	}

	template <typename T>
	T Max(T a, T b, T c)
	{
		return a > b ? (a > c ? a : c) : (b > c ? b : c);
	}

	template <typename T>
	T Lerp(T a, T b, T s)
	{
		return a + s * (b - a);
	}

	template <typename T>
	T Clamp(T v, T min, T max)
	{
		return v < min ? min : (v < max ? v : max);
	}

	template <typename T>
	T Round(T v, T min, T max);

	template <typename T>
	T Random(T range);

	template <typename T>
	T Random(T min, T max);

	template <typename V, typename T>
	V & Subscribe(T & t, size_t i)
	{
		_ASSERT(i < sizeof(t) / sizeof(V));

		return ((V*)&t)[i];
	}

	template <typename V, size_t i, typename T>
	V & Subscribe(T & t)
	{
		BOOST_STATIC_ASSERT(i < sizeof(t) / sizeof(V));

		return ((V*)&t)[i];
	}

	class Vector4;

	class Quaternion;

	class Matrix4;

	class Vector2
	{
	public:
		float x, y;

	public:
		Vector2(void)
			//: x(0)
			//, y(0)
		{
		}

		Vector2(float _v)
			: x(_v)
			, y(_v)
		{
		}

		Vector2(float _x, float _y)
			: x(_x)
			, y(_y)
		{
		}

		template <class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(x);
			ar & BOOST_SERIALIZATION_NVP(y);
		}

	public:
		float & operator [](size_t i)
		{
			return Subscribe<float>(*this, i);
		}

		const float & operator [](size_t i) const
		{
			return Subscribe<float>(*this, i);
		}

		bool operator ==(const Vector2 & rhs) const
		{
			return x == rhs.x
				&& y == rhs.y;
		}

		bool operator !=(const Vector2 & rhs) const
		{
			return !operator ==(rhs);
		}

	public:
		Vector2 operator - (void) const
		{
			return Vector2(-x, -y);
		}

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
			float invScaler = 1 / scaler;

			return Vector2(x * invScaler, y * invScaler);
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
			float invScaler = 1 / scaler;
			x *= invScaler;
			y *= invScaler;
			return *this;
		}

	public:
		float cross(const Vector2 & rhs) const
		{
			return x * rhs.y - y * rhs.x;
		}

		float dot(const Vector2 & rhs) const
		{
			return x * rhs.x + y * rhs.y;
		}

		float magnitude(void) const
		{
			return sqrt(magnitudeSq());
		}

		float magnitudeSq(void) const
		{
			return x * x + y * y;
		}

		Vector2 lerp(const Vector2 & rhs, float s) const
		{
			return Vector2(
				x + s * (rhs.x - x),
				y + s * (rhs.y - y));
		}

		Vector2 & lerpSelf(const Vector2 & rhs, float s)
		{
			x = x + s * (rhs.x - x);
			y = y + s * (rhs.y - y);
			return *this;
		}

		Vector2 normalize(void) const
		{
			float invLength = 1 / magnitude();

			return Vector2(x * invLength, y * invLength);
		}

		Vector2 & normalizeSelf(void)
		{
			float invLength = 1 / magnitude();
			x *= invLength;
			y *= invLength;
			return *this;
		}

		Vector4 transform(const Matrix4 & m) const;

		Vector4 transformTranspose(const Matrix4 & m) const;

		Vector2 transformCoord(const Matrix4 & m) const;

		Vector2 transformCoordTranspose(const Matrix4 & m) const;

		Vector2 transformNormal(const Matrix4 & m) const;

		Vector2 transformNormalTranspose(const Matrix4 & m) const;

	public:
		static const Vector2 zero;

		static const Vector2 unitX;

		static const Vector2 unitY;
	};

	class Vector3
	{
	public:
		union
		{
			struct
			{
				float x;

				union
				{
					struct  
					{
						float y, z;
					};

					struct
					{
						Vector2 yz;
					};
				};
			};

			struct
			{
				Vector2 xy;
			};
		};

	public:
		Vector3(void)
			//: x(0)
			//, y(0)
			//, z(0)
		{
		}

		Vector3(float _v)
			: x(_v)
			, y(_v)
			, z(_v)
		{
		}

		Vector3(const Vector2 & _v, float _z)
			: x(_v.x)
			, y(_v.y)
			, z(_z)
		{
		}

		Vector3(float _x, float _y, float _z)
			: x(_x)
			, y(_y)
			, z(_z)
		{
		}

		template <class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(x);
			ar & BOOST_SERIALIZATION_NVP(y);
			ar & BOOST_SERIALIZATION_NVP(z);
		}

	public:
		float & operator [](size_t i)
		{
			return Subscribe<float>(*this, i);
		}

		const float & operator [](size_t i) const
		{
			return Subscribe<float>(*this, i);
		}

		bool operator ==(const Vector3 & rhs) const
		{
			return x == rhs.x
				&& y == rhs.y
				&& z == rhs.z;
		}

		bool operator !=(const Vector3 & rhs) const
		{
			return !operator ==(rhs);
		}

	public:
		Vector3 operator - (void) const
		{
			return Vector3(-x, -y, -z);
		}

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
			float invScaler = 1 / scaler;

			return Vector3(x * invScaler, y * invScaler, z * invScaler);
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
			float invScaler = 1 / scaler;
			x *= invScaler;
			y *= invScaler;
			z *= invScaler;
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

		float magnitude(void) const
		{
			return sqrt(magnitudeSq());
		}

		float magnitudeSq(void) const
		{
			return x * x + y * y + z * z;
		}

		Vector3 lerp(const Vector3 & rhs, float s) const
		{
			return Vector3(
				x + s * (rhs.x - x),
				y + s * (rhs.y - y),
				z + s * (rhs.z - z));
		}

		Vector3 & lerpSelf(const Vector3 & rhs, float s)
		{
			x = x + s * (rhs.x - x);
			y = y + s * (rhs.y - y);
			z = z + s * (rhs.z - z);
			return *this;
		}

		Vector3 normalize(void) const
		{
			float invLength = 1 / magnitude();

			return Vector3(x * invLength, y * invLength, z * invLength);
		}

		Vector3 & normalizeSelf(void)
		{
			float invLength = 1 / magnitude();
			x *= invLength;
			y *= invLength;
			z *= invLength;
			return *this;
		}

		Vector4 transform(const Matrix4 & m) const;

		Vector4 transformTranspose(const Matrix4 & m) const;

		Vector3 transformCoord(const Matrix4 & m) const;

		Vector3 transformCoordTranspose(const Matrix4 & m) const;

		Vector3 transformNormal(const Matrix4 & m) const;

		Vector3 transformNormalTranspose(const Matrix4 & m) const;

		Vector3 transform(const Quaternion & q) const;

	public:
		static Vector3 SphericalToCartesian(float x, float y, float z)
		{
			return Vector3(x * sin(y) * sin(z), x * cos(y), x * sin(y) * cos(z));
		}

		static Vector3 CartesianToSpherical(float x, float y, float z)
		{
			float r = sqrt(x * x + y * y + z * z);

			return Vector3(r, acos(y / r), atan2(x, z));
		}

		static float Angle(const Vector3 & v0, const Vector3 & v1)
		{
			const float cos = v0.dot(v1);					// |v0|*|v1|*Cos(Angle)
			const float sin = (v0.cross(v1)).magnitude();	// |v0|*|v1|*Sin(Angle)
			return atan2(sin, cos);
		}

		static const Vector3 zero;

		static const Vector3 one;

		static const Vector3 unitX;

		static const Vector3 unitY;

		static const Vector3 unitZ;

		static const Vector3 Gravity;
	};

	class Vector4
	{
	public:
		union
		{
			struct
			{
				float x;

				union
				{
					struct  
					{
						float y, z, w;
					};

					struct
					{
						Vector2 yz;
					};
				};
			};

			struct
			{
				Vector2 xy;
			};

			struct
			{
				Vector3 xyz;
			};
		};

	public:
		Vector4(void)
			//: x(0)
			//, y(0)
			//, z(0)
			//, w(0)
		{
		}

		Vector4(float _v)
			: x(_v)
			, y(_v)
			, z(_v)
			, w(_v)
		{
		}

		Vector4(const Vector3 & _v, float _w)
			: x(_v.x)
			, y(_v.y)
			, z(_v.z)
			, w(_w)
		{
		}

		Vector4(float _x, float _y, float _z, float _w)
			: x(_x)
			, y(_y)
			, z(_z)
			, w(_w)
		{
		}

		template <class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(x);
			ar & BOOST_SERIALIZATION_NVP(y);
			ar & BOOST_SERIALIZATION_NVP(z);
			ar & BOOST_SERIALIZATION_NVP(w);
		}

	public:
		float & operator [](size_t i)
		{
			return Subscribe<float>(*this, i);
		}

		const float & operator [](size_t i) const
		{
			return Subscribe<float>(*this, i);
		}

		bool operator ==(const Vector4 & rhs) const
		{
			return x == rhs.x
				&& y == rhs.y
				&& z == rhs.z
				&& w == rhs.w;
		}

		bool operator !=(const Vector4 & rhs) const
		{
			return !operator ==(rhs);
		}

	public:
		Vector4 operator - (void) const
		{
			return Vector4(-x, -y, -z, -w);
		}

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
			float invScaler = 1 / scaler;

			return Vector4(x * invScaler, y * invScaler, z * invScaler, w * invScaler);
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
			float invScaler = 1 / scaler;
			x *= invScaler;
			y *= invScaler;
			z *= invScaler;
			w *= invScaler;
			return *this;
		}

	public:
		Vector4 cross(const Vector4 & rhs, const Vector4 & srd) const
		{
			Vector4 ret;
			D3DXVec4Cross((D3DXVECTOR4 *)&ret, (D3DXVECTOR4 *)this, (D3DXVECTOR4 *)&rhs, (D3DXVECTOR4 *)&srd);
			return ret;
		}

		float dot(const Vector4 & rhs) const
		{
			return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
		}

		float magnitude(void) const
		{
			return sqrt(magnitudeSq());
		}

		float magnitudeSq(void) const
		{
			return x * x + y * y + z * z + w * w;
		}

		Vector4 lerp(const Vector4 & rhs, float s) const
		{
			return Vector4(
				x + s * (rhs.x - x),
				y + s * (rhs.y - y),
				z + s * (rhs.z - z),
				w + s * (rhs.w - w));
		}

		Vector4 & lerpSelf(const Vector4 & rhs, float s)
		{
			x = x + s * (rhs.x - x);
			y = y + s * (rhs.y - y);
			z = z + s * (rhs.z - z);
			w = w + s * (rhs.w - w);
			return *this;
		}

		Vector4 normalize(void) const
		{
			float invLength = 1 / magnitude();

			return Vector4(x * invLength, y * invLength, z * invLength, w * invLength);
		}

		Vector4 & normalizeSelf(void)
		{
			float invLength = 1 / magnitude();
			x *= invLength;
			y *= invLength;
			z *= invLength;
			w *= invLength;
			return *this;
		}

		Vector4 transform(const Matrix4 & m) const;

		Vector4 transformTranspose(const Matrix4 & m) const;

	public:
		static const Vector4 zero;

		static const Vector4 unitX;

		static const Vector4 unitY;

		static const Vector4 unitZ;

		static const Vector4 unitW;
	};

	class Rectangle
	{
	public:
		float l, t, r, b;

	public:
		Rectangle(void)
			//: l(0)
			//, t(0)
			//, r(0)
			//, b(0)
		{
		}

		Rectangle(float left, float top, float right, float bottom)
			: l(left)
			, t(top)
			, r(right)
			, b(bottom)
		{
		}

		Rectangle(const Vector2 & leftTop, const Vector2 & rightBottom)
			: l(leftTop.x)
			, t(leftTop.y)
			, r(rightBottom.x)
			, b(rightBottom.y)
		{
		}

		template <class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(l);
			ar & BOOST_SERIALIZATION_NVP(t);
			ar & BOOST_SERIALIZATION_NVP(r);
			ar & BOOST_SERIALIZATION_NVP(b);
		}

	public:
		Rectangle intersect(const Rectangle & rhs) const
		{
			return Rectangle(
				Max(l, rhs.l),
				Max(t, rhs.t),
				Min(r, rhs.r),
				Min(b, rhs.b));
		}

		Rectangle & intersectSelf(const Rectangle & rhs)
		{
			l = Max(l, rhs.l);
			t = Max(t, rhs.t);
			r = Min(r, rhs.r);
			b = Min(b, rhs.b);
			return *this;
		}

		Rectangle Union(const Rectangle & rhs) const
		{
			return Rectangle(
				Min(l, rhs.l),
				Min(t, rhs.t),
				Max(r, rhs.r),
				Max(b, rhs.b));
		}

		Rectangle & unionSelf(const Rectangle & rhs)
		{
			l = Min(l, rhs.l);
			t = Min(t, rhs.t);
			r = Max(r, rhs.r);
			b = Max(b, rhs.b);
			return *this;
		}

		Rectangle offset(float x, float y) const
		{
			return Rectangle(
				l + x,
				t + y,
				r + x,
				b + y);
		}

		Rectangle & offsetSelf(float x, float y)
		{
			l += x;
			t += y;
			r += x;
			b += y;
			return *this;
		}

		Rectangle offset(const Vector2 & v) const
		{
			return Rectangle(
				l + v.x,
				t + v.y,
				r + v.x,
				b + v.y);
		}

		Rectangle & offsetSelf(const Vector2 & v)
		{
			l += v.x;
			t += v.y;
			r += v.x;
			b += v.y;
			return *this;
		}

		Rectangle shrink(float x, float y) const
		{
			return Rectangle(
				l + x,
				t + y,
				r - x,
				b - y);
		}

		Rectangle & shrinkSelf(float x, float y)
		{
			l += x;
			t += y;
			r -= x;
			b -= y;
			return *this;
		}

		Rectangle shrink(const Vector2 & v) const
		{
			return Rectangle(
				l + v.x,
				t + v.y,
				r - v.x,
				b - v.y);
		}

		Rectangle & shrinkSelf(const Vector2 & v)
		{
			l += v.x;
			t += v.y;
			r -= v.x;
			b -= v.y;
			return *this;
		}

		Rectangle shrink(float x, float y, float z, float w) const
		{
			return Rectangle(
				l + x,
				t + y,
				r - z,
				b - w);
		}

		Rectangle & shrinkSelf(float x, float y, float z, float w)
		{
			l += x;
			t += y;
			r -= z;
			b -= w;
			return *this;
		}

		Rectangle shrink(const Vector4 & v) const
		{
			return Rectangle(
				l + v.x,
				t + v.y,
				r - v.z,
				b - v.w);
		}

		Rectangle & shrinkSelf(const Vector4 & v)
		{
			l += v.x;
			t += v.y;
			r -= v.z;
			b -= v.w;
			return *this;
		}

	public:
		Vector2 LeftTop(void) const
		{
			return Vector2(l, t);
		}

		Vector2 LeftMiddle(void) const
		{
			return Vector2(l, Lerp(t,b,0.5f));
		}

		Vector2 LeftBottom(void) const
		{
			return Vector2(l, b);
		}

		Vector2 CenterTop(void) const
		{
			return Vector2(Lerp(l,r,0.5f), t);
		}

		Vector2 CenterMiddle(void) const
		{
			return Vector2(Lerp(l,r,0.5f), Lerp(t,b,0.5f));
		}

		Vector2 CenterBottom(void) const
		{
			return Vector2(Lerp(l,r,0.5f), b);
		}

		Vector2 RightTop(void) const
		{
			return Vector2(r, t);
		}

		Vector2 RightMiddle(void) const
		{
			return Vector2(r, Lerp(t,b,0.5f));
		}

		Vector2 RightBottom(void) const
		{
			return Vector2(r, b);
		}

		Vector2 Center(void) const
		{
			return Vector2(l + (r - l) * 0.5f, t + (b - t) * 0.5f);
		}

		float Width(void) const
		{
			return r - l;
		}

		float Height(void) const
		{
			return b - t;
		}

		Vector2 Extent(void) const
		{
			return Vector2(Width(), Height());
		}

		bool PtInRect(const Vector2 & pt)
		{
			return pt.x >= l && pt.x < r && pt.y >= t && pt.y < b;
		}

		static Rectangle LeftTop(float x, float y, float width, float height)
		{
			return Rectangle(
				x,
				y,
				x + width,
				y + height);
		}

		static Rectangle LeftTop(const Vector2 & point, const Vector2 & size)
		{
			return Rectangle(
				point.x,
				point.y,
				point.x + size.x,
				point.y + size.y);
		}

		static Rectangle LeftMiddle(float x, float y, float width, float height)
		{
			float hh = height * 0.5f;

			return Rectangle(
				x,
				y - hh,
				x + width,
				y + hh);
		}

		static Rectangle LeftMiddle(const Vector2 & point, const Vector2 & size)
		{
			float hh = size.y * 0.5f;

			return Rectangle(
				point.x,
				point.y - hh,
				point.x + size.x,
				point.y + hh);
		}

		static Rectangle LeftBottom(float x, float y, float width, float height)
		{
			return Rectangle(
				x,
				y - height,
				x + width,
				y);
		}

		static Rectangle LeftBottom(const Vector2 & point, const Vector2 & size)
		{
			return Rectangle(
				point.x,
				point.y - size.y,
				point.x + size.x,
				point.y);
		}

		static Rectangle CenterTop(float x, float y, float width, float height)
		{
			float hw = width * 0.5f;

			return Rectangle(
				x - hw,
				y,
				x + hw,
				y + height);
		}

		static Rectangle CenterTop(const Vector2 & point, const Vector2 & size)
		{
			float hw = size.x * 0.5f;

			return Rectangle(
				point.x - hw,
				point.y,
				point.x + hw,
				point.y + size.y);
		}

		static Rectangle CenterMiddle(float x, float y, float width, float height)
		{
			float hw = width * 0.5f;
			float hh = height * 0.5f;

			return Rectangle(
				x - hw,
				y - hh,
				x + hw,
				y + hh);
		}

		static Rectangle CenterMiddle(const Vector2 & point, const Vector2 & size)
		{
			float hw = size.x * 0.5f;
			float hh = size.y * 0.5f;

			return Rectangle(
				point.x - hw,
				point.y - hh,
				point.x + hw,
				point.y + hh);
		}

		static Rectangle CenterBottom(float x, float y, float width, float height)
		{
			float hw = width * 0.5f;

			return Rectangle(
				x - hw,
				y - height,
				x + hw,
				y);
		}

		static Rectangle CenterBottom(const Vector2 & point, const Vector2 & size)
		{
			float hw = size.x * 0.5f;

			return Rectangle(
				point.x - hw,
				point.y - size.y,
				point.x + hw,
				point.y);
		}

		static Rectangle RightTop(float x, float y, float width, float height)
		{
			return Rectangle(
				x - width,
				y,
				x,
				y + height);
		}

		static Rectangle RightTop(const Vector2 & point, const Vector2 & size)
		{
			return Rectangle(
				point.x - size.x,
				point.y,
				point.x,
				point.y + size.y);
		}

		static Rectangle RightMiddle(float x, float y, float width, float height)
		{
			float hh = height * 0.5f;

			return Rectangle(
				x - width,
				y - hh,
				x,
				y + hh);
		}

		static Rectangle RightMiddle(const Vector2 & point, const Vector2 & size)
		{
			float hh = size.y * 0.5f;

			return Rectangle(
				point.x - size.x,
				point.y - hh,
				point.x,
				point.y + hh);
		}

		static Rectangle RightBottom(float x, float y, float width, float height)
		{
			return Rectangle(
				x - width,
				y - height,
				x,
				y);
		}

		static Rectangle RightBottom(const Vector2 & point, const Vector2 & size)
		{
			return Rectangle(
				point.x - size.x,
				point.y - size.y,
				point.x,
				point.y);
		}
	};

	class Quaternion
	{
	public:
		union
		{
			struct
			{
				float x, y, z, w;
			};

			struct
			{
				Vector3 xyz;
			};
		};

	public:
		Quaternion(void)
			//: x(0)
			//, y(0)
			//, z(0)
			//, w(1)
		{
		}

		Quaternion(float _x, float _y, float _z, float _w)
			: x(_x)
			, y(_y)
			, z(_z)
			, w(_w)
		{
		}

		template <class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(x);
			ar & BOOST_SERIALIZATION_NVP(y);
			ar & BOOST_SERIALIZATION_NVP(z);
			ar & BOOST_SERIALIZATION_NVP(w);
		}

	public:
		Quaternion operator - (void) const
		{
			return Quaternion(-x, -y, -z, -w);
		}

		Quaternion operator + (const Quaternion & rhs) const
		{
			return Quaternion(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
		}

		Quaternion operator + (float scaler) const
		{
			return Quaternion(x + scaler, y + scaler, z + scaler, w + scaler);
		}

		Quaternion operator - (const Quaternion & rhs) const
		{
			return Quaternion(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
		}

		Quaternion operator - (float scaler) const
		{
			return Quaternion(x - scaler, y - scaler, z - scaler, w - scaler);
		}

		Quaternion operator * (const Quaternion & rhs) const
		{
			return Quaternion(
				rhs.w * x + rhs.x * w + rhs.y * z - rhs.z * y,
				rhs.w * y + rhs.y * w + rhs.z * x - rhs.x * z,
				rhs.w * z + rhs.z * w + rhs.x * y - rhs.y * x,
				rhs.w * w - rhs.x * x - rhs.y * y - rhs.z * z);
		}

		Quaternion operator * (float scaler) const
		{
			return Quaternion(x * scaler, y * scaler, z * scaler, w * scaler);
		}

		Quaternion operator / (const Quaternion & rhs) const
		{
			return *this * rhs.inverse();
		}

		Quaternion operator / (float scaler) const
		{
			float invScaler = 1 / scaler;

			return Quaternion(x * invScaler, y * invScaler, z * invScaler, w * invScaler);
		}

		Quaternion & operator += (const Quaternion & rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			w += rhs.w;
			return *this;
		}

		Quaternion & operator += (float scaler)
		{
			x += scaler;
			y += scaler;
			z += scaler;
			w += scaler;
			return *this;
		}

		Quaternion & operator -= (const Quaternion & rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			w -= rhs.w;
			return *this;
		}

		Quaternion & operator -= (float scaler)
		{
			x -= scaler;
			y -= scaler;
			z -= scaler;
			w -= scaler;
			return *this;
		}

		Quaternion & operator *= (const Quaternion & rhs)
		{
			return *this = *this * rhs;
		}

		Quaternion & operator *= (float scaler)
		{
			x *= scaler;
			y *= scaler;
			z *= scaler;
			w *= scaler;
			return *this;
		}

		Quaternion & operator /= (const Quaternion & rhs)
		{
			return *this = *this / rhs;
		}

		Quaternion & operator /= (float scaler)
		{
			float invScaler = 1 / scaler;
			x *= invScaler;
			y *= invScaler;
			z *= invScaler;
			w *= invScaler;
			return *this;
		}

	public:
		Quaternion conjugate(void) const
		{
			return Quaternion(-x, -y, -z, w);
		}

		Quaternion & conjugateSelf(void)
		{
			x = -x;
			y = -y;
			z = -z;
			return *this;
		}

		float dot(const Quaternion & rhs) const
		{
			return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
		}

		static Quaternion Identity(void)
		{
			return Quaternion(0, 0, 0, 1);
		}

		Quaternion inverse(void) const
		{
			return conjugate() / dot(*this);
		}

		float magnitude(void) const
		{
			return sqrt(magnitudeSq());
		}

		float magnitudeSq(void) const
		{
			return x * x + y * y + z * z + w * w;
		}

		Quaternion ln(void) const
		{
			Quaternion ret;
			D3DXQuaternionLn((D3DXQUATERNION *)&ret, (D3DXQUATERNION *)this);
			return ret;
		}

		Quaternion multiply(const Quaternion & rhs) const
		{
			return *this * rhs;
		}

		Quaternion normalize(void) const
		{
			float invLength = 1 / magnitude();

			return Quaternion(x * invLength, y * invLength, z * invLength, w * invLength);
		}

		Quaternion & normalizeSelf(void)
		{
			float invLength = 1 / magnitude();
			x *= invLength;
			y *= invLength;
			z *= invLength;
			w *= invLength;
			return *this;
		}

		static Quaternion RotationAxis(const Vector3 & v, float angle)
		{
			//float c = cos(angle / 2);
			//float s = sin(angle / 2);
			//return Quaternion(v.x * s, v.y * s, v.z * s, c);

			Quaternion ret;
			D3DXQuaternionRotationAxis((D3DXQUATERNION *)&ret, (D3DXVECTOR3 *)&v, angle);
			return ret;
		}

		static Quaternion RotationMatrix(const Matrix4 & m)
		{
			Quaternion ret;
			D3DXQuaternionRotationMatrix((D3DXQUATERNION *)&ret, (D3DXMATRIX *)&m);
			return ret;
		}

		static Quaternion RotationYawPitchRoll(float yaw, float pitch, float roll)
		{
			Quaternion ret;
			D3DXQuaternionRotationYawPitchRoll((D3DXQUATERNION *)&ret, yaw, pitch, roll);
			return ret;
		}

		static Quaternion RotationFromTo(const Vector3 & from, const Vector3 & to)
		{
			Vector3 c = from.cross(to); return Quaternion(c.x, c.y, c.z, sqrt(from.magnitudeSq() * to.magnitudeSq()) + from.dot(to)).normalize();
		}

		Quaternion lerp(const Quaternion & rhs, float t) const
		{
			return Quaternion(
				x + t * (rhs.x - x),
				y + t * (rhs.y - y),
				z + t * (rhs.z - z),
				w + t * (rhs.w - w)).normalize();
		}

		Quaternion & lerpSelf(const Quaternion & rhs, float t)
		{
			x = x + t * (rhs.x - x);
			y = y + t * (rhs.y - y);
			z = z + t * (rhs.z - z);
			w = w + t * (rhs.w - w);
			return this->normalizeSelf();
		}

		Quaternion slerp(const Quaternion & rhs, float t) const
		{
			Quaternion ret;
			D3DXQuaternionSlerp((D3DXQUATERNION *)&ret, (D3DXQUATERNION *)this, (D3DXQUATERNION *)&rhs, t);
			return ret;
		}

		Quaternion & slerpSelf(const Quaternion & rhs, float t)
		{
			D3DXQuaternionSlerp((D3DXQUATERNION *)this, (D3DXQUATERNION *)this, (D3DXQUATERNION *)&rhs, t);
			return *this;
		}

		Quaternion squad(const Quaternion & a, const Quaternion & b, const Quaternion & c, float t) const
		{
			Quaternion ret;
			D3DXQuaternionSquad((D3DXQUATERNION *)&ret, (D3DXQUATERNION *)this, (D3DXQUATERNION *)&a, (D3DXQUATERNION *)&b, (D3DXQUATERNION *)&c, t);
			return ret;
		}

		Quaternion & squadSelf(const Quaternion & a, const Quaternion & b, const Quaternion & c, float t)
		{
			D3DXQuaternionSquad((D3DXQUATERNION *)this, (D3DXQUATERNION *)this, (D3DXQUATERNION *)&a, (D3DXQUATERNION *)&b, (D3DXQUATERNION *)&c, t);
			return *this;
		}

		void ToAxisAngle(Vector3 & outAxis, float & outAngle) const
		{
			D3DXQuaternionToAxisAngle((D3DXQUATERNION *)this, (D3DXVECTOR3 *)&outAxis, &outAngle);
		}

		static Quaternion RotationEulerAngles(const Vector3 & angles)
		{
			return Quaternion(
				sin(angles.x * 0.5f) * cos(angles.y * 0.5f) * cos(angles.z * 0.5f) - cos(angles.x * 0.5f) * sin(angles.y * 0.5f) * sin(angles.z * 0.5f),
				cos(angles.x * 0.5f) * sin(angles.y * 0.5f) * cos(angles.z * 0.5f) + sin(angles.x * 0.5f) * cos(angles.y * 0.5f) * sin(angles.z * 0.5f),
				cos(angles.x * 0.5f) * cos(angles.y * 0.5f) * sin(angles.z * 0.5f) - sin(angles.x * 0.5f) * sin(angles.y * 0.5f) * cos(angles.z * 0.5f),
				cos(angles.x * 0.5f) * cos(angles.y * 0.5f) * cos(angles.z * 0.5f) + sin(angles.x * 0.5f) * sin(angles.y * 0.5f) * sin(angles.z * 0.5f));
		}

		float ToEulerAngleX(void) const
		{
			return atan2((w * x + y * z) * 2, 1 - (x * x + y * y) * 2);
		}

		float ToEulerAngleY(void) const
		{
			return asin((w * y - z * x) * 2);
		}

		float ToEulerAngleZ(void) const
		{
			return atan2((w * z + x * y) * 2, 1 - (y * y + z * z) * 2);
		}

		Vector3 ToEulerAngles(void) const
		{
			return Vector3(ToEulerAngleX(), ToEulerAngleY(), ToEulerAngleZ());
		}

	public:
		static const Quaternion identity;
	};

	class Matrix4
	{
	public:
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};

	public:
		Matrix4(void)
			//: _11(0), _12(0), _13(0), _14(0)
			//, _21(0), _22(0), _23(0), _24(0)
			//, _31(0), _32(0), _33(0), _34(0)
			//, _41(0), _42(0), _43(0), _44(0)
		{
		}

		Matrix4(float _v)
			: _11(_v), _12(_v), _13(_v), _14(_v)
			, _21(_v), _22(_v), _23(_v), _24(_v)
			, _31(_v), _32(_v), _33(_v), _34(_v)
			, _41(_v), _42(_v), _43(_v), _44(_v)
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

		template <class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(_11); ar & BOOST_SERIALIZATION_NVP(_12); ar & BOOST_SERIALIZATION_NVP(_13); ar & BOOST_SERIALIZATION_NVP(_14);
			ar & BOOST_SERIALIZATION_NVP(_21); ar & BOOST_SERIALIZATION_NVP(_22); ar & BOOST_SERIALIZATION_NVP(_23); ar & BOOST_SERIALIZATION_NVP(_24);
			ar & BOOST_SERIALIZATION_NVP(_31); ar & BOOST_SERIALIZATION_NVP(_32); ar & BOOST_SERIALIZATION_NVP(_33); ar & BOOST_SERIALIZATION_NVP(_34);
			ar & BOOST_SERIALIZATION_NVP(_41); ar & BOOST_SERIALIZATION_NVP(_42); ar & BOOST_SERIALIZATION_NVP(_43); ar & BOOST_SERIALIZATION_NVP(_44);
		}

	public:
		Vector4 & operator [](size_t i)
		{
			return Subscribe<Vector4>(*this, i);
		}

		const Vector4 & operator [](size_t i) const
		{
			return Subscribe<Vector4>(*this, i);
		}

		template <size_t i>
		Vector4 & row(void)
		{
			return Subscribe<Vector4, i>(*this);
		}

		template <size_t i>
		const Vector4 & row(void) const
		{
			return Subscribe<Vector4, i>(*this);
		}

		template <size_t i>
		Vector4 column(void) const
		{
			return Vector4(
				Subscribe<float, i>(Subscribe<Vector4, 0>(*this)),
				Subscribe<float, i>(Subscribe<Vector4, 1>(*this)),
				Subscribe<float, i>(Subscribe<Vector4, 2>(*this)),
				Subscribe<float, i>(Subscribe<Vector4, 3>(*this)));
		}

	public:
		Matrix4 operator - (void) const
		{
			return Matrix4(
				-_11, -_12, -_13, -_14,
				-_21, -_22, -_23, -_24,
				-_31, -_32, -_33, -_34,
				-_41, -_42, -_43, -_44);
		}

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
			return Matrix4(
				_11 * rhs._11 + _12 * rhs._21 + _13 * rhs._31 + _14 * rhs._41,
				_11 * rhs._12 + _12 * rhs._22 + _13 * rhs._32 + _14 * rhs._42,
				_11 * rhs._13 + _12 * rhs._23 + _13 * rhs._33 + _14 * rhs._43,
				_11 * rhs._14 + _12 * rhs._24 + _13 * rhs._34 + _14 * rhs._44,

				_21 * rhs._11 + _22 * rhs._21 + _23 * rhs._31 + _24 * rhs._41,
				_21 * rhs._12 + _22 * rhs._22 + _23 * rhs._32 + _24 * rhs._42,
				_21 * rhs._13 + _22 * rhs._23 + _23 * rhs._33 + _24 * rhs._43,
				_21 * rhs._14 + _22 * rhs._24 + _23 * rhs._34 + _24 * rhs._44,

				_31 * rhs._11 + _32 * rhs._21 + _33 * rhs._31 + _34 * rhs._41,
				_31 * rhs._12 + _32 * rhs._22 + _33 * rhs._32 + _34 * rhs._42,
				_31 * rhs._13 + _32 * rhs._23 + _33 * rhs._33 + _34 * rhs._43,
				_31 * rhs._14 + _32 * rhs._24 + _33 * rhs._34 + _34 * rhs._44,

				_41 * rhs._11 + _42 * rhs._21 + _43 * rhs._31 + _44 * rhs._41,
				_41 * rhs._12 + _42 * rhs._22 + _43 * rhs._32 + _44 * rhs._42,
				_41 * rhs._13 + _42 * rhs._23 + _43 * rhs._33 + _44 * rhs._43,
				_41 * rhs._14 + _42 * rhs._24 + _43 * rhs._34 + _44 * rhs._44);
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
			return *this * rhs.inverse();
		}

		Matrix4 operator / (float scaler) const
		{
			float invScaler = 1 / scaler;

			return Matrix4(
				_11 * invScaler, _12 * invScaler, _13 * invScaler, _14 * invScaler,
				_21 * invScaler, _22 * invScaler, _23 * invScaler, _24 * invScaler,
				_31 * invScaler, _32 * invScaler, _33 * invScaler, _34 * invScaler,
				_41 * invScaler, _42 * invScaler, _43 * invScaler, _44 * invScaler);
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
			float invScaler = 1 / scaler;
			_11 *= invScaler; _12 *= invScaler; _13 *= invScaler; _14 *= invScaler;
			_21 *= invScaler; _22 *= invScaler; _23 *= invScaler; _24 *= invScaler;
			_31 *= invScaler; _32 *= invScaler; _33 *= invScaler; _34 *= invScaler;
			_41 *= invScaler; _42 *= invScaler; _43 *= invScaler; _44 *= invScaler;
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

		void Decompose(Vector3 & outScale, Quaternion & outRotation, Vector3 & outTranslation) const
		{
			D3DXMatrixDecompose((D3DXVECTOR3 *)&outScale, (D3DXQUATERNION *)&outRotation, (D3DXVECTOR3 *)&outTranslation, (const D3DXMATRIX *)this);
		}

		static Matrix4 Compose(const Vector3 & Scale, const Quaternion & Rotate, const Vector3 & Translate)
		{
			return Scaling(Scale) * RotationQuaternion(Rotate) * Translation(Translate);
		}

		static Matrix4 Compose(const Vector3 & Scale, const Vector3 & ypr, const Vector3 & Translate)
		{
			return Scaling(Scale) * RotationYawPitchRoll(ypr.y, ypr.x, ypr.z) * Translation(Translate);
		}

		float determinant(void) const
		{
			return _11 * a11() - _12 * a12() + _13 * a13() - _14 * a14();
		}

		static Matrix4 Identity(void)
		{
			return Matrix4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
		}

		Matrix4 inverse(void) const
		{
			//return adjoint() / determinant();

			Matrix4 ret;
			D3DXMatrixInverse((D3DXMATRIX *)&ret, NULL, (D3DXMATRIX *)this);
			return ret;
		}

		static Matrix4 LookAtLH(const Vector3 & eye, const Vector3 & at, const Vector3 & up)
		{
			Vector3 zaxis = (at - eye).normalize();
			Vector3 xaxis = up.cross(zaxis).normalize();
			Vector3 yaxis = zaxis.cross(xaxis);

			return Matrix4(
				xaxis.x,			yaxis.x,			zaxis.x,			0,
				xaxis.y,			yaxis.y,			zaxis.y,			0,
				xaxis.z,			yaxis.z,			zaxis.z,			0,
				-xaxis.dot(eye),	-yaxis.dot(eye),	-zaxis.dot(eye),	1);
		}

		static Matrix4 LookAtRH(const Vector3 & eye, const Vector3 & at, const Vector3 & up)
		{
			Vector3 zaxis = (eye - at).normalize();
			Vector3 xaxis = up.cross(zaxis).normalize();
			Vector3 yaxis = zaxis.cross(xaxis);

			return Matrix4(
				xaxis.x,			yaxis.x,			zaxis.x,			0,
				xaxis.y,			yaxis.y,			zaxis.y,			0,
				xaxis.z,			yaxis.z,			zaxis.z,			0,
				-xaxis.dot(eye),	-yaxis.dot(eye),	-zaxis.dot(eye),	1);
		}

		Matrix4 multiply(const Matrix4 & rhs) const
		{
			return *this * rhs;
		}

		Matrix4 multiplyTranspose(const Matrix4 & rhs) const
		{
			return multiply(rhs).transpose();
		}

		static Matrix4 OrthoLH(float w, float h, float zn, float zf)
		{
			return Matrix4(
				2 / w,	0,		0,					0,
				0,		2 / h,	0,					0,
				0,		0,		1 / (zf - zn),		0,
				0,		0,		-zn / (zf - zn),	1);
		}

		static Matrix4 OrthoRH(float w, float h, float zn, float zf)
		{
			return Matrix4(
				2 / w,	0,		0,					0,
				0,		2 / h,	0,					0,
				0,		0,		1 / (zn - zf),		0,
				0,		0,		zn / (zn - zf),		1);
		}

		static Matrix4 OrthoOffCenterLH(float l, float r, float b, float t, float zn, float zf)
		{
			return Matrix4(
				2 / (r - l),		0,					0,					0,
				0,					2 / (t - b),		0,					0,
				0,					0,					1 / (zf - zn),		0,
				(l + r) / (l - r),	(t + b) / (b - t),	zn / (zn - zf),		1);
		}

		static Matrix4 OrthoOffCenterRH(float l, float r, float b, float t, float zn, float zf)
		{
			return Matrix4(
				2 / (r - l),		0,					0,					0,
				0,					2 / (t - b),		0,					0,
				0,					0,					1 / (zn - zf),		0,
				(l + r) / (l - r),	(t + b) / (b - t),	zn / (zn - zf),		1);
		}

		static Matrix4 PerspectiveFovLH(float fovy, float aspect, float zn, float zf)
		{
			float yScale = cot(fovy / 2);
			float xScale = yScale / aspect;

			return Matrix4(
				xScale,	0,		0,						0,
				0,		yScale,	0,						0,
				0,		0,		zf / (zf - zn),			1,
				0,		0,		-zn * zf / (zf - zn),	0);

		}

		static Matrix4 PerspectiveFovRH(float fovy, float aspect, float zn, float zf)
		{
			float yScale = cot(fovy / 2);
			float xScale = yScale / aspect;

			return Matrix4(
				xScale,	0,		0,						0,
				0,		yScale,	0,						0,
				0,		0,		zf / (zn - zf),			-1,
				0,		0,		zn * zf / (zn - zf),	0);
		}

		static Matrix4 PerspectiveAovLH(float fovx, float aspect, float zn, float zf)
		{
			float xScale = cot(fovx / 2);
			float yScale = xScale * aspect;

			return Matrix4(
				xScale,	0,		0,						0,
				0,		yScale,	0,						0,
				0,		0,		zf / (zf - zn),			1,
				0,		0,		-zn * zf / (zf - zn),	0);
		}

		static Matrix4 PerspectiveAovRH(float fovx, float aspect, float zn, float zf)
		{
			float xScale = cot(fovx / 2);
			float yScale = xScale * aspect;

			return Matrix4(
				xScale,	0,		0,						0,
				0,		yScale,	0,						0,
				0,		0,		zf / (zn - zf),			-1,
				0,		0,		zn * zf / (zn - zf),	0);
		}

		static Matrix4 PerspectiveLH(float w, float h, float zn, float zf)
		{
			return Matrix4(
				2 * zn / w,	0,			0,						0,
				0,			2 * zn / h,	0,						0,
				0,			0,			zf / (zf - zn),			1,
				0,			0,			zn * zf / (zn - zf),	0);
		}

		static Matrix4 PerspectiveRH(float w, float h, float zn, float zf)
		{
			return Matrix4(
				2 * zn / w,	0,			0,						0,
				0,			2 * zn / h,	0,						0,
				0,			0,			zf / (zn - zf),			-1,
				0,			0,			zn * zf / (zn - zf),	0);
		}

		static Matrix4 PerspectiveOffCenterLH(float l, float r, float b, float t, float zn, float zf)
		{
			return Matrix4(
				2 * zn / (r - l),	0,					0,						0,
				0,					2 * zn / (t - b),	0,						0,
				(l + r) / (l - r),	(t + b) / (b - t),	zf / (zf - zn),			1,
				0,					0,					zn * zf / (zn - zf),	0);
		}

		static Matrix4 PerspectiveOffCenterRH(float l, float r, float b, float t, float zn, float zf)
		{
			return Matrix4(
				2 * zn / (r - l),	0,					0,						0,
				0,					2 * zn / (t - b),	0,						0,
				(l + r) / (r - l),	(t + b) / (t - b),	zf / (zn - zf),			-1,
				0,					0,					zn * zf / (zn - zf),	0);
		}

		static Matrix4 RotationAxis(const Vector3 & v, float angle)
		{
			Matrix4 ret;
			D3DXMatrixRotationAxis((D3DXMATRIX *)&ret, (D3DXVECTOR3 *)&v, angle);
			return ret;
		}

		static Matrix4 RotationQuaternion(const Quaternion & q)
		{
			//return Matrix4(
			//	1 - 2 * (q.y * q.y + q.z * q.z), 2 * (q.x * q.y + q.w * q.z), 2 * (q.x * q.z - q.w * q.y), 0,
			//	2 * (q.x * q.y - q.w * q.z), 1 - 2 * (q.x * q.x + q.z * q.z), 2 * (q.y * q.z + q.w * q.x), 0,
			//	2 * (q.x * q.z + q.w * q.y), 2 * (q.y * q.z - q.w * q.x), 1 - 2 * (q.x * q.x + q.y * q.y), 0,
			//	0, 0, 0, 1);

			Matrix4 ret;
			D3DXMatrixRotationQuaternion((D3DXMATRIX *)&ret, (D3DXQUATERNION *)&q);
			return ret;
		}

		static Matrix4 RotationX(float angle)
		{
			float c = cos(angle);
			float s = sin(angle);

			return Matrix4(1, 0, 0, 0, 0, c, s, 0, 0, -s, c, 0, 0, 0, 0, 1);
		}

		static Matrix4 RotationY(float angle)
		{
			float c = cos(angle);
			float s = sin(angle);

			return Matrix4(c, 0, -s, 0, 0, 1, 0, 0, s, 0, c, 0, 0, 0, 0, 1);
		}

		static Matrix4 RotationZ(float angle)
		{
			float c = cos(angle);
			float s = sin(angle);

			return Matrix4(c, s, 0, 0, -s, c, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
		}

		static Matrix4 RotationYawPitchRoll(float yaw, float pitch, float roll)
		{
			//return RotationZ(roll).rotateX(pitch).rotateY(yaw);

			Matrix4 ret;
			D3DXMatrixRotationYawPitchRoll((D3DXMATRIX *)&ret, yaw, pitch, roll);
			return ret;
		}

		static Matrix4 Scaling(float x, float y, float z)
		{
			return Matrix4(x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, 0, 0, 0, 1);
		}

		static Matrix4 Scaling(const Vector3 & v)
		{
			return Matrix4(v.x, 0, 0, 0, 0, v.y, 0, 0, 0, 0, v.z, 0, 0, 0, 0, 1);
		}

		static Matrix4 Transformation(
			const Vector3 & scalingCenter,
			const Quaternion & scalingRotation,
			const Vector3 & scaling,
			const Vector3 & rotationCenter,
			const Quaternion & rotation,
			const Vector3 & translation)
		{
			Matrix4 ret;
			D3DXMatrixTransformation(
				(D3DXMATRIX *)&ret,
				(D3DXVECTOR3 *)&scalingCenter,
				(D3DXQUATERNION *)&scalingRotation,
				(D3DXVECTOR3 *)&scaling,
				(D3DXVECTOR3 *)&rotationCenter,
				(D3DXQUATERNION *)&rotation,
				(D3DXVECTOR3 *)&translation);
			return ret;
		}

		static Matrix4 Transformation2D(
			const Vector2 & scalingCenter,
			float scalingRotation,
			const Vector2 & scaling,
			const Vector2 & rotationCenter,
			float rotation,
			const Vector2 & translation)
		{
			Matrix4 ret;
			D3DXMatrixTransformation2D(
				(D3DXMATRIX *)&ret,
				(D3DXVECTOR2 *)&scalingCenter,
				scalingRotation,
				(D3DXVECTOR2 *)&scaling,
				(D3DXVECTOR2 *)&rotationCenter,
				rotation,
				(D3DXVECTOR2 *)&translation);
			return ret;
		}

		static Matrix4 Translation(float x, float y, float z)
		{
			return Matrix4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x, y, z, 1);
		}

		static Matrix4 Translation(const Vector3 & v)
		{
			return Matrix4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, v.x, v.y, v.z, 1);
		}

		Matrix4 transpose(void) const
		{
			return Matrix4(
				_11, _21, _31, _41,
				_12, _22, _32, _42,
				_13, _23, _33, _43,
				_14, _24, _34, _44);
		}

		Matrix4 transformTranspose(const Matrix4 & rhs) const
		{
			return Matrix4(
				_11 * rhs._11 + _12 * rhs._12 + _13 * rhs._13 + _14 * rhs._14,
				_11 * rhs._21 + _12 * rhs._22 + _13 * rhs._23 + _14 * rhs._24,
				_11 * rhs._31 + _12 * rhs._32 + _13 * rhs._33 + _14 * rhs._34,
				_11 * rhs._41 + _12 * rhs._42 + _13 * rhs._43 + _14 * rhs._44,

				_21 * rhs._11 + _22 * rhs._12 + _23 * rhs._13 + _24 * rhs._14,
				_21 * rhs._21 + _22 * rhs._22 + _23 * rhs._23 + _24 * rhs._24,
				_21 * rhs._31 + _22 * rhs._32 + _23 * rhs._33 + _24 * rhs._34,
				_21 * rhs._41 + _22 * rhs._42 + _23 * rhs._43 + _24 * rhs._44,

				_31 * rhs._11 + _32 * rhs._12 + _33 * rhs._13 + _34 * rhs._14,
				_31 * rhs._21 + _32 * rhs._22 + _33 * rhs._23 + _34 * rhs._24,
				_31 * rhs._31 + _32 * rhs._32 + _33 * rhs._33 + _34 * rhs._34,
				_31 * rhs._41 + _32 * rhs._42 + _33 * rhs._43 + _34 * rhs._44,

				_41 * rhs._11 + _42 * rhs._12 + _43 * rhs._13 + _44 * rhs._14,
				_41 * rhs._21 + _42 * rhs._22 + _43 * rhs._23 + _44 * rhs._24,
				_41 * rhs._31 + _42 * rhs._32 + _43 * rhs._33 + _44 * rhs._34,
				_41 * rhs._41 + _42 * rhs._42 + _43 * rhs._43 + _44 * rhs._44);
		}

	public:
		Matrix4 scale(float x, float y, float z) const
		{
			return Matrix4(
				_11 * x, _12 * y, _13 * z, _14 * 1,
				_21 * x, _22 * y, _23 * z, _24 * 1,
				_31 * x, _32 * y, _33 * z, _34 * 1,
				_41 * x, _42 * y, _43 * z, _44 * 1);
		}

		Matrix4 scale(const Vector3 & scaling) const
		{
			return Matrix4(
				_11 * scaling.x, _12 * scaling.y, _13 * scaling.z, _14 * 1,
				_21 * scaling.x, _22 * scaling.y, _23 * scaling.z, _24 * 1,
				_31 * scaling.x, _32 * scaling.y, _33 * scaling.z, _34 * 1,
				_41 * scaling.x, _42 * scaling.y, _43 * scaling.z, _44 * 1);
		}

		Matrix4 & scaleSelf(float x, float y, float z)
		{
			_11 = _11 * x; _12 = _12 * y; _13 = _13 * z; _14 = _14 * 1;
			_21 = _21 * x; _22 = _22 * y; _23 = _23 * z; _24 = _24 * 1;
			_31 = _31 * x; _32 = _32 * y; _33 = _33 * z; _34 = _34 * 1;
			_41 = _41 * x; _42 = _42 * y; _43 = _43 * z; _44 = _44 * 1;
			return *this;
		}

		Matrix4 & scaleSelf(const Vector3 & scaling)
		{
			_11 = _11 * scaling.x; _12 = _12 * scaling.y; _13 = _13 * scaling.z; _14 = _14 * 1;
			_21 = _21 * scaling.x; _22 = _22 * scaling.y; _23 = _23 * scaling.z; _24 = _24 * 1;
			_31 = _31 * scaling.x; _32 = _32 * scaling.y; _33 = _33 * scaling.z; _34 = _34 * 1;
			_41 = _41 * scaling.x; _42 = _42 * scaling.y; _43 = _43 * scaling.z; _44 = _44 * 1;
			return *this;
		}

		Matrix4 rotateX(float angle) const
		{
			float c = cos(angle);
			float s = sin(angle);

			return Matrix4(
				_11, _12 * c - _13 * s, _12 * s + _13 * c, _14,
				_21, _22 * c - _23 * s, _22 * s + _23 * c, _24,
				_31, _32 * c - _33 * s, _32 * s + _33 * c, _34,
				_41, _42 * c - _43 * s, _42 * s + _43 * c, _44);
		}

		Matrix4 rotateY(float angle) const
		{
			float c = cos(angle);
			float s = sin(angle);

			return Matrix4(
				_11 * c + _13 * s, _12, _13 * c - _11 * s, _14,
				_21 * c + _23 * s, _22, _23 * c - _21 * s, _24,
				_31 * c + _33 * s, _32, _33 * c - _31 * s, _34,
				_41 * c + _43 * s, _42, _43 * c - _41 * s, _44);
		}

		Matrix4 rotateZ(float angle) const
		{
			float c = cos(angle);
			float s = sin(angle);

			return Matrix4(
				_11 * c - _12 * s, _11 * s + _12 * c, _13, _14,
				_21 * c - _22 * s, _21 * s + _22 * c, _23, _24,
				_31 * c - _32 * s, _31 * s + _32 * c, _33, _34,
				_41 * c - _42 * s, _41 * s + _42 * c, _43, _44);
		}

		Matrix4 rotate(const Quaternion & q) const
		{
			return *this * RotationQuaternion(q);
		}

		Matrix4 translate(float x, float y, float z) const
		{
			return Matrix4(
				_11 + _14 * x, _12 + _14 * y, _13 + _14 * z, _14,
				_21 + _24 * x, _22 + _24 * y, _23 + _24 * z, _24,
				_31 + _34 * x, _32 + _34 * y, _33 + _34 * z, _34,
				_41 + _44 * x, _42 + _44 * y, _43 + _44 * z, _44);
		}

		Matrix4 translate(const Vector3 & v) const
		{
			return Matrix4(
				_11 + _14 * v.x, _12 + _14 * v.y, _13 + _14 * v.z, _14,
				_21 + _24 * v.x, _22 + _24 * v.y, _23 + _24 * v.z, _24,
				_31 + _34 * v.x, _32 + _34 * v.y, _33 + _34 * v.z, _34,
				_41 + _44 * v.x, _42 + _44 * v.y, _43 + _44 * v.z, _44);
		}

		Matrix4 & translateSelf(float x, float y, float z)
		{
			_11 = _11 + _14 * x; _12 = _12 + 14 * y; _13 = _13 + _14 * z;
			_21 = _21 + _24 * x; _22 = _22 + 14 * y; _23 = _23 + _24 * z;
			_31 = _31 + _34 * x; _32 = _32 + 14 * y; _33 = _33 + _34 * z;
			_41 = _41 + _44 * x; _42 = _42 + 14 * y; _43 = _43 + _44 * z;
			return *this;
		}

		Matrix4 & translateSelf(const Vector3 & v)
		{
			_11 = _11 + _14 * v.x; _12 = _12 + 14 * v.y; _13 = _13 + _14 * v.z;
			_21 = _21 + _24 * v.x; _22 = _22 + 14 * v.y; _23 = _23 + _24 * v.z;
			_31 = _31 + _34 * v.x; _32 = _32 + 14 * v.y; _33 = _33 + _34 * v.z;
			_41 = _41 + _44 * v.x; _42 = _42 + 14 * v.y; _43 = _43 + _44 * v.z;
			return *this;
		}

		Matrix4 lerp(const Matrix4 & rhs, float s) const
		{
			return Matrix4(
				_11 + s * (rhs._11 - _11), _12 + s * (rhs._12 - _12), _13 + s * (rhs._13 - _13), _14 + s * (rhs._14 - _14),
				_21 + s * (rhs._21 - _21), _22 + s * (rhs._22 - _22), _23 + s * (rhs._23 - _23), _24 + s * (rhs._24 - _24),
				_31 + s * (rhs._31 - _31), _32 + s * (rhs._32 - _32), _33 + s * (rhs._33 - _33), _34 + s * (rhs._34 - _34),
				_41 + s * (rhs._41 - _41), _42 + s * (rhs._42 - _42), _43 + s * (rhs._43 - _43), _44 + s * (rhs._44 - _44));
		}

		Matrix4 & lerpSelf(const Matrix4 & rhs, float s)
		{
			_11 = _11 + s * (rhs._11 - _11); _12 = _12 + s * (rhs._12 - _12); _13 = _13 + s * (rhs._13 - _13); _14 = _14 + s * (rhs._14 - _14);
			_21 = _21 + s * (rhs._21 - _21); _22 = _22 + s * (rhs._22 - _22); _23 = _23 + s * (rhs._23 - _23); _24 = _24 + s * (rhs._24 - _24);
			_31 = _31 + s * (rhs._31 - _31); _32 = _32 + s * (rhs._32 - _32); _33 = _33 + s * (rhs._33 - _33); _34 = _34 + s * (rhs._34 - _34);
			_41 = _41 + s * (rhs._41 - _41); _42 = _42 + s * (rhs._42 - _42); _43 = _43 + s * (rhs._43 - _43); _44 = _44 + s * (rhs._44 - _44);
			return *this;
		}

		static Matrix4 UDQtoRM(const Matrix4 & dual);

	public:
		static const Matrix4 identity;
	};

	class TransformList
		: public std::vector<Matrix4>
	{
	public:
		Matrix4 BuildSkinnedDualQuaternion(DWORD indices, const Vector4 & weights) const;

		Vector3 TransformVertexWithDualQuaternionList(const Vector3 & position, DWORD indices, const Vector4 & weights) const;
	};

	class Plane
	{
	public:
		union
		{
			struct 
			{
				float a, b, c;
			};

			struct 
			{
				Vector3 normal;
			};
		};

		float d;

	public:
		Plane(void)
			//: a(1)
			//, b(0)
			//, c(0)
			//, d(0)
		{
		}

		Plane(float _a, float _b, float _c, float _d)
			: a(_a)
			, b(_b)
			, c(_c)
			, d(_d)
		{
		}

		static Plane NormalDistance(const Vector3 & normal, float distance);

		static Plane NormalPosition(const Vector3 & normal, const Vector3 & pos);

		static Plane FromTriangle(const Vector3 & v0, const Vector3 & v1, const Vector3 & v2);

		float magnitude(void) const
		{
			return sqrt(magnitudeSq());
		}

		float magnitudeSq(void) const
		{
			return a * a + b * b + c * c;
		}

		Plane normalize(void) const
		{
			float invLength = 1 / magnitude();

			return Plane(a * invLength, b * invLength, c * invLength, d * invLength);
		}

		Plane & normalizeSelf(void)
		{
			float invLength = 1 / magnitude();
			a *= invLength;
			b *= invLength;
			c *= invLength;
			d *= invLength;
			return *this;
		}

		float DistanceToPoint(const Vector3 & pt) const
		{
			return a * pt.x + b * pt.y + c * pt.z + d;
		}

		Plane transform(const Matrix4 & InverseTranspose) const;

		Plane & transformSelf(const Matrix4 & InverseTranspose);
	};

	class Ray
	{
	public:
		Vector3 p;

		Vector3 d;

	public:
		Ray(const Vector3 & _p, const Vector3 & _d)
			: p(_p)
			, d(_d)
		{
			IS_NORMALIZED(d);
		}

		Ray transform(const Matrix4 & m) const;

		Ray & transformSelf(const Matrix4 & m);
	};

	typedef std::pair<bool, float> RayResult;

	class Frustum
	{
	public:
		Plane Up, Down, Left, Right, Near, Far;

	public:
		Frustum(void)
			//: Up()
			//, Down()
			//, Left()
			//, Right()
			//, Near()
			//, Far()
		{
		}

		Frustum(const Plane & _Up, const Plane & _Down, const Plane & _Left, const Plane & _Right, const Plane & _Near, const Plane & _Far)
			: Up(_Up)
			, Down(_Down)
			, Left(_Left)
			, Right(_Right)
			, Near(_Near)
			, Far(_Far)
		{
		}

		Plane & operator [](size_t i)
		{
			return Subscribe<Plane>(*this, i);
		}

		const Plane & operator [](size_t i) const
		{
			return Subscribe<Plane>(*this, i);
		}

		static Frustum ExtractMatrix(const Matrix4 & m)
		{
			// ! need normalize ?
			return Frustum(
				Plane(m._14 - m._12, m._24 - m._22, m._34 - m._32, m._44 - m._42),
				Plane(m._14 + m._12, m._24 + m._22, m._34 + m._32, m._44 + m._42),
				Plane(m._14 + m._11, m._24 + m._21, m._34 + m._31, m._44 + m._41),
				Plane(m._14 - m._11, m._24 - m._21, m._34 - m._31, m._44 - m._41),
				Plane(m._13, m._23, m._33, m._43),
				Plane(m._14 - m._13, m._24 - m._23, m._34 - m._33, m._44 - m._43));
		}

		Frustum transform(const Matrix4 & InverseTranspose) const;

		Frustum & transformSelf(const Matrix4 & InverseTranspose);
	};

	class AABB
	{
	public:
		enum Quadrant
		{
			QuadrantPxPyPz = 0,
			QuadrantPxPyNz,
			QuadrantPxNyPz,
			QuadrantPxNyNz,
			QuadrantNxPyPz,
			QuadrantNxPyNz,
			QuadrantNxNyPz,
			QuadrantNxNyNz,
			QuadrantNum
		};

		Vector3 m_min;

		Vector3 m_max;

		static const AABB invalid;

	public:
		AABB(void)
			//: m_min(-FLT_MAX,-FLT_MAX,-FLT_MAX)
			//, m_max( FLT_MAX, FLT_MAX, FLT_MAX)
		{
		}

		AABB(float minv, float maxv)
			: m_min(minv)
			, m_max(maxv)
		{
		}

		AABB(float minx, float miny, float minz, float maxx, float maxy, float maxz)
			: m_min(minx, miny, minz)
			, m_max(maxx, maxy, maxz)
		{
		}

		AABB(const Vector3 & _Min, const Vector3 & _Max)
			: m_min(_Min)
			, m_max(_Max)
		{
		}

		AABB(const Vector3 & center, float radius)
			: m_min(center.x - radius, center.y - radius, center.z - radius)
			, m_max(center.x + radius, center.y + radius, center.z + radius)
		{
			_ASSERT(radius >= 0);
		}

		static AABB Invalid(void)
		{
			return AABB(FLT_MAX, -FLT_MAX);
		}

		bool IsValid(void) const
		{
			return m_min.x < m_max.x
				&& m_min.y < m_max.y
				&& m_min.z < m_max.z;
		}

		template <class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(m_min);
			ar & BOOST_SERIALIZATION_NVP(m_max);
		}

	public:
		Vector3 Center(void) const
		{
			return Vector3(
				Lerp(m_min.x, m_max.x, 0.5f),
				Lerp(m_min.y, m_max.y, 0.5f),
				Lerp(m_min.z, m_max.z, 0.5f));
		}

		Vector3 Extent(void) const
		{
			return m_max - m_min;
		}

		template <UINT Quad>
		AABB Slice(const Vector3 & cente);

		AABB Intersect(const AABB & rhs) const
		{
			return AABB(
				Max(m_min.x, rhs.m_min.x),
				Max(m_min.y, rhs.m_min.y),
				Max(m_min.z, rhs.m_min.z),
				Min(m_max.x, rhs.m_max.x),
				Min(m_max.y, rhs.m_max.y),
				Min(m_max.z, rhs.m_max.z));
		}

		AABB & intersectSelf(const AABB & rhs)
		{
			m_min.x = Max(m_min.x, rhs.m_min.x);
			m_min.y = Max(m_min.y, rhs.m_min.y);
			m_min.z = Max(m_min.z, rhs.m_min.z);
			m_max.x = Min(m_max.x, rhs.m_max.x);
			m_max.y = Min(m_max.y, rhs.m_max.y);
			m_max.z = Min(m_max.z, rhs.m_max.z);
			return *this;
		}

		AABB Union(const Vector3 & pos) const
		{
			return AABB(
				Min(m_min.x, pos.x),
				Min(m_min.y, pos.y),
				Min(m_min.z, pos.z),
				Max(m_max.x, pos.x),
				Max(m_max.y, pos.y),
				Max(m_max.z, pos.z));
		};

		AABB & unionSelf(const Vector3 & pos)
		{
			m_min.x = Min(m_min.x, pos.x);
			m_min.y = Min(m_min.y, pos.y);
			m_min.z = Min(m_min.z, pos.z);
			m_max.x = Max(m_max.x, pos.x);
			m_max.y = Max(m_max.y, pos.y);
			m_max.z = Max(m_max.z, pos.z);
			return *this;
		}

		AABB Union(const AABB & rhs) const
		{
			return AABB(
				Min(m_min.x, rhs.m_min.x),
				Min(m_min.y, rhs.m_min.y),
				Min(m_min.z, rhs.m_min.z),
				Max(m_max.x, rhs.m_max.x),
				Max(m_max.y, rhs.m_max.y),
				Max(m_max.z, rhs.m_max.z));
		}

		AABB & unionSelf(const AABB & rhs)
		{
			m_min.x = Min(m_min.x, rhs.m_min.x);
			m_min.y = Min(m_min.y, rhs.m_min.y);
			m_min.z = Min(m_min.z, rhs.m_min.z);
			m_max.x = Max(m_max.x, rhs.m_max.x);
			m_max.y = Max(m_max.y, rhs.m_max.y);
			m_max.z = Max(m_max.z, rhs.m_max.z);
			return *this;
		}

		Vector3 p(const Vector3 & normal) const
		{
			return Vector3(
				normal.x > 0 ? m_min.x : m_max.x,
				normal.y > 0 ? m_min.y : m_max.y,
				normal.z > 0 ? m_min.z : m_max.z);
		}

		Vector3 n(const Vector3 & normal) const
		{
			return Vector3(
				normal.x > 0 ? m_max.x : m_min.x,
				normal.y > 0 ? m_max.y : m_min.y,
				normal.z > 0 ? m_max.z : m_min.z);
		}

		AABB transform(const Matrix4 & m) const;

		AABB & transformSelf(const Matrix4 & m);
	};
}
