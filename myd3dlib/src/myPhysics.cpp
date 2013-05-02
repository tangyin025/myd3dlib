#include "stdafx.h"
#include "myPhysics.h"

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) (P)
#endif

namespace my
{
	// /////////////////////////////////////////////////////////////////////////////////////
	// Particle
	// /////////////////////////////////////////////////////////////////////////////////////

	bool Particle::hasFiniteMass(void) const
	{
		return inverseMass > 0.0f;
	}

	void Particle::integrate(float duration)
	{
		_ASSERT(duration >= 0);

		Vector3 resultingAcc = acceleration + forceAccum * inverseMass;

		addVelocity(resultingAcc * duration);		

		velocity *= pow(damping, duration);

		addPosition(velocity * duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleForceRegistry
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleForceRegistry::ParticleForceRegistration::ParticleForceRegistration(Particle * _particle, ParticleForceGenerator * _forceGenerator)
		: particle(_particle)
		, forceGenerator(_forceGenerator)
	{
	}

	void ParticleForceRegistry::add(Particle * particle, ParticleForceGenerator * forceGenerator)
	{
		forceRegistrationList.push_back(ParticleForceRegistration(particle, forceGenerator));
	}

	void ParticleForceRegistry::remove(Particle * particle, ParticleForceGenerator * forceGenerator)
	{
		ParticleForceRegistrationList::iterator f_iter = forceRegistrationList.begin();
		for(; f_iter != forceRegistrationList.end(); f_iter++)
		{
			if(f_iter->particle == particle
				&& f_iter->forceGenerator == forceGenerator)
			{
				forceRegistrationList.erase(f_iter);
				return;
			}
		}
	}

	void ParticleForceRegistry::clear(void)
	{
		forceRegistrationList.clear();
	}

	void ParticleForceRegistry::updateForces(float duration)
	{
		ParticleForceRegistrationList::iterator f_iter = forceRegistrationList.begin();
		for(; f_iter != forceRegistrationList.end(); f_iter++)
		{
			f_iter->forceGenerator->updateForce(f_iter->particle, duration);
		}
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleForceGenerator
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleForceGenerator::~ParticleForceGenerator(void)
	{
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleGravity
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleGravity::ParticleGravity(const Vector3 & _gravity)
		: gravity(_gravity)
	{
	}

	void ParticleGravity::updateForce(Particle * particle, float duration)
	{
		if(!particle->hasFiniteMass())
			return;

		particle->addForce(gravity * particle->getMass());

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleDrag
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleDrag::ParticleDrag(float _k1, float _k2)
		: k1(_k1)
		, k2(_k2)
	{
	}

	void ParticleDrag::updateForce(Particle * particle, float duration)
	{
		Vector3 velocity = particle->getVelocity();

		float len = velocity.magnitude();

		float coefficient = k1 * len + k2 * len * len;

		particle->addForce(velocity.normalize() * -coefficient);

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleSpring
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleSpring::ParticleSpring(Particle * _other, float _springConstant, float _restLength)
		: other(_other)
		, springConstant(_springConstant)
		, restLength(_restLength)
	{
	}

	void ParticleSpring::updateForce(Particle * particle, float duration)
	{
		Vector3 distance = particle->getPosition() - other->getPosition();

		float length = distance.magnitude();

		particle->addForce(distance.normalize() * (-springConstant * (length - restLength)));

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleAnchoredSpring
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleAnchoredSpring::ParticleAnchoredSpring(const Vector3 & _anchor, float _springConstant, float _restLength)
		: anchor(_anchor)
		, springConstant(_springConstant)
		, restLength(_restLength)
	{
	}

	void ParticleAnchoredSpring::updateForce(Particle * particle, float duration)
	{
		Vector3 distance = particle->getPosition() - anchor;

		float length = distance.magnitude();

		particle->addForce(distance.normalize() * (-springConstant * (length - restLength)));

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleBungee
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleBungee::ParticleBungee(Particle * _other, float _springConstant, float _restLength)
		: other(_other)
		, springConstant(_springConstant)
		, restLength(_restLength)
	{
	}

	void ParticleBungee::updateForce(Particle * particle, float duration)
	{
		Vector3 distance = particle->getPosition() - other->getPosition();

		float length = distance.magnitude();

		if(length > restLength)
		{
			particle->addForce(distance.normalize() * (-springConstant * (length - restLength)));
		}

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleAnchoredBungee
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleAnchoredBungee::ParticleAnchoredBungee(const Vector3 & _anchor, float _springConstant, float _restLength)
		: anchor(_anchor)
		, springConstant(_springConstant)
		, restLength(_restLength)
	{
	}

	void ParticleAnchoredBungee::updateForce(Particle * particle, float duration)
	{
		Vector3 distance = particle->getPosition() - anchor;

		float length = distance.magnitude();

		if(length > restLength)
		{
			particle->addForce(distance.normalize() * (-springConstant * (length - restLength)));
		}

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleBuoyancy
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleBuoyancy::ParticleBuoyancy(float _maxDepth, float _volumn, float _waterHeight, float _liquidDensity)
		: maxDepth(_maxDepth)
		, volumn(_volumn)
		, waterHeight(_waterHeight)
		, liquidDensity(_liquidDensity)
	{
	}

	void ParticleBuoyancy::updateForce(Particle * particle, float duration)
	{
		float depth = particle->getPosition().y;

		if(depth >= waterHeight + maxDepth)
		{
			return;
		}

		if(depth <= waterHeight - maxDepth)
		{
			particle->addForce(Vector3(0, volumn * liquidDensity, 0));
			return;
		}

		particle->addForce(Vector3(0, (depth - maxDepth - waterHeight) / (2 * maxDepth) * volumn * liquidDensity, 0));

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleFakeSpring
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleFakeSpring::ParticleFakeSpring(const Vector3 & _anchor, float _springConstant, float _damping)
		: anchor(_anchor)
		, springConstant(_springConstant)
		, damping(_damping)
	{
	}

	void ParticleFakeSpring::updateForce(Particle * particle, float duration)
	{
		float gamma = 0.5f * sqrt(4 * springConstant - damping * damping);

		if(gamma != 0)
		{
			Vector3 position = particle->getPosition() - anchor;

			Vector3 C = position * (damping / (2 * gamma)) + particle->getVelocity() / gamma;

			Vector3 target = position * cos(gamma * duration) + C * sin(gamma * duration);

			target *= exp(-0.5f * damping * duration);

			Vector3 accel = (target - position) / (duration * duration) - particle->getVelocity() /** duration*/;

			particle->addForce(accel * particle->getMass());
		}
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleContact
	// /////////////////////////////////////////////////////////////////////////////////////

	void ParticleContact::resolve(float duration)
	{
		resolveVelocity(duration);

		resolveInterpenetration(duration);
	}

	float ParticleContact::calculateSeparatingVelocity(void) const
	{
		_ASSERT(NULL != particles[0]);

		Vector3 relativeVelocity = particles[0]->getVelocity();

		if(NULL != particles[1])
		{
			relativeVelocity -= particles[2]->getVelocity();
		}

		return relativeVelocity.dot(contactNormal);
	}

	void ParticleContact::resolveVelocity(float duration) // ***
	{
		_ASSERT(NULL != particles[0]);

		float separatingVelocity = calculateSeparatingVelocity();

		if(separatingVelocity >= 0)
		{
			return;
		}

		float newSeparatingVelocity = -restitution * separatingVelocity;

		/*
		 * NOTE: the accCausedSeparatingVelocity is used to solve the resting contact
		 */

		Vector3 accCausedVelocity = particles[0]->getAcceleration();

		if(NULL != particles[1])
		{
			accCausedVelocity += particles[1]->getAcceleration();
		}

		float accCausedSeparatingVelocity = accCausedVelocity.dot(contactNormal) * duration;

		if(accCausedSeparatingVelocity < 0)
		{
			newSeparatingVelocity += restitution * accCausedSeparatingVelocity;

			if(newSeparatingVelocity < 0)
			{
				newSeparatingVelocity = 0;
			}
		}

		float deltaVelocity = newSeparatingVelocity - separatingVelocity; // ***

		float totalInverseMass = particles[0]->getInverseMass();

		if(NULL != particles[1])
		{
			totalInverseMass += particles[1]->getInverseMass();
		}

		if(totalInverseMass <= 0)
		{
			return;
		}

		float impulse = deltaVelocity / totalInverseMass;

		Vector3 impulsePerInverseMass = contactNormal * impulse;

		particles[0]->addVelocity(impulsePerInverseMass * particles[0]->getInverseMass());

		if(NULL != particles[1])
		{
			particles[1]->addVelocity(impulsePerInverseMass * -particles[1]->getInverseMass());
		}

		UNREFERENCED_PARAMETER(duration);
	}

	void ParticleContact::resolveInterpenetration(float duration)
	{
		if(penetration <= 0)
		{
			return;
		}

		float totalMass = particles[0]->getMass();

		if(NULL != particles[1])
		{
			totalMass += particles[1]->getMass();
		}

		if(totalMass <= 0)
		{
			return;
		}

		Vector3 movePerMass = contactNormal * (penetration / totalMass);

		particles[0]->setPosition(particles[0]->getPosition() + movePerMass * particles[0]->getMass());

		if(NULL != particles[1])
		{
			particles[1]->setPosition(particles[1]->getPosition() + movePerMass * -particles[1]->getMass());
		}

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleContactResolver
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleContactResolver::ParticleContactResolver(unsigned _iterations)
		: iterations(_iterations)
	{
	}

	void ParticleContactResolver::setIterations(unsigned _iterations)
	{
		iterations = _iterations;
	}

	unsigned ParticleContactResolver::getIterations(void) const
	{
		return iterations;
	}

	void ParticleContactResolver::resolveContacts(ParticleContact * contactArray, unsigned numContacts, float duration)
	{
		for(iterationsUsed = 0; iterationsUsed < iterations; iterationsUsed++)
		{
			float min = FLT_MAX; // ***

			unsigned minIndex = numContacts;

			for(unsigned i = 0; i < numContacts; i++)
			{
				float separatingVelocity = contactArray[i].calculateSeparatingVelocity();

				if(separatingVelocity < min)
				{
					min = separatingVelocity;

					minIndex = i;
				}
			}

			if(minIndex == numContacts)
			{
				return;
			}

			contactArray[minIndex].resolve(duration);

			/*
			 * NOTE: havent update interpenetrations for all particles yet!
			 */
		}
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleContactGenerator
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleContactGenerator::~ParticleContactGenerator(void)
	{
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleLink
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleLink::ParticleLink(Particle * particle0, Particle * particle1)
	{
		particles[0] = particle0;
		particles[1] = particle1;

		_ASSERT(NULL != particles[0]);
		_ASSERT(NULL != particles[1]);
	}

	float ParticleLink::currentLength() const
	{
		_ASSERT(NULL != particles[0]);
		_ASSERT(NULL != particles[1]);

		return (particles[0]->getPosition() - particles[1]->getPosition()).magnitude();
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleCable
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleCable::ParticleCable(Particle * particle0, Particle * particle1, float _maxLength, float _restitution)
		: ParticleLink(particle0, particle1)
		, maxLength(_maxLength)
		, restitution(_restitution)
	{
	}

	unsigned ParticleCable::addContact(ParticleContact * contacts, unsigned limits) const
	{
		_ASSERT(limits > 0);

		float length = currentLength();

		if(length < maxLength)
		{
			return 0;
		}

		contacts->particles[0] = particles[0];
		contacts->particles[1] = particles[1];

		contacts->contactNormal = (particles[1]->getPosition() - particles[0]->getPosition()).normalize();
		contacts->penetration = length - maxLength;
		contacts->restitution = restitution;
		return 1;
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleRod
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleRod::ParticleRod(Particle * particle0, Particle * particle1, float _length)
		: ParticleLink(particle0, particle1)
		, length(_length)
	{
	}

	unsigned ParticleRod::addContact(ParticleContact * contacts, unsigned limits) const
	{
		_ASSERT(limits > 0);

		float currentLength = ParticleRod::currentLength();

		if(currentLength == length)
		{
			return 0;
		}

		contacts->particles[0] = particles[0];
		contacts->particles[1] = particles[1];

		if(currentLength > length)
		{
			contacts->contactNormal = (particles[1]->getPosition() - particles[0]->getPosition()).normalize();
			contacts->penetration = currentLength - length;
		}
		else
		{
			contacts->contactNormal = (particles[0]->getPosition() - particles[1]->getPosition()).normalize();
			contacts->penetration = length - currentLength;
		}

		contacts->restitution = 0;
		return 1;
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleConstraint
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleConstraint::ParticleConstraint(const Vector3 & _anchor, Particle * _particle)
		: anchor(_anchor)
		, particle(_particle)
	{
		_ASSERT(NULL != particle);
	}

	float ParticleConstraint::currentLength(void) const
	{
		_ASSERT(NULL != particle);

		return (particle->getPosition() - anchor).magnitude();
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleCableConstraint
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleCableConstraint::ParticleCableConstraint(const Vector3 & _anchor, Particle * particle, float _maxLength, float _restitution)
		: ParticleConstraint(_anchor, particle)
		, maxLength(_maxLength)
		, restitution(_restitution)
	{
	}

	unsigned ParticleCableConstraint::addContact(ParticleContact * contacts, unsigned limits) const
	{
		_ASSERT(limits > 0);

		float length = currentLength();

		if(length < maxLength)
		{
			return 0;
		}

		contacts->particles[0] = particle;
		contacts->particles[1] = NULL;

		contacts->contactNormal = (anchor - particle->getPosition()).normalize();
		contacts->penetration = length - maxLength;
		contacts->restitution = restitution;
		return 1;
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleRodConstraint
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleRodConstraint::ParticleRodConstraint(const Vector3 & _anchor, Particle * particle, float _length)
		: ParticleConstraint(_anchor, particle)
		, length(_length)
	{
	}

	unsigned ParticleRodConstraint::addContact(ParticleContact * contacts, unsigned limits) const
	{
		_ASSERT(limits > 0);

		float currentLength = ParticleRodConstraint::currentLength();

		if(currentLength == length)
		{
			return 0;
		}

		contacts->particles[0] = particle;
		contacts->particles[1] = NULL;

		if(currentLength > length)
		{
			contacts->contactNormal = (anchor - particle->getPosition()).normalize();
			contacts->penetration = currentLength - length;
		}
		else
		{
			contacts->contactNormal = (particle->getPosition() - anchor).normalize();
			contacts->penetration = length - currentLength;
		}

		contacts->restitution = 0;
		return 1;
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleWorld
	// /////////////////////////////////////////////////////////////////////////////////////

	void ParticleWorld::startFrame(void)
	{
		ParticlePtrList::iterator p_iter = particleList.begin();
		for(; p_iter != particleList.end(); p_iter++)
		{
			_ASSERT((*p_iter != NULL));

			(*p_iter)->clearAccumulator();
		}
	}

	void ParticleWorld::integrate(float duration)
	{
		ParticlePtrList::iterator p_iter = particleList.begin();
		for(; p_iter != particleList.end(); p_iter++)
		{
			_ASSERT((*p_iter != NULL));

			(*p_iter)->integrate(duration);
		}
	}

	unsigned ParticleWorld::generateContacts(ParticleContact * contacts, unsigned limits)
	{
		_ASSERT(limits > 0);

		unsigned used = 0;

		ParticleContactGeneratorPtrList::const_iterator c_iter = particleContactGeneratorList.begin();
		for(; c_iter != particleContactGeneratorList.end() && limits > used; c_iter++)
		{
			used += (*c_iter)->addContact(&contacts[used], limits - used);
		}

		return used;
	}

	void ParticleWorld::runPhysics(float duration)
	{
		startFrame();

		registry.updateForces(duration);

		integrate(duration);

		unsigned usedContacts = generateContacts(&particleContactArray[0], particleContactArray.size());

		resolver.setIterations(usedContacts * resolveIteration);

		resolver.resolveContacts(&particleContactArray[0], usedContacts, duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// RigidBody
	// /////////////////////////////////////////////////////////////////////////////////////

	static Matrix4 _transformInertiaTensor(
		const Matrix4 & inertiaTensor,
		const Matrix4 & transformMat)
	{
		float t4 =
			transformMat._11 * inertiaTensor._11 +
			transformMat._21 * inertiaTensor._12 +
			transformMat._31 * inertiaTensor._13;

		float t9 =
			transformMat._11 * inertiaTensor._21 +
			transformMat._21 * inertiaTensor._22 +
			transformMat._31 * inertiaTensor._23;

		float t14 =
			transformMat._11 * inertiaTensor._31 +
			transformMat._21 * inertiaTensor._32 +
			transformMat._31 * inertiaTensor._33;

		float t28 =
			transformMat._12 * inertiaTensor._11 +
			transformMat._22 * inertiaTensor._12 +
			transformMat._32 * inertiaTensor._13;

		float t33 =
			transformMat._12 * inertiaTensor._21 +
			transformMat._22 * inertiaTensor._22 +
			transformMat._32 * inertiaTensor._23;

		float t38 =
			transformMat._12 * inertiaTensor._31 +
			transformMat._22 * inertiaTensor._32 +
			transformMat._32 * inertiaTensor._33;

		float t52 =
			transformMat._13 * inertiaTensor._11 +
			transformMat._23 * inertiaTensor._12 +
			transformMat._33 * inertiaTensor._13;

		float t57 =
			transformMat._13 * inertiaTensor._21 +
			transformMat._23 * inertiaTensor._22 +
			transformMat._33 * inertiaTensor._23;

		float t62 =
			transformMat._13 * inertiaTensor._31 +
			transformMat._23 * inertiaTensor._32 +
			transformMat._33 * inertiaTensor._33;

		return Matrix4(
			t4 * transformMat._11 + t9 * transformMat._21 + t14 * transformMat._31, t28 * transformMat._11 + t33 * transformMat._21 + t38 * transformMat._31, t52 * transformMat._11 + t57 * transformMat._21 + t62 * transformMat._31, 0,
			t4 * transformMat._12 + t9 * transformMat._22 + t14 * transformMat._32, t28 * transformMat._12 + t33 * transformMat._22 + t38 * transformMat._32, t52 * transformMat._12 + t57 * transformMat._22 + t62 * transformMat._32, 0,
			t4 * transformMat._13 + t9 * transformMat._23 + t14 * transformMat._33, t28 * transformMat._13 + t33 * transformMat._23 + t38 * transformMat._33, t52 * transformMat._13 + t57 * transformMat._23 + t62 * transformMat._33, 0,
			0, 0, 0, 1);
	}

	void RigidBody::calculateDerivedData(void)
	{
		Matrix4 rotationTransform = Matrix4::RotationQuaternion(orientation.normalizeSelf()); // ***

		transform = rotationTransform.translate(position);

		inverseInertiaTensorWorld = _transformInertiaTensor(inverseInertiaTensor, transform); // ***
	}

	void RigidBody::setAwake(bool _isAwake)
	{
		isAwake = _isAwake;

		if(isAwake)
		{
			motion = 2.0f * getSleepEpsilon();
		}
		else
		{
			setVelocity(Vector3::zero);

			setRotation(Vector3::zero);
		}
	}

	bool RigidBody::getAwake(void) const
	{
		return isAwake;
	}

	void RigidBody::setCanSleep(bool _canSleep)
	{
		canSleep = _canSleep;

		if(!canSleep && !isAwake)
		{
			setAwake();
		}
	}

	bool RigidBody::getCanSleep(void) const
	{
		return canSleep;
	}

	void RigidBody::addForceAtPoint(const Vector3 & force, const Vector3 & point)
	{
		addForce(force);

		addTorque((point - position).cross(force));
	}

	bool RigidBody::hasFiniteMass(void) const
	{
		return inverseMass > 0.0f;
	}

	void RigidBody::integrate(float duration)
	{
		_ASSERT(duration >= 0);

		if(!getAwake())
		{
			return;
		}

		// Calculate linear acceleration from force inputs.
		resultingAcc = acceleration + forceAccum * inverseMass;

		// Calculate angular acceleration from torque inputs.
		resultingAngularAcc = torqueAccum.transformCoord(inverseInertiaTensorWorld);

		// Update linear velocity from both acceleration and impulse.
		addVelocity(resultingAcc * duration);

		// Update angular velocity from both acceleration and impulse.
		addRotation(resultingAngularAcc * duration);

		// Impose drag.
		velocity *= pow(damping, duration);

		rotation *= pow(angularDamping, duration);

		// Update linear position.
		addPosition(velocity * duration);

		// Update angular position.
		addOrientationRH(rotation * duration);

		// Normalise the orientation, and update the matrices with the new
		// position and orientation
		calculateDerivedData();

		// Update the kinetic energy store, and possibly put the body to
		// sleep.
		if(getCanSleep())
		{
			float currentMotion = velocity.magnitudeSq() + rotation.magnitudeSq();

			float bias = pow(0.5f, duration);

			motion = bias * motion + (1 - bias) * currentMotion;

//REPORT_ERROR(str_printf(_T("motion: %.2f"), motion));

			if(motion < sleepEpsilon)
			{
				setAwake(false);
			}
			else if(motion > 10.0f * sleepEpsilon)
			{
				motion = 10.0f * sleepEpsilon;
			}
		}
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ForceRegistry
	// /////////////////////////////////////////////////////////////////////////////////////

	ForceRegistry::ForceRegistration::ForceRegistration(RigidBody * _body, ForceGenerator * _forceGenerator)
		: body(_body)
		, forceGenerator(_forceGenerator)
	{
	}

	void ForceRegistry::add(RigidBody * body, ForceGenerator * forceGenerator)
	{
		forceRegistrationList.push_back(ForceRegistration(body, forceGenerator));
	}

	void ForceRegistry::remove(RigidBody * body, ForceGenerator * forceGenerator)
	{
		ForceRegistrationList::iterator f_iter = forceRegistrationList.begin();
		for(; f_iter != forceRegistrationList.end(); f_iter++)
		{
			if(f_iter->body == body
				&& f_iter->forceGenerator == forceGenerator)
			{
				forceRegistrationList.erase(f_iter);
				return;
			}
		}
	}

	void ForceRegistry::clear(void)
	{
		forceRegistrationList.clear();
	}

	void ForceRegistry::updateForces(float duration)
	{
		ForceRegistrationList::iterator f_iter = forceRegistrationList.begin();
		for(; f_iter != forceRegistrationList.end(); f_iter++)
		{
			f_iter->forceGenerator->updateForce(f_iter->body, duration);
		}
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ForceGenerator
	// /////////////////////////////////////////////////////////////////////////////////////

	ForceGenerator::~ForceGenerator(void)
	{
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// Gravity
	// /////////////////////////////////////////////////////////////////////////////////////

	Gravity::Gravity(const Vector3 & _gravity)
		: gravity(_gravity)
	{
	}

	void Gravity::updateForce(RigidBody * body, float duration)
	{
		if(!body->hasFiniteMass())
			return;

		body->addForce(gravity * body->getMass());

		body->setAwake();

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// Spring
	// /////////////////////////////////////////////////////////////////////////////////////

	Spring::Spring(
		const Vector3 & _connectionPoint,
		RigidBody * _other,
		const Vector3 & _otherConnectionPoint,
		float _springConstant,
		float _restLength)
		: connectionPoint(_connectionPoint)
		, other(_other)
		, otherConnectionPoint(_otherConnectionPoint)
		, springConstant(_springConstant)
		, restLength(_restLength)
	{
	}

	void Spring::updateForce(RigidBody * body, float duration)
	{
		Vector3 connectionPointWorld = connectionPoint.transformCoord(body->getTransform());

		Vector3 otherConnectionPointWorld = otherConnectionPoint.transformCoord(other->getTransform());

		Vector3 distance = connectionPointWorld - otherConnectionPointWorld;

		float length = distance.magnitude();

		body->addForceAtPoint(distance.normalize() * (-springConstant * (length - restLength)), connectionPointWorld);

		body->setAwake();

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// Aero
	// /////////////////////////////////////////////////////////////////////////////////////

	Aero::Aero(
		const Matrix4 & _tensor,
		const Vector3 & _position,
		const Vector3 * _pwindSpeed)
		: tensor(_tensor)
		, position(_position)
		, pwindSpeed(_pwindSpeed)
	{
	}

	void Aero::updateForce(RigidBody * body, float duration)
	{
		updateForceFromTensor(body, duration, tensor);
	}

	void Aero::updateForceFromTensor(RigidBody * body, float duration, const Matrix4 & _tensor)
	{
		Vector3 velocity = body->getVelocity() + *pwindSpeed;

		Vector3 localVelocity = velocity.transformNormalTranspose(body->getTransform());

		Vector3 localForce = localVelocity.transformCoord(_tensor);

		Vector3 force = localForce.transformNormal(body->getTransform());

		body->addForceAtPoint(force, position.transformCoord(body->getTransform()));

		body->setAwake();

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// AeroControl
	// /////////////////////////////////////////////////////////////////////////////////////

	void AeroControl::setControl(float value)
	{
		controlSetting = value;
	}

	AeroControl::AeroControl(
		const Matrix4 & _tensor,
		const Matrix4 & _minTensor,
		const Matrix4 & _maxTensor,
		const Vector3 & _position,
		const Vector3 * _pwindSpeed)
		: Aero(_tensor, _position, _pwindSpeed)
		, minTensor(_minTensor)
		, maxTensor(_maxTensor)
	{
	}

	void AeroControl::updateForce(RigidBody * body, float duration)
	{
		updateForceFromControl(body, duration, controlSetting);
	}

	void AeroControl::updateForceFromControl(RigidBody * body, float duration, float _controlSetting)
	{
		_ASSERT(_controlSetting >= -1 && _controlSetting <= 1);

		if(_controlSetting < 0)
		{
			updateForceFromTensor(body, duration, minTensor.lerp(tensor, 1 + _controlSetting));
		}
		else
		{
			updateForceFromTensor(body, duration, tensor.lerp(maxTensor, _controlSetting));
		}
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// AngledAero
	// /////////////////////////////////////////////////////////////////////////////////////

	AngledAero::AngledAero(
		const Matrix4 & _tensor,
		const Vector3 & _position,
		const Vector3 * _pwindSpeed)
		: Aero(_tensor, _position, _pwindSpeed)
	{
	}

	void AngledAero::updateForce(RigidBody * body, float duration)
	{
		_ASSERT(false);

		UNREFERENCED_PARAMETER(body);
		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// Buoyancy
	// /////////////////////////////////////////////////////////////////////////////////////

	Buoyancy::Buoyancy(
		const Vector3 & _centerOfBuoyancy,
		float _maxDepth,
		float _volume,
		float _waterHeight,
		float _liquidDensity)
		: centerOfBuoyancy(_centerOfBuoyancy)
		, maxDepth(_maxDepth)
		, volume(_volume)
		, waterHeight(_waterHeight)
		, liquidDensity(_liquidDensity)
	{
	}

	void Buoyancy::updateForce(RigidBody * body, float duration)
	{
		_ASSERT(false);

		UNREFERENCED_PARAMETER(body);
		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// Contact
	// /////////////////////////////////////////////////////////////////////////////////////

	void Contact::swapBodies(void)
	{
		contactNormal = -contactNormal;

		std::swap(bodys[0], bodys[1]);
	}

	void Contact::matchAwakeState(void)
	{
		// Collisions with the world never cause a body to wake up.
		if(NULL == bodys[1])
		{
			return;
		}

		// Wake up only the sleeping one
		if(bodys[0]->getAwake())
		{
			if(!bodys[1]->getAwake())
			{
				bodys[1]->setAwake();
			}
		}
		else
		{
			if( bodys[1]->getAwake())
			{
				bodys[0]->setAwake();
			}
		}
	}

	void Contact::calculateContactBasis(void)
	{
		_ASSERT(IS_NORMALIZED(contactNormal));

		Vector3 contantTangents[2];

		// Check whether the Z-axis is nearer to the X or Y axis
		if(abs(contactNormal.x) > abs(contactNormal.y))
		{
			// Scaling factor to ensure the results are normalised
			float s_inv = 1.0f / sqrt(contactNormal.x * contactNormal.x + contactNormal.z * contactNormal.z);

			// The new X-axis is at right angles to the world Y-axis
			contantTangents[0] = Vector3(contactNormal.z * s_inv, 0, -contactNormal.x * s_inv);

			// The new Y-axis is at right angles to the new X- and Z- axes
			contantTangents[1] = contactNormal.cross(contantTangents[0]);
		}
		else
		{
			// Scaling factor to ensure the results are normalised
			float s_inv = 1.0f / sqrt(contactNormal.y * contactNormal.y + contactNormal.z * contactNormal.z);

			// The new X-axis is at right angles to the world X-axis
			contantTangents[0] = Vector3(0, -contactNormal.z * s_inv, contactNormal.y * s_inv);

			// The new Y-axis is at right angles to the new X- and Z- axes
			contantTangents[1] = contactNormal.cross(contantTangents[0]);
		}

		// Make a matrix from the three vectors.
		contactToWorld = Matrix4(
			contactNormal.x,		contactNormal.y,		contactNormal.z,		0,
			contantTangents[0].x,	contantTangents[0].y,	contantTangents[0].z,	0,
			contantTangents[1].x,	contantTangents[1].y,	contantTangents[1].z,	0,
			0,						0,						0,						1);
	}

	Vector3 Contact::calculateLocalVelocity(const RigidBody & body, const Vector3 & relativeContactPosition, float duration) const // ***
	{
		// Work out the velocity of the contact point.
		Vector3 velocity = body.getRotation().cross(relativeContactPosition) + body.getVelocity();

		// Turn the velocity into contact-coordinates.
		Vector3 velocityLocal = velocity.transformCoordTranspose(contactToWorld);

		// Calculate the ammount of velocity that is due to forces without
		// reactions.
		Vector3 accVelocity = body.getResultingAcc() * duration;

		// Calculate the velocity in contact-coordinates.
		Vector3 accVelocityLocal = accVelocity.transformCoordTranspose(contactToWorld);

		// We ignore any component of acceleration in the contact normal
		// direction, we are only interested in planar acceleration
		accVelocityLocal.x = 0;

		// Add the planar velocities - if there's enough friction they will
		// be removed during velocity resolution
		velocityLocal += accVelocityLocal;

		// And return it
		return velocityLocal;
	}

	void Contact::calculateDesiredDeltaVelocity(float duration) // ***
	{
		_ASSERT(NULL != bodys[0]);

		const float velocityLimit = 0.25f;

		// Calculate the acceleration induced velocity accumulated this frame
		float velocityFromAcc = 0;

		if(bodys[0]->getAwake())
		{
			velocityFromAcc = (bodys[0]->getResultingAcc() * duration).dot(contactNormal);
		}

		if(NULL != bodys[1] && bodys[1]->getAwake())
		{
			velocityFromAcc -= (bodys[1]->getResultingAcc() * duration).dot(contactNormal);
		}

		// If the velocity is very slow, limit the restitution
		float thisRestitution = abs(contactVelocity.x) < velocityLimit ? 0.0f : restitution;

		// Combine the bounce velocity with the removed
		// acceleration velocity.
		desiredDeltaVelocity = -contactVelocity.x - thisRestitution * (contactVelocity.x - velocityFromAcc);
	}

	void Contact::calculateInternals(float duration)
	{
		// Check if the first object is NULL, and swap if it is.
		if(NULL == bodys[0])
		{
			swapBodies();
		}

		_ASSERT(NULL != bodys[0]);

		// Calculate an set of axis at the contact point.
		calculateContactBasis();

		// Store the relative position of the contact relative to each body
		relativeContactPositions[0] = contactPoint - bodys[0]->getPosition();

		if(NULL != bodys[1])
		{
			relativeContactPositions[1] = contactPoint - bodys[1]->getPosition();
		}

		// Find the relative velocity of the bodies at the contact point.
		contactVelocity = calculateLocalVelocity(*bodys[0], relativeContactPositions[0], duration);

		if(NULL != bodys[1])
		{
			contactVelocity -= calculateLocalVelocity(*bodys[1], relativeContactPositions[1], duration);
		}

		// Calculate the desired change in velocity for resolution
		calculateDesiredDeltaVelocity(duration);
	}

	Vector3 Contact::calculateFrictionlessImpulse(const Matrix4 inverseInertialTensors[]) const
	{
		_ASSERT(NULL != bodys[0]);

		// Build a vector that shows the change in velocity in
		// world space for a unit impulse in the direction of the contact
		// normal.
		Vector3 deltaVelocityWorld = relativeContactPositions[0].cross(contactNormal).transformCoord(inverseInertialTensors[0]).cross(relativeContactPositions[0]);

		// Work out the change in velocity in contact coordiantes.
		float deltaVelocity = deltaVelocityWorld.dot(contactNormal);

		// Add the linear component of velocity change
		deltaVelocity += bodys[0]->getInverseMass();

		// Check if we need to the second body's data
		if(NULL != bodys[1])
		{
			// Go through the same transformation sequence again
			deltaVelocityWorld = relativeContactPositions[1].cross(contactNormal).transformCoord(inverseInertialTensors[1]).cross(relativeContactPositions[1]);

			// Add the change in velocity due to rotation
			deltaVelocity += deltaVelocityWorld.dot(contactNormal);

			// Add the change in velocity due to linear motion
			deltaVelocity += bodys[1]->getInverseMass();
		}

		// Calculate the required size of the impulse
		return Vector3(desiredDeltaVelocity / deltaVelocity, 0, 0);
	}

	static inline Matrix4 buildSkewSymmetricMatrxi(const Vector3 & rhs)
	{
		return Matrix4(
			 0,		 rhs.z,	-rhs.y,	0,
			-rhs.z,	 0,		 rhs.x,	0,
			 rhs.y,	-rhs.x,	 0,		0,
			 0,		 0,		 0,		1);

	}

	Vector3 Contact::calculateFrictionImpulse(const Matrix4 inverseInertialTensors[]) const // ***
	{
		_ASSERT(NULL != bodys[0]);

		// The equivalent of a cross product in matrices is multiplication
		// by a skew symmetric matrix - we build the matrix for converting
		// between linear and angular quantities.
		Matrix4 impulseToTorque = buildSkewSymmetricMatrxi(relativeContactPositions[0]);

		// Build the matrix to convert contact impulse to change in velocity
		// in world coordinates.
		Matrix4 deltaVelocityWorld = - (impulseToTorque * inverseInertialTensors[0] * impulseToTorque);

		float inverseMass = bodys[0]->getInverseMass();

		// Check if we need to add body two's data
		if(NULL != bodys[1])
		{
			// Set the cross product matrix
			impulseToTorque = buildSkewSymmetricMatrxi(relativeContactPositions[1]);

			// Calculate the velocity change matrix
			// Add to the total delta velocity.
			deltaVelocityWorld += - (impulseToTorque * inverseInertialTensors[1] * impulseToTorque);

			// Add to the inverse mass
			inverseMass += bodys[1]->getInverseMass();
		}

		// Do a change of basis to convert into contact coordinates.
		Matrix4 deltaVelocity = (contactToWorld * deltaVelocityWorld).transformTranspose(contactToWorld); // ***

		// Add in the linear velocity change
		deltaVelocity._11 += inverseMass;
		deltaVelocity._22 += inverseMass;
		deltaVelocity._33 += inverseMass;

		// Invert to get the impulse needed per unit velocity
		Matrix4 impulseMatrix = deltaVelocity.inverse();

		// Find the target velocities to kill
		Vector3 velKill(desiredDeltaVelocity, -contactVelocity.y, -contactVelocity.z);

		// Find the impulse to kill target velocities
		Vector3 impulseContact = velKill.transformCoord(impulseMatrix);

		// Check for exceeding friction
		float planarImpulse = sqrt(impulseContact.y * impulseContact.y + impulseContact.z * impulseContact.z);

		if (planarImpulse > impulseContact.x * friction)
		{
			// We need to use dynamic friction
			impulseContact.y /= planarImpulse;
			impulseContact.z /= planarImpulse;

			impulseContact.x =
				deltaVelocity._11 +
				deltaVelocity._21 * friction * impulseContact.y +
				deltaVelocity._31 * friction * impulseContact.z;

			impulseContact.x = desiredDeltaVelocity / impulseContact.x;
			impulseContact.y *= friction * impulseContact.x;
			impulseContact.z *= friction * impulseContact.x;
		}

		return impulseContact;
	}

	static float _calculateBodyInertias(
		const RigidBody & body,
		const Vector3 & relativeContactPosition,
		const Vector3 & contactNormal,
		float & linearInertia,
		float & angularInertia)
	{
		// Use the same procedure as for calculating frictionless
		// velocity change to work out the angular inertia.
		Vector3 angularInertiaWorld = relativeContactPosition.cross(contactNormal).transformCoord(body.getInverseInertialTensor()).cross(relativeContactPosition);

		angularInertia = angularInertiaWorld.dot(contactNormal);

		// The linear component is simply the inverse mass
		linearInertia = body.getInverseMass();

		// Keep track of the total inertia from all components
		return linearInertia + angularInertia;
	}

	static void _limitAngularMove(
		const Vector3 & relativeContactPosition,
		const Vector3 & contactNormal,
		float & linearMove,
		float & angularMove)
	{
		const float angularLimit = 2.0f;

		// To avoid angular projections that are too great (when mass is large
		// but inertia tensor is small) limit the angular move.
		Vector3 projection = relativeContactPosition + contactNormal * -relativeContactPosition.dot(contactNormal); // ***

		// Use the small angle approximation for the sine of the angle (i.e.
		// the magnitude would be sine(angularLimit) * projection.magnitude
		// but we approximate sine(angularLimit) to angularLimit).
		float maxMagnitude = angularLimit * projection.magnitude();

		float totalMove = linearMove + angularMove;

		angularMove = Clamp(angularMove, -maxMagnitude, maxMagnitude);

		linearMove = totalMove - angularMove;
	}

	static void _applyInertiaToBody(
		RigidBody & body,
		const Vector3 & relativeContactPosition,
		const Vector3 & contactNormal,
		float linearMove,
		float angularMove,
		float angularInertia,
		Vector3 & linearChange,
		Vector3 & angularChange) // ***
	{
		// Velocity change is easier - it is just the linear movement
		// along the contact normal.
		linearChange = contactNormal * linearMove;

		// We have the linear amount of movement required by turning
		// the rigid body (in angularMove[i]). We now need to
		// calculate the desired rotation to achieve that.
		if(0 == angularMove || 0 == body.getAngularDamping()) // *** angularDamping
		{
			// Easy case - no angular movement means no rotation.
			angularChange = Vector3::zero;
		}
		else
		{
			// Work out the direction we'd like to rotate in.
			Vector3 angularPerMove = relativeContactPosition.cross(contactNormal).transformCoord(body.getInverseInertialTensor()) / angularInertia; // ***

			// Work out the direction we'd need to rotate to achieve that
			angularChange = angularPerMove * angularMove;
		}

		// Now we can start to apply the values we've calculated.
		// Apply the linear movement
		body.addPosition(linearChange);

		// And the change in orientation
		body.addOrientationRH(angularChange);

		// BUG FIX:
		// too much call RigidBody::addOrientation(...) to one body will lead float overflow
		// this will occur when 'positionIterations' was much large, so need normalized here
		body.setOrientation(body.getOrientation().normalize()); // ***

		// We need to calculate the derived data for any body that is
		// asleep, so that the changes are reflected in the object's
		// data. Otherwise the resolution will not change the position
		// of the object, and the next collision detection round will
		// have the same penetration.
		if(!body.getAwake())
		{
			body.calculateDerivedData();
		}
	}

	void Contact::applyPositionChange(Vector3 linearChanges[2], Vector3 angularChanges[2])
	{
		_ASSERT(NULL != bodys[0]);

		float linearInertias[2];
		float angularInertias[2];

		// We need to work out the inertia of each object in the direction
		// of the contact normal, due to angular inertia only.
		float totalInertia = _calculateBodyInertias(*bodys[0], relativeContactPositions[0], contactNormal, linearInertias[0], angularInertias[0]);

		if(NULL != bodys[1])
		{
			totalInertia += _calculateBodyInertias(*bodys[1], relativeContactPositions[1], contactNormal, linearInertias[1], angularInertias[1]);
		}

		// The linear and angular movements required are in proportion to
		// the two inverse inertias.
		float linearMoves[2];
		float angularMoves[2];
		linearMoves[0] = penetration * linearInertias[0] / totalInertia;
		angularMoves[0] = penetration * angularInertias[0] / totalInertia;

		_limitAngularMove(relativeContactPositions[0], contactNormal, linearMoves[0], angularMoves[0]);

		if(NULL != bodys[1])
		{
			linearMoves[1] = -penetration * linearInertias[1] / totalInertia;
			angularMoves[1] = -penetration * angularInertias[1] / totalInertia;

			_limitAngularMove(relativeContactPositions[1], contactNormal, linearMoves[1], angularMoves[1]);
		}

		_applyInertiaToBody(*bodys[0], relativeContactPositions[0], contactNormal, linearMoves[0], angularMoves[0], angularInertias[0], linearChanges[0], angularChanges[0]); // ***

		if(NULL != bodys[1])
		{
			_applyInertiaToBody(*bodys[1], relativeContactPositions[1], contactNormal, linearMoves[1], angularMoves[1], angularInertias[1], linearChanges[1], angularChanges[1]); // ***
		}
	}

	void Contact::applyVelocityChange(Vector3 velocityChanges[2], Vector3 rotationChanges[2])
	{
		_ASSERT(NULL != bodys[0]);

		// Get hold of the inverse mass and inverse inertia tensor, both in
		// world coordinates.
		Matrix4 inverseInertiaTensors[2];

		inverseInertiaTensors[0] = bodys[0]->getInverseInertialTensor();

		if(NULL != bodys[1])
		{
			inverseInertiaTensors[1] = bodys[1]->getInverseInertialTensor();
		}

		// We will calculate the impulse for each contact axis
		Vector3 impulseContact;
		
		if(0 == friction)
		{
			// Use the short format for frictionless contacts
			impulseContact = calculateFrictionlessImpulse(inverseInertiaTensors);
		}
		else
		{
			// Otherwise we may have impulses that aren't in the direction of the
			// contact, so we need the more complex version.
			impulseContact = calculateFrictionImpulse(inverseInertiaTensors);
		}

		// Convert impulse to world coordinates
		Vector3 impulse = impulseContact.transformCoord(contactToWorld);

		// Split in the impulse into linear and rotational components
		velocityChanges[0] = impulse * bodys[0]->getInverseMass();

		rotationChanges[0] = relativeContactPositions[0].cross(impulse).transformCoord(inverseInertiaTensors[0]);

		// Apply the changes
		bodys[0]->addVelocity(velocityChanges[0]);

		bodys[0]->addRotation(rotationChanges[0]);

		if(NULL != bodys[1])
		{
			// Work out body one's linear and angular changes
			velocityChanges[1] = impulse * -bodys[1]->getInverseMass();

			rotationChanges[1] = impulse.cross(relativeContactPositions[1]).transformCoord(inverseInertiaTensors[1]);

			// And apply them.
			bodys[1]->addVelocity(velocityChanges[1]);

			bodys[1]->addRotation(rotationChanges[1]);
		}
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ContactResolver
	// /////////////////////////////////////////////////////////////////////////////////////

	ContactResolver::ContactResolver(
		unsigned _positionIterations,
		unsigned _velocityIterations,
		float _positionEpsilon,
		float _velocityEpsilon)
		: positionIterations(_positionIterations)
		, velocityIterations(_velocityIterations)
		, positionEpsilon(_positionEpsilon)
		, velocityEpsilon(_velocityEpsilon)
	{
	}

	void ContactResolver::prepareContacts(
		Contact * contacts,
		unsigned numContacts,
		float duration)
	{
		for(unsigned i = 0; i < numContacts; i++)
		{
			contacts[i].calculateInternals(duration);
		}
	}

	void ContactResolver::_updateContactPenetration(
		Contact & contact,
		const Vector3 & relativeContactPosition,
		const Vector3 & linearChange,
		const Vector3 & angularChange,
		unsigned bodyIndex)
	{
		Vector3 deltaPosition = linearChange + angularChange.cross(relativeContactPosition);

		_ASSERT(IS_NORMALIZED(contact.contactNormal));

		if(0 == bodyIndex)
		{
			contact.penetration -= deltaPosition.dot(contact.contactNormal);
		}
		else
		{
			// The sign of the change is positive if we're
			// dealing with the second body in a contact
			// and negative otherwise (because we're
			// subtracting the resolution)..
			_ASSERT(1 == bodyIndex);
			contact.penetration += deltaPosition.dot(contact.contactNormal);
		}
	}

	void ContactResolver::adjustPositions(
		Contact * contacts,
		unsigned numContacts,
		float duration)
	{
		// iteratively resolve interpenetrations in order of severity
		positionIterationsUsed = 0;
		while(positionIterationsUsed < positionIterations)
		{
			// Find biggest penetration
			float maxPenetration = positionEpsilon;
			unsigned maxPenetrationIndex = numContacts;
			for(unsigned i = 0; i < numContacts; i++)
			{
				if(contacts[i].penetration > maxPenetration)
				{
					maxPenetration = contacts[i].penetration;
					maxPenetrationIndex = i;
				}
			}

			if(maxPenetrationIndex == numContacts)
			{
				break;
			}

			// Match the awake state at the contact
			contacts[maxPenetrationIndex].matchAwakeState();

			// Resolve the penetration
			Vector3 linearChanges[2];
			Vector3 angularChanges[2];
			contacts[maxPenetrationIndex].applyPositionChange(
				linearChanges,
				angularChanges);

			// Again this action may have changed the penetration of other bodies,
			// so we update contacts
			for(unsigned i = 0; i < numContacts; i++)
			{
				if(contacts[i].bodys[0] == contacts[maxPenetrationIndex].bodys[0])
				{
					_updateContactPenetration(
						contacts[i],
						contacts[i].relativeContactPositions[0],
						linearChanges[0],
						angularChanges[0],
						0);
				}
			}

			if(NULL != contacts[maxPenetrationIndex].bodys[1])
			{
				for(unsigned i = 0; i < numContacts; i++)
				{
					if(contacts[maxPenetrationIndex].bodys[1] == contacts[i].bodys[1])
					{
						_updateContactPenetration(
							contacts[i],
							contacts[i].relativeContactPositions[1],
							linearChanges[1],
							angularChanges[1],
							1);
					}
				}
			}

			// increase iterator
			positionIterationsUsed++;
		}
//REPORT_ERROR(str_printf(_T("positionIterationsUsed = %u"), positionIterationsUsed));

		UNREFERENCED_PARAMETER(duration);
	}

	void ContactResolver::_updateContactVelocity(
		Contact & contact,
		const Vector3 & relativeContactPosition,
		const Vector3 & velocityChange,
		const Vector3 & rotationChange,
		unsigned bodyIndex,
		float duration)
	{
		Vector3 deltaVelocity = velocityChange + rotationChange.cross(relativeContactPosition);

		if(0 == bodyIndex)
		{
			contact.contactVelocity += deltaVelocity.transformCoordTranspose(contact.contactToWorld);
		}
		else
		{
			// The sign of the change is negative if we're dealing
			// with the second body in a contact.
			_ASSERT(1 == bodyIndex);
			contact.contactVelocity -= deltaVelocity.transformCoordTranspose(contact.contactToWorld);
		}

		contact.calculateDesiredDeltaVelocity(duration);
	}

	void ContactResolver::adjustVelocities(
		Contact * contacts,
		unsigned numContacts,
		float duration)
	{
		// iteratively handle impacts in order of severity
		velocityIterationsUsed = 0;
		while(velocityIterationsUsed < velocityIterations)
		{
			// Find contact with maximum magnitude of probable velocity change
			float maxVelocity = velocityEpsilon;
			unsigned maxVelocityIndex = numContacts;
			for(unsigned i = 0; i < numContacts; i++)
			{
				if(contacts[i].desiredDeltaVelocity > maxVelocity)
				{
					maxVelocity = contacts[i].desiredDeltaVelocity;
					maxVelocityIndex = i;
				}
			}

			if(maxVelocityIndex == numContacts)
			{
				break;
			}

			// Match the awake state at the contact
			contacts[maxVelocityIndex].matchAwakeState();

			// Do the resolution on the contact that came out top
			Vector3 velocityChanges[2];
			Vector3 rotationChanges[2];
			contacts[maxVelocityIndex].applyVelocityChange(
				velocityChanges,
				rotationChanges);

			// With the change in velocity of the two bodies,
			// the update of contact velocities means
			// that some of the relative closing velocities need recomputing
			for(unsigned i = 0; i < numContacts; i++)
			{
				for(unsigned j = 0; j < 2; j++)
				{
					if(contacts[maxVelocityIndex].bodys[0] == contacts[i].bodys[j])
					{
						_updateContactVelocity(
							contacts[i],
							contacts[i].relativeContactPositions[j],
							velocityChanges[0],
							rotationChanges[0],
							j,
							duration);
					}
				}
			}

			if(NULL != contacts[maxVelocityIndex].bodys[1])
			{
				for(unsigned i = 0; i < numContacts; i++)
				{
					for(unsigned j = 0; j < 2; j++)
					{
						if(contacts[maxVelocityIndex].bodys[1] == contacts[i].bodys[j])
						{
							_updateContactVelocity(
								contacts[i],
								contacts[i].relativeContactPositions[j],
								velocityChanges[1],
								rotationChanges[1],
								j,
								duration);
						}
					}
				}
			}

			// increase iterator
			velocityIterationsUsed++;
		}
//REPORT_ERROR(str_printf(_T("velocityIterationsUsed = %u"), velocityIterationsUsed));
	}

	void ContactResolver::resolveContacts(
		Contact * contacts,
		unsigned numContacts,
		float duration)
	{
		if(numContacts > 0)
		{
			prepareContacts(contacts, numContacts, duration);

			adjustPositions(contacts, numContacts, duration);

			adjustVelocities(contacts, numContacts, duration);
		}
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// World
	// /////////////////////////////////////////////////////////////////////////////////////

	void World::startFrame(void)
	{
		RigidBodyPtrList::iterator b_iter = bodyList.begin();
		for(; b_iter != bodyList.end(); b_iter++)
		{
			_ASSERT((*b_iter != NULL));

			(*b_iter)->clearAccumulator();

			(*b_iter)->clearTorqueAccumulator();
		}
	}

	void World::integrate(float duration)
	{
		RigidBodyPtrList::iterator b_iter = bodyList.begin();
		for(; b_iter != bodyList.end(); b_iter++)
		{
			_ASSERT((*b_iter != NULL));

			(*b_iter)->integrate(duration);
		}
	}

	unsigned World::generateContacts(Contact * contacts, unsigned limits)
	{
		return 0;
	}

	void World::runPhysics(float duration)
	{
		startFrame();

		registry.updateForces(duration);

		integrate(duration);

		unsigned usedContacts = generateContacts(&contactList[0], contactList.size());

		resolver.setPositionIterations(usedContacts * resolvePositionIteration);

		resolver.setVelocityIterations(usedContacts * resolveVelocityIteration);

		resolver.resolveContacts(&contactList[0], usedContacts, duration);
	}
}
