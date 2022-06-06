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

	m_desc.position = physx::PxExtendedVec3(m_Actor->m_Position.x, m_Actor->m_Position.y /*+ m_desc.contactOffset + m_desc.radius + m_desc.height * 0.5f*/, m_Actor->m_Position.z);
	m_desc.material = m_PxMaterial.get();
	m_desc.reportCallback = this;
	m_desc.behaviorCallback = this;
	m_desc.userData = this;

	m_PxController.reset(scene->m_ControllerMgr->createController(m_desc), PhysxDeleter<physx::PxController>());

	physx::PxRigidDynamic * actor = m_PxController->getActor();
	_ASSERT(actor);
	//actor->userData = m_Actor; // ! PhysxScene::TickPostRender, actor->SetPose
	std::vector<physx::PxShape*> shapes(actor->getNbShapes());
	actor->getShapes(shapes.data(), shapes.size());
	_ASSERT(!shapes.empty());
	shapes.front()->userData = this; // ! trigger_iter->otherShape->userData
}

void Controller::ReleaseResource(void)
{
	PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Actor->m_Node->GetTopNode());

	_ASSERT(m_PxController && m_PxController->getActor()->getScene() == scene->m_PxScene.get());

	scene->removeRenderActorsFromPhysicsActor(m_PxController->getActor());

	m_PxController.reset();

	m_PxMaterial.reset();

	Component::ReleaseResource();
}

void Controller::EnterPhysxScene(PhysxScene * scene)
{
	Component::EnterPhysxScene(scene);
}

void Controller::LeavePhysxScene(PhysxScene * scene)
{
	Component::LeavePhysxScene(scene);
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

	SetPosition(Pos);
}

unsigned int Controller::Move(const my::Vector3 & disp, float minDist, float elapsedTime, unsigned int filterWord0)
{
	m_PxControllerMoveMuted = true;
	BOOST_SCOPE_EXIT(&m_PxControllerMoveMuted)
	{
		m_PxControllerMoveMuted = false;
	}
	BOOST_SCOPE_EXIT_END
		
	physx::PxControllerCollisionFlags moveFlags;

	if (m_PxController)
	{
		struct FilterCallback : physx::PxControllerFilterCallback
		{
			FilterCallback(void)
			{
			}

			virtual bool filter(const physx::PxController & a, const physx::PxController & b)
			{
				if (a.getUserData() && b.getUserData())
				{
					Controller* controller0 = static_cast<Controller*>((Component*)a.getUserData());
					Controller* controller1 = static_cast<Controller*>((Component*)b.getUserData());
					if (controller0->m_Actor->m_Base != controller1->m_Actor && controller0->m_Actor != controller1->m_Actor->m_Base)
					{
						return true;
					}
				}
				return false;
			}
		};

		moveFlags = m_PxController->move((physx::PxVec3&)disp, minDist, elapsedTime, physx::PxControllerFilters(&physx::PxFilterData(filterWord0, 0, 0, 0), NULL, &FilterCallback()), NULL);

		// ! recursively call other Component::SetPxPoseOrbyPxThread
		m_Actor->SetPxPoseOrbyPxThread(GetPosition(), m_Actor->m_Rotation, this);
	}

	return moveFlags;
}

void Controller::SetHeight(float Height)
{
	static_cast<physx::Cct::CapsuleController*>(m_PxController.get())->setHeight(Height);
}

float Controller::GetHeight(void) const
{
	return static_cast<physx::Cct::CapsuleController*>(m_PxController.get())->getHeight();
}

void Controller::SetRadius(float Radius)
{
	static_cast<physx::Cct::CapsuleController*>(m_PxController.get())->setRadius(Radius);
}

float Controller::GetRadius(void) const
{
	return static_cast<physx::Cct::CapsuleController*>(m_PxController.get())->getRadius();
}

void Controller::SetStepOffset(float StepOffset)
{
	m_PxController->setStepOffset(StepOffset);
}

float Controller::GetStepOffset(void) const
{
	return m_PxController->getStepOffset();
}

void Controller::SetContactOffset(float ContactOffset)
{
	m_PxController->setContactOffset(ContactOffset);
}

float Controller::GetContactOffset(void) const
{
	return m_PxController->getContactOffset();
}

void Controller::SetUpDirection(const my::Vector3 & Up)
{
	m_PxController->setUpDirection((physx::PxVec3&)Up);
}

my::Vector3 Controller::GetUpDirection(void) const
{
	return (my::Vector3&)m_PxController->getUpDirection();
}

void Controller::SetPosition(const my::Vector3 & Pos)
{
	m_PxController->setPosition(physx::PxExtendedVec3(Pos.x, Pos.y, Pos.z));
}

my::Vector3 Controller::GetPosition(void) const
{
	return (my::Vector3&)physx::toVec3(m_PxController->getPosition());
}

void Controller::SetFootPosition(const my::Vector3 & Pos)
{
	m_PxController->setFootPosition(physx::PxExtendedVec3(Pos.x, Pos.y, Pos.z));
}

my::Vector3 Controller::GetFootPosition(void) const
{
	return (my::Vector3&)physx::toVec3(m_PxController->getFootPosition());
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
	return (flags & physx::Cct::STF_VALIDATE_TRIANGLE_DOWN) ? physx::PxControllerCollisionFlag::eCOLLISION_DOWN :
		(flags & physx::Cct::STF_VALIDATE_TRIANGLE_SIDE) ? physx::PxControllerCollisionFlag::eCOLLISION_SIDES : 0;
}

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
	return physx::PxControllerBehaviorFlag::eCCT_USER_DEFINED_RIDE;
}

physx::PxControllerBehaviorFlags Controller::getBehaviorFlags(const physx::PxController & controller)
{
	return physx::PxControllerBehaviorFlag::eCCT_USER_DEFINED_RIDE;
}

physx::PxControllerBehaviorFlags Controller::getBehaviorFlags(const physx::PxObstacle & obstacle)
{
	return physx::PxControllerBehaviorFlag::eCCT_USER_DEFINED_RIDE;
}
