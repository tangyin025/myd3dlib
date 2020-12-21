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
#include "ActionTrack.h"
#include "Animation.h"
#include "myDxutApp.h"

using namespace my;

BOOST_CLASS_EXPORT(Character)

ShapeHitEventArg::ShapeHitEventArg(Character * _self)
	: ActorEventArg(_self)
	, worldPos(0, 0, 0)
	, worldNormal(1, 0, 0)
	, dir(1, 0, 0)
	, length(0)
	, cmp(NULL)
	, other(NULL)
	, triangleIndex(0)
{
}

Character::~Character(void)
{
	if (IsEnteredPhysx())
	{
		_ASSERT(false);

		if (m_PxActor && m_PxActor->getScene())
		{
			LeavePhysxScene((PhysxScene*)m_PxActor->getScene()->userData);
		}
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

void Character::RequestResource(void)
{
	Actor::RequestResource();
}

void Character::ReleaseResource(void)
{
	Actor::ReleaseResource();
}

void Character::EnterPhysxScene(PhysxScene * scene)
{
	Actor::EnterPhysxScene(scene);

	m_PxMaterial.reset(PhysxSdk::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysxDeleter<physx::PxMaterial>());

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

	scene->m_EventPxThreadSubstep.connect(boost::bind(&Character::OnPxThreadSubstep, this, boost::placeholders::_1));
}

void Character::LeavePhysxScene(PhysxScene * scene)
{
	scene->m_EventPxThreadSubstep.disconnect(boost::bind(&Character::OnPxThreadSubstep, this, boost::placeholders::_1));

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
	ActionInstPtrList::iterator action_inst_iter = m_ActionInstList.begin();
	for (; action_inst_iter != m_ActionInstList.end(); )
	{
		if ((*action_inst_iter)->m_Time < (*action_inst_iter)->m_Template->m_Length)
		{
			(*action_inst_iter)->Update(fElapsedTime);

			action_inst_iter++;
		}
		else
		{
			(*action_inst_iter)->Stop();

			// ! make sure action inst was not in parallel task list
			action_inst_iter = m_ActionInstList.erase(action_inst_iter);
		}
	}

	if (m_PxController)
	{
		if (!m_Base && m_ActionTrackPoseInstRef == 0)
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
				Uvn[2].x * ForwardSpeed + Uvn[0].x * LeftwardSpeed, m_Velocity.y + PhysxSdk::getSingleton().Gravity.y * fElapsedTime,
				Uvn[2].z * ForwardSpeed + Uvn[0].z * LeftwardSpeed);

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

			m_MoveFlags = m_PxController->move((physx::PxVec3 &)m_Velocity * fElapsedTime,
				0.01f * fElapsedTime, fElapsedTime, physx::PxControllerFilters(&physx::PxFilterData(m_filterWord0, 0, 0, 0)), NULL);

			if (m_MoveFlags.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN))
			{
				m_Velocity.y = 0;
			}

			m_Position = (Vector3 &)physx::toVec3(m_PxController->getPosition());

			m_Rotation = Quaternion::RotationYawPitchRoll(m_Orientation, 0, 0);

			UpdateWorld();

			UpdateOctNode();
		}
	}

	if (m_Animation)
	{
		m_Animation->Update(fElapsedTime);
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		if ((*cmp_iter)->m_LodMask & m_Lod)
		{
			(*cmp_iter)->Update(fElapsedTime);
		}
	}

	AttachPairList::iterator att_iter = m_Attaches.begin();
	for (; att_iter != m_Attaches.end(); att_iter++)
	{
		if (m_Animation && att_iter->second >= 0 && att_iter->second < (int)m_Animation->anim_pose_hier.size())
		{
			const Bone & bone = m_Animation->anim_pose_hier[att_iter->second];
			att_iter->first->SetPose(bone.m_position.transformCoord(m_World), bone.m_rotation * m_Rotation);
		}
		else
		{
			att_iter->first->SetPose(m_Position, m_Rotation);
		}

		att_iter->first->Update(fElapsedTime);
	}
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

bool Character::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	return Actor::AddToPipeline(frustum, pipeline, PassMask, ViewPos, TargetPos);
}

void Character::OnPxThreadSubstep(float dtime)
{

}

void Character::onShapeHit(const physx::PxControllerShapeHit& hit)
{
	if (m_EventShapeHit)
	{
		ShapeHitEventArg arg(this);
		arg.worldPos = (Vector3 &)hit.worldPos;
		arg.worldNormal = (Vector3 &)hit.worldNormal;
		arg.dir = (Vector3 &)hit.dir;
		arg.length = hit.length;
		arg.cmp = (Component *)hit.shape->userData;
		_ASSERT(arg.cmp);
		arg.other = (Actor *)hit.actor->userData;
		_ASSERT(arg.other);
		arg.triangleIndex = hit.triangleIndex;
		m_EventShapeHit(&arg);
	}
}

void Character::onControllerHit(const physx::PxControllersHit& hit)
{

}

void Character::onObstacleHit(const physx::PxControllerObstacleHit& hit)
{

}

physx::PxControllerBehaviorFlags Character::getBehaviorFlags(const physx::PxShape& shape, const physx::PxActor& actor)
{
	return physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
}

physx::PxControllerBehaviorFlags Character::getBehaviorFlags(const physx::PxController& controller)
{
	return physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
}

physx::PxControllerBehaviorFlags Character::getBehaviorFlags(const physx::PxObstacle& obstacle)
{
	return physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
}
