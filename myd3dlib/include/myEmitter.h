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
		: public DeviceRelatedObjectBase
	{
	public:
		class Particle
		{
		public:
			Vector3 m_Position;

			Vector3 m_Velocity;

			D3DCOLOR m_Color;

			Vector4 m_Texcoord1;

		public:
			Particle(const Vector3 & Position, const Vector3 & Velocity, D3DCOLOR Color, const Vector2 & Size, float Angle)
				: m_Position(Position)
				, m_Velocity(Velocity)
				, m_Color(Color)
				, m_Texcoord1(Size.x,Size.y,Angle,1)
			{
			}
		};

		typedef std::deque<std::pair<float, Particle> > ParticlePairList;

		float m_ParticleLifeTime;

		std::string m_MaterialName;

		ParticlePairList m_ParticleList;

	public:
		Emitter(void)
			: m_ParticleLifeTime(FLT_MAX)
		{
		}

		virtual ~Emitter(void);

		template <class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(m_ParticleLifeTime);
			ar & BOOST_SERIALIZATION_NVP(m_MaterialName);
		}

		void OnResetDevice(void);

		void OnLostDevice(void);

		void OnDestroyDevice(void);

		void Reset(void);

		void Spawn(const Vector3 & Position, const Vector3 & Velocity, D3DCOLOR Color, const Vector2 & Size, float Angle);

		virtual void Update(float fElapsedTime);
	};

	typedef boost::shared_ptr<Emitter> EmitterPtr;

	class SphericalEmitter
		: public Emitter
	{
	public:
		float m_Time;

		float m_SpawnInterval;

		float m_RemainingSpawnTime;

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
			: m_Time(0)
			, m_SpawnInterval(FLT_MAX)
			, m_RemainingSpawnTime(0)
			, m_HalfSpawnArea(0,0,0)
			, m_SpawnSpeed(0)
			, m_SpawnLoopTime(5)
		{
		}

		template <class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Emitter);
			ar & BOOST_SERIALIZATION_NVP(m_Time);
			ar & BOOST_SERIALIZATION_NVP(m_SpawnInterval);
			ar & BOOST_SERIALIZATION_NVP(m_RemainingSpawnTime);
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

		virtual void Update(float fElapsedTime);
	};

	typedef boost::shared_ptr<SphericalEmitter> SphericalEmitterPtr;
}
