#pragma once

#include "myMath.h"
#include <boost/shared_ptr.hpp>

namespace my
{
	class SteeringOutput
	{
	public:
		Vector3 linear;

		float angular;

	public:
		SteeringOutput(void)
			: linear(Vector3::zero)
			, angular(0)
		{
		}

		SteeringOutput(const Vector3 & _linear, float _angular = 0)
			: linear(_linear)
			, angular(_angular)
		{
		}

		void clear(void)
		{
			linear = Vector3::zero;

			angular = 0;
		}

		bool operator == (const SteeringOutput & other) const
		{
			return linear.x == other.linear.x
				&& linear.y == other.linear.y
				&& linear.z == other.linear.z
				&& angular == other.angular;
		}

		bool operator != (const SteeringOutput & other) const
		{
			return linear.x != other.linear.x
				|| linear.y != other.linear.y
				|| linear.z != other.linear.z
				|| angular != other.angular;
		}

		float magnitudeSq(void)
		{
			return linear.magnitudeSq() + angular * angular;
		}

		float magnitude(void)
		{
			return sqrt(magnitudeSq());
		}
	};

	class Location
	{
	public:
		Vector3 position;

		float orientation;

	public:
		Location(void)
			: position(Vector3::zero)
			, orientation(0)
		{
		}

		Location(const Vector3 & _position, float _orientation = 0)
			: position(_position)
			, orientation(_orientation)
		{
		}

		void clear(void)
		{
			position = Vector3::zero;

			orientation = 0;
		}

		bool operator == (const Location & other) const
		{
			return position.x == other.position.x
				&& position.y == other.position.y
				&& position.z == other.position.z
				&& orientation == other.orientation;
		}

		bool operator != (const Location & other) const
		{
			return position.x != other.position.x
				|| position.y != other.position.y
				|| position.z != other.position.z
				|| orientation != other.orientation;
		}

		void integrate(const SteeringOutput & steer, float duration);

		void setOrientationFromVelocityLH(const Vector3 & velocity);

		Vector3 getOrientationAsVector(void) const;
	};

	class Kinematic : public Location
	{
	public:
		Vector3 velocity;

		float rotation;

	public:
		Kinematic(void)
			: Location()
			, velocity(Vector3::zero)
			, rotation(0)
		{
		}

		Kinematic(const Location & _location, const Vector3 & _velocity = Vector3::zero, float _rotation = 0)
			: Location(_location)
			, velocity(_velocity)
			, rotation(_rotation)
		{
		}

		Kinematic(const Vector3 & _position, float _orientation, const Vector3 & _velocity, float _rotation)
			: Location(_position, _orientation)
			, velocity(_velocity)
			, rotation(_rotation)
		{
		}

		void clear(void)
		{
			Location::clear();

			velocity = Vector3::zero;

			rotation = 0;
		}

		bool operator == (const Kinematic & other) const
		{
			return Location::operator ==(other)
				&& velocity.x == other.velocity.x
				&& velocity.y == other.velocity.y
				&& velocity.z == other.velocity.z
				&& rotation == other.rotation;
		}

		bool operator != (const Kinematic & other) const
		{
			return Location::operator !=(other)
				|| velocity.x != other.velocity.x
				|| velocity.y != other.velocity.y
				|| velocity.z != other.velocity.z
				|| rotation != other.rotation;
		}

		bool operator < (const Kinematic & other) const
		{
			return position.x < other.position.x;
		}

		Kinematic & operator += (const Kinematic & other)
		{
			position += other.position;
			orientation += other.orientation;
			velocity += other.velocity;
			rotation += other.rotation;
			return *this;
		}

		Kinematic & operator -= (const Kinematic & other)
		{
			position -= other.position;
			orientation -= other.orientation;
			velocity -= other.velocity;
			rotation -= other.rotation;
			return *this;
		}

		Kinematic & operator *= (const Kinematic & other)
		{
			position *= other.position;
			orientation *= other.orientation;
			velocity *= other.velocity;
			rotation *= other.rotation;
			return *this;
		}

		void integrate(float duration);

		void integrate(const SteeringOutput & steer, float duration);

		void integrate(
			const SteeringOutput & steer,
			float drag,
			float duration);

		void integrate(
			const SteeringOutput & steer,
			const SteeringOutput & drag,
			float duration);
	};

	typedef boost::shared_ptr<Kinematic> KinematicPtr;

	class SteeringBehaviour
	{
	public:
		Kinematic * character;

		virtual ~SteeringBehaviour(void) {}

		virtual void getSteering(SteeringOutput * output) = 0;
	};

	class Seek : public SteeringBehaviour
	{
	public:
		const Vector3 * target;

		float maxAcceleration;

		virtual void getSteering(SteeringOutput * output);
	};

	class Flee : public Seek
	{
	public:
		virtual void getSteering(SteeringOutput * output);
	};

	class SeekWithInternalTarget : public Seek
	{
	protected:
		Vector3 internal_target;

		SeekWithInternalTarget(void)
		{
			target = &internal_target;
		}
	};

	class Wander : public SeekWithInternalTarget
	{
	public:
		float volatility;

		float turnSpeed;

		virtual void getSteering(SteeringOutput * output);
	};
}
