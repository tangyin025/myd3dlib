#pragma once

#include "mySingleton.h"
#include "myMath.h"
#include "mySpline.h"
#include <deque>
#include <set>
#include <boost/serialization/nvp.hpp>

#define PARTICLE_INSTANCE_MAX 4096u

namespace my
{
	class Emitter
	{
	public:
		class Particle
		{
		public:
			Vector3 m_Position;

			Vector3 m_Velocity;

			Vector4 m_Color;

			Vector2 m_Size;

			float m_Angle;

			float m_Time;

		public:
			Particle(const Vector3 & Position, const Vector3 & Velocity, const Vector4 & Color, const Vector2 & Size, float Angle, float Time)
				: m_Position(Position)
				, m_Velocity(Velocity)
				, m_Color(Color)
				, m_Size(Size)
				, m_Angle(Angle)
				, m_Time(Time)
			{
			}

			Particle(void)
				: m_Position(0,0,0)
				, m_Velocity(0,0,0)
				, m_Color(1,1,1,1)
				, m_Size(1,1)
				, m_Angle(0)
				, m_Time(0)
			{
			}

			template <class Archive>
			void serialize(Archive & ar, const unsigned int version)
			{
				ar & BOOST_SERIALIZATION_NVP(m_Position);
				ar & BOOST_SERIALIZATION_NVP(m_Velocity);
				ar & BOOST_SERIALIZATION_NVP(m_Color);
				ar & BOOST_SERIALIZATION_NVP(m_Size);
				ar & BOOST_SERIALIZATION_NVP(m_Angle);
				ar & BOOST_SERIALIZATION_NVP(m_Time);
			}
		};

		typedef std::deque<Particle> ParticleList;

		ParticleList m_ParticleList;

		float m_Time;

	public:
		Emitter(void)
			: m_Time(0)
		{
		}

		virtual ~Emitter(void)
		{
		}

		template <class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(m_ParticleList);
		}

		void Spawn(const Vector3 & Position, const Vector3 & Velocity, const Vector4 & Color, const Vector2 & Size, float Angle);

		virtual void Reset(void);

		virtual void Update(float fElapsedTime);
	};

	typedef boost::shared_ptr<Emitter> EmitterPtr;

	class DynamicEmitter
		: public Emitter
	{
	public:
		float m_ParticleLifeTime;

	public:
		DynamicEmitter(void)
			: m_ParticleLifeTime(FLT_MAX)
		{
		}

		template <class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			boost::serialization::void_cast_register<DynamicEmitter, Emitter>();
			ar & BOOST_SERIALIZATION_NVP(m_ParticleLifeTime);
		}

		virtual void Reset(void);

		virtual void Update(float fElapsedTime);
	};

	typedef boost::shared_ptr<DynamicEmitter> DynamicEmitterPtr;

	class SphericalEmitter
		: public DynamicEmitter
	{
	public:
		float m_RemainingSpawnTime;

		float m_SpawnInterval;

		Vector3 m_HalfSpawnArea;

		float m_SpawnSpeed;

		Spline m_SpawnInclination;

		Spline m_SpawnAzimuth;

		Spline m_SpawnColorA;

		Spline m_SpawnColorR;

		Spline m_SpawnColorG;

		Spline m_SpawnColorB;

		Spline m_SpawnSizeX;

		Spline m_SpawnSizeY;

		Spline m_SpawnAngle;

		float m_SpawnLoopTime;

	public:
		SphericalEmitter(void)
			: m_RemainingSpawnTime(0)
			, m_SpawnInterval(FLT_MAX)
			, m_HalfSpawnArea(0,0,0)
			, m_SpawnSpeed(0)
			, m_SpawnLoopTime(5)
		{
		}

		template <class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(DynamicEmitter);
			ar & BOOST_SERIALIZATION_NVP(m_SpawnInterval);
			ar & BOOST_SERIALIZATION_NVP(m_HalfSpawnArea);
			ar & BOOST_SERIALIZATION_NVP(m_SpawnSpeed);
			ar & BOOST_SERIALIZATION_NVP(m_SpawnInclination);
			ar & BOOST_SERIALIZATION_NVP(m_SpawnAzimuth);
			ar & BOOST_SERIALIZATION_NVP(m_SpawnColorA);
			ar & BOOST_SERIALIZATION_NVP(m_SpawnColorR);
			ar & BOOST_SERIALIZATION_NVP(m_SpawnColorG);
			ar & BOOST_SERIALIZATION_NVP(m_SpawnColorB);
			ar & BOOST_SERIALIZATION_NVP(m_SpawnSizeX);
			ar & BOOST_SERIALIZATION_NVP(m_SpawnSizeY);
			ar & BOOST_SERIALIZATION_NVP(m_SpawnAngle);
			ar & BOOST_SERIALIZATION_NVP(m_SpawnLoopTime);
		}

		virtual void Reset(void);

		virtual void Update(float fElapsedTime);
	};

	typedef boost::shared_ptr<SphericalEmitter> SphericalEmitterPtr;
}
