
#ifndef __MYPHYSICS_H__
#define __MYPHYSICS_H__

#include "myMath.h"
#include <boost/shared_ptr.hpp>
#include <list>
#include <vector>

namespace my
{
	// /////////////////////////////////////////////////////////////////////////////////////
	// Particle
	// /////////////////////////////////////////////////////////////////////////////////////

	class Particle
	{
	protected:
		Vector3 position;

		Vector3 velocity;

		Vector3 acceleration;

		Vector3 forceAccum;

		float inverseMass;

		float damping;

	public:
		void setPosition(const Vector3 & _position)
		{
			position = _position;
		}

		void addPosition(const Vector3 & _position)
		{
			position += _position;
		}

		const Vector3 & getPosition(void) const
		{
			return position;
		}

		void setVelocity(const Vector3 & _velocity)
		{
			velocity = _velocity;
		}

		void addVelocity(const Vector3 & _velocity)
		{
			velocity += _velocity;
		}

		const Vector3 & getVelocity(void) const
		{
			return velocity;
		}

		void setAcceleration(const Vector3 & _acceleration)
		{
			acceleration = _acceleration;
		}

		const Vector3 & getAcceleration(void) const
		{
			return acceleration;
		}

		void addForce(const Vector3 & force)
		{
			forceAccum += force;
		}

		const Vector3 & getAccumlator(void) const
		{
			return forceAccum;
		}

		void clearAccumulator(void)
		{
			forceAccum = Vector3::zero;
		}

		void setDamping(float _damping)
		{
			damping = _damping;
		}

		float getDamping(void) const
		{
			return damping;
		}

		void setInverseMass(float _inverseMass)
		{
			inverseMass = _inverseMass;
		}

		float getInverseMass(void) const
		{
			return inverseMass;
		}

		void setMass(float mass)
		{
			_ASSERT(0 != mass);

			inverseMass = 1 / mass;
		}

		float getMass(void) const
		{
			if(inverseMass == 0)
			{
				return FLT_MAX;
			}

			return 1 / inverseMass;
		}

	public:
		Particle(
			const Vector3 & _position,
			const Vector3 & _velocity,
			const Vector3 & _acceleration,
			const Vector3 & _forceAccum,
			float _inverseMass,
			float _damping);

		bool hasFiniteMass(void) const;

		void integrate(float duration);
	};

	typedef boost::shared_ptr<Particle> ParticlePtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleForceRegistry
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleForceGenerator;

	class ParticleForceRegistry
	{
	protected:
		class ParticleForceRegistration
		{
		public:
			Particle * particle;

			ParticleForceGenerator * forceGenerator;

		public:
			ParticleForceRegistration(Particle * _particle, ParticleForceGenerator * _forceGenerator);
		};

		typedef std::list<ParticleForceRegistration> ParticleForceRegistrationList;

		ParticleForceRegistrationList forceRegistrationList;

	public:
		void add(Particle * particle, ParticleForceGenerator * forceGenerator);

		void remove(Particle * particle, ParticleForceGenerator * forceGenerator);

		void clear(void);

		void updateForces(float duration);
	};

	typedef boost::shared_ptr<ParticleForceRegistry> ParticleForceRegistryPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleForceGenerator
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleForceGenerator
	{
	public:
		virtual ~ParticleForceGenerator(void);

	public:
		virtual void updateForce(Particle * particle, float duration) = 0;
	};

	typedef boost::shared_ptr<ParticleForceGenerator> ParticleForceGeneratorPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleGravity
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleGravity : public ParticleForceGenerator
	{
	protected:
		Vector3 gravity;

	public:
		ParticleGravity(const Vector3 & _gravity);

		void updateForce(Particle * particle, float duration);
	};

	typedef boost::shared_ptr<ParticleGravity> ParticleGravityPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleDrag
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleDrag : public ParticleForceGenerator
	{
	protected:
		float k1;

		float k2;

	public:
		ParticleDrag(float _k1, float _k2);

		void updateForce(Particle * particle, float duration);
	};

	typedef boost::shared_ptr<ParticleDrag> ParticleDragPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleSpring
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleSpring : public ParticleForceGenerator
	{
	protected:
		Particle * other;

		float springConstant;

		float restLength;

	public:
		ParticleSpring(Particle * _other, float _springConstant, float _restLength);

		void updateForce(Particle * particle, float duration);
	};

	typedef boost::shared_ptr<ParticleSpring> ParticleSpringPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleAnchoredSpring
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleAnchoredSpring : public ParticleForceGenerator
	{
	protected:
		const Vector3 & anchor;

		float springConstant;

		float restLength;

	public:
		ParticleAnchoredSpring(const Vector3 & _anchor, float _springConstant, float _restLength);

		void updateForce(Particle * particle, float duration);
	};

	typedef boost::shared_ptr<ParticleAnchoredSpring> ParticleAnchoredSpringPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleBungee
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleBungee : public ParticleForceGenerator
	{
	protected:
		Particle * other;

		float springConstant;

		float restLength;

	public:
		ParticleBungee(Particle * _other, float _springConstant, float _restLength);

		void updateForce(Particle * particle, float duration);
	};

	typedef boost::shared_ptr<ParticleBungee> ParticleBungeePtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleAnchoredBungee
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleAnchoredBungee : public ParticleForceGenerator
	{
	protected:
		const Vector3 & anchor;

		float springConstant;

		float restLength;

	public:
		ParticleAnchoredBungee(const Vector3 & _anchor, float _springConstant, float _restLength);

		void updateForce(Particle * particle, float duration);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleBuoyancy
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleBuoyancy : public ParticleForceGenerator
	{
	protected:
		float maxDepth;

		float volumn;

		float waterHeight;

		float liquidDensity;

	public:
		ParticleBuoyancy(float _maxDepth, float _volumn, float _waterHeight, float _liquidDensity = 1000.0f);

		void updateForce(Particle * particle, float duration);
	};

	typedef boost::shared_ptr<ParticleBuoyancy> ParticleBuoyancyPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleFakeSpring
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleFakeSpring : public ParticleForceGenerator
	{
	protected:
		const Vector3 & anchor;

		float springConstant;

		float damping;

	public:
		ParticleFakeSpring(const Vector3 & _anchor, float _springConstant, float _damping);

		void updateForce(Particle * particle, float duration);
	};

	typedef boost::shared_ptr<ParticleFakeSpring> ParticleFakeSpringPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleContact
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleContact
	{
	public:
		Particle * particles[2];

		float restitution;

		Vector3 contactNormal;

		float penetration;

	public:
		void resolve(float duration);

		float calculateSeparatingVelocity(void) const;

		void resolveVelocity(float duration);

		void resolveInterpenetration(float duration);
	};

	typedef boost::shared_ptr<ParticleContact> ParticleContactPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleContactResolver
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleContactResolver
	{
	protected:
		unsigned iterations;

		unsigned iterationsUsed;

	public:
		ParticleContactResolver(unsigned _iterations = 0);

		void setIterations(unsigned _iterations);

		unsigned getIterations(void) const;

		void resolveContacts(ParticleContact * contactArray, unsigned numContacts, float duration);
	};

	typedef boost::shared_ptr<ParticleContactResolver> ParticleContactResolverPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleContactGenerator
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleContactGenerator
	{
	public:
		virtual ~ParticleContactGenerator(void);

	public:
		virtual unsigned addContact(ParticleContact * contacts, unsigned limits) const = 0;
	};

	typedef boost::shared_ptr<ParticleContactGenerator> ParticleContactGeneratorPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleLink
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleLink : public ParticleContactGenerator
	{
	protected:
		Particle * particles[2];

	public:
		ParticleLink(Particle * particle0, Particle * particle1);

	protected:
		float currentLength() const;
	};

	typedef boost::shared_ptr<ParticleLink> ParticleLinkPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleCable
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleCable : public ParticleLink
	{
	protected:
		float maxLength;

		float restitution;

	public:
		ParticleCable(Particle * particle0, Particle * particle1, float _maxLength, float _restitution);

		unsigned addContact(ParticleContact * contacts, unsigned limits) const;
	};

	typedef boost::shared_ptr<ParticleCable> ParticleCablePtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleRod
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleRod : public ParticleLink
	{
	protected:
		float length;

	public:
		ParticleRod(Particle * particle0, Particle * particle1, float _length);

		unsigned addContact(ParticleContact * contacts, unsigned limits) const;
	};

	typedef boost::shared_ptr<ParticleRod> ParticleRodPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleConstraint
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleConstraint : public ParticleContactGenerator
	{
	protected:
		const Vector3 & anchor;

		Particle * particle;

	public:
		ParticleConstraint(const Vector3 & _anchor, Particle * _particle);

	protected:
		float currentLength() const;
	};

	typedef boost::shared_ptr<ParticleConstraint> ParticleConstraintPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleCableConstraint
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleCableConstraint : public ParticleConstraint
	{
	protected:
		float maxLength;

		float restitution;

	public:
		ParticleCableConstraint(const Vector3 & _anchor, Particle * particle, float _maxLength, float _restitution);

		unsigned addContact(ParticleContact * contacts, unsigned limits) const;
	};

	typedef boost::shared_ptr<ParticleCableConstraint> ParticleCableConstraintPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleRodConstraint
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleRodConstraint : public ParticleConstraint
	{
	protected:
		float length;

	public:
		ParticleRodConstraint(const Vector3 & _anchor, Particle * particle, float _length);

		unsigned addContact(ParticleContact * contacts, unsigned limits) const;
	};

	typedef boost::shared_ptr<ParticleRodConstraint> ParticleRodConstraintPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleWorld
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleWorld
	{
	protected:
		typedef std::vector<my::ParticlePtr> ParticlePtrList;

		ParticlePtrList particleList;

		ParticleForceRegistry registry;

		unsigned resolveIteration;

		ParticleContactResolver resolver;

		typedef std::vector<my::ParticleContactGeneratorPtr> ParticleContactGeneratorPtrList;

		ParticleContactGeneratorPtrList particleContactGeneratorList;

		typedef std::vector<my::ParticleContact> ParticleContactArray;

		ParticleContactArray particleContactArray;

	public:
		void setMaxContacts(unsigned maxContacts)
		{
			particleContactArray.resize(maxContacts);
		}

		size_t getMaxContacts(void) const
		{
			return particleContactArray.size();
		}

		void setIteration(unsigned iteration)
		{
			resolveIteration = iteration;
		}

		unsigned getIteration(void) const
		{
			return resolveIteration;
		}

	public:
		ParticleWorld(unsigned maxContacts = 256, unsigned _resolveIterations = 16);

		virtual ~ParticleWorld(void);

		virtual void startFrame(void);

		virtual void integrate(float duration);

		virtual unsigned generateContacts(ParticleContact * contacts, unsigned limits);

		void runPhysics(float duration);
	};

	typedef boost::shared_ptr<ParticleWorld> ParticleWorldPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// RigidBody
	// /////////////////////////////////////////////////////////////////////////////////////

	class RigidBody
	{
	protected:
		Vector3 position;

		Quaternion orientation;

		Vector3 velocity;

		Vector3 rotation;

		Matrix4 transform;

		Vector3 acceleration;

		Vector3 forceAccum;

		Vector3 torqueAccum;

		Vector3 resultingAcc;

		Vector3 resultingAngularAcc;

		float inverseMass;

		Matrix4 inverseInertiaTensor;

		Matrix4 inverseInertiaTensorWorld;

		float damping;

		float angularDamping;

		float sleepEpsilon;

		float motion;

		bool isAwake;

		bool canSleep;

	public:
		void setPosition(const Vector3 & _position)
		{
			position = _position;
		}

		void addPosition(const Vector3 & _position)
		{
			position += _position;
		}

		const Vector3 & getPosition(void) const
		{
			return position;
		}

		void setOrientation(const Quaternion & _orientation)
		{
			orientation = _orientation;
		}

		void addOrientationRH(const Vector3 & _rotation)
		{
			orientation += Quaternion(_rotation.x, _rotation.y, _rotation.z, 0) * orientation * 0.5f; // ! right-handed coordinate
		}

		const Quaternion & getOrientation(void) const
		{
			return orientation;
		}

		void setVelocity(const Vector3 & _velocity)
		{
			velocity = _velocity;
		}

		const Vector3 & getVelocity(void) const
		{
			return velocity;
		}

		void addVelocity(const Vector3 & _velocity)
		{
			velocity += _velocity;
		}

		void setRotation(const Vector3 & _rotation)
		{
			rotation = _rotation;
		}

		const Vector3 & getRotation(void) const
		{
			return rotation;
		}

		void addRotation(const Vector3 & _rotation)
		{
			rotation += _rotation;
		}

		const Matrix4 & getTransform(void) const
		{
			return transform;
		}

		void setAcceleration(const Vector3 & _acceleration)
		{
			acceleration = _acceleration;
		}

		const Vector3 & getAcceleration(void) const
		{
			return acceleration;
		}

		void addForce(const Vector3 & force)
		{
			forceAccum += force;
		}

		const Vector3 & getAccumulator(void) const
		{
			return forceAccum;
		}

		void clearAccumulator(void)
		{
			forceAccum = Vector3::zero;
		}

		void addTorque(const Vector3 & torque)
		{
			torqueAccum += torque;
		}

		const Vector3 & getTorqueAccumulator(void) const
		{
			return torqueAccum;
		}

		void clearTorqueAccumulator(void)
		{
			torqueAccum = Vector3::zero;
		}

		const Vector3 & getResultingAcc(void) const
		{
			return resultingAcc;
		}

		const Vector3 & getResultingAngularAcc(void) const
		{
			return resultingAngularAcc;
		}

		void setMass(float mass)
		{
			_ASSERT(0 != mass);

			inverseMass = 1 / mass;
		}

		float getMass(void) const
		{
			if(inverseMass == 0)
			{
				return FLT_MAX;
			}

			return 1 / inverseMass;
		}

		void setInverseMass(float _inverseMass)
		{
			inverseMass = _inverseMass;
		}

		float getInverseMass(void) const
		{
			return inverseMass;
		}

		void setInertialTensor(const Matrix4 & inertialTensor)
		{
			_ASSERT(0 != inertialTensor.determinant());

			inverseInertiaTensor = inertialTensor.inverse();
		}

		Matrix4 getInertialTensor(void) const
		{
			_ASSERT(0 != inverseInertiaTensor.determinant());

			return inverseInertiaTensor.inverse();
		}

		void setInverseInertialTensor(const Matrix4 & _inverseInertiaTensor)
		{
			inverseInertiaTensor = _inverseInertiaTensor;
		}

		const Matrix4 & getInverseInertialTensor(void) const
		{
			return inverseInertiaTensor;
		}

		void setDamping(float _damping)
		{
			damping = _damping;
		}

		float getDamping(void) const
		{
			return damping;
		}

		void setAngularDamping(float _angularDamping)
		{
			angularDamping = _angularDamping;
		}

		float getAngularDamping(void) const
		{
			return angularDamping;
		}

		void setSleepEpsilon(float _sleepEpsilon)
		{
			sleepEpsilon = _sleepEpsilon;
		}

		float getSleepEpsilon(void) const
		{
			return sleepEpsilon;
		}

	public:
		RigidBody(
			const Vector3 & _position,
			const Quaternion & _orientation,
			const Vector3 & _velocity,
			const Vector3 & _rotation,
			const Vector3 & _acceleration,
			const Vector3 & _accumulator,
			const Vector3 & _torqueAccumulator,
			const float _inverseMass,
			const Matrix4 & _inverseInertialTensor,
			float _damping,
			float _angularDamping);

		void calculateDerivedData(void);

		void setAwake(bool _isAwake = true);

		bool getAwake(void) const;

		void setCanSleep(bool _canSleep = true);

		bool getCanSleep(void) const;

		void addForceAtPoint(const Vector3 & force, const Vector3 & point);

		bool hasFiniteMass(void) const;

		void integrate(float duration);
	};

	typedef boost::shared_ptr<RigidBody> RigidBodyPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ForceRegistry
	// /////////////////////////////////////////////////////////////////////////////////////

	class ForceGenerator;

	class ForceRegistry
	{
	protected:
		class ForceRegistration
		{
		public:
			RigidBody * body;

			ForceGenerator * forceGenerator;

		public:
			ForceRegistration(RigidBody * _body, ForceGenerator * _forceGenerator);
		};

		typedef std::list<ForceRegistration> ForceRegistrationList;

		ForceRegistrationList forceRegistrationList;

	public:
		void add(RigidBody * body, ForceGenerator * forceGenerator);

		void remove(RigidBody * body, ForceGenerator * forceGenerator);

		void clear(void);

		void updateForces(float duration);
	};

	typedef boost::shared_ptr<ForceRegistry> ForceRegistryPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ForceGenerator
	// /////////////////////////////////////////////////////////////////////////////////////

	class ForceGenerator
	{
	public:
		virtual ~ForceGenerator(void);

		virtual void updateForce(RigidBody * body, float duration) = 0;
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// Gravity
	// /////////////////////////////////////////////////////////////////////////////////////

	class Gravity : public ForceGenerator
	{
	protected:
		Vector3 gravity;

	public:
		Gravity(const Vector3 & _gravity);

		void updateForce(RigidBody * body, float duration);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// Spring
	// /////////////////////////////////////////////////////////////////////////////////////

	class Spring : public ForceGenerator
	{
	protected:
		Vector3 connectionPoint;

		RigidBody * other;

		Vector3 otherConnectionPoint;

		float springConstant;

		float restLength;

	public:
		Spring(
			const Vector3 & _connectionPoint,
			RigidBody * _other,
			const Vector3 & _otherConnectionPoint,
			float _springConstant,
			float _restLength);

		void updateForce(RigidBody * body, float duration);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// Aero
	// /////////////////////////////////////////////////////////////////////////////////////

	class Aero : public ForceGenerator
	{
	protected:
		Matrix4 tensor;

		Vector3 position;

		const Vector3 * pwindSpeed;

	public:
		Aero(
			const Matrix4 & _tensor,
			const Vector3 & _position,
			const Vector3 * _pwindSpeed);

		void updateForce(RigidBody * body, float duration);

		void updateForceFromTensor(RigidBody * body, float duration, const Matrix4 & _tensor);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// AeroControl
	// /////////////////////////////////////////////////////////////////////////////////////

	class AeroControl : public Aero
	{
	protected:
		Matrix4 minTensor;

		Matrix4 maxTensor;

		float controlSetting;

	public:
		void setControl(float value);

	public:
		AeroControl(
			const Matrix4 & _tensor,
			const Matrix4 & _minTensor,
			const Matrix4 & _maxTensor,
			const Vector3 & _position,
			const Vector3 * _pwindSpeed);

		void updateForce(RigidBody * body, float duration);

		void updateForceFromControl(RigidBody * body, float duration, float _controlSetting);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// AngledAero
	// /////////////////////////////////////////////////////////////////////////////////////

	class AngledAero : public Aero
	{
	protected:
		Quaternion orientation;

	public:
		void setOrientation(const Quaternion & _orientation);

		const Quaternion & getOrientation(void) const;

	public:
		AngledAero(
			const Matrix4 & _tensor,
			const Vector3 & _position,
			const Vector3 * _pwindSpeed);

		void updateForce(RigidBody * body, float duration);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// Buoyancy
	// /////////////////////////////////////////////////////////////////////////////////////

	class Buoyancy : public ForceGenerator
	{
	protected:
		float maxDepth;

		float volume;

		float waterHeight;

		float liquidDensity;

		Vector3 centerOfBuoyancy;

	public:
		Buoyancy(
			const Vector3 & _centerOfBuoyancy,
			float _maxDepth,
			float _volume,
			float _waterHeight,
			float _liquidDensity = 1000.0f);

		void updateForce(RigidBody * body, float duration);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// Contact
	// /////////////////////////////////////////////////////////////////////////////////////

	class Contact
	{
	public:
		RigidBody * bodys[2];

		float friction;

		float restitution;

		Vector3 contactPoint;

		Vector3 contactNormal;

		float penetration;

	public:
		Matrix4 contactToWorld;

		Vector3 contactVelocity;

		float desiredDeltaVelocity;

		Vector3 relativeContactPositions[2];

	public:
		void swapBodies(void);

		void matchAwakeState(void);

		void calculateContactBasis(void);

		Vector3 calculateLocalVelocity(const RigidBody & body, const Vector3 & relativeContactPosition, float duration) const;

		void calculateDesiredDeltaVelocity(float duration);

		void calculateInternals(float duration);

		Vector3 calculateFrictionlessImpulse(const Matrix4 inverseInertialTensors[]) const;

		Vector3 calculateFrictionImpulse(const Matrix4 inverseInertialTensors[]) const;

		void applyPositionChange(Vector3 linearChanges[2], Vector3 angularChanges[2]);

		void applyVelocityChange(Vector3 velocityChanges[2], Vector3 rotationChanges[2]);
	};

	typedef std::vector<Contact> ContactList;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ContactResolver
	// /////////////////////////////////////////////////////////////////////////////////////

	class ContactResolver
	{
	protected:
		unsigned positionIterations;

		unsigned velocityIterations;

		float positionEpsilon;

		float velocityEpsilon;

		unsigned positionIterationsUsed;

		unsigned velocityIterationsUsed;

	public:
		void setPositionIterations(unsigned iterations)
		{
			positionIterations = iterations;
		}

		unsigned getPositionIterations(void) const
		{
			return positionIterations;
		}

		void setVelocityIterations(unsigned iterations)
		{
			velocityIterations = iterations;
		}

		unsigned getVelocityIterations(void) const
		{
			return velocityIterations;
		}

		void setPositionEpsilon(float value)
		{
			positionEpsilon = value;
		}

		float getPositionEpsilon(void) const
		{
			return positionEpsilon;
		}

		void setVelocityEpsilon(float value)
		{
			velocityEpsilon = value;
		}

		float getVelocityEpsilon(void) const
		{
			return velocityEpsilon;
		}

	public:
		ContactResolver(
			unsigned _positionIterations = 0,
			unsigned _velocityIterations = 0,
			float _positionEpsilon = 0.01f,
			float _velocityEpsilon = 0.01f);

	protected:
		void prepareContacts(
			Contact * contacts,
			unsigned numContacts,
			float duration);

		static void _updateContactPenetration(
			Contact & contact,
			const Vector3 & relativeContactPosition,
			const Vector3 & linearChange,
			const Vector3 & angularChange,
			unsigned bodyIndex);

		void adjustPositions(
			Contact * contacts,
			unsigned numContacts,
			float duration);

		static void _updateContactVelocity(
			Contact & contact,
			const Vector3 & relativeContactPosition,
			const Vector3 & velocityChange,
			const Vector3 & rotationChange,
			unsigned bodyIndex,
			float duration);

		void adjustVelocities(
			Contact * contacts,
			unsigned numContacts,
			float duration);

	public:
		void resolveContacts(
			Contact * contacts,
			unsigned numContacts,
			float duration);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// World
	// /////////////////////////////////////////////////////////////////////////////////////

	typedef std::vector<RigidBodyPtr> RigidBodyPtrList;

	class World
	{
	protected:
		RigidBodyPtrList bodyList;

		ForceRegistry registry;

		ContactList contactList;

		unsigned resolvePositionIteration;

		unsigned resolveVelocityIteration;

		ContactResolver resolver;

	public:
		void setMaxContacts(unsigned maxContacts)
		{
			contactList.resize(maxContacts);
		}

		size_t getMaxContacts(void) const
		{
			return contactList.size();
		}

		void setPositionIteration(unsigned posIteration)
		{
			resolvePositionIteration = posIteration;
		}

		unsigned getPositionIteration(void) const
		{
			return resolvePositionIteration;
		}

		void setVelocityIteration(unsigned velIteration)
		{
			resolveVelocityIteration = velIteration;
		}

		unsigned getVelocityIteration(void) const
		{
			return resolveVelocityIteration;
		}

	public:
		World(
			unsigned maxContacts = 256,
			unsigned _resolvePositionIteration = 4,
			unsigned _resolveVelocityIteration = 4,
			float resolvePositionEpsilon = 0.01f,
			float resolveVelocityEpsilon = 0.01f);

		virtual ~World(void);

		virtual void startFrame(void);

		virtual void integrate(float duration);

		virtual unsigned generateContacts(Contact * contacts, unsigned limits);

		void runPhysics(float duration);
	};

	typedef boost::shared_ptr<World> WorldPtr;
}

#endif // __MYPHYSICS_H__
