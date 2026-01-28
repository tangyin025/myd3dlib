// Copyright (c) 2011-2024 tangyin025
// License: MIT
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
#include "Animator.h"
#include "myDxutApp.h"
#include "CctCapsuleController.h"
#include <boost/range/iterator_range.hpp>
#include <boost/shared_container_iterator.hpp>

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

	PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Actor->m_Node->GetTopNode());

	m_PxMaterial.reset(PhysxSdk::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysxDeleter<physx::PxMaterial>());

	m_desc.position = physx::PxExtendedVec3(m_Actor->m_Position.x, m_Actor->m_Position.y + m_desc.contactOffset + m_desc.radius + m_desc.height * 0.5f, m_Actor->m_Position.z);
	m_desc.upDirection = physx::PxVec3(0.0f, 1.0f, 0.0f);
	m_desc.material = m_PxMaterial.get();

	m_PxController.reset(scene->m_ControllerMgr->createController(m_desc), PhysxDeleter<physx::PxController>());

	physx::PxRigidDynamic * actor = m_PxController->getActor();
	_ASSERT(actor);
	//actor->userData = m_Actor; // ! PhysxScene::TickPostRender, actor->SetPose
	std::vector<physx::PxShape*> shapes(actor->getNbShapes());
	actor->getShapes(shapes.data(), shapes.size());
	_ASSERT(shapes.size() == 1);
	m_PxShape.reset(shapes.front(), PhysxDeleter<physx::PxShape>());
	m_PxShape->acquireReference();

	m_PxShape->userData = this; // ! trigger_iter->otherShape->userData

	SetSimulationFilterWord0(m_DescSimulationFilterWord0);

	SetQueryFilterWord0(m_DescQueryFilterWord0);

	SetShapeFlags(m_DescShapeFlags);
}

void Controller::ReleaseResource(void)
{
	PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Actor->m_Node->GetTopNode());

	_ASSERT(m_PxController && m_PxController->getActor()->getScene() == scene->m_PxScene.get());

	scene->removeRenderActorsFromPhysicsActor(m_PxController->getActor());

	m_PxShape.reset();

	m_PxController.reset();

	m_PxMaterial.reset();

	Component::ReleaseResource();
}

void Controller::Update(float fElapsedTime)
{
	//m_Actor->SetPose(GetPosition());
}

void Controller::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
}

void Controller::SetPxPoseOrbyPxThread(const my::Vector3 & Pos, const my::Quaternion & Rot)
{
	_ASSERT(!m_PxControllerMoveMuted);

	SetFootPosition(Pos);
}

void Controller::SetSimulationFilterWord0(unsigned int filterWord0)
{
	m_DescSimulationFilterWord0 = filterWord0;

	if (m_PxShape)
	{
		physx::PxFilterData filter_data(filterWord0, 0, 0, 0);
		m_PxShape->setSimulationFilterData(filter_data);
	}
}

unsigned int Controller::GetSimulationFilterWord0(void) const
{
	if (m_PxShape)
	{
		return m_PxShape->getSimulationFilterData().word0;
	}

	return m_DescSimulationFilterWord0;
}

void Controller::SetQueryFilterWord0(unsigned int filterWord0)
{
	m_DescQueryFilterWord0 = filterWord0;

	if (m_PxShape)
	{
		physx::PxFilterData filter_data(filterWord0, 0, 0, 0);
		m_PxShape->setQueryFilterData(filter_data);
	}
}

unsigned int Controller::GetQueryFilterWord0(void) const
{
	if (m_PxShape)
	{
		return m_PxShape->getQueryFilterData().word0;
	}

	return m_DescQueryFilterWord0;
}

void Controller::SetShapeFlags(unsigned int Flags)
{
	m_DescShapeFlags = Flags;

	if (m_PxShape)
	{
		m_PxShape->setFlags(physx::PxShapeFlags(Flags));
	}
}

unsigned int Controller::GetShapeFlags(void) const
{
	if (m_PxShape)
	{
		return m_PxShape->getFlags();
	}

	return m_DescShapeFlags;
}

unsigned int Controller::Move(const my::Vector3 & disp, float minDist, float elapsedTime, unsigned int filterWord0)
{
#ifdef _DEBUG
	m_PxControllerMoveMuted = true;
	BOOST_SCOPE_EXIT(&m_PxControllerMoveMuted)
	{
		m_PxControllerMoveMuted = false;
	}
	BOOST_SCOPE_EXIT_END
#endif
		
	physx::PxControllerCollisionFlags moveFlags;

	if (m_PxController)
	{
		PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Actor->m_Node->GetTopNode());

		moveFlags = m_PxController->move((physx::PxVec3&)disp, minDist, elapsedTime, physx::PxControllerFilters(&physx::PxFilterData(filterWord0, 0, 0, 0), NULL, &scene->m_ControllerFilter), NULL);

		//// ! recursively call other Component::SetPxPoseOrbyPxThread
		//m_Actor->SetPxPoseOrbyPxThread(GetFootPosition(), m_Actor->m_Rotation, this);
	}

	return moveFlags;
}

void Controller::SetHeight(float Height)
{
	m_desc.height = Height;

	if (m_PxController)
	{
		static_cast<physx::Cct::CapsuleController*>(m_PxController.get())->setHeight(Height);
	}
}

float Controller::GetHeight(void) const
{
	if (m_PxController)
	{
		return static_cast<physx::Cct::CapsuleController*>(m_PxController.get())->getHeight();
	}

	return m_desc.height;
}

void Controller::SetRadius(float Radius)
{
	m_desc.radius = Radius;

	if (m_PxController)
	{
		static_cast<physx::Cct::CapsuleController*>(m_PxController.get())->setRadius(Radius);
	}
}

float Controller::GetRadius(void) const
{
	if (m_PxController)
	{
		return static_cast<physx::Cct::CapsuleController*>(m_PxController.get())->getRadius();
	}

	return m_desc.radius;
}

void Controller::SetStepOffset(float StepOffset)
{
	m_desc.stepOffset = StepOffset;

	if (m_PxController)
	{
		m_PxController->setStepOffset(StepOffset);
	}
}

float Controller::GetStepOffset(void) const
{
	if (m_PxController)
	{
		return m_PxController->getStepOffset();
	}

	return m_desc.stepOffset;
}

void Controller::SetContactOffset(float ContactOffset)
{
	m_desc.contactOffset = ContactOffset;

	if (m_PxController)
	{
		m_PxController->setContactOffset(ContactOffset);
	}
}

float Controller::GetContactOffset(void) const
{
	if (m_PxController)
	{
		return m_PxController->getContactOffset();
	}

	return m_desc.contactOffset;
}

void Controller::SetSlopeLimit(float SlopeLimit)
{
	m_desc.slopeLimit = SlopeLimit;

	if (m_PxController)
	{
		m_PxController->setSlopeLimit(SlopeLimit);
		static_cast<physx::Cct::CapsuleController*>(m_PxController.get())->mUserParams.mHandleSlope = SlopeLimit != 0.0f;
	}
}

float Controller::GetSlopeLimit(void) const
{
	if (m_PxController)
	{
		return m_PxController->getSlopeLimit();
	}

	return m_desc.slopeLimit;
}

void Controller::SetUpDirection(const my::Vector3 & Up)
{
	_ASSERT(IS_NORMALIZED(Up));

	m_desc.upDirection = (physx::PxVec3&)Up;

	if (m_PxController)
	{
		// ! Controller::setUpDirectionInternal
		static_cast<physx::Cct::CapsuleController*>(m_PxController.get())->mUserParams.mUpDirection = (physx::PxVec3&)Up;
	}
}

const my::Vector3 & Controller::GetUpDirection(void) const
{
	if (m_PxController)
	{
		return (my::Vector3&)static_cast<physx::Cct::CapsuleController*>(m_PxController.get())->mUserParams.mUpDirection;
	}

	return (my::Vector3&)m_desc.upDirection;
}

void Controller::SetPosition(const my::Vector3 & Pos)
{
	m_desc.position = physx::PxExtendedVec3(Pos.x, Pos.y, Pos.z);

	if (m_PxController)
	{
		m_PxController->setPosition(m_desc.position);
	}
}

my::Vector3 Controller::GetPosition(void) const
{
	if (m_PxController)
	{
		return (my::Vector3&)physx::toVec3(m_PxController->getPosition());
	}

	return (my::Vector3&)m_desc.position;
}

my::Vector3 Controller::GetFootOffset(void) const
{
	// ! PhysXCharacterKinematic/src/CctSweptCapsule.cpp, SweptCapsule::computeTemporalBox
	return Vector3(0, 1, 0) * (GetContactOffset() + GetRadius() + GetHeight() * 0.5f);
}

void Controller::SetFootPosition(const my::Vector3 & Pos)
{
	SetPosition(Pos + GetFootOffset());
}

my::Vector3 Controller::GetFootPosition(void) const
{
	return GetPosition() - GetFootOffset();
}

const my::Vector3 & Controller::GetContactNormalDownPass(void) const
{
	return (Vector3&)static_cast<physx::Cct::CapsuleController*>(m_PxController.get())->mCctModule.mContactNormalDownPass;
}

const my::Vector3 & Controller::GetContactNormalSidePass(void) const
{
	return (Vector3&)static_cast<physx::Cct::CapsuleController*>(m_PxController.get())->mCctModule.mContactNormalSidePass;
}

static const physx::PxU32 GeomSizes[] =
{
	sizeof(physx::Cct::TouchedUserBox),
	sizeof(physx::Cct::TouchedUserCapsule),
	sizeof(physx::Cct::TouchedMesh),
	sizeof(physx::Cct::TouchedBox),
	sizeof(physx::Cct::TouchedSphere),
	sizeof(physx::Cct::TouchedCapsule),
};

typedef std::vector<Component*> cmp_list;

typedef boost::shared_container_iterator<cmp_list> shared_cmp_list_iter;

boost::iterator_range<shared_cmp_list_iter> controller_get_geom_stream(const Controller* self)
{
	boost::shared_ptr<cmp_list> cmps(new cmp_list());
	physx::Cct::CapsuleController* controller = static_cast<physx::Cct::CapsuleController*>(self->m_PxController.get());
	const physx::PxU32* Data = controller->mCctModule.mGeomStream.begin();
	const physx::PxU32* Last = controller->mCctModule.mGeomStream.end();
	while (Data != Last)
	{
		const physx::Cct::TouchedGeom* CurrentGeom = reinterpret_cast<const physx::Cct::TouchedGeom*>(Data);
		if (CurrentGeom->mType == physx::Cct::TouchedGeomType::eMESH
			|| CurrentGeom->mType == physx::Cct::TouchedGeomType::eBOX
			|| CurrentGeom->mType == physx::Cct::TouchedGeomType::eSPHERE
			|| CurrentGeom->mType == physx::Cct::TouchedGeomType::eCAPSULE)
		{
			const physx::PxShape* touchedShape = reinterpret_cast<const physx::PxShape*>(CurrentGeom->mTGUserData);
			Component* cmp = (Component*)touchedShape->userData;
			cmps->push_back(cmp);
		}

		const physx::PxU8* ptr = reinterpret_cast<const physx::PxU8*>(Data);
		ptr += GeomSizes[CurrentGeom->mType];
		Data = reinterpret_cast<const physx::PxU32*>(ptr);
	}
	return boost::make_iterator_range(shared_cmp_list_iter(cmps->begin(), cmps), shared_cmp_list_iter(cmps->end(), cmps));
}

Component* Controller::GetTouchedComponent(void) const
{
	physx::Cct::CapsuleController* controller = static_cast<physx::Cct::CapsuleController*>(m_PxController.get());
	if (controller->mCctModule.mTouchedShape && controller->mCctModule.mTouchedShape->userData)
	{
		Component* other_cmp = (Component*)controller->mCctModule.mTouchedShape->userData;
		return other_cmp;
	}
	return NULL;
}

my::Vector3 & Controller::GetTouchedPosWorld(void) const
{
	return (my::Vector3&)static_cast<physx::Cct::CapsuleController*>(m_PxController.get())->mCctModule.mTouchedPosShape_World;
}

my::Vector3 & Controller::GetTouchedPosLocal(void) const
{
	return (my::Vector3&)static_cast<physx::Cct::CapsuleController*>(m_PxController.get())->mCctModule.mTouchedPosShape_Local;
}

static unsigned int _get_collision_flag(unsigned int flags)
{
	// ! SweepTest::moveCharacter, SweepTest::doSweepTest, first side next down
	return (flags & physx::Cct::STF_VALIDATE_TRIANGLE_DOWN) ? physx::PxControllerCollisionFlag::eCOLLISION_DOWN
		: (flags & physx::Cct::STF_VALIDATE_TRIANGLE_SIDE) ? physx::PxControllerCollisionFlag::eCOLLISION_SIDES : physx::PxControllerCollisionFlag::eCOLLISION_UP;
}
//
//__declspec(thread) static physx::PxControllerBehaviorFlags g_behaviorflags;

void Controller::onShapeHit(const physx::PxControllerShapeHit & hit)
{
	_ASSERT(m_Actor && hit.controller == this->m_PxController.get());

	if (hit.shape->userData)
	{
		// ! PxControllerShapeHit::actor may not have userData for Controller objs
		Component* other_cmp = (Component*)hit.shape->userData;
		ShapeHitEventArg arg(m_Actor, this, other_cmp->m_Actor, other_cmp);
		arg.worldPos = (Vector3&)physx::toVec3(hit.worldPos);
		arg.worldNormal = (Vector3&)hit.worldNormal;
		arg.dir = (Vector3&)hit.dir;
		arg.length = hit.length;
		arg.flag = _get_collision_flag(static_cast<physx::Cct::CapsuleController*>(m_PxController.get())->mCctModule.mFlags);
		arg.triangleIndex = hit.triangleIndex;
		m_Actor->m_EventPxThreadShapeHit(&arg);
	}
}

void Controller::onControllerHit(const physx::PxControllersHit & hit)
{
	_ASSERT(m_Actor && hit.controller == this->m_PxController.get());

	if (hit.other->getUserData())
	{
		Component* other_cmp = (Component*)hit.other->getUserData();
		ControllerHitEventArg arg(m_Actor, this, other_cmp->m_Actor, other_cmp);
		arg.worldPos = (Vector3&)physx::toVec3(hit.worldPos);
		arg.worldNormal = (Vector3&)hit.worldNormal;
		arg.dir = (Vector3&)hit.dir;
		arg.length = hit.length;
		arg.flag = _get_collision_flag(static_cast<physx::Cct::CapsuleController*>(m_PxController.get())->mCctModule.mFlags);
		m_Actor->m_EventPxThreadControllerHit(&arg);
	}
}

void Controller::onObstacleHit(const physx::PxControllerObstacleHit & hit)
{
	_ASSERT(m_Actor && hit.controller == this->m_PxController.get());

	if (hit.userData)
	{
		ObstacleHitEventArg arg(m_Actor, this);
		arg.worldPos = (Vector3&)physx::toVec3(hit.worldPos);
		arg.worldNormal = (Vector3&)hit.worldNormal;
		arg.dir = (Vector3&)hit.dir;
		arg.length = hit.length;
		arg.flag = _get_collision_flag(static_cast<physx::Cct::CapsuleController*>(m_PxController.get())->mCctModule.mFlags);
		m_Actor->m_EventPxThreadObstacleHit(&arg);
	}
}

physx::PxControllerBehaviorFlags Controller::getBehaviorFlags(const physx::PxShape & shape, const physx::PxActor & actor)
{
	return physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
}

physx::PxControllerBehaviorFlags Controller::getBehaviorFlags(const physx::PxController & controller)
{
	return physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
}

physx::PxControllerBehaviorFlags Controller::getBehaviorFlags(const physx::PxObstacle & obstacle)
{
	return physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
}
