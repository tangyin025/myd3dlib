#include "Character.h"
#include "Actor.h"
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

Character::~Character(void)
{
	if (m_PxController)
	{
		_ASSERT(false);

		LeavePhysxScene((PhysxScene*)m_PxController->getActor()->getScene()->userData);
	}
}

template<class Archive>
void Character::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
}

template<class Archive>
void Character::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
}

void Character::RequestResource(void)
{
	Component::RequestResource();
}

void Character::ReleaseResource(void)
{
	Component::ReleaseResource();
}

void Character::EnterPhysxScene(PhysxScene * scene)
{
	Component::EnterPhysxScene(scene);

	_ASSERT(m_Actor);

	m_PxMaterial.reset(PhysxSdk::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysxDeleter<physx::PxMaterial>());

	physx::PxCapsuleControllerDesc desc;
	desc.height = m_Height;
	desc.radius = m_Radius;
	desc.contactOffset = m_ContactOffset;
	desc.position = physx::PxExtendedVec3(m_Actor->m_Position.x, m_Actor->m_Position.y, m_Actor->m_Position.z);
	desc.material = m_PxMaterial.get();
	desc.reportCallback = this;
	desc.behaviorCallback = this;
	desc.userData = this;
	m_PxController.reset(scene->m_ControllerMgr->createController(desc), PhysxDeleter<physx::PxController>());

	//// ! recursively call Actor::OnPxTransformChanged
	//physx::PxActor * actor = m_PxController->getActor();
	//actor->userData = m_Actor;

	scene->m_EventPxThreadSubstep.connect(boost::bind(&Character::OnPxThreadSubstep, this, boost::placeholders::_1));
}

void Character::LeavePhysxScene(PhysxScene * scene)
{
	_ASSERT(!m_PxController || m_PxController->getActor()->getScene() == scene->m_PxScene.get());

	scene->m_EventPxThreadSubstep.disconnect(boost::bind(&Character::OnPxThreadSubstep, this, boost::placeholders::_1));

	if (m_PxController)
	{
		scene->removeRenderActorsFromPhysicsActor(m_PxController->getActor());
	}

	m_PxController.reset();

	m_PxMaterial.reset();

	Component::LeavePhysxScene(scene);
}

void Character::Update(float fElapsedTime)
{
}

void Character::OnSetShader(IDirect3DDevice9* pd3dDevice, my::Effect* shader, LPARAM lparam)
{
}

void Character::OnSetPose(void)
{
	if (m_muted)
	{
		return;
	}

	if (m_PxController)
	{
		m_PxController->setPosition(physx::PxExtendedVec3(m_Actor->m_Position.x, m_Actor->m_Position.y, m_Actor->m_Position.z));
	}
}

unsigned int Character::Move(const my::Vector3& disp, float minDist, float elapsedTime)
{
	physx::PxControllerCollisionFlags moveFlags;

	if (m_PxController)
	{
		moveFlags = m_PxController->move((physx::PxVec3&)disp, minDist, elapsedTime, physx::PxControllerFilters(&physx::PxFilterData(m_filterWord0, 0, 0, 0)), NULL);

		// ! recursively call Character::OnSetPose
		ScopeSetter<bool> setter(m_muted, true);
		m_Actor->SetPose((Vector3&)physx::toVec3(m_PxController->getPosition()), m_Actor->m_Rotation);
	}

	return moveFlags;
}

void Character::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
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
