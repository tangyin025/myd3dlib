#pragma once

#include "mySingleton.h"
#include "myMath.h"
#include "mySpline.h"
#include <set>
#include <boost/circular_buffer.hpp>
#include <boost/serialization/nvp.hpp>

namespace my
{
	class Emitter
	{
	public:
		struct Particle
		{
		public:
			Vector4 m_Position;

			Vector4 m_Velocity;

			Vector4 m_Color;

			Vector2 m_Size;

			float m_Angle;

			float m_Time;

		public:
			Particle(void)
				//: m_Position(0, 0, 0, 1)
				//, m_Velocity(0, 0, 0, 1)
				//, m_Color(1, 1, 1, 1)
				//, m_Size(1, 1)
				//, m_Angle(0)
				//, m_Time(0)
			{
			}

			Particle(const Vector4 & Position, const Vector4 & Velocity, const Vector4 & Color, const Vector2 & Size, float Angle, float Time)
				: m_Position(Position)
				, m_Velocity(Velocity)
				, m_Color(Color)
				, m_Size(Size)
				, m_Angle(Angle)
				, m_Time(Time)
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

	public:
		Emitter(void)
		{
		}

		Emitter(unsigned int Capacity)
			: m_ParticleList(Capacity)
		{
		}

		size_t GetCapacity(void) const
		{
			return m_ParticleList.capacity();
		}

		void SetCapacity(size_t new_capacity)
		{
			m_ParticleList.set_capacity(new_capacity);
		}

		void Spawn(const my::Vector4 & Position, const my::Vector4 & Velocity, const my::Vector4 & Color, const my::Vector2 & Size, float Angle, float Time);

		void RemoveAllParticle(void);
	};
}
