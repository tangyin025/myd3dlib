#include "Character.h"
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>
#include "PhysxContext.h"

using namespace my;

BOOST_CLASS_EXPORT(Character)

my::Vector3 PoseTrackInst::GetPos(void) const
{
	return my::Vector3(
		m_StartPos.x + m_Template->m_InterpolateX.Interpolate(m_Time, 0),
		m_StartPos.y + m_Template->m_InterpolateY.Interpolate(m_Time, 0),
		m_StartPos.z + m_Template->m_InterpolateZ.Interpolate(m_Time, 0));
}

Character::~Character(void)
{
	if (IsEnteredPhysx())
	{
		_ASSERT(false); LeavePhysxScene(PhysxSceneContext::getSingletonPtr());
	}
}

template<class Archive>
void Character::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Actor);
}

template<class Archive>
void Character::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Actor);
}

template
void Character::save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive & ar, const unsigned int version) const;

template
void Character::save<boost::archive::text_oarchive>(boost::archive::text_oarchive & ar, const unsigned int version) const;

template
void Character::save<boost::archive::binary_oarchive>(boost::archive::binary_oarchive & ar, const unsigned int version) const;

template
void Character::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const;

template
void Character::load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive & ar, const unsigned int version);

template
void Character::load<boost::archive::text_iarchive>(boost::archive::text_iarchive & ar, const unsigned int version);

template
void Character::load<boost::archive::binary_iarchive>(boost::archive::binary_iarchive & ar, const unsigned int version);

template
void Character::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version);

void Character::RequestResource(void)
{
	Actor::RequestResource();
}

void Character::ReleaseResource(void)
{
Actor::ReleaseResource();
}

void Character::EnterPhysxScene(PhysxSceneContext * scene)
{
	Actor::EnterPhysxScene(scene);

	m_PxMaterial.reset(PhysxContext::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysxDeleter<physx::PxMaterial>());

	physx::PxCapsuleControllerDesc desc;
	desc.height = m_Height;
	desc.radius = m_Radius;
	desc.contactOffset = m_ContactOffset;
	desc.position = physx::PxExtendedVec3(m_Position.x, m_Position.y, m_Position.z);
	desc.material = m_PxMaterial.get();
	desc.reportCallback = this;
	desc.behaviorCallback = this;
	desc.userData = this;
	m_PxController.reset(scene->m_ControllerMgr->createController(desc), PhysxDeleter<physx::PxController>());

	physx::PxActor * actor = m_PxController->getActor();
	actor->userData = this;

	scene->m_EventPxThreadSubstep.connect(boost::bind(&Character::OnPxThreadSubstep, this, _1));
}

void Character::LeavePhysxScene(PhysxSceneContext * scene)
{
	scene->m_EventPxThreadSubstep.disconnect(boost::bind(&Character::OnPxThreadSubstep, this, _1));

	scene->removeRenderActorsFromPhysicsActor(m_PxController->getActor());

	m_PxController.reset();

	m_PxMaterial.reset();

	Actor::LeavePhysxScene(scene);
}

void Character::OnPxTransformChanged(const physx::PxTransform & trans)
{

}

void Character::Update(float fElapsedTime)
{
	if (m_PxController)
	{
		PoseTrackInstPtrList::iterator track_iter = m_PoseTracks.begin();
		if (track_iter != m_PoseTracks.end())
		{
			if (((*track_iter)->m_Time += fElapsedTime) <= (*track_iter)->m_Template->m_Length)
			{
				m_Position = (*track_iter)->GetPos();
				m_PxController->setPosition(physx::PxExtendedVec3(m_Position.x, m_Position.y, m_Position.z));
			}
			else
			{
				track_iter = m_PoseTracks.erase(track_iter);
			}
		}

		if (track_iter == m_PoseTracks.end())
		{
			Matrix4 Uvn(Matrix4::RotationY(m_TargetOrientation));
			float ForwardSpeed = m_Velocity.dot(Uvn[2].xyz);
			float LeftwardSpeed = m_Velocity.dot(Uvn[0].xyz);
			if (ForwardSpeed > m_TargetSpeed)
			{
				ForwardSpeed = my::Max(ForwardSpeed - m_Resistance * fElapsedTime, m_TargetSpeed);
			}
			else
			{
				ForwardSpeed = my::Min(ForwardSpeed + m_SteeringLinear * fElapsedTime, m_TargetSpeed);
			}
			if (LeftwardSpeed > 0)
			{
				LeftwardSpeed = my::Max(LeftwardSpeed - m_Resistance * fElapsedTime, 0.0f);
			}
			else
			{
				LeftwardSpeed = my::Min(LeftwardSpeed + m_Resistance * fElapsedTime, 0.0f);
			}
			m_Velocity = Vector3(
				Uvn[2].x * ForwardSpeed + Uvn[0].x * LeftwardSpeed, m_Velocity.y + PhysxContext::getSingleton().Gravity.y * fElapsedTime,
				Uvn[2].z * ForwardSpeed + Uvn[0].z * LeftwardSpeed);
			physx::PxControllerCollisionFlags flags = m_PxController->move(
				(physx::PxVec3&)m_Velocity * fElapsedTime, 0.01f * fElapsedTime, fElapsedTime, physx::PxControllerFilters(&physx::PxFilterData(m_filterWord0, 0, 0, 0)), NULL);
			if (flags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
			{
				m_Velocity.y = 0;
			}

			if (ForwardSpeed > EPSILON_E6)
			{
				const float TargetOrientation = atan2f(m_Velocity.x, m_Velocity.z);
				const float Delta = my::Round(TargetOrientation - m_Orientation, -D3DX_PI, D3DX_PI);
				if (Delta > EPSILON_E6)
				{
					m_Orientation += Min(Delta, m_SteeringAngular * fElapsedTime);
				}
				else if (Delta < EPSILON_E6)
				{
					m_Orientation += Max(Delta, -m_SteeringAngular * fElapsedTime);
				}
			}

			m_Position = (my::Vector3 &)physx::toVec3(m_PxController->getPosition());
		}

		m_Rotation = Quaternion::RotationYawPitchRoll(m_Orientation, 0, 0);

		UpdateWorld();

		UpdateOctNode();
	}

	Actor::Update(fElapsedTime);
}

void Character::SetPose(const my::Vector3 & Pos, const my::Quaternion & Rot)
{
	if (m_PxController)
	{
		m_PxController->setPosition(physx::PxExtendedVec3(Pos.x, Pos.y, Pos.z));
	}

	m_Position = Pos;

	m_Rotation = Quaternion::RotationYawPitchRoll(m_Orientation, 0, 0);

	UpdateWorld();

	UpdateOctNode();
}

void Character::AddPoseTrack(PoseTrack * track)
{
	m_PoseTracks.insert(m_PoseTracks.begin(), PoseTrackInstPtr(new PoseTrackInst(track, m_Position)));
}

void Character::OnPxThreadSubstep(float dtime)
{

}

void Character::onShapeHit(const physx::PxControllerShapeHit& hit)
{

}

void Character::onControllerHit(const physx::PxControllersHit& hit)
{

}

void Character::onObstacleHit(const physx::PxControllerObstacleHit& hit)
{

}

physx::PxControllerBehaviorFlags Character::getBehaviorFlags(const physx::PxShape& shape, const physx::PxActor& actor)
{
	return physx::PxControllerBehaviorFlag::eCCT_USER_DEFINED_RIDE;
}

physx::PxControllerBehaviorFlags Character::getBehaviorFlags(const physx::PxController& controller)
{
	return physx::PxControllerBehaviorFlag::eCCT_USER_DEFINED_RIDE;
}

physx::PxControllerBehaviorFlags Character::getBehaviorFlags(const physx::PxObstacle& obstacle)
{
	return physx::PxControllerBehaviorFlag::eCCT_USER_DEFINED_RIDE;
}
