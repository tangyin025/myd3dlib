
#ifndef __MYPHYSICS_H__
#define __MYPHYSICS_H__

//#include "myCommon.h"
#include <cmath>
#include <list>
#include <vector>
#include <boost/shared_ptr.hpp>
//#include "myMath.h"

namespace my
{
	// /////////////////////////////////////////////////////////////////////////////////////
	// Particle
	// /////////////////////////////////////////////////////////////////////////////////////

	class Particle
	{
	protected:
		t3d::Vec4<real> position;

		t3d::Vec4<real> velocity;

		t3d::Vec4<real> acceleration;

		t3d::Vec4<real> forceAccum;

		real damping;

		real inverseMass;

	public:
		void setPosition(const t3d::Vec4<real> & _position)
		{
			position = _position;
		}

		void addPosition(const t3d::Vec4<real> & _position)
		{
			t3d::vec3AddSelf(position, _position);
		}

		const t3d::Vec4<real> & getPosition(void) const
		{
			return position;
		}

		void setVelocity(const t3d::Vec4<real> & _velocity)
		{
			velocity = _velocity;
		}

		void addVelocity(const t3d::Vec4<real> & _velocity)
		{
			t3d::vec3AddSelf(velocity, _velocity);
		}

		const t3d::Vec4<real> & getVelocity(void) const
		{
			return velocity;
		}

		void setAcceleration(const t3d::Vec4<real> & _acceleration)
		{
			acceleration = _acceleration;
		}

		const t3d::Vec4<real> & getAcceleration(void) const
		{
			return acceleration;
		}

		void addForce(const t3d::Vec4<real> & force)
		{
			t3d::vec3AddSelf(forceAccum, force);
		}

		const t3d::Vec4<real> & getAccumlator(void) const
		{
			return forceAccum;
		}

		void clearAccumulator(void)
		{
			forceAccum = my::Vec4<real>::ZERO;
		}

		void setDamping(real _damping)
		{
			damping = _damping;
		}

		real getDamping(void) const
		{
			return damping;
		}

		void setInverseMass(real _inverseMass)
		{
			inverseMass = _inverseMass;
		}

		real getInverseMass(void) const
		{
			return inverseMass;
		}

		void setMass(real mass)
		{
			_ASSERT(!IS_ZERO_FLOAT(mass));

			inverseMass = 1 / mass;
		}

		real getMass(void) const
		{
			if(inverseMass == 0)
			{
				return REAL_MAX;
			}

			return 1 / inverseMass;
		}

	public:
		Particle(void);

		bool hasFiniteMass(void) const;

		void integrate(real duration);
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

		void updateForces(real duration);
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
		virtual void updateForce(Particle * particle, real duration) = 0;
	};

	typedef boost::shared_ptr<ParticleForceGenerator> ParticleForceGeneratorPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleGravity
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleGravity : public ParticleForceGenerator
	{
	protected:
		t3d::Vec4<real> gravity;

	public:
		ParticleGravity(const t3d::Vec4<real> & _gravity);

		void updateForce(Particle * particle, real duration);
	};

	typedef boost::shared_ptr<ParticleGravity> ParticleGravityPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleDrag
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleDrag : public ParticleForceGenerator
	{
	protected:
		real k1;

		real k2;

	public:
		ParticleDrag(real _k1, real _k2);

		void updateForce(Particle * particle, real duration);
	};

	typedef boost::shared_ptr<ParticleDrag> ParticleDragPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleSpring
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleSpring : public ParticleForceGenerator
	{
	protected:
		Particle * other;

		real springConstant;

		real restLength;

	public:
		ParticleSpring(Particle * _other, real _springConstant, real _restLength);

		void updateForce(Particle * particle, real duration);
	};

	typedef boost::shared_ptr<ParticleSpring> ParticleSpringPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleAnchoredSpring
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleAnchoredSpring : public ParticleForceGenerator
	{
	protected:
		const t3d::Vec4<real> & anchor;

		real springConstant;

		real restLength;

	public:
		ParticleAnchoredSpring(const t3d::Vec4<real> & _anchor, real _springConstant, real _restLength);

		void updateForce(Particle * particle, real duration);
	};

	typedef boost::shared_ptr<ParticleAnchoredSpring> ParticleAnchoredSpringPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleBungee
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleBungee : public ParticleForceGenerator
	{
	protected:
		Particle * other;

		real springConstant;

		real restLength;

	public:
		ParticleBungee(Particle * _other, real _springConstant, real _restLength);

		void updateForce(Particle * particle, real duration);
	};

	typedef boost::shared_ptr<ParticleBungee> ParticleBungeePtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleAnchoredBungee
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleAnchoredBungee : public ParticleForceGenerator
	{
	protected:
		const t3d::Vec4<real> & anchor;

		real springConstant;

		real restLength;

	public:
		ParticleAnchoredBungee(const t3d::Vec4<real> & _anchor, real _springConstant, real _restLength);

		void updateForce(Particle * particle, real duration);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleBuoyancy
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleBuoyancy : public ParticleForceGenerator
	{
	protected:
		real maxDepth;

		real volumn;

		real waterHeight;

		real liquidDensity;

	public:
		ParticleBuoyancy(real _maxDepth, real _volumn, real _waterHeight, real _liquidDensity = 1000.0f);

		void updateForce(Particle * particle, real duration);
	};

	typedef boost::shared_ptr<ParticleBuoyancy> ParticleBuoyancyPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleFakeSpring
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleFakeSpring : public ParticleForceGenerator
	{
	protected:
		const t3d::Vec4<real> & anchor;

		real springConstant;

		real damping;

	public:
		ParticleFakeSpring(const t3d::Vec4<real> & _anchor, real _springConstant, real _damping);

		void updateForce(Particle * particle, real duration);
	};

	typedef boost::shared_ptr<ParticleFakeSpring> ParticleFakeSpringPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleContact
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleContact
	{
	public:
		Particle * particles[2];

		real restitution;

		t3d::Vec4<real> contactNormal;

		real penetration;

	public:
		void resolve(real duration);

		real calculateSeparatingVelocity(void) const;

		void resolveVelocity(real duration);

		void resolveInterpenetration(real duration);
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

		void resolveContacts(ParticleContact * contactArray, unsigned numContacts, real duration);
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
		real currentLength() const;
	};

	typedef boost::shared_ptr<ParticleLink> ParticleLinkPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleCable
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleCable : public ParticleLink
	{
	protected:
		real maxLength;

		real restitution;

	public:
		ParticleCable(Particle * particle0, Particle * particle1, real _maxLength, real _restitution);

		unsigned addContact(ParticleContact * contacts, unsigned limits) const;
	};

	typedef boost::shared_ptr<ParticleCable> ParticleCablePtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleRod
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleRod : public ParticleLink
	{
	protected:
		real length;

	public:
		ParticleRod(Particle * particle0, Particle * particle1, real _length);

		unsigned addContact(ParticleContact * contacts, unsigned limits) const;
	};

	typedef boost::shared_ptr<ParticleRod> ParticleRodPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleConstraint
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleConstraint : public ParticleContactGenerator
	{
	protected:
		const t3d::Vec4<real> & anchor;

		Particle * particle;

	public:
		ParticleConstraint(const t3d::Vec4<real> & _anchor, Particle * _particle);

	protected:
		real currentLength() const;
	};

	typedef boost::shared_ptr<ParticleConstraint> ParticleConstraintPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleCableConstraint
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleCableConstraint : public ParticleConstraint
	{
	protected:
		real maxLength;

		real restitution;

	public:
		ParticleCableConstraint(const t3d::Vec4<real> & _anchor, Particle * particle, real _maxLength, real _restitution);

		unsigned addContact(ParticleContact * contacts, unsigned limits) const;
	};

	typedef boost::shared_ptr<ParticleCableConstraint> ParticleCableConstraintPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleRodConstraint
	// /////////////////////////////////////////////////////////////////////////////////////

	class ParticleRodConstraint : public ParticleConstraint
	{
	protected:
		real length;

	public:
		ParticleRodConstraint(const t3d::Vec4<real> & _anchor, Particle * particle, real _length);

		unsigned addContact(ParticleContact * contacts, unsigned limits) const;
	};

	typedef boost::shared_ptr<ParticleRodConstraint> ParticleRodConstraintPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleWorld
	// /////////////////////////////////////////////////////////////////////////////////////

	typedef std::vector<my::ParticlePtr> ParticlePtrList;

	typedef std::vector<my::ParticleContactGeneratorPtr> ParticleContactGeneratorPtrList;

	typedef std::vector<my::ParticleContact> ParticleContactArray;

	class ParticleWorld
	{
	protected:
		ParticlePtrList particleList;

		ParticleForceRegistry registry;

		ParticleContactResolver resolver;

		ParticleContactGeneratorPtrList particleContactGeneratorList;

		ParticleContactArray particleContactArray;

		unsigned maxContacts;

	public:
		ParticleWorld(unsigned _maxContacts = 256, unsigned _iterations = 0);

		virtual ~ParticleWorld(void);

		virtual void startFrame(void);

		virtual void integrate(real duration);

		virtual unsigned generateContacts(ParticleContact * contacts, unsigned limits);

		void runPhysics(real duration);
	};

	typedef boost::shared_ptr<ParticleWorld> ParticleWorldPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// RigidBody
	// /////////////////////////////////////////////////////////////////////////////////////

	class RigidBody
	{
	protected:
		real inverseMass;

		t3d::Vec4<real> position;

		t3d::Quat<real> orientation;

		t3d::Vec4<real> velocity;

		t3d::Vec4<real> rotation;

		t3d::Mat4<real> transform;

		t3d::Mat4<real> rotationTransform;

		t3d::Mat4<real> inverseInertiaTensor;

		t3d::Mat4<real> inverseInertiaTensorWorld;

		t3d::Vec4<real> acceleration;

		t3d::Vec4<real> forceAccum;

		t3d::Vec4<real> torqueAccum;

		t3d::Vec4<real> resultingAcc;

		t3d::Vec4<real> resultingAngularAcc;

		real damping;

		real angularDamping;

		real sleepEpsilon;

		real motion;

		bool isAwake;

		bool canSleep;

	public:
		void setMass(real mass)
		{
			_ASSERT(!IS_ZERO_FLOAT(mass));

			inverseMass = 1 / mass;
		}

		real getMass(void) const
		{
			if(inverseMass == 0)
			{
				return REAL_MAX;
			}

			return 1 / inverseMass;
		}

		void setInverseMass(real _inverseMass)
		{
			inverseMass = _inverseMass;
		}

		real getInverseMass(void) const
		{
			return inverseMass;
		}

		void setPosition(const t3d::Vec4<real> & _position)
		{
			position = _position;
		}

		void addPosition(const t3d::Vec4<real> & _position)
		{
			t3d::vec3AddSelf(position, _position);
		}

		const t3d::Vec4<real> & getPosition(void) const
		{
			return position;
		}

		void setOrientation(const t3d::Quat<real> & _orientation)
		{
			orientation = _orientation;
		}

		void addOrientation(const t3d::Vec4<real> & _rotation) // ***
		{
			t3d::quatAddAngularVelocitySelf(orientation, _rotation);
		}

		const t3d::Quat<real> & getOrientation(void) const
		{
			return orientation;
		}

		void setVelocity(const t3d::Vec4<real> & _velocity)
		{
			velocity = _velocity;
		}

		const t3d::Vec4<real> & getVelocity(void) const
		{
			return velocity;
		}

		void addVelocity(const t3d::Vec4<real> & _velocity)
		{
			t3d::vec3AddSelf(velocity, _velocity);
		}

		void setRotation(const t3d::Vec4<real> & _rotation)
		{
			rotation = _rotation;
		}

		const t3d::Vec4<real> & getRotation(void) const
		{
			return rotation;
		}

		void addRotation(const t3d::Vec4<real> & _rotation)
		{
			t3d::vec3AddSelf(rotation, _rotation);
		}

		const t3d::Mat4<real> & getTransform(void) const
		{
			return transform;
		}

		t3d::Mat4<real> getInverseTransform(void) const
		{
			return transform.inverse();
		}

		const t3d::Mat4<real> & getRotationTransform(void) const
		{
			return rotationTransform;
		}

		t3d::Mat4<real> getInverseRotationTransform(void) const
		{
			return rotationTransform.inverse();
		}

		void setInertialTensor(const t3d::Mat4<real> & inertialTensor)
		{
			_ASSERT(!IS_ZERO_FLOAT(inertialTensor.determinant()));

			inverseInertiaTensor = inertialTensor.inverse();
		}

		t3d::Mat4<real> getInertialTensor(void) const
		{
			_ASSERT(!IS_ZERO_FLOAT(inverseInertiaTensor.determinant()));

			return inverseInertiaTensor.inverse();
		}

		void setInverseInertialTensor(const t3d::Mat4<real> & _inverseInertiaTensor)
		{
			inverseInertiaTensor = _inverseInertiaTensor;
		}

		const t3d::Mat4<real> & getInverseInertialTensor(void) const
		{
			return inverseInertiaTensor;
		}

		void setAcceleration(const t3d::Vec4<real> & _acceleration)
		{
			acceleration = _acceleration;
		}

		const t3d::Vec4<real> & getAcceleration(void) const
		{
			return acceleration;
		}

		void addForce(const t3d::Vec4<real> & force)
		{
			t3d::vec3AddSelf(forceAccum, force);
		}

		const t3d::Vec4<real> & getAccumulator(void) const
		{
			return forceAccum;
		}

		void clearAccumulator(void)
		{
			forceAccum = my::Vec4<real>::ZERO;
		}

		void addTorque(const t3d::Vec4<real> & torque)
		{
			t3d::vec3AddSelf(torqueAccum, torque);
		}

		const t3d::Vec4<real> & getTorqueAccumulator(void) const
		{
			return torqueAccum;
		}

		void clearTorqueAccumulator(void)
		{
			torqueAccum = my::Vec4<real>::ZERO;
		}

		const t3d::Vec4<real> & getResultingAcc(void) const
		{
			return resultingAcc;
		}

		const t3d::Vec4<real> & getResultingAngularAcc(void) const
		{
			return resultingAngularAcc;
		}

		void setDamping(real _damping)
		{
			damping = _damping;
		}

		real getDamping(void) const
		{
			return damping;
		}

		void setAngularDamping(real _angularDamping)
		{
			angularDamping = _angularDamping;
		}

		real getAngularDamping(void) const
		{
			return angularDamping;
		}

		void setSleepEpsilon(real _sleepEpsilon)
		{
			sleepEpsilon = _sleepEpsilon;
		}

		real getSleepEpsilon(void) const
		{
			return sleepEpsilon;
		}

	public:
		RigidBody(void);

		void calculateDerivedData(void);

		void setAwake(bool _isAwake = true);

		bool getAwake(void) const;

		void setCanSleep(bool _canSleep = true);

		bool getCanSleep(void) const;

		void addForceAtPoint(const t3d::Vec4<real> & force, const t3d::Vec4<real> & point);

		bool hasFiniteMass(void) const;

		void integrate(real duration);
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

		void updateForces(real duration);
	};

	typedef boost::shared_ptr<ForceRegistry> ForceRegistryPtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// ForceGenerator
	// /////////////////////////////////////////////////////////////////////////////////////

	class ForceGenerator
	{
	public:
		virtual ~ForceGenerator(void);

		virtual void updateForce(RigidBody * body, real duration) = 0;
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// Gravity
	// /////////////////////////////////////////////////////////////////////////////////////

	class Gravity : public ForceGenerator
	{
	protected:
		t3d::Vec4<real> gravity;

	public:
		Gravity(const t3d::Vec4<real> & _gravity);

		void updateForce(RigidBody * body, real duration);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// Spring
	// /////////////////////////////////////////////////////////////////////////////////////

	class Spring : public ForceGenerator
	{
	protected:
		t3d::Vec4<real> connectionPoint;

		RigidBody * other;

		t3d::Vec4<real> otherConnectionPoint;

		real springConstant;

		real restLength;

	public:
		Spring(
			const t3d::Vec4<real> & _connectionPoint,
			RigidBody * _other,
			const t3d::Vec4<real> & _otherConnectionPoint,
			real _springConstant,
			real _restLength);

		void updateForce(RigidBody * body, real duration);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// Aero
	// /////////////////////////////////////////////////////////////////////////////////////

	class Aero : public ForceGenerator
	{
	protected:
		t3d::Mat4<real> tensor;

		t3d::Vec4<real> position;

		const t3d::Vec4<real> * pwindSpeed;

	public:
		Aero(
			const t3d::Mat4<real> & _tensor,
			const t3d::Vec4<real> & _position,
			const t3d::Vec4<real> * _pwindSpeed);

		void updateForce(RigidBody * body, real duration);

		void updateForceFromTensor(RigidBody * body, real duration, const t3d::Mat4<real> & _tensor);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// AeroControl
	// /////////////////////////////////////////////////////////////////////////////////////

	class AeroControl : public Aero
	{
	protected:
		t3d::Mat4<real> minTensor;

		t3d::Mat4<real> maxTensor;

		real controlSetting;

	public:
		void setControl(real value);

	public:
		AeroControl(
			const t3d::Mat4<real> & _tensor,
			const t3d::Mat4<real> & _minTensor,
			const t3d::Mat4<real> & _maxTensor,
			const t3d::Vec4<real> & _position,
			const t3d::Vec4<real> * _pwindSpeed);

		void updateForce(RigidBody * body, real duration);

		void updateForceFromControl(RigidBody * body, real duration, real _controlSetting);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// AngledAero
	// /////////////////////////////////////////////////////////////////////////////////////

	class AngledAero : public Aero
	{
	protected:
		t3d::Quat<real> orientation;

	public:
		void setOrientation(const t3d::Quat<real> & _orientation);

		const t3d::Quat<real> & getOrientation(void) const;

	public:
		AngledAero(
			const t3d::Mat4<real> & _tensor,
			const t3d::Vec4<real> & _position,
			const t3d::Vec4<real> * _pwindSpeed);

		void updateForce(RigidBody * body, real duration);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// Buoyancy
	// /////////////////////////////////////////////////////////////////////////////////////

	class Buoyancy : public ForceGenerator
	{
	protected:
		real maxDepth;

		real volume;

		real waterHeight;

		real liquidDensity;

		t3d::Vec4<real> centerOfBuoyancy;

	public:
		Buoyancy(
			const t3d::Vec4<real> & _centerOfBuoyancy,
			real _maxDepth,
			real _volume,
			real _waterHeight,
			real _liquidDensity = 1000.0f);

		void updateForce(RigidBody * body, real duration);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// Contact
	// /////////////////////////////////////////////////////////////////////////////////////

	class Contact
	{
	public:
		RigidBody * bodys[2];

		real friction;

		real restitution;

		t3d::Vec4<real> contactPoint;

		t3d::Vec4<real> contactNormal;

		real penetration;

	public:
		t3d::Mat4<real> contactToWorld;

		t3d::Vec4<real> contactVelocity;

		real desiredDeltaVelocity;

		t3d::Vec4<real> relativeContactPositions[2];

	public:
		void swapBodies(void);

		void matchAwakeState(void);

		void calculateContactBasis(void);

		t3d::Vec4<real> calculateLocalVelocity(const RigidBody & body, const t3d::Vec4<real> & relativeContactPosition, real duration) const;

		void calculateDesiredDeltaVelocity(real duration);

		void calculateInternals(real duration);

		t3d::Vec4<real> calculateFrictionlessImpulse(const t3d::Mat4<real> inverseInertialTensors[]) const;

		t3d::Vec4<real> calculateFrictionImpulse(const t3d::Mat4<real> inverseInertialTensors[]) const;

		void applyPositionChange(t3d::Vec4<real> linearChanges[2], t3d::Vec4<real> angularChanges[2]);

		void applyVelocityChange(t3d::Vec4<real> velocityChanges[2], t3d::Vec4<real> rotationChanges[2]);
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

		real positionEpsilon;

		real velocityEpsilon;

		unsigned positionIterationsUsed;

		unsigned velocityIterationsUsed;

	public:
		void setPositionIterations(unsigned iterations);

		unsigned getPositionIterations(void) const;

		void setVelocityIterations(unsigned iterations);

		unsigned getVelocityIterations(void) const;

		void setPositionEpsilon(real value);

		real getPositionEpsilon(void) const;

		void setVelocityEpsilon(real value);

		real getVelocityEpsilon(void) const;

	public:
		ContactResolver(
			unsigned _positionIterations = 0,
			unsigned _velocityIterations = 0,
			real _positionEpsilon = 0.01f,
			real _velocityEpsilon = 0.01f);

	protected:
		void prepareContacts(
			Contact * contacts,
			unsigned numContacts,
			real duration);

		static void _updateContactPenetration(
			Contact & contact,
			const t3d::Vec4<real> & relativeContactPosition,
			const t3d::Vec4<real> & linearChange,
			const t3d::Vec4<real> & angularChange,
			unsigned bodyIndex);

		void adjustPositions(
			Contact * contacts,
			unsigned numContacts,
			real duration);

		static void _updateContactVelocity(
			Contact & contact,
			const t3d::Vec4<real> & relativeContactPosition,
			const t3d::Vec4<real> & velocityChange,
			const t3d::Vec4<real> & rotationChange,
			unsigned bodyIndex,
			real duration);

		void adjustVelocities(
			Contact * contacts,
			unsigned numContacts,
			real duration);

	public:
		void resolveContacts(
			Contact * contacts,
			unsigned numContacts,
			real duration);
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

		unsigned maxContacts;

		ContactResolver resolver;

	public:
		World(unsigned _maxContacts = 256);

		virtual ~World(void);

		virtual void startFrame(void);

		virtual void integrate(real duration);

		virtual unsigned generateContacts(Contact * contacts, unsigned limits);

		void runPhysics(real duration);
	};

	typedef boost::shared_ptr<World> WorldPtr;
}

#endif // __MYPHYSICS_H__
