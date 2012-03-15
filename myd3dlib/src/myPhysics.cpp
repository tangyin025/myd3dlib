
#include "stdafx.h"
#include "myPhysics.h"
#include "myGame.h"
#include "libc.h"

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) (P)
#endif

namespace my
{
	// /////////////////////////////////////////////////////////////////////////////////////
	// Particle
	// /////////////////////////////////////////////////////////////////////////////////////

	Particle::Particle(void)
	{
		setPosition(my::Vec4<real>::ZERO);
		setVelocity(my::Vec4<real>::ZERO);
		setAcceleration(my::Vec4<real>::ZERO);
		clearAccumulator();
		setDamping(1);
		setInverseMass(1);
	}

	bool Particle::hasFiniteMass(void) const
	{
		return inverseMass > 0.0f;
	}

	void Particle::integrate(real duration)
	{
		_ASSERT(duration >= 0);

		t3d::Vec4<real> resultingAcc = t3d::vec3Add(acceleration, t3d::vec3Mul(forceAccum, inverseMass));

		addVelocity(t3d::vec3Mul(resultingAcc, duration));		

		t3d::vec3MulSelf(velocity, pow(damping, duration));

		addPosition(t3d::vec3Mul(velocity, duration));
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

	void ParticleForceRegistry::updateForces(real duration)
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

	ParticleGravity::ParticleGravity(const t3d::Vec4<real> & _gravity)
		: gravity(_gravity)
	{
	}

	void ParticleGravity::updateForce(Particle * particle, real duration)
	{
		if(!particle->hasFiniteMass())
			return;

		particle->addForce(t3d::vec3Mul(gravity, particle->getMass()));

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleDrag
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleDrag::ParticleDrag(real _k1, real _k2)
		: k1(_k1)
		, k2(_k2)
	{
	}

	void ParticleDrag::updateForce(Particle * particle, real duration)
	{
		t3d::Vec4<real> velocity = particle->getVelocity();

		real len = t3d::vec3Length(velocity);

		real coefficient = k1 * len + k2 * len * len;

		particle->addForce(t3d::vec3Mul(t3d::vec3Normalize(velocity), -coefficient));

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleSpring
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleSpring::ParticleSpring(Particle * _other, real _springConstant, real _restLength)
		: other(_other)
		, springConstant(_springConstant)
		, restLength(_restLength)
	{
	}

	void ParticleSpring::updateForce(Particle * particle, real duration)
	{
		t3d::Vec4<real> distance = t3d::vec3Sub(particle->getPosition(), other->getPosition());

		real length = t3d::vec3Length(distance);

		particle->addForce(t3d::vec3Mul(t3d::vec3Normalize(distance), -springConstant * (length - restLength)));

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleAnchoredSpring
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleAnchoredSpring::ParticleAnchoredSpring(const t3d::Vec4<real> & _anchor, real _springConstant, real _restLength)
		: anchor(_anchor)
		, springConstant(_springConstant)
		, restLength(_restLength)
	{
	}

	void ParticleAnchoredSpring::updateForce(Particle * particle, real duration)
	{
		t3d::Vec4<real> distance = t3d::vec3Sub(particle->getPosition(), anchor);

		real length = t3d::vec3Length(distance);

		particle->addForce(t3d::vec3Mul(t3d::vec3Normalize(distance), -springConstant * (length - restLength)));

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleBungee
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleBungee::ParticleBungee(Particle * _other, real _springConstant, real _restLength)
		: other(_other)
		, springConstant(_springConstant)
		, restLength(_restLength)
	{
	}

	void ParticleBungee::updateForce(Particle * particle, real duration)
	{
		t3d::Vec4<real> distance = t3d::vec3Sub(particle->getPosition(), other->getPosition());

		real length = t3d::vec3Length(distance);

		if(length > restLength)
		{
			particle->addForce(t3d::vec3Mul(t3d::vec3Normalize(distance), -springConstant * (length - restLength)));
		}

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleAnchoredBungee
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleAnchoredBungee::ParticleAnchoredBungee(const t3d::Vec4<real> & _anchor, real _springConstant, real _restLength)
		: anchor(_anchor)
		, springConstant(_springConstant)
		, restLength(_restLength)
	{
	}

	void ParticleAnchoredBungee::updateForce(Particle * particle, real duration)
	{
		t3d::Vec4<real> distance = t3d::vec3Sub(particle->getPosition(), anchor);

		real length = t3d::vec3Length(distance);

		if(length > restLength)
		{
			particle->addForce(t3d::vec3Mul(t3d::vec3Normalize(distance), -springConstant * (length - restLength)));
		}

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleBuoyancy
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleBuoyancy::ParticleBuoyancy(real _maxDepth, real _volumn, real _waterHeight, real _liquidDensity /*= 1000.0f*/)
		: maxDepth(_maxDepth)
		, volumn(_volumn)
		, waterHeight(_waterHeight)
		, liquidDensity(_liquidDensity)
	{
	}

	void ParticleBuoyancy::updateForce(Particle * particle, real duration)
	{
		real depth = particle->getPosition().y;

		if(depth >= waterHeight + maxDepth)
		{
			return;
		}

		if(depth <= waterHeight - maxDepth)
		{
			particle->addForce(my::Vec4<real>(0, volumn * liquidDensity, 0));
			return;
		}

		particle->addForce(my::Vec4<real>(0, (depth - maxDepth - waterHeight) / (2 * maxDepth) * volumn * liquidDensity, 0));

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleFakeSpring
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleFakeSpring::ParticleFakeSpring(const t3d::Vec4<real> & _anchor, real _springConstant, real _damping)
		: anchor(_anchor)
		, springConstant(_springConstant)
		, damping(_damping)
	{
	}

	void ParticleFakeSpring::updateForce(Particle * particle, real duration)
	{
		real gamma = 0.5f * sqrt(4 * springConstant - damping * damping);

		if(gamma != 0)
		{
			t3d::Vec4<real> position = t3d::vec3Sub(particle->getPosition(), anchor);

			t3d::Vec4<real> C = t3d::vec3Add(
				t3d::vec3Mul(position, damping / (2 * gamma)), t3d::vec3Mul(particle->getVelocity(), 1 / gamma));

			t3d::Vec4<real> target = t3d::vec3Add(
				t3d::vec3Mul(position, cos(gamma * duration)), t3d::vec3Mul(C, sin(gamma * duration)));

			t3d::vec3MulSelf(target, exp(-0.5f * damping * duration));

			t3d::Vec4<real> accel = t3d::vec3Sub(
				t3d::vec3Mul(t3d::vec3Sub(target, position), 1 / (duration * duration)), particle->getVelocity() /** duration*/);

			particle->addForce(t3d::vec3Mul(accel, particle->getMass()));
		}
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleContact
	// /////////////////////////////////////////////////////////////////////////////////////

	void ParticleContact::resolve(real duration)
	{
		resolveVelocity(duration);

		resolveInterpenetration(duration);
	}

	real ParticleContact::calculateSeparatingVelocity(void) const
	{
		_ASSERT(NULL != particles[0]);

		t3d::Vec4<real> relativeVelocity = particles[0]->getVelocity();

		if(NULL != particles[1])
		{
			t3d::vec3SubSelf(relativeVelocity, particles[2]->getVelocity());
		}

		return t3d::vec3Dot(relativeVelocity, contactNormal);
	}

	void ParticleContact::resolveVelocity(real duration) // ***
	{
		_ASSERT(NULL != particles[0]);

		real separatingVelocity = calculateSeparatingVelocity();

		if(separatingVelocity >= 0)
		{
			return;
		}

		real newSeparatingVelocity = -restitution * separatingVelocity;

		/*
		 * NOTE: the accCausedSeparatingVelocity is used to solve the resting contact
		 */

		t3d::Vec4<real> accCausedVelocity = particles[0]->getAcceleration();

		if(NULL != particles[1])
		{
			t3d::vec3AddSelf(accCausedVelocity, particles[1]->getAcceleration());
		}

		real accCausedSeparatingVelocity = t3d::vec3Dot(accCausedVelocity, contactNormal) * duration;

		if(accCausedSeparatingVelocity < 0)
		{
			newSeparatingVelocity += restitution * accCausedSeparatingVelocity;

			if(newSeparatingVelocity < 0)
			{
				newSeparatingVelocity = 0;
			}
		}

		real deltaVelocity = newSeparatingVelocity - separatingVelocity; // ***

		real totalInverseMass = particles[0]->getInverseMass();

		if(NULL != particles[1])
		{
			totalInverseMass += particles[1]->getInverseMass();
		}

		if(totalInverseMass <= 0)
		{
			return;
		}

		real impulse = deltaVelocity / totalInverseMass;

		t3d::Vec4<real> impulsePerInverseMass = t3d::vec3Mul(contactNormal, impulse);

		particles[0]->addVelocity(t3d::vec3Mul(impulsePerInverseMass, particles[0]->getInverseMass()));

		if(NULL != particles[1])
		{
			particles[1]->addVelocity(t3d::vec3Mul(impulsePerInverseMass, -particles[1]->getInverseMass()));
		}

		UNREFERENCED_PARAMETER(duration);
	}

	void ParticleContact::resolveInterpenetration(real duration)
	{
		if(penetration <= 0)
		{
			return;
		}

		real totalMass = particles[0]->getMass();

		if(NULL != particles[1])
		{
			totalMass += particles[1]->getMass();
		}

		if(totalMass <= 0)
		{
			return;
		}

		t3d::Vec4<real> movePerMass = t3d::vec3Mul(contactNormal, penetration * (1 / totalMass));

		particles[0]->setPosition(t3d::vec3Add(
			particles[0]->getPosition(), t3d::vec3Mul(movePerMass, particles[0]->getMass())));

		if(NULL != particles[1])
		{
			particles[1]->setPosition(t3d::vec3Add(
				particles[1]->getPosition(), t3d::vec3Mul(movePerMass, -particles[1]->getMass())));
		}

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleContactResolver
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleContactResolver::ParticleContactResolver(unsigned _iterations /*= 0*/)
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

	void ParticleContactResolver::resolveContacts(ParticleContact * contactArray, unsigned numContacts, real duration)
	{
		for(iterationsUsed = 0; iterationsUsed < iterations; iterationsUsed++)
		{
			real min = REAL_MAX; // ***

			unsigned minIndex = numContacts;

			for(unsigned i = 0; i < numContacts; i++)
			{
				real separatingVelocity = contactArray[i].calculateSeparatingVelocity();

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

	real ParticleLink::currentLength() const
	{
		_ASSERT(NULL != particles[0]);
		_ASSERT(NULL != particles[1]);

		return t3d::vec3Length(t3d::vec3Sub(particles[0]->getPosition(), particles[1]->getPosition()));
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleCable
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleCable::ParticleCable(Particle * particle0, Particle * particle1, real _maxLength, real _restitution)
		: ParticleLink(particle0, particle1)
		, maxLength(_maxLength)
		, restitution(_restitution)
	{
	}

	unsigned ParticleCable::addContact(ParticleContact * contacts, unsigned limits) const
	{
		_ASSERT(limits > 0);

		real length = currentLength();

		if(length < maxLength)
		{
			return 0;
		}

		contacts->particles[0] = particles[0];
		contacts->particles[1] = particles[1];

		contacts->contactNormal = t3d::vec3Normalize(
			t3d::vec3Sub(particles[1]->getPosition(), particles[0]->getPosition()));
		contacts->penetration = length - maxLength;
		contacts->restitution = restitution;
		return 1;
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleRod
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleRod::ParticleRod(Particle * particle0, Particle * particle1, real _length)
		: ParticleLink(particle0, particle1)
		, length(_length)
	{
	}

	unsigned ParticleRod::addContact(ParticleContact * contacts, unsigned limits) const
	{
		_ASSERT(limits > 0);

		real currentLength = ParticleRod::currentLength();

		if(currentLength == length)
		{
			return 0;
		}

		contacts->particles[0] = particles[0];
		contacts->particles[1] = particles[1];

		if(currentLength > length)
		{
			contacts->contactNormal = t3d::vec3Normalize(
				t3d::vec3Sub(particles[1]->getPosition(), particles[0]->getPosition()));
			contacts->penetration = currentLength - length;
		}
		else
		{
			contacts->contactNormal = t3d::vec3Normalize(
				t3d::vec3Sub(particles[0]->getPosition(), particles[1]->getPosition()));
			contacts->penetration = length - currentLength;
		}

		contacts->restitution = 0;
		return 1;
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleConstraint
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleConstraint::ParticleConstraint(const t3d::Vec4<real> & _anchor, Particle * _particle)
		: anchor(_anchor)
		, particle(_particle)
	{
		_ASSERT(NULL != particle);
	}

	real ParticleConstraint::currentLength(void) const
	{
		_ASSERT(NULL != particle);

		return t3d::vec3Length(t3d::vec3Sub(particle->getPosition(), anchor));
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleCableConstraint
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleCableConstraint::ParticleCableConstraint(const t3d::Vec4<real> & _anchor, Particle * particle, real _maxLength, real _restitution)
		: ParticleConstraint(_anchor, particle)
		, maxLength(_maxLength)
		, restitution(_restitution)
	{
	}

	unsigned ParticleCableConstraint::addContact(ParticleContact * contacts, unsigned limits) const
	{
		_ASSERT(limits > 0);

		real length = currentLength();

		if(length < maxLength)
		{
			return 0;
		}

		contacts->particles[0] = particle;
		contacts->particles[1] = NULL;

		contacts->contactNormal = t3d::vec3Normalize(
			t3d::vec3Sub(anchor, particle->getPosition()));
		contacts->penetration = length - maxLength;
		contacts->restitution = restitution;
		return 1;
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleRodConstraint
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleRodConstraint::ParticleRodConstraint(const t3d::Vec4<real> & _anchor, Particle * particle, real _length)
		: ParticleConstraint(_anchor, particle)
		, length(_length)
	{
	}

	unsigned ParticleRodConstraint::addContact(ParticleContact * contacts, unsigned limits) const
	{
		_ASSERT(limits > 0);

		real currentLength = ParticleRodConstraint::currentLength();

		if(currentLength == length)
		{
			return 0;
		}

		contacts->particles[0] = particle;
		contacts->particles[1] = NULL;

		if(currentLength > length)
		{
			contacts->contactNormal = t3d::vec3Normalize(
				t3d::vec3Sub(anchor, particle->getPosition()));
			contacts->penetration = currentLength - length;
		}
		else
		{
			contacts->contactNormal = t3d::vec3Normalize(
				t3d::vec3Sub(particle->getPosition(), anchor));
			contacts->penetration = length - currentLength;
		}

		contacts->restitution = 0;
		return 1;
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ParticleWorld
	// /////////////////////////////////////////////////////////////////////////////////////

	ParticleWorld::ParticleWorld(unsigned _maxContacts /*= 256*/, unsigned _iterations /*= 0*/)
		: resolver(_iterations)
		, particleContactArray(_maxContacts)
		, maxContacts(_maxContacts)
	{
	}

	ParticleWorld::~ParticleWorld(void)
	{
	}

	void ParticleWorld::startFrame(void)
	{
		ParticlePtrList::iterator p_iter = particleList.begin();
		for(; p_iter != particleList.end(); p_iter++)
		{
			_ASSERT((*p_iter != NULL));

			(*p_iter)->clearAccumulator();
		}
	}

	void ParticleWorld::integrate(real duration)
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

	void ParticleWorld::runPhysics(real duration)
	{
		startFrame();

		registry.updateForces(duration);

		integrate(duration);

		unsigned used = generateContacts(&particleContactArray[0], maxContacts);

		resolver.setIterations(used * 2);

		resolver.resolveContacts(&particleContactArray[0], used, duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// RigidBody
	// /////////////////////////////////////////////////////////////////////////////////////

	RigidBody::RigidBody(void)
	{
		setInverseMass(1);
		setPosition(my::Vec4<real>::ZERO);
		setOrientation(my::Quat<real>::IDENTITY);
		setVelocity(my::Vec4<real>::ZERO);
		setRotation(my::Vec4<real>::ZERO);
		//t3d::Mat4<real> transform;
		//t3d::Mat4<real> rotationTransform;
		setInverseInertialTensor(my::Mat4<real>::IDENTITY);
		//t3d::Mat4<real> inverseInertiaTensorWorld;
		setAcceleration(my::Vec4<real>::ZERO);
		clearAccumulator();
		clearTorqueAccumulator();
		//t3d::Vec4<real> resultingAcc;
		//t3d::Vec4<real> resultingAngularAcc;
		setDamping(1);
		setAngularDamping(1);
		setSleepEpsilon(0.3);
		//real motion;
		setAwake(true);
		setCanSleep(true);
		calculateDerivedData();
	}

	static t3d::Mat4<real> _transformInertiaTensor(
		const t3d::Mat4<real> & inertiaTensor,
		const t3d::Mat4<real> & transformMat)
	{
		real t4 =
			transformMat.m00 * inertiaTensor.m00 +
			transformMat.m10 * inertiaTensor.m01 +
			transformMat.m20 * inertiaTensor.m02;

		real t9 =
			transformMat.m00 * inertiaTensor.m10 +
			transformMat.m10 * inertiaTensor.m11 +
			transformMat.m20 * inertiaTensor.m12;

		real t14 =
			transformMat.m00 * inertiaTensor.m20 +
			transformMat.m10 * inertiaTensor.m21 +
			transformMat.m20 * inertiaTensor.m22;

		real t28 =
			transformMat.m01 * inertiaTensor.m00 +
			transformMat.m11 * inertiaTensor.m01 +
			transformMat.m21 * inertiaTensor.m02;

		real t33 =
			transformMat.m01 * inertiaTensor.m10 +
			transformMat.m11 * inertiaTensor.m11 +
			transformMat.m21 * inertiaTensor.m12;

		real t38 =
			transformMat.m01 * inertiaTensor.m20 +
			transformMat.m11 * inertiaTensor.m21 +
			transformMat.m21 * inertiaTensor.m22;

		real t52 =
			transformMat.m02 * inertiaTensor.m00 +
			transformMat.m12 * inertiaTensor.m01 +
			transformMat.m22 * inertiaTensor.m02;

		real t57 =
			transformMat.m02 * inertiaTensor.m10 +
			transformMat.m12 * inertiaTensor.m11 +
			transformMat.m22 * inertiaTensor.m12;

		real t62 =
			transformMat.m02 * inertiaTensor.m20 +
			transformMat.m12 * inertiaTensor.m21 +
			transformMat.m22 * inertiaTensor.m22;

		return t3d::Mat4<real>(
			t4 * transformMat.m00 + t9 * transformMat.m10 + t14 * transformMat.m20, t28 * transformMat.m00 + t33 * transformMat.m10 + t38 * transformMat.m20, t52 * transformMat.m00 + t57 * transformMat.m10 + t62 * transformMat.m20, 0,
			t4 * transformMat.m01 + t9 * transformMat.m11 + t14 * transformMat.m21, t28 * transformMat.m01 + t33 * transformMat.m11 + t38 * transformMat.m21, t52 * transformMat.m01 + t57 * transformMat.m11 + t62 * transformMat.m21, 0,
			t4 * transformMat.m02 + t9 * transformMat.m12 + t14 * transformMat.m22, t28 * transformMat.m02 + t33 * transformMat.m12 + t38 * transformMat.m22, t52 * transformMat.m02 + t57 * transformMat.m12 + t62 * transformMat.m22, 0,
			0, 0, 0, 1);
	}

	void RigidBody::calculateDerivedData(void)
	{
		rotationTransform = t3d::buildRotationMatrixFromQuatLH(orientation.normalizeSelf()); // ***

		transform = rotationTransform * t3d::mat3Mov(position);

		inverseInertiaTensorWorld = _transformInertiaTensor(inverseInertiaTensor, transform); // ***
	}

	void RigidBody::setAwake(bool _isAwake /*= true*/)
	{
		isAwake = _isAwake;

		if(isAwake)
		{
			motion = 2.0f * getSleepEpsilon();
		}
		else
		{
			setVelocity(my::Vec4<real>::ZERO);

			setRotation(my::Vec4<real>::ZERO);
		}
	}

	bool RigidBody::getAwake(void) const
	{
		return isAwake;
	}

	void RigidBody::setCanSleep(bool _canSleep /*= true*/)
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

	void RigidBody::addForceAtPoint(const t3d::Vec4<real> & force, const t3d::Vec4<real> & point)
	{
		addForce(force);

		addTorque(t3d::vec3Cross(t3d::vec3Sub(point, position), force));
	}

	bool RigidBody::hasFiniteMass(void) const
	{
		return inverseMass > 0.0f;
	}

	void RigidBody::integrate(real duration)
	{
		_ASSERT(duration >= 0);

		if(!getAwake())
		{
			return;
		}

		// Calculate linear acceleration from force inputs.
		resultingAcc = t3d::vec3Add(acceleration, t3d::vec3Mul(forceAccum, inverseMass));

		// Calculate angular acceleration from torque inputs.
		resultingAngularAcc = torqueAccum * inverseInertiaTensorWorld;

		// Update linear velocity from both acceleration and impulse.
		addVelocity(t3d::vec3Mul(resultingAcc, duration));

		// Update angular velocity from both acceleration and impulse.
		addRotation(t3d::vec3Mul(resultingAngularAcc, duration));

		// Impose drag.
		t3d::vec3MulSelf(velocity, pow(damping, duration));

		t3d::vec3MulSelf(rotation, pow(angularDamping, duration));

		// Update linear position.
		addPosition(t3d::vec3Mul(velocity, duration));

		// Update angular position.
		addOrientation(t3d::vec3Mul(rotation, duration));

		// Normalise the orientation, and update the matrices with the new
		// position and orientation
		calculateDerivedData();

		// Update the kinetic energy store, and possibly put the body to
		// sleep.
		if(getCanSleep())
		{
			real currentMotion = t3d::vec3LengthSquare(velocity) + t3d::vec3LengthSquare(rotation);

			real bias = pow((real)0.5, duration);

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

	void ForceRegistry::updateForces(real duration)
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

	Gravity::Gravity(const t3d::Vec4<real> & _gravity)
		: gravity(_gravity)
	{
	}

	void Gravity::updateForce(RigidBody * body, real duration)
	{
		if(!body->hasFiniteMass())
			return;

		body->addForce(t3d::vec3Mul(gravity, body->getMass()));

		body->setAwake();

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// Spring
	// /////////////////////////////////////////////////////////////////////////////////////

	Spring::Spring(
		const t3d::Vec4<real> & _connectionPoint,
		RigidBody * _other,
		const t3d::Vec4<real> & _otherConnectionPoint,
		real _springConstant,
		real _restLength)
		: connectionPoint(_connectionPoint)
		, other(_other)
		, otherConnectionPoint(_otherConnectionPoint)
		, springConstant(_springConstant)
		, restLength(_restLength)
	{
	}

	void Spring::updateForce(RigidBody * body, real duration)
	{
		t3d::Vec4<real> connectionPointWorld = connectionPoint * body->getTransform();

		t3d::Vec4<real> otherConnectionPointWorld = otherConnectionPoint * other->getTransform();

		t3d::Vec4<real> distance = t3d::vec3Sub(connectionPointWorld, otherConnectionPointWorld);

		real length = t3d::vec3Length(distance);

		body->addForceAtPoint(t3d::vec3Mul(t3d::vec3Normalize(distance), -springConstant * (length - restLength)), connectionPointWorld);

		body->setAwake();

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// Aero
	// /////////////////////////////////////////////////////////////////////////////////////

	Aero::Aero(
		const t3d::Mat4<real> & _tensor,
		const t3d::Vec4<real> & _position,
		const t3d::Vec4<real> * _pwindSpeed)
		: tensor(_tensor)
		, position(_position)
		, pwindSpeed(_pwindSpeed)
	{
	}

	void Aero::updateForce(RigidBody * body, real duration)
	{
		updateForceFromTensor(body, duration, tensor);
	}

	void Aero::updateForceFromTensor(RigidBody * body, real duration, const t3d::Mat4<real> & _tensor)
	{
		t3d::Vec4<real> velocity = t3d::vec3Add(body->getVelocity(), *pwindSpeed);

		t3d::Vec4<real> localVelocity = velocity * body->getInverseRotationTransform();

		t3d::Vec4<real> localForce = localVelocity * _tensor;

		t3d::Vec4<real> force = localForce * body->getRotationTransform();

		body->addForceAtPoint(force, position * body->getTransform());

		body->setAwake();

		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// AeroControl
	// /////////////////////////////////////////////////////////////////////////////////////

	void AeroControl::setControl(real value)
	{
		controlSetting = value;
	}

	AeroControl::AeroControl(
		const t3d::Mat4<real> & _tensor,
		const t3d::Mat4<real> & _minTensor,
		const t3d::Mat4<real> & _maxTensor,
		const t3d::Vec4<real> & _position,
		const t3d::Vec4<real> * _pwindSpeed)
		: Aero(_tensor, _position, _pwindSpeed)
		, minTensor(_minTensor)
		, maxTensor(_maxTensor)
	{
	}

	void AeroControl::updateForce(RigidBody * body, real duration)
	{
		updateForceFromControl(body, duration, controlSetting);
	}

	void AeroControl::updateForceFromControl(RigidBody * body, real duration, real _controlSetting)
	{
		_ASSERT(_controlSetting >= -1 && _controlSetting <= 1);

		if(_controlSetting < 0)
		{
			updateForceFromTensor(
				body, duration, t3d::mat4Intersect(minTensor, tensor, (real)-1, (real)0, _controlSetting));
		}
		else
		{
			updateForceFromTensor(
				body, duration, t3d::mat4Intersect(tensor, maxTensor, (real) 0, (real)1, _controlSetting));
		}
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// AngledAero
	// /////////////////////////////////////////////////////////////////////////////////////

	AngledAero::AngledAero(
		const t3d::Mat4<real> & _tensor,
		const t3d::Vec4<real> & _position,
		const t3d::Vec4<real> * _pwindSpeed)
		: Aero(_tensor, _position, _pwindSpeed)
	{
	}

	void AngledAero::updateForce(RigidBody * body, real duration)
	{
		_ASSERT(false);

		UNREFERENCED_PARAMETER(body);
		UNREFERENCED_PARAMETER(duration);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// Buoyancy
	// /////////////////////////////////////////////////////////////////////////////////////

	Buoyancy::Buoyancy(
		const t3d::Vec4<real> & _centerOfBuoyancy,
		real _maxDepth,
		real _volume,
		real _waterHeight,
		real _liquidDensity /*= 1000.0f*/)
		: centerOfBuoyancy(_centerOfBuoyancy)
		, maxDepth(_maxDepth)
		, volume(_volume)
		, waterHeight(_waterHeight)
		, liquidDensity(_liquidDensity)
	{
	}

	void Buoyancy::updateForce(RigidBody * body, real duration)
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
		t3d::vec3NegSelf(contactNormal);

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
		_ASSERT(IS_ZERO_FLOAT(t3d::vec3Length(contactNormal) - 1));

		t3d::Vec4<real> contantTangents[2];

		// Check whether the Z-axis is nearer to the X or Y axis
		if(abs(contactNormal.x) > abs(contactNormal.y))
		{
			// Scaling factor to ensure the results are normalised
			real s_inv = 1.0f / sqrt(contactNormal.x * contactNormal.x + contactNormal.z * contactNormal.z);

			// The new X-axis is at right angles to the world Y-axis
			contantTangents[0] = my::Vec4<real>(contactNormal.z * s_inv, 0, -contactNormal.x * s_inv);

			// The new Y-axis is at right angles to the new X- and Z- axes
			contantTangents[1] = t3d::vec3Cross(contactNormal, contantTangents[0]);
		}
		else
		{
			// Scaling factor to ensure the results are normalised
			real s_inv = 1.0f / sqrt(contactNormal.y * contactNormal.y + contactNormal.z * contactNormal.z);

			// The new X-axis is at right angles to the world X-axis
			contantTangents[0] = my::Vec4<real>(0, -contactNormal.z * s_inv, contactNormal.y * s_inv);

			// The new Y-axis is at right angles to the new X- and Z- axes
			contantTangents[1] = t3d::vec3Cross(contactNormal, contantTangents[0]);
		}

		// Make a matrix from the three vectors.
		contactToWorld = t3d::mat3TransposUvn(contactNormal, contantTangents[0], contantTangents[1]);
	}

	t3d::Vec4<real> Contact::calculateLocalVelocity(const RigidBody & body, const t3d::Vec4<real> & relativeContactPosition, real duration) const // ***
	{
		// Work out the velocity of the contact point.
		t3d::Vec4<real> velocity = t3d::vec3Add(
			t3d::vec3Cross(body.getRotation(), relativeContactPosition), body.getVelocity());

		// Turn the velocity into contact-coordinates.
		t3d::Vec4<real> velocityLocal = velocity.transformTranspose(contactToWorld);

		// Calculate the ammount of velocity that is due to forces without
		// reactions.
		t3d::Vec4<real> accVelocity = body.getResultingAcc() * duration;

		// Calculate the velocity in contact-coordinates.
		t3d::Vec4<real> accVelocityLocal = accVelocity.transformTranspose(contactToWorld);

		// We ignore any component of acceleration in the contact normal
		// direction, we are only interested in planar acceleration
		accVelocityLocal.x = 0;

		// Add the planar velocities - if there's enough friction they will
		// be removed during velocity resolution
		t3d::vec3AddSelf(velocityLocal, accVelocityLocal);

		// And return it
		return velocityLocal;
	}

	void Contact::calculateDesiredDeltaVelocity(real duration) // ***
	{
		_ASSERT(NULL != bodys[0]);

		const real velocityLimit = (real)0.25f;

		// Calculate the acceleration induced velocity accumulated this frame
		real velocityFromAcc = 0;

		if(bodys[0]->getAwake())
		{
			velocityFromAcc = t3d::vec3Dot(t3d::vec3Mul(bodys[0]->getResultingAcc(), duration), contactNormal);
		}

		if(NULL != bodys[1] && bodys[1]->getAwake())
		{
			velocityFromAcc -= t3d::vec3Dot(t3d::vec3Mul(bodys[1]->getResultingAcc(), duration), contactNormal);
		}

		// If the velocity is very slow, limit the restitution
		real thisRestitution = abs(contactVelocity.x) < velocityLimit ? 0.0f : restitution;

		// Combine the bounce velocity with the removed
		// acceleration velocity.
		desiredDeltaVelocity = -contactVelocity.x - thisRestitution * (contactVelocity.x - velocityFromAcc);
	}

	void Contact::calculateInternals(real duration)
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
		relativeContactPositions[0] = t3d::vec3Sub(contactPoint, bodys[0]->getPosition());

		if(NULL != bodys[1])
		{
			relativeContactPositions[1] = t3d::vec3Sub(contactPoint, bodys[1]->getPosition());
		}

		// Find the relative velocity of the bodies at the contact point.
		contactVelocity = calculateLocalVelocity(*bodys[0], relativeContactPositions[0], duration);

		if(NULL != bodys[1])
		{
			t3d::vec3SubSelf(contactVelocity, calculateLocalVelocity(*bodys[1], relativeContactPositions[1], duration));
		}

		// Calculate the desired change in velocity for resolution
		calculateDesiredDeltaVelocity(duration);
	}

	t3d::Vec4<real> Contact::calculateFrictionlessImpulse(const t3d::Mat4<real> inverseInertialTensors[]) const
	{
		_ASSERT(NULL != bodys[0]);

		// Build a vector that shows the change in velocity in
		// world space for a unit impulse in the direction of the contact
		// normal.
		t3d::Vec4<real> deltaVelocityWorld = t3d::vec3Cross(t3d::vec3Cross(relativeContactPositions[0], contactNormal) * inverseInertialTensors[0], relativeContactPositions[0]);

		// Work out the change in velocity in contact coordiantes.
		real deltaVelocity = t3d::vec3Dot(deltaVelocityWorld, contactNormal);

		// Add the linear component of velocity change
		deltaVelocity += bodys[0]->getInverseMass();

		// Check if we need to the second body's data
		if(NULL != bodys[1])
		{
			// Go through the same transformation sequence again
			deltaVelocityWorld = t3d::vec3Cross(t3d::vec3Cross(relativeContactPositions[1], contactNormal) * inverseInertialTensors[1], relativeContactPositions[1]);

			// Add the change in velocity due to rotation
			deltaVelocity += t3d::vec3Dot(deltaVelocityWorld, contactNormal);

			// Add the change in velocity due to linear motion
			deltaVelocity += bodys[1]->getInverseMass();
		}

		// Calculate the required size of the impulse
		return my::Vec4<real>(desiredDeltaVelocity / deltaVelocity, 0, 0);
	}

	t3d::Vec4<real> Contact::calculateFrictionImpulse(const t3d::Mat4<real> inverseInertialTensors[]) const // ***
	{
		_ASSERT(NULL != bodys[0]);

		// The equivalent of a cross product in matrices is multiplication
		// by a skew symmetric matrix - we build the matrix for converting
		// between linear and angular quantities.
		t3d::Mat4<real> impulseToTorque = t3d::buildSkewSymmetricMatrxi(relativeContactPositions[0]);

		// Build the matrix to convert contact impulse to change in velocity
		// in world coordinates.
		t3d::Mat4<real> deltaVelocityWorld = - (impulseToTorque * inverseInertialTensors[0] * impulseToTorque);

		real inverseMass = bodys[0]->getInverseMass();

		// Check if we need to add body two's data
		if(NULL != bodys[1])
		{
			// Set the cross product matrix
			impulseToTorque = t3d::buildSkewSymmetricMatrxi(relativeContactPositions[1]);

			// Calculate the velocity change matrix
			// Add to the total delta velocity.
			deltaVelocityWorld += - (impulseToTorque * inverseInertialTensors[1] * impulseToTorque);

			// Add to the inverse mass
			inverseMass += bodys[1]->getInverseMass();
		}

		// Do a change of basis to convert into contact coordinates.
		t3d::Mat4<real> deltaVelocity = (contactToWorld * deltaVelocityWorld).transformTranspose(contactToWorld); // ***

		// Add in the linear velocity change
		deltaVelocity.m00 += inverseMass;
		deltaVelocity.m11 += inverseMass;
		deltaVelocity.m22 += inverseMass;

		// Invert to get the impulse needed per unit velocity
		t3d::Mat4<real> impulseMatrix = deltaVelocity.inverse();

		// Find the target velocities to kill
		my::Vec4<real> velKill(desiredDeltaVelocity, -contactVelocity.y, -contactVelocity.z);

		// Find the impulse to kill target velocities
		t3d::Vec4<real> impulseContact = velKill * impulseMatrix;

		// Check for exceeding friction
		real planarImpulse = sqrt(impulseContact.y * impulseContact.y + impulseContact.z * impulseContact.z);

		if (planarImpulse > impulseContact.x * friction)
		{
			// We need to use dynamic friction
			impulseContact.y /= planarImpulse;
			impulseContact.z /= planarImpulse;

			impulseContact.x =
				deltaVelocity.m00 +
				deltaVelocity.m10 * friction * impulseContact.y +
				deltaVelocity.m20 * friction * impulseContact.z;

			impulseContact.x = desiredDeltaVelocity / impulseContact.x;
			impulseContact.y *= friction * impulseContact.x;
			impulseContact.z *= friction * impulseContact.x;
		}

		return impulseContact;
	}

	static real _calculateBodyInertias(
		const RigidBody & body,
		const t3d::Vec4<real> & relativeContactPosition,
		const t3d::Vec4<real> & contactNormal,
		real & linearInertia,
		real & angularInertia)
	{
		// Use the same procedure as for calculating frictionless
		// velocity change to work out the angular inertia.
		t3d::Vec4<real> angularInertiaWorld = t3d::vec3Cross(t3d::vec3Cross(relativeContactPosition, contactNormal) * body.getInverseInertialTensor(), relativeContactPosition);

		angularInertia = t3d::vec3Dot(angularInertiaWorld, contactNormal);

		// The linear component is simply the inverse mass
		linearInertia = body.getInverseMass();

		// Keep track of the total inertia from all components
		return linearInertia + angularInertia;
	}

	static void _limitAngularMove(
		const t3d::Vec4<real> & relativeContactPosition,
		const t3d::Vec4<real> & contactNormal,
		real & linearMove,
		real & angularMove)
	{
		const real angularLimit = 2.0f;

		// To avoid angular projections that are too great (when mass is large
		// but inertia tensor is small) limit the angular move.
		t3d::Vec4<real> projection = t3d::vec3Add(relativeContactPosition, t3d::vec3Mul(contactNormal, -t3d::vec3Dot(relativeContactPosition, contactNormal))); // ***

		// Use the small angle approximation for the sine of the angle (i.e.
		// the magnitude would be sine(angularLimit) * projection.magnitude
		// but we approximate sine(angularLimit) to angularLimit).
		real maxMagnitude = angularLimit * t3d::vec3Length(projection);

		real totalMove = linearMove + angularMove;

		angularMove = std::min(maxMagnitude, std::max(-maxMagnitude, angularMove));

		linearMove = totalMove - angularMove;
	}

	static void _applyInertiaToBody(
		RigidBody & body,
		const t3d::Vec4<real> & relativeContactPosition,
		const t3d::Vec4<real> & contactNormal,
		real linearMove,
		real angularMove,
		real angularInertia,
		t3d::Vec4<real> & linearChange,
		t3d::Vec4<real> & angularChange) // ***
	{
		// Velocity change is easier - it is just the linear movement
		// along the contact normal.
		linearChange = t3d::vec3Mul(contactNormal, linearMove);

		// We have the linear amount of movement required by turning
		// the rigid body (in angularMove[i]). We now need to
		// calculate the desired rotation to achieve that.
		if(0 == angularMove || 0 == body.getAngularDamping()) // *** angularDamping
		{
			// Easy case - no angular movement means no rotation.
			angularChange = my::Vec4<real>::ZERO;
		}
		else
		{
			// Work out the direction we'd like to rotate in.
			t3d::Vec4<real> angularPerMove = t3d::vec3Div(t3d::vec3Cross(relativeContactPosition, contactNormal) * body.getInverseInertialTensor(), angularInertia); // ***

			// Work out the direction we'd need to rotate to achieve that
			angularChange = t3d::vec3Mul(angularPerMove, angularMove);
		}

		// Now we can start to apply the values we've calculated.
		// Apply the linear movement
		body.addPosition(linearChange);

		// And the change in orientation
		body.addOrientation(angularChange);

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

	void Contact::applyPositionChange(t3d::Vec4<real> linearChanges[2], t3d::Vec4<real> angularChanges[2])
	{
		_ASSERT(NULL != bodys[0]);

		real linearInertias[2];
		real angularInertias[2];

		// We need to work out the inertia of each object in the direction
		// of the contact normal, due to angular inertia only.
		real totalInertia = _calculateBodyInertias(*bodys[0], relativeContactPositions[0], contactNormal, linearInertias[0], angularInertias[0]);

		if(NULL != bodys[1])
		{
			totalInertia += _calculateBodyInertias(*bodys[1], relativeContactPositions[1], contactNormal, linearInertias[1], angularInertias[1]);
		}

		// The linear and angular movements required are in proportion to
		// the two inverse inertias.
		real linearMoves[2];
		real angularMoves[2];
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

	void Contact::applyVelocityChange(t3d::Vec4<real> velocityChanges[2], t3d::Vec4<real> rotationChanges[2])
	{
		_ASSERT(NULL != bodys[0]);

		// Get hold of the inverse mass and inverse inertia tensor, both in
		// world coordinates.
		t3d::Mat4<real> inverseInertiaTensors[2];

		inverseInertiaTensors[0] = bodys[0]->getInverseInertialTensor();

		if(NULL != bodys[1])
		{
			inverseInertiaTensors[1] = bodys[1]->getInverseInertialTensor();
		}

		// We will calculate the impulse for each contact axis
		t3d::Vec4<real> impulseContact;
		
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
		t3d::Vec4<real> impulse = impulseContact * contactToWorld;

		// Split in the impulse into linear and rotational components
		velocityChanges[0] = t3d::vec3Mul(impulse, bodys[0]->getInverseMass());

		rotationChanges[0] = t3d::vec3Cross(relativeContactPositions[0], impulse) * inverseInertiaTensors[0];

		// Apply the changes
		bodys[0]->addVelocity(velocityChanges[0]);

		bodys[0]->addRotation(rotationChanges[0]);

		if(NULL != bodys[1])
		{
			// Work out body one's linear and angular changes
			velocityChanges[1] = t3d::vec3Mul(impulse, -bodys[1]->getInverseMass());

			rotationChanges[1] = t3d::vec3Cross(impulse, relativeContactPositions[1]) * inverseInertiaTensors[1];

			// And apply them.
			bodys[1]->addVelocity(velocityChanges[1]);

			bodys[1]->addRotation(rotationChanges[1]);
		}
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// ContactResolver
	// /////////////////////////////////////////////////////////////////////////////////////

	void ContactResolver::setPositionIterations(unsigned iterations)
	{
		positionIterations = iterations;
	}

	unsigned ContactResolver::getPositionIterations(void) const
	{
		return positionIterations;
	}

	void ContactResolver::setVelocityIterations(unsigned iterations)
	{
		velocityIterations = iterations;
	}

	unsigned ContactResolver::getVelocityIterations(void) const
	{
		return velocityIterations;
	}

	void ContactResolver::setPositionEpsilon(real value)
	{
		positionEpsilon = value;
	}

	real ContactResolver::getPositionEpsilon(void) const
	{
		return positionEpsilon;
	}

	void ContactResolver::setVelocityEpsilon(real value)
	{
		velocityEpsilon = value;
	}

	real ContactResolver::getVelocityEpsilon(void) const
	{
		return velocityEpsilon;
	}

	ContactResolver::ContactResolver(
		unsigned _positionIterations /*= 0*/,
		unsigned _velocityIterations /*= 0*/,
		real _positionEpsilon /*= 0.01f*/,
		real _velocityEpsilon /*= 0.01f*/)
		: positionIterations(_positionIterations)
		, velocityIterations(_velocityIterations)
		, positionEpsilon(_positionEpsilon)
		, velocityEpsilon(_velocityEpsilon)
	{
	}

	void ContactResolver::prepareContacts(
		Contact * contacts,
		unsigned numContacts,
		real duration)
	{
		for(unsigned i = 0; i < numContacts; i++)
		{
			contacts[i].calculateInternals(duration);
		}
	}

	void ContactResolver::_updateContactPenetration(
		Contact & contact,
		const t3d::Vec4<real> & relativeContactPosition,
		const t3d::Vec4<real> & linearChange,
		const t3d::Vec4<real> & angularChange,
		unsigned bodyIndex)
	{
		t3d::Vec4<real> deltaPosition = t3d::vec3Add(linearChange, t3d::vec3Cross(angularChange, relativeContactPosition));

		_ASSERT(IS_ZERO_FLOAT(vec3Length(contact.contactNormal) - 1));

		if(0 == bodyIndex)
		{
			contact.penetration -= t3d::vec3Dot(deltaPosition, contact.contactNormal);
		}
		else
		{
			// The sign of the change is positive if we're
			// dealing with the second body in a contact
			// and negative otherwise (because we're
			// subtracting the resolution)..
			_ASSERT(1 == bodyIndex);
			contact.penetration += t3d::vec3Dot(deltaPosition, contact.contactNormal);
		}
	}

	void ContactResolver::adjustPositions(
		Contact * contacts,
		unsigned numContacts,
		real duration)
	{
		// iteratively resolve interpenetrations in order of severity
		positionIterationsUsed = 0;
		while(positionIterationsUsed < positionIterations)
		{
			// Find biggest penetration
			real maxPenetration = positionEpsilon;
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
			t3d::Vec4<real> linearChanges[2];
			t3d::Vec4<real> angularChanges[2];
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
		const t3d::Vec4<real> & relativeContactPosition,
		const t3d::Vec4<real> & velocityChange,
		const t3d::Vec4<real> & rotationChange,
		unsigned bodyIndex,
		real duration)
	{
		t3d::Vec4<real> deltaVelocity = t3d::vec3Add(velocityChange, t3d::vec3Cross(rotationChange, relativeContactPosition));

		if(0 == bodyIndex)
		{
			t3d::vec3AddSelf(contact.contactVelocity, deltaVelocity.transformTranspose(contact.contactToWorld));
		}
		else
		{
			// The sign of the change is negative if we're dealing
			// with the second body in a contact.
			_ASSERT(1 == bodyIndex);
			t3d::vec3SubSelf(contact.contactVelocity, deltaVelocity.transformTranspose(contact.contactToWorld));
		}

		contact.calculateDesiredDeltaVelocity(duration);
	}

	void ContactResolver::adjustVelocities(
		Contact * contacts,
		unsigned numContacts,
		real duration)
	{
		// iteratively handle impacts in order of severity
		velocityIterationsUsed = 0;
		while(velocityIterationsUsed < velocityIterations)
		{
			// Find contact with maximum magnitude of probable velocity change
			real maxVelocity = velocityEpsilon;
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
			t3d::Vec4<real> velocityChanges[2];
			t3d::Vec4<real> rotationChanges[2];
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
		real duration)
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

	World::World(unsigned _maxContacts /*= 256*/)
		: resolver(0, 0, 0.01f, 0.01f)
		, maxContacts(_maxContacts)
	{
		contactList.resize(maxContacts);
	}

	World::~World(void)
	{
	}

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

	void World::integrate(real duration)
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

	void World::runPhysics(real duration)
	{
		startFrame();

		registry.updateForces(duration);

		integrate(duration);

		unsigned usedContacts = generateContacts(&contactList[0], maxContacts);

		resolver.setPositionIterations(usedContacts * 4);

		resolver.setVelocityIterations(usedContacts * 4);

		resolver.resolveContacts(&contactList[0], usedContacts, duration);
	}
}
