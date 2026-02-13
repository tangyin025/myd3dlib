// Copyright (c) 2011-2024 tangyin025
// License: MIT
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

#define cotf(x)	tanf(D3DX_PI / 2 - (x))

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
	T Max(T a, T b)
	{
		return a > b ? a : b;
	}

	template <typename T>
	T Lerp(T a, T b, float s)
	{
		return (T)(a + s * (b - a));
	}

	template <typename T>
	T Clamp(T v, T min, T max)
	{
		return v < min ? min : (v < max ? v : max);
	}

	template <typename T>
	T Wrap(T v, T min, T max);

	template <typename T>
	T Terrace(T v, T align)
	{
		return (T)(align * floor(v / align + 0.5f));
	}

	template <typename T>
	T Random(T range);

	template <typename T>
	T Random(T min, T max);

	template <typename Type1, typename Type2>
	Type1 ipow(Type1 a, Type2 ex)
	{
		// Return a**ex
		if (0 == ex)  return 1;
		else
		{
			Type1 z = a;
			Type1 y = 1;
			while (1)
			{
				if (ex & 1)  y *= z;
				ex /= 2;
				if (0 == ex)  break;
				z *= z;
			}
			return y;
		}
	}

	template <typename ForwardRange, typename ForwardRangeIterator>
	ForwardRange & union_insert(ForwardRange & rng, ForwardRangeIterator begin, ForwardRangeIterator end)
	{
		for (ForwardRangeIterator iter = begin; iter != end; iter++)
		{
			ForwardRangeIterator find_iter = std::find(rng.begin(), rng.end(), *iter);
			if (find_iter == rng.end())
			{
				rng.push_back(*iter);
			}
		}
		return rng;
	}

	template <typename ForwardRange, typename ForwardRangeIterator>
	ForwardRange & intersection_remove(ForwardRange & rng, ForwardRangeIterator begin, ForwardRangeIterator end)
	{
		for (ForwardRangeIterator iter = begin; iter != end; iter++)
		{
			ForwardRangeIterator find_iter = std::find(rng.begin(), rng.end(), *iter);
			if (find_iter != rng.end())
			{
				rng.erase(find_iter);
			}
		}
		return rng;
	}

	class Vector4;

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

		explicit Vector2(float _v)
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
			_ASSERT(i < 2);
			return (&x)[i];
		}

		const float & operator [](size_t i) const
		{
			_ASSERT(i < 2);
			return (&x)[i];
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
		float kross(const Vector2 & rhs) const
		{
			return x * rhs.y - y * rhs.x;
		}

		float dot(const Vector2 & rhs) const
		{
			return x * rhs.x + y * rhs.y;
		}

		float magnitude(void) const
		{
			return sqrtf(magnitudeSq());
		}

		float magnitudeSq(void) const
		{
			return x * x + y * y;
		}
				
		float distance(const Vector2 & rhs) const
		{
			return operator-(rhs).magnitude();
		}

		float distanceSq(const Vector2 & rhs) const
		{
			return operator-(rhs).magnitudeSq();
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
			float length = magnitude();

			_ASSERT(length > EPSILON_E6);

			return operator *(1.0f / length);
		}

		Vector2 & normalizeSelf(void)
		{
			float length = magnitude();

			_ASSERT(length > EPSILON_E6);

			return operator *=(1.0f / length);
		}

		Vector4 transform(const Matrix4 & m) const;

		Vector4 transformTranspose(const Matrix4 & m) const;

		Vector2 transformCoord(const Matrix4 & m) const;

		Vector2 transformCoordTranspose(const Matrix4 & m) const;

		Vector2 transformNormal(const Matrix4 & m) const;

		Vector2 transformNormalTranspose(const Matrix4 & m) const;

	public:
		static Vector2 PolarToCartesian(float length, float theta)
		{
			return Vector2(length * cosf(theta), length * sinf(theta));
		}

		Vector2 cartesianToPolar(void) const
		{
			float length = magnitude();

			return Vector2(length, atan2f(y, x));
		}

		static Vector2 RandomUnit(void);

		static Vector2 RandomUnitCircle(void);

		static const Vector2 zero;

		static const Vector2 one;

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

					Vector2 yz;
				};
			};

			Vector2 xy;
		};

	public:
		Vector3(void)
			//: x(0)
			//, y(0)
			//, z(0)
		{
		}

		explicit Vector3(float _v)
			: x(_v)
			, y(_v)
			, z(_v)
		{
		}

		Vector3(const Vector2 & _v, float _y)
			: x(_v.x)
			, y(_y)
			, z(_v.y)
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
			_ASSERT(i < 3);
			return (&x)[i];
		}

		const float & operator [](size_t i) const
		{
			_ASSERT(i < 3);
			return (&x)[i];
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

		float dot2D(const Vector3 & rhs) const
		{
			return x * rhs.x + z * rhs.z;
		}

		Vector3 perpendicular(const Vector3 & side) const
		{
			return side - operator *(dot(side));
		}

		Vector3 slide(const Vector3 & plane_normal) const
		{
			//_ASSERT(IS_NORMALIZED(plane_normal));

			return operator -(plane_normal * dot(plane_normal));
		}

		float magnitude(void) const
		{
			return sqrtf(magnitudeSq());
		}

		float magnitudeSq(void) const
		{
			return x * x + y * y + z * z;
		}

		float magnitude2D(void) const
		{
			return sqrtf(magnitudeSq2D());
		}

		float magnitudeSq2D(void) const
		{
			return x * x + z * z;
		}
		
		float distance(const Vector3 & rhs) const
		{
			return operator-(rhs).magnitude();
		}

		float distanceSq(const Vector3 & rhs) const
		{
			return operator-(rhs).magnitudeSq();
		}

		float distance2D(const Vector3 & rhs) const
		{
			return operator-(rhs).magnitude2D();
		}

		float distanceSq2D(const Vector3 & rhs) const
		{
			return operator-(rhs).magnitudeSq2D();
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
			float length = magnitude();

			_ASSERT(length > EPSILON_E6);

			return operator *(1.0f / length);
		}

		Vector3 normalize(const Vector3 & safe) const
		{
			float length = magnitude();

			if (length > EPSILON_E6)
			{
				return operator *(1.0f / length);
			}
			return safe;
		}

		Vector3 normalize2D(void) const
		{
			float length = magnitude2D();

			_ASSERT(length > EPSILON_E6);

			return Vector3(x, 0, z) * (1.0f / length);
		}

		Vector3 normalize2D(const Vector3 & safe) const
		{
			float length = magnitude2D();

			if (length > EPSILON_E6)
			{
				return Vector3(x, 0, z) * (1.0f / length);
			}
			return safe;
		}

		Vector3 & normalizeSelf(void)
		{
			float length = magnitude();

			_ASSERT(length > EPSILON_E6);

			return operator *=(1.0f / length);
		}

		Vector3 & normalizeSelf(const Vector3 & safe)
		{
			float length = magnitude();

			if (length > EPSILON_E6)
			{
				return operator *=(1.0f / length);
			}
			return operator =(safe);
		}

		Vector4 transform(const Matrix4 & m) const;

		Vector4 transformTranspose(const Matrix4 & m) const;

		Vector3 transformCoord(const Matrix4 & m) const;

		Vector3 transformCoordSafe(const Matrix4 & m) const;

		Vector3 transformCoordTranspose(const Matrix4 & m) const;

		Vector3 transformNormal(const Matrix4 & m) const;

		Vector3 transformNormalTranspose(const Matrix4 & m) const;

	public:
		static Vector3 PolarToCartesian(float length, float phi, float theta)
		{
			return Vector3(length * cosf(phi) * cosf(theta), length * sinf(phi), length * cosf(phi) * sinf(theta));
		}

		Vector3 cartesianToPolar(void) const
		{
			float length = magnitude();

			_ASSERT(length > EPSILON_E6);

			return Vector3(length, asinf(y / length), atan2f(z, x));
		}

		float cosTheta(const Vector3 & rhs) const
		{
			float determinantSq = magnitudeSq() * rhs.magnitudeSq(); // |v0|*|v1|*Cos(Angle)

			if (determinantSq != 0)
			{
				return Min(dot(rhs) / sqrtf(determinantSq), 1.0f);
			}
			throw std::exception(__FUNCTION__ ": determinantSq == 0");
		}

		//float sinTheta(const Vector3 & rhs) const
		//{
		//	float determinantSq = magnitudeSq() * rhs.magnitudeSq(); // |v0|*|v1|*Sin(Angle)

		//	_ASSERT(determinantSq != 0);

		//	return sqrtf(cross(rhs).magnitudeSq() / determinantSq);
		//}

		float angle(const Vector3 & rhs) const
		{
			return acosf(cosTheta(rhs));
		}

		float signedAngle(const Vector3 & rhs, const Vector3 & axis) const
		{
			_ASSERT(IS_NORMALIZED(axis));

			// ! https://stackoverflow.com/questions/5188561/signed-angle-between-two-3d-vectors-with-same-origin-within-the-same-plane
			return atan2f(cross(rhs).dot(axis), dot(rhs));
		}

		static float Cosine(float a, float b, float c)
		{
			return (b * b + c * c - a * a) / (2 * b * c);
		}

		static Vector3 RandomUnit(void);

		static Vector3 RandomUnitSphere(void);

		Vector2 xz(void) const
		{
			return Vector2(x, z);
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

					Vector2 yz;
				};
			};

			Vector2 xy;

			Vector3 xyz;
		};

	public:
		Vector4(void)
			//: x(0)
			//, y(0)
			//, z(0)
			//, w(0)
		{
		}

		explicit Vector4(float _v)
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
			_ASSERT(i < 4);
			return (&x)[i];
		}

		const float & operator [](size_t i) const
		{
			_ASSERT(i < 4);
			return (&x)[i];
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
			return sqrtf(magnitudeSq());
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
			float length = magnitude();

			_ASSERT(length > EPSILON_E6);

			return operator *(1.0f / length);
		}

		Vector4 & normalizeSelf(void)
		{
			float length = magnitude();

			_ASSERT(length > EPSILON_E6);

			return operator *=(1.0f / length);
		}

		Vector4 transform(const Matrix4 & m) const;

		Vector4 transformTranspose(const Matrix4 & m) const;

	public:
		Vector2 xz(void) const
		{
			return Vector2(x, z);
		}

		static Vector4 FromArgb(DWORD argb)
		{
			return Vector4(
				LOBYTE(argb >> 16) / 255.f,
				LOBYTE(argb >> 8) / 255.f,
				(argb & 0xff) / 255.f,
				LOBYTE(argb >> 24) / 255.f);
		}

		DWORD toArgb(void) const
		{
			return D3DCOLOR_COLORVALUE(x, y, z, w);
		}

		static const Vector4 zero;

		static const Vector4 one;

		static const Vector4 unitX;

		static const Vector4 unitY;

		static const Vector4 unitZ;

		static const Vector4 unitW;
	};

	class Rectangle
	{
	public:
		union
		{
			struct
			{
				float l, t;

				union
				{
					struct
					{
						float r, b;
					};

					Vector2 rb;
				};
			};

			Vector2 lt;
		};

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

		explicit Rectangle(const Vector2 & leftTop)
			: l(leftTop.x)
			, t(leftTop.y)
			, r(leftTop.x)
			, b(leftTop.y)
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

		Rectangle Union(const Vector2 & rhs) const
		{
			return Rectangle(
				Min(l, rhs.x),
				Min(t, rhs.y),
				Max(r, rhs.x),
				Max(b, rhs.y));
		}

		Rectangle & unionSelf(const Vector2 & rhs)
		{
			l = Min(l, rhs.x);
			t = Min(t, rhs.y);
			r = Max(r, rhs.x);
			b = Max(b, rhs.y);
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
			return Vector2(l, (t + b) * 0.5f);
		}

		Vector2 LeftBottom(void) const
		{
			return Vector2(l, b);
		}

		Vector2 CenterTop(void) const
		{
			return Vector2((l + r) * 0.5f, t);
		}

		Vector2 CenterMiddle(void) const
		{
			return Vector2((l + r) * 0.5f, (t + b) * 0.5f);
		}

		Vector2 CenterBottom(void) const
		{
			return Vector2((l + r) * 0.5f, b);
		}

		Vector2 RightTop(void) const
		{
			return Vector2(r, t);
		}

		Vector2 RightMiddle(void) const
		{
			return Vector2(r, (t + b) * 0.5f);
		}

		Vector2 RightBottom(void) const
		{
			return Vector2(r, b);
		}

		Vector2 Center(void) const
		{
			return Vector2((l + r) * 0.5f, (t + b) * 0.5f);
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

		bool PtInRect(const Vector2 & pt) const
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

			Vector3 xyz;
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

		Vector3 operator * (const Vector3& rhs) const
		{
			return Quaternion(conjugate() * Quaternion(rhs.x, rhs.y, rhs.z, 0) * *this).xyz;
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
			return sqrtf(magnitudeSq());
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
			float length = magnitude();

			_ASSERT(length > EPSILON_E6);

			return operator *(1.0f / length);
		}

		Quaternion & normalizeSelf(void)
		{
			float length = magnitude();

			_ASSERT(length > EPSILON_E6);

			return operator *=(1.0f / length);
		}

		static Quaternion RotationAxis(const Vector3 & v, float angle)
		{
			//float c = cosf(angle / 2);
			//float s = sinf(angle / 2);
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

		static Quaternion RotationFromTo(const Vector3 & from, const Vector3 & to, const Vector3 & _fallback_axis);

		static Quaternion RotationFromToSafe(const Vector3 & from, const Vector3 & to);

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

		void toAxisAngle(Vector3 & outAxis, float & outAngle) const
		{
			D3DXQuaternionToAxisAngle((D3DXQUATERNION *)this, (D3DXVECTOR3 *)&outAxis, &outAngle);
		}

		static Quaternion RotationEulerAngles(float x, float y, float z)
		{
			float cX(cosf(x / 2.0f));
			float sX(sinf(x / 2.0f));

			float cY(cosf(y / 2.0f));
			float sY(sinf(y / 2.0f));

			float cZ(cosf(z / 2.0f));
			float sZ(sinf(z / 2.0f));

			Quaternion qX(sX, 0.0f, 0.0f, cX);
			Quaternion qY(0.0f, sY, 0.0f, cY);
			Quaternion qZ(0.0f, 0.0f, sZ, cZ);

			Quaternion q = (qX * qY) * qZ;
			_ASSERT(abs(q.magnitude() - 1.0f) < EPSILON_E6);
			return q;
		}

		Vector3 toEulerAngles(void) const;

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

		explicit Matrix4(float _v)
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
			_ASSERT(i < 4);
			return ((Vector4*)&_11)[i];
		}

		const Vector4 & operator [](size_t i) const
		{
			_ASSERT(i < 4);
			return ((Vector4*)&_11)[i];
		}

		template <int i>
		const Vector4 & getRow(void) const
		{
			return operator [](i);
		}

		template <int i>
		void setRow(Vector4 & value)
		{
			operator [](i) = value;
		}

		template <int i>
		Vector4 getColumn(void) const
		{
			return Vector4(
				operator[](0)[i],
				operator[](1)[i],
				operator[](2)[i],
				operator[](3)[i]);
		}

		template <int i>
		void setColumn(Vector4 & value)
		{
			operator[](0)[i] = value.x;
			operator[](1)[i] = value.y;
			operator[](2)[i] = value.z;
			operator[](3)[i] = value.w;
		}

		bool operator ==(const Matrix4 & rhs) const
		{
			return operator[](0) == rhs[0]
				&& operator[](1) == rhs[1]
				&& operator[](2) == rhs[2]
				&& operator[](3) == rhs[3];
		}

		bool operator !=(const Matrix4 & rhs) const
		{
			return !operator ==(rhs);
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
				0,		0,		1 / (zn - zf),		0,
				0,		0,		-zf / (zn - zf),	1);
		}

		static Matrix4 OrthoRH(float w, float h, float zn, float zf)
		{
			return Matrix4(
				2 / w,	0,		0,					0,
				0,		2 / h,	0,					0,
				0,		0,		1 / (zf - zn),		0,
				0,		0,		zf / (zf - zn),		1);
		}

		static Matrix4 OrthoOffCenterLH(float l, float r, float b, float t, float zn, float zf)
		{
			return Matrix4(
				2 / (r - l),		0,					0,					0,
				0,					2 / (t - b),		0,					0,
				0,					0,					1 / (zn - zf),		0,
				(l + r) / (l - r),	(t + b) / (b - t),	-zf / (zn - zf),		1);
		}

		static Matrix4 OrthoOffCenterRH(float l, float r, float b, float t, float zn, float zf)
		{
			return Matrix4(
				2 / (r - l),		0,					0,					0,
				0,					2 / (t - b),		0,					0,
				0,					0,					1 / (zf - zn),		0,
				(l + r) / (l - r),	(t + b) / (b - t),	zf / (zf - zn),		1);
		}

		static Matrix4 PerspectiveFovLH(float fovy, float aspect, float zn, float zf)
		{
			float yScale = cotf(fovy / 2);
			float xScale = yScale / aspect;

			return Matrix4(
				xScale,	0,		0,						0,
				0,		yScale,	0,						0,
				0,		0,		zn / (zn - zf),			1,
				0,		0,		-zn * zf / (zn - zf),	0);

		}

		static Matrix4 PerspectiveFovRH(float fovy, float aspect, float zn, float zf)
		{
			float yScale = cotf(fovy / 2);
			float xScale = yScale / aspect;

			// ! Reversed-Z, ref: https://developer.nvidia.com/content/depth-precision-visualized
			return Matrix4(
				xScale,	0,		0,						0,
				0,		yScale,	0,						0,
				0,		0,		zn / (zf - zn),			-1,
				0,		0,		zn * zf / (zf - zn),	0);
		}

		static Matrix4 PerspectiveAovLH(float fovx, float aspect, float zn, float zf)
		{
			float xScale = cotf(fovx / 2);
			float yScale = xScale * aspect;

			return Matrix4(
				xScale,	0,		0,						0,
				0,		yScale,	0,						0,
				0,		0,		zn / (zn - zf),			1,
				0,		0,		-zn * zf / (zn - zf),	0);
		}

		static Matrix4 PerspectiveAovRH(float fovx, float aspect, float zn, float zf)
		{
			float xScale = cotf(fovx / 2);
			float yScale = xScale * aspect;

			return Matrix4(
				xScale,	0,		0,						0,
				0,		yScale,	0,						0,
				0,		0,		zn / (zf - zn),			-1,
				0,		0,		zn * zf / (zf - zn),	0);
		}

		static Matrix4 PerspectiveLH(float w, float h, float zn, float zf)
		{
			return Matrix4(
				2 * zn / w,	0,			0,						0,
				0,			2 * zn / h,	0,						0,
				0,			0,			zn / (zn - zf),			1,
				0,			0,			-zn * zf / (zn - zf),	0);
		}

		static Matrix4 PerspectiveRH(float w, float h, float zn, float zf)
		{
			return Matrix4(
				2 * zn / w,	0,			0,						0,
				0,			2 * zn / h,	0,						0,
				0,			0,			zn / (zf - zn),			-1,
				0,			0,			zn * zf / (zf - zn),	0);
		}

		static Matrix4 PerspectiveOffCenterLH(float l, float r, float b, float t, float zn, float zf)
		{
			return Matrix4(
				2 * zn / (r - l),	0,					0,						0,
				0,					2 * zn / (t - b),	0,						0,
				(l + r) / (l - r),	(t + b) / (b - t),	zn / (zn - zf),			1,
				0,					0,					-zn * zf / (zn - zf),	0);
		}

		static Matrix4 PerspectiveOffCenterRH(float l, float r, float b, float t, float zn, float zf)
		{
			return Matrix4(
				2 * zn / (r - l),	0,					0,						0,
				0,					2 * zn / (t - b),	0,						0,
				(l + r) / (r - l),	(t + b) / (t - b),	zn / (zf - zn),			-1,
				0,					0,					zn * zf / (zf - zn),	0);
		}

		static Matrix4 RotationAxis(const Vector3 & v, float angle)
		{
			Matrix4 ret;
			D3DXMatrixRotationAxis((D3DXMATRIX *)&ret, (D3DXVECTOR3 *)&v, angle);
			return ret;
		}

		static Matrix4 RotationQuaternion(const Quaternion & q)
		{
			// https://www.cs.cmu.edu/~kiranb/animation/p245-shoemake.pdf
			return Matrix4(
				1 - 2.0f * q.y * q.y - 2.0f * q.z * q.z, 2.0f * q.x * q.y + 2.0f * q.w * q.z, 2.0f * q.x * q.z - 2.0f * q.w * q.y, 0,
				2.0f * q.x * q.y - 2.0f * q.w * q.z, 1 - 2.0f * q.x * q.x - 2.0f * q.z * q.z, 2.0f * q.y * q.z + 2.0f * q.w * q.x, 0,
				2.0f * q.x * q.z + 2.0f * q.w * q.y, 2.0f * q.y * q.z - 2.0f * q.w * q.x, 1 - 2.0f * q.x * q.x - 2.0f * q.y * q.y, 0,
				0, 0, 0, 1);

			//Matrix4 ret;
			//D3DXMatrixRotationQuaternion((D3DXMATRIX *)&ret, (D3DXQUATERNION *)&q);
			//return ret;
		}

		static Matrix4 RotationX(float angle)
		{
			float c = cosf(angle);
			float s = sinf(angle);

			return Matrix4(1, 0, 0, 0, 0, c, s, 0, 0, -s, c, 0, 0, 0, 0, 1);
		}

		static Matrix4 RotationY(float angle)
		{
			float c = cosf(angle);
			float s = sinf(angle);

			return Matrix4(c, 0, -s, 0, 0, 1, 0, 0, s, 0, c, 0, 0, 0, 0, 1);
		}

		static Matrix4 RotationZ(float angle)
		{
			float c = cosf(angle);
			float s = sinf(angle);

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

		static Matrix4 AffineTransformation(float Scaling, const Vector3 & rotationCenter, const Quaternion & rotation, const Vector3 & translation)
		{
			Matrix4 ret;
			D3DXMatrixAffineTransformation(
				(D3DXMATRIX *)&ret,
				Scaling,
				(D3DXVECTOR3 *)&rotationCenter,
				(D3DXQUATERNION *)&rotation,
				(D3DXVECTOR3 *)&translation);
			return ret;
		}

		static Matrix4 AffineTransformation2D(float Scaling, const Vector2 & rotationCenter, float rotation, const Vector2 & translation)
		{
			Matrix4 ret;
			D3DXMatrixAffineTransformation2D(
				(D3DXMATRIX *)&ret,
				Scaling,
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
			float c = cosf(angle);
			float s = sinf(angle);

			return Matrix4(
				_11, _12 * c - _13 * s, _12 * s + _13 * c, _14,
				_21, _22 * c - _23 * s, _22 * s + _23 * c, _24,
				_31, _32 * c - _33 * s, _32 * s + _33 * c, _34,
				_41, _42 * c - _43 * s, _42 * s + _43 * c, _44);
		}

		Matrix4 rotateY(float angle) const
		{
			float c = cosf(angle);
			float s = sinf(angle);

			return Matrix4(
				_11 * c + _13 * s, _12, _13 * c - _11 * s, _14,
				_21 * c + _23 * s, _22, _23 * c - _21 * s, _24,
				_31 * c + _33 * s, _32, _33 * c - _31 * s, _34,
				_41 * c + _43 * s, _42, _43 * c - _41 * s, _44);
		}

		Matrix4 rotateZ(float angle) const
		{
			float c = cosf(angle);
			float s = sinf(angle);

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

		Vector3 toEulerAngles(void) const;

		Quaternion toRotation(void) const;

	public:
		static const Matrix4 identity;
	};

	class Bone
	{
	public:
		Quaternion m_rotation;

		Vector3 m_position;

	public:
		Bone(void)
		{
		}

		Bone(const Vector3 & position, const Quaternion & rotation)
			: m_rotation(rotation)
			, m_position(position)
		{
		}

		explicit Bone(const Vector3 & position)
			: m_rotation(Quaternion::Identity())
			, m_position(position)
		{
		}

		template <class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(m_rotation);
			ar & BOOST_SERIALIZATION_NVP(m_position);
		}

		Bone Increment(const Bone & rhs) const
		{
			return Bone(
				m_position + rhs.m_position,
				m_rotation * rhs.m_rotation);
		}

		Bone & IncrementSelf(const Bone & rhs)
		{
			m_position += rhs.m_position;
			m_rotation *= rhs.m_rotation;
			return *this;
		}

		Bone Lerp(const Bone & rhs, float t) const
		{
			return Bone(
				m_position.lerp(rhs.m_position, t),
				m_rotation.slerp(rhs.m_rotation, t));
		}

		Bone & LerpSelf(const Bone & rhs, float t)
		{
			m_position.lerpSelf(rhs.m_position, t);
			m_rotation.slerpSelf(rhs.m_rotation, t);
			return *this;
		}

		Bone Transform(const my::Vector3 & pos, const my::Quaternion & rot) const
		{
			return Bone(rot * m_position + pos, m_rotation * rot);
		}

		Bone Transform(const Bone & parent) const
		{
			return Transform(parent.m_position, parent.m_rotation);
		}

		Bone & TransformSelf(const my::Vector3 & pos, const my::Quaternion & rot)
		{
			m_position = rot * m_position + pos;
			m_rotation = m_rotation * rot;
			return *this;
		}

		Bone & TransformSelf(const Bone & parent)
		{
			return TransformSelf(parent.m_position, parent.m_rotation);
		}

		Bone TransformTranspose(const my::Vector3 & pos, const my::Quaternion & rot) const
		{
			const Quaternion rot_con = rot.conjugate();
			return Bone(rot_con * (m_position - pos), m_rotation * rot_con);
		}

		Bone TransformTranspose(const Bone & parent) const
		{
			return TransformTranspose(parent.m_position, parent.m_rotation);
		}

		Matrix4 BuildTransform(void) const
		{
			return Matrix4::RotationQuaternion(m_rotation) * Matrix4::Translation(m_position);
		}

		Matrix4 BuildInverseTransform(void) const
		{
			return Matrix4::Translation(-m_position) * Matrix4::RotationQuaternion(m_rotation.conjugate());
		}
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

			Vector3 normal;
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
			return sqrtf(magnitudeSq());
		}

		float magnitudeSq(void) const
		{
			return a * a + b * b + c * c;
		}

		Plane normalize(void) const
		{
			float length = magnitude();

			_ASSERT(length > EPSILON_E6);

			float invLength = 1 / length;

			return Plane(a * invLength, b * invLength, c * invLength, d * invLength);
		}

		Plane & normalizeSelf(void)
		{
			float length = magnitude();

			_ASSERT(length > EPSILON_E6);

			float invLength = 1 / length;

			a *= invLength;
			b *= invLength;
			c *= invLength;
			d *= invLength;
			return *this;
		}

		float DistanceToPoint(const Vector3 & pt) const
		{
			return normal.dot(pt) + d;
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
		Ray(void)
			//: p(Vector3(0,0,0))
			//, d(Vector3(1,0,0))
		{
		}

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
			_ASSERT(i < 6);
			return (&Up)[i];
		}

		const Plane & operator [](size_t i) const
		{
			_ASSERT(i < 6);
			return (&Up)[i];
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
			return m_min.x <= m_max.x
				&& m_min.y <= m_max.y
				&& m_min.z <= m_max.z;
		}

		AABB valid(void) const
		{
			AABB ret;
			if (m_min.x > m_max.x)
			{
				ret.m_min.x = ret.m_max.x = (m_min.x + m_max.x) * 0.5f;
			}
			else
			{
				ret.m_min.x = m_min.x;
				ret.m_max.x = m_max.x;
			}
			if (m_min.y > m_max.y)
			{
				ret.m_min.y = ret.m_max.y = (m_min.y + m_max.y) * 0.5f;
			}
			else
			{
				ret.m_min.y = m_min.y;
				ret.m_max.y = m_max.y;
			}
			if (m_min.z > m_max.z)
			{
				ret.m_min.z = ret.m_max.z = (m_min.z + m_max.z) * 0.5f;
			}
			else
			{
				ret.m_min.z = m_min.z;
				ret.m_max.z = m_max.z;
			}
			return ret;
		}

		AABB & validSelf(void)
		{
			if (m_min.x > m_max.x)
			{
				m_min.x = m_max.x = (m_min.x + m_max.x) * 0.5f;
			}
			if (m_min.y > m_max.y)
			{
				m_min.y = m_max.y = (m_min.y + m_max.y) * 0.5f;
			}
			if (m_min.z > m_max.z)
			{
				m_min.z = m_max.z = (m_min.z + m_max.z) * 0.5f;
			}
			return *this;
		}

		template <class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(m_min);
			ar & BOOST_SERIALIZATION_NVP(m_max);
		}

	public:
		bool operator ==(const AABB & rhs) const
		{
			return m_min == rhs.m_min
				&& m_max == rhs.m_max;
		}

		bool operator !=(const AABB & rhs) const
		{
			return !operator ==(rhs);
		}

		AABB operator + (const Vector3 & rhs) const
		{
			return AABB(m_min + rhs, m_max + rhs);
		}

		AABB operator + (float scaler) const
		{
			return AABB(m_min + scaler, m_max + scaler);
		}

		AABB operator - (const Vector3 & rhs) const
		{
			return AABB(m_min - rhs, m_max - rhs);
		}

		AABB operator - (float scaler) const
		{
			return AABB(m_min - scaler, m_max - scaler);
		}

		AABB operator * (const Vector3& rhs) const
		{
			return AABB(m_min * rhs, m_max * rhs);
		}

		AABB operator * (float scaler) const
		{
			return AABB(m_min * scaler, m_max * scaler);
		}

		AABB operator / (const Vector3 & rhs) const
		{
			return AABB(m_min / rhs, m_max / rhs);
		}

		AABB operator / (float scaler) const
		{
			float invScaler = 1 / scaler;

			return AABB(m_min * invScaler, m_max * invScaler);
		}

		AABB & operator += (const Vector3 & rhs)
		{
			m_min += rhs;
			m_max += rhs;
			return *this;
		}

		AABB & operator += (float scaler)
		{
			m_min += scaler;
			m_max += scaler;
			return *this;
		}

		AABB& operator -= (const Vector3& rhs)
		{
			m_min -= rhs;
			m_max -= rhs;
			return *this;
		}

		AABB& operator -= (float scaler)
		{
			m_min -= scaler;
			m_max -= scaler;
			return *this;
		}

		AABB& operator *= (const Vector3& rhs)
		{
			m_min *= rhs;
			m_max *= rhs;
			return *this;
		}

		AABB& operator *= (float scaler)
		{
			m_min *= scaler;
			m_max *= scaler;
			return *this;
		}

		AABB & operator /= (const Vector3 & rhs)
		{
			m_min /= rhs;
			m_max /= rhs;
			return *this;
		}

		AABB & operator /= (float scaler)
		{
			float invScaler = 1 / scaler;
			m_min *= invScaler;
			m_max *= invScaler;
			return *this;
		}

	public:
		Vector3 Center(void) const
		{
			return Vector3(
				(m_min.x + m_max.x) * 0.5f,
				(m_min.y + m_max.y) * 0.5f,
				(m_min.z + m_max.z) * 0.5f);
		}

		Vector3 Extent(void) const
		{
			return m_max - m_min;
		}

		bool PtInAABB(const Vector3 & pt) const
		{
			return pt.x >= m_min.x && pt.x <= m_max.x
				&& pt.y >= m_min.y && pt.y <= m_max.y
				&& pt.z >= m_min.z && pt.z <= m_max.z;
		}

		bool Contains(const AABB & rhs) const
		{
			return PtInAABB(rhs.m_min) && PtInAABB(rhs.m_max);
		}

		template <UINT Quad>
		AABB Slice(const Vector3 & cente) const;

		bool Intersect(const Vector3 & pos) const
		{
			if (Intersect2D(pos)
				&& pos.y >= m_min.y && pos.y < m_max.y)
			{
				return true;
			}
			return false;
		}

		bool Intersect2D(const Vector3 & pos) const
		{
			if (pos.x >= m_min.x && pos.x < m_max.x
				&& pos.z >= m_min.z && pos.z < m_max.z)
			{
				return true;
			}
			return false;
		}

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

		AABB shrink(float x, float y, float z) const
		{
			return AABB(
				m_min.x + x,
				m_min.y + y,
				m_min.z + z,
				m_max.x - x,
				m_max.y - y,
				m_max.z - z).valid();
		}

		AABB & shrinkSelf(float x, float y, float z)
		{
			m_min.x += x;
			m_min.y += y;
			m_min.z += z;
			m_max.x -= x;
			m_max.y -= y;
			m_max.z -= z;
			return validSelf();
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

	class UDim // ref: CEGUI::UDim
	{
	public:
		float scale, offset;

	public:
		inline UDim()
		{}

		inline UDim(float _scale, float _offset) :
			scale(_scale),
			offset(_offset)
		{}

		inline UDim(const UDim& v) :
			scale(v.scale),
			offset(v.offset)
		{}

		template <class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(scale);
			ar & BOOST_SERIALIZATION_NVP(offset);
		}

		inline UDim operator+(const UDim& other) const
		{
			return UDim(scale + other.scale, offset + other.offset);
		}

		inline UDim operator-(const UDim& other) const
		{
			return UDim(scale - other.scale, offset - other.offset);
		}

		inline UDim operator*(const float val) const
		{
			return UDim(scale * val, offset * val);
		}

		inline friend UDim operator*(const float val, const UDim& u)
		{
			return UDim(val * u.scale, val * u.offset);
		}

		inline UDim operator*(const UDim& other) const
		{
			return UDim(scale * other.scale, offset * other.offset);
		}

		inline UDim operator/(const UDim& other) const
		{
			// division by zero sets component to zero.  Not technically correct
			// but probably better than exceptions and/or NaN values.
			return UDim(other.scale == 0.0f ? 0.0f : scale / other.scale,
				other.offset == 0.0f ? 0.0f : offset / other.offset);
		}

		inline const UDim& operator+=(const UDim& other)
		{
			scale += other.scale;
			offset += other.offset;
			return *this;
		}

		inline const UDim& operator-=(const UDim& other)
		{
			scale -= other.scale;
			offset -= other.offset;
			return *this;
		}

		inline const UDim& operator*=(const UDim& other)
		{
			scale *= other.scale;
			offset *= other.offset;
			return *this;
		}

		inline const UDim& operator/=(const UDim& other)
		{
			// division by zero sets component to zero.  Not technically correct
			// but probably better than exceptions and/or NaN values.
			scale = (other.scale == 0.0f ? 0.0f : scale / other.scale);
			offset = (other.offset == 0.0f ? 0.0f : offset / other.offset);
			return *this;
		}

		inline bool operator==(const UDim& other) const
		{
			return scale == other.scale && offset == other.offset;
		}

		inline bool operator!=(const UDim& other) const
		{
			return !operator==(other);
		}

		/*!
		\brief finger saving convenience method returning UDim(0, 0)
		*/
		inline static UDim zero()
		{
			return UDim(0.0f, 0.0f);
		}

		/*!
		\brief finger saving convenience method returning UDim(1, 0)

		\note
		Allows quite neat 0.5 * UDim::relative() self documenting syntax
		*/
		inline static UDim relative()
		{
			return UDim(1.0f, 0.0f);
		}

		/*!
		\brief finger saving convenience method returning UDim(0.01, 0)

		\note
		Allows quite neat 50 * UDim::percent() self documenting syntax
		*/
		inline static UDim percent()
		{
			return UDim(0.01f, 0.0f);
		}

		/*!
		\brief finger saving convenience method returning UDim(0, 1)

		\note
		Allows quite neat 100 * UDim::px() self documenting syntax,
		you can combine it with UDim::relative() as well (using operator+)
		*/
		inline static UDim px()
		{
			return UDim(0.0f, 1.0f);
		}
	};

	template <>
	inline int my::Wrap<int>(int v, int min, int max)
	{
		return v >= max ? min + (v - max) % (max - min) : (v < min ? max - (min - v) % (max - min) : v);
	}

	template <>
	inline float my::Wrap<float>(float v, float min, float max)
	{
		return v >= max ? min + fmodf(v - max, max - min) : (v < min ? max - fmodf(min - v, max - min) : v);
	}

	template <>
	inline int my::Random<int>(int range)
	{
		return rand() % range;
	}

	template <>
	inline float my::Random<float>(float range)
	{
		return range * ((float)rand() / RAND_MAX);
	}

	template <>
	inline int my::Random<int>(int min, int max)
	{
		return min + rand() % (max - min);
	}

	template <>
	inline float my::Random<float>(float min, float max)
	{
		return min + (max - min) * ((float)rand() / RAND_MAX);
	}

	inline Vector4 Vector2::transform(const Matrix4 & m) const
	{
		return Vector4(x, y, 0, 1).transform(m);
	}

	inline Vector4 Vector2::transformTranspose(const Matrix4 & m) const
	{
		return Vector4(x, y, 0, 1).transformTranspose(m);
	}

	inline Vector2 Vector2::transformCoord(const Matrix4 & m) const
	{
		Vector4 ret = transform(m);

		_ASSERT(abs(ret.w) > EPSILON_E6);

		return ret.xy / ret.w;
	}

	inline Vector2 Vector2::transformCoordTranspose(const Matrix4 & m) const
	{
		Vector4 ret = transformTranspose(m);

		_ASSERT(abs(ret.w) > EPSILON_E6);

		return ret.xy / ret.w;
	}

	inline Vector2 Vector2::transformNormal(const Matrix4 & m) const
	{
		return Vector4(x, y, 0, 0).transform(m).xy;
	}

	inline Vector2 Vector2::transformNormalTranspose(const Matrix4 & m) const
	{
		return Vector4(x, y, 0, 0).transformTranspose(m).xy;
	}

	inline Vector4 Vector3::transform(const Matrix4 & m) const
	{
		return Vector4(x, y, z, 1).transform(m);
	}

	inline Vector4 Vector3::transformTranspose(const Matrix4 & m) const
	{
		return Vector4(x, y, z, 1).transformTranspose(m);
	}

	inline Vector3 Vector3::transformCoord(const Matrix4 & m) const
	{
		Vector4 ret = transform(m);

		_ASSERT(abs(ret.w) > EPSILON_E6);

		return ret.xyz / ret.w;
	}

	inline Vector3 Vector3::transformCoordSafe(const Matrix4 & m) const
	{
		Vector4 ret = transform(m);

		if (abs(ret.w) > EPSILON_E6)
		{
			return ret.xyz / ret.w;
		}
		return Vector3(0, 0, 0);
	}

	inline Vector3 Vector3::transformCoordTranspose(const Matrix4 & m) const
	{
		Vector4 ret = transformTranspose(m);

		_ASSERT(abs(ret.w) > EPSILON_E6);

		return ret.xyz / ret.w;
	}

	inline Vector3 Vector3::transformNormal(const Matrix4 & m) const
	{
		return Vector4(x, y, z, 0).transform(m).xyz;
	}

	inline Vector3 Vector3::transformNormalTranspose(const Matrix4 & m) const
	{
		return Vector4(x, y, z, 0).transformTranspose(m).xyz;
	}

	inline Vector4 Vector4::transform(const Matrix4 & m) const
	{
		return Vector4(
			x * m._11 + y * m._21 + z * m._31 + w * m._41,
			x * m._12 + y * m._22 + z * m._32 + w * m._42,
			x * m._13 + y * m._23 + z * m._33 + w * m._43,
			x * m._14 + y * m._24 + z * m._34 + w * m._44);
	}

	inline Vector4 Vector4::transformTranspose(const Matrix4 & m) const
	{
		return Vector4(
			x * m._11 + y * m._12 + z * m._13 + w * m._14,
			x * m._21 + y * m._22 + z * m._23 + w * m._24,
			x * m._31 + y * m._32 + z * m._33 + w * m._34,
			x * m._41 + y * m._42 + z * m._43 + w * m._44);
	}

	template <>
	inline AABB AABB::Slice<AABB::QuadrantPxPyPz>(const Vector3 & cente) const
	{
		return AABB(cente.x, cente.y, cente.z, m_max.x, m_max.y, m_max.z);
	}

	template <>
	inline AABB AABB::Slice<AABB::QuadrantPxPyNz>(const Vector3 & cente) const
	{
		return AABB(cente.x, cente.y, m_min.z, m_max.x, m_max.y, cente.z);
	}

	template <>
	inline AABB AABB::Slice<AABB::QuadrantPxNyPz>(const Vector3 & cente) const
	{
		return AABB(cente.x, m_min.y, cente.z, m_max.x, cente.y, m_max.z);
	}

	template <>
	inline AABB AABB::Slice<AABB::QuadrantPxNyNz>(const Vector3 & cente) const
	{
		return AABB(cente.x, m_min.y, m_min.z, m_max.x, cente.y, cente.z);
	}

	template <>
	inline AABB AABB::Slice<AABB::QuadrantNxPyPz>(const Vector3 & cente) const
	{
		return AABB(m_min.x, cente.y, cente.z, cente.x, m_max.y, m_max.z);
	}

	template <>
	inline AABB AABB::Slice<AABB::QuadrantNxPyNz>(const Vector3 & cente) const
	{
		return AABB(m_min.x, cente.y, m_min.z, cente.x, m_max.y, cente.z);
	}

	template <>
	inline AABB AABB::Slice<AABB::QuadrantNxNyPz>(const Vector3 & cente) const
	{
		return AABB(m_min.x, m_min.y, cente.z, cente.x, cente.y, m_max.z);
	}

	template <>
	inline AABB AABB::Slice<AABB::QuadrantNxNyNz>(const Vector3 & cente) const
	{
		return AABB(m_min.x, m_min.y, m_min.z, cente.x, cente.y, cente.z);
	}
}
