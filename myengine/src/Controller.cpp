#include "Controller.h"
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
#include <boost/scope_exit.hpp>
#include "PhysxContext.h"
#include "ActionTrack.h"
#include "Animation.h"
#include "myDxutApp.h"

using namespace my;

BOOST_CLASS_EXPORT(Controller)

Controller::~Controller(void)
{
	_ASSERT(!m_PxController || !m_PxController->getActor()->getScene());
}

template<class Archive>
void Controller::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
}

template<class Archive>
void Controller::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
}

void Controller::RequestResource(void)
{
	Component::RequestResource();
}

void Controller::ReleaseResource(void)
{
	Component::ReleaseResource();
}

void Controller::EnterPhysxScene(PhysxScene * scene)
{
	Component::EnterPhysxScene(scene);

	m_PxMaterial.reset(PhysxSdk::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysxDeleter<physx::PxMaterial>());

	physx::PxCapsuleControllerDesc desc;
	desc.height = m_Height;
	desc.radius = m_Radius;
	desc.contactOffset = m_ContactOffset;
	desc.position = physx::PxExtendedVec3(m_Actor->m_Position.x, m_Actor->m_Position.y + m_ContactOffset + m_Radius + m_Height * 0.5f, m_Actor->m_Position.z);
	desc.material = m_PxMaterial.get();
	desc.reportCallback = this;
	desc.behaviorCallback = this;
	desc.userData = NULL;
	m_PxController.reset(scene->m_ControllerMgr->createController(desc), PhysxDeleter<physx::PxController>());

	//// ! recursively call Actor::SetPose by PhysxScene::TickPostRender
	physx::PxRigidDynamic * actor = m_PxController->getActor()->is<physx::PxRigidDynamic>();
	_ASSERT(actor);
	//actor->userData = m_Actor;
	std::vector<physx::PxShape*> shapes(actor->getNbShapes());
	actor->getShapes(shapes.data(), shapes.size());
	_ASSERT(!shapes.empty());
	shapes.front()->userData = this;
}

void Controller::LeavePhysxScene(PhysxScene * scene)
{
	Component::LeavePhysxScene(scene);

	_ASSERT(!m_PxController || m_PxController->getActor()->getScene() == scene->m_PxScene.get());

	if (m_PxController)
	{
		scene->removeRenderActorsFromPhysicsActor(m_PxController->getActor());
	}

	m_PxController.reset();

	m_PxMaterial.reset();
}

void Controller::Update(float fElapsedTime)
{
	m_Actor->SetPose((my::Vector3&)physx::toVec3(m_PxController->getFootPosition()), m_Actor->m_Rotation);
}

void Controller::OnSetShader(IDirect3DDevice9* pd3dDevice, my::Effect* shader, LPARAM lparam)
{
}

void Controller::SetPxPoseOrbyPxThread(const physx::PxTransform& pose)
{
	if (m_PxControllerMoveMuted)
	{
		return;
	}

	if (m_PxController)
	{
		m_PxController->setFootPosition(physx::PxExtendedVec3(pose.p.x, pose.p.y, pose.p.z));
	}
}

unsigned int Controller::Move(const my::Vector3& disp, float minDist, float elapsedTime)
{
	physx::PxControllerCollisionFlags moveFlags;

	if (m_PxController)
	{
		moveFlags = m_PxController->move((physx::PxVec3&)disp, minDist, elapsedTime, physx::PxControllerFilters(&physx::PxFilterData(m_filterWord0, 0, 0, 0)), NULL);

		physx::PxTransform pose(physx::toVec3(m_PxController->getFootPosition()), (physx::PxQuat&)m_Actor->m_Rotation);

		// ! recursively call Controller::SetPxPoseOrbyPxThread
		m_PxControllerMoveMuted = true;
		BOOST_SCOPE_EXIT(&m_PxControllerMoveMuted)
		{
			m_PxControllerMoveMuted = false;
		}
		BOOST_SCOPE_EXIT_END

		m_Actor->SetPxPoseOrbyPxThread(pose);
	}

	return moveFlags;
}

void Controller::onShapeHit(const physx::PxControllerShapeHit& hit)
{
	_ASSERT(m_Actor);

	if (hit.shape->userData)
	{
		// ! PxControllerShapeHit::actor may not have userData for Controller objs
		Component* other_cmp = (Component*)hit.shape->userData;
		ShapeHitEventArg arg(m_Actor, this, other_cmp->m_Actor, other_cmp);
		arg.worldPos = (Vector3&)hit.worldPos;
		arg.worldNormal = (Vector3&)hit.worldNormal;
		arg.dir = (Vector3&)hit.dir;
		arg.length = hit.length;
		arg.triangleIndex = hit.triangleIndex;
		m_Actor->m_EventPxThreadShapeHit(&arg);
	}
}

void Controller::onControllerHit(const physx::PxControllersHit& hit)
{

}

void Controller::onObstacleHit(const physx::PxControllerObstacleHit& hit)
{

}

physx::PxControllerBehaviorFlags Controller::getBehaviorFlags(const physx::PxShape& shape, const physx::PxActor& actor)
{
	return physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
}

physx::PxControllerBehaviorFlags Controller::getBehaviorFlags(const physx::PxController& controller)
{
	return physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
}

physx::PxControllerBehaviorFlags Controller::getBehaviorFlags(const physx::PxObstacle& obstacle)
{
	return physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
}
