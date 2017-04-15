#pragma once

#include "mySingleton.h"
#include "myMath.h"
#include "mySpline.h"
#include <set>
#include <boost/circular_buffer.hpp>
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

		typedef boost::circular_buffer<Particle> ParticleList;

		ParticleList m_ParticleList;

		float m_Time;

	public:
		Emitter(void)
			: m_ParticleList(PARTICLE_INSTANCE_MAX)
			, m_Time(0)
		{
		}

		virtual ~Emitter(void)
		{
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
		}

		void Spawn(const Vector3 & Position, const Vector3 & Velocity, const Vector4 & Color, const Vector2 & Size, float Angle);

		void RemoveDeadParticle(float fParticleLifeTime);

		void Update(float fElapsedTime);
	};

	typedef boost::shared_ptr<Emitter> EmitterPtr;
}

namespace boost { 
	namespace serialization {
		template<class Archive>
		inline void serialize(Archive & ar, my::Emitter::ParticleList & t, const unsigned int version)
		{
			boost::serialization::split_free(ar, t, version);
		}

		template<class Archive>
		void save(Archive & ar, const my::Emitter::ParticleList &t, const unsigned int version);

		template<class Archive>
		void load(Archive & ar, my::Emitter::ParticleList &t, const unsigned int version);
	}
}
