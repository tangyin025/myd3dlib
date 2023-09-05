#include "stdafx.h"
#include "PlayerAgent.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "ChildView.h"
#include "Controller.h"
#include "Steering.h"
#include "Animator.h"
#include "ActionTrack.h"
#include "Material.h"
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

using namespace my;

ActionTbl::ActionTbl(void)
	: Jump(new Action(0.5f))
	, Climb(new Action(0.5f))
{
	boost::shared_ptr<ActionTrackVelocity> JumpVel(new ActionTrackVelocity);
	JumpVel->AddKeyFrame(0.0f, 0.1f);
	Jump->AddTrack(JumpVel);

	boost::shared_ptr<ActionTrackPose> ClimbPos(new ActionTrackPose);
	ClimbPos->AddKeyFrame(0.0f, 0.5f);
	Climb->AddTrack(ClimbPos);
}

class NodeRunBlendList : public AnimationNodeBlendList
{
public:
	PlayerAgent* m_Agent;

	float m_Rate;

	NodeRunBlendList(const char* Name, PlayerAgent* Agent)
		: AnimationNodeBlendList(Name, 4)
		, m_Agent(Agent)
		, m_Rate(1.0f)
	{
	}


	virtual void Tick(float fElapsedTime, float fTotalWeight)
	{
		const Vector3& Up = m_Agent->m_Controller->GetUpDirection();
		if (m_Agent->m_Suspending <= 0.0f)
		{
			if (GetTargetWeight(0) < 0.5f)
			{
				SetActiveChild(0, 0.5f);
				m_Rate = 1.0f;
			}
		}
		else if (GetTargetWeight(3) < 0.5f && Up.y < theApp.default_player_climb_enter_slope
			|| GetTargetWeight(3) >= 0.5f && Up.y <= theApp.default_player_climb_leave_slope)
		{
			if (GetTargetWeight(3) < 0.5f)
			{
				SetActiveChild(3, 0.1f);
			}
			m_Rate = m_Agent->m_Steering->m_Speed / 2.6f;
		}
		else if (m_Agent->m_Steering->m_Speed > 0.1f)
		{
			if (GetTargetWeight(2) < 0.5f)
			{
				SetActiveChild(2, 0.1f);
			}
			m_Rate = m_Agent->m_Steering->m_Speed / 2.6f;
		}
		else
		{
			if (GetTargetWeight(1) < 0.5f)
			{
				SetActiveChild(1, 0.1f);
				m_Rate = 1.0f;
			}
		}
		AnimationNodeBlendList::Tick(fElapsedTime * m_Rate, fTotalWeight);
	}
};

PlayerAgent::~PlayerAgent(void)
{
	_ASSERT(!IsRequested());
}

void PlayerAgent::RequestResource(void)
{
	Component::RequestResource();

	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	pFrame->m_EventPxThreadSubstep.connect(boost::bind(&PlayerAgent::OnPxThreadSubstep, this, boost::placeholders::_1));

	m_Actor->m_EventPxThreadShapeHit.connect(boost::bind(&PlayerAgent::OnPxThreadShapeHit, this, boost::placeholders::_1));

	theApp.m_mouse->Unacquire();
	theApp.m_mouse->SetCooperativeLevel(AfxGetMainWnd()->GetSafeHwnd(), DISCL_FOREGROUND | DISCL_EXCLUSIVE);

	VERIFY(m_Controller = m_Actor->GetFirstComponent<Controller>());
	VERIFY(m_Steering = m_Actor->GetFirstComponent<Steering>());
	VERIFY(m_Animator = m_Actor->GetFirstComponent<Animator>());

	AnimationNodePtr node_run_blend_list(new NodeRunBlendList("node_run_blend_list", this));
	node_run_blend_list->SetChild(0, AnimationNodePtr(new AnimationNodeSequence("clip_drop")));
	node_run_blend_list->SetChild(1, AnimationNodePtr(new AnimationNodeSequence("clip_stand")));
	node_run_blend_list->SetChild(2, AnimationNodePtr(new AnimationNodeSequence("clip_run", 1.0f, true, "move")));
	node_run_blend_list->SetChild(3, AnimationNodePtr(new AnimationNodeSequence("clip_run", 1.0f, true, "move")));

	AnimationNodeSlotPtr node_run_blend_list_slot(new AnimationNodeSlot("node_run_blend_list_slot"));
	node_run_blend_list_slot->SetChild(0, node_run_blend_list);

	m_Animator->SetChild(0, node_run_blend_list_slot);
	m_Animator->ReloadSequenceGroup();

	if (!m_Skel && !theApp.default_player_anim_list.empty() && theApp.CheckPath(theApp.default_player_anim_list.front().c_str()))
	{
		m_Skel = theApp.LoadSkeleton(theApp.default_player_anim_list.front().c_str());
		for (int i = 1; i < theApp.default_player_anim_list.size(); i++)
		{
			m_Skel->AddOgreSkeletonAnimationFromFile(theApp.default_player_anim_list[i].c_str());
		}

		m_Animator->AddIK(m_Skel->GetBoneIndex("joint1"), m_Skel->m_boneHierarchy, 0.08f, theApp.default_physx_shape_filterword0);
		m_Animator->AddIK(m_Skel->GetBoneIndex("joint82"), m_Skel->m_boneHierarchy, 0.08f, theApp.default_physx_shape_filterword0);
	}
}

void PlayerAgent::ReleaseResource(void)
{
	Component::ReleaseResource();

	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	pFrame->m_EventPxThreadSubstep.disconnect(boost::bind(&PlayerAgent::OnPxThreadSubstep, this, boost::placeholders::_1));

	m_Actor->m_EventPxThreadShapeHit.disconnect(boost::bind(&PlayerAgent::OnPxThreadShapeHit, this, boost::placeholders::_1));

	theApp.m_mouse->Unacquire();
	theApp.m_mouse->SetCooperativeLevel(AfxGetMainWnd()->GetSafeHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	for (int i = 0; i < m_Meshes.size(); i++)
	{
		m_Actor->RemoveComponent(m_Meshes[i]->GetSiblingId());
	}
	m_Meshes.clear();

	m_Animator->RemoveChild(0);
}

void PlayerAgent::Update(float fElapsedTime)
{
	if (theApp.m_keyboard->IsKeyPress(KeyCode::KC_ESCAPE))
	{
		AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_TOOLS_PLAYING);
	}

	Vector3 pos = m_Controller->GetFootPosition();
	const Vector3& up = m_Controller->GetUpDirection();
	AnimationNodeBlendListPtr node_run_blend_list = boost::dynamic_pointer_cast<AnimationNodeBlendList>(m_Animator->GetChild(0)->GetChild(0));
	Quaternion rot;
	if (node_run_blend_list->GetTargetWeight(3) >= 0.5f)
	{
		rot = Quaternion::RotationAxis(Vector3(0, 1, 0), atan2f(-up.x, -up.z));
	}
	else if (m_Steering->m_Speed > 0)
	{
		rot = Quaternion::RotationAxis(Vector3(0, 1, 0), atan2f(m_Steering->m_Forward.x, m_Steering->m_Forward.z));
	}
	else
	{
		rot = m_Actor->m_Rotation;
	}
	if (m_Suspending > 0.0f)
	{
		m_ClimbLerp = m_ClimbLerp.lerp(m_Controller->GetUpDirection(), 1.0f - pow(0.7f, 30 * fElapsedTime)).normalize();
		Vector3 act_pos = pos - m_ClimbLerp * Clamp(m_ClimbLerp.dot(pos - m_Actor->m_Position), 0.0f, m_Controller->GetStepOffset());
		m_Actor->SetPose(act_pos.lerp(pos, 1.0f - pow(0.7f, 30 * fElapsedTime)), rot);
	}
	else
	{
		m_Actor->SetPose(pos, rot);
	}

	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CChildView* pView = DYNAMIC_DOWNCAST(CChildView, pFrame->GetActiveView());
	ASSERT_VALID(pView);

	ModelViewerCamera* model_view_camera = dynamic_cast<ModelViewerCamera*>(pView->m_Camera.get());
	model_view_camera->m_Euler.y -= D3DXToRadian(theApp.m_mouse->GetX()) * 0.5;
	model_view_camera->m_Euler.x -= D3DXToRadian(theApp.m_mouse->GetY()) * 0.25;

	Vector2 dir(
		(theApp.m_keyboard->IsKeyDown(KeyCode::KC_A) ? -1.0f : 0.0f) + (theApp.m_keyboard->IsKeyDown(KeyCode::KC_D) ? 1.0f : 0.0f),
		(theApp.m_keyboard->IsKeyDown(KeyCode::KC_W) ? -1.0f : 0.0f) + (theApp.m_keyboard->IsKeyDown(KeyCode::KC_S) ? 1.0f : 0.0f));
	float lensq = dir.magnitudeSq();
	if (lensq > 0)
	{
		dir /= sqrt(lensq);
		if (node_run_blend_list->GetTargetWeight(3) < 0.5f)
		{
			Vector3 forward = model_view_camera->m_View.getColumn<0>().xyz.cross(up).normalize(Vector3(0, 0, 0));
			Vector3 right = up.cross(forward);
			m_MoveDir = forward * dir.y + right * dir.x;
		}
		else
		{
			Vector3 right = Vector3(0, 1, 0).cross(up).normalize(rot * Vector3(-1, 0, 0));
			Vector3 forward = right.cross(up);
			m_MoveDir = forward * dir.y + right * dir.x;
		}

		//if (m_Suspending > 0 && m_LastMoveFlags != 0)
		//{
		//	physx::PxSweepBuffer hit;
		//	physx::PxCapsuleGeometry capsule(m_Controller->GetRadius() + m_Controller->GetContactOffset(), m_Controller->GetHeight() * 0.5f);
		//	physx::PxQueryFilterData filterData = physx::PxQueryFilterData(
		//		physx::PxFilterData(theApp.default_physx_shape_filterword0, 0, 0, 0), physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC);
		//	my::Vector3 start = m_Controller->GetPosition() + m_Controller->GetContactNormalDownPass() * m_Controller->GetStepOffset() + m_MoveDir * 3.0f;
		//	if (pFrame->m_PxScene->sweep(capsule,
		//		physx::PxTransform((physx::PxVec3&)start, static_cast<physx::Cct::CapsuleController*>(m_Controller->m_PxController.get())->mUserParams.mQuatFromUp),
		//		(physx::PxVec3&)-m_Controller->GetContactNormalDownPass(),
		//		m_Controller->GetStepOffset() * 2,
		//		hit, physx::PxHitFlag::eDEFAULT, filterData, NULL, NULL, 0.0f))
		//	{
		//		my::Vector3 end = start - m_Controller->GetContactNormalDownPass() * hit.block.distance;
		//		m_MoveDir = (end - m_Controller->GetPosition()).normalize();
		//		m_Controller->SetUpDirection(m_MoveDir.perpendicular(m_Controller->GetContactNormalDownPass()).normalize(Vector3(0, 1, 0)));
		//	}
		//	else
		//	{
		//		my::Vector3 end = start - m_Controller->GetContactNormalDownPass() * m_Controller->GetStepOffset();
		//		m_MoveDir = (end - m_Controller->GetPosition()).normalize();
		//		m_Controller->SetUpDirection(m_MoveDir.perpendicular(m_Controller->GetContactNormalDownPass()).normalize(Vector3(0, 1, 0)));
		//	}
		//}
	}
	else
	{
		m_MoveDir = Vector3(0, 0, 0);
	}

	model_view_camera->m_LookAt = m_Actor->m_World.getRow<3>().xyz + Vector3(0, m_Controller->GetContactOffset() + m_Controller->GetHeight() + m_Controller->GetRadius(), 0);
	physx::PxSweepBuffer hit;
	physx::PxSphereGeometry sphere(0.1f);
	physx::PxQueryFilterData filterData = physx::PxQueryFilterData(
		physx::PxFilterData(theApp.default_physx_shape_filterword0 | theApp.default_player_water_filterword0, 0, 0, 0), physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC);
	bool status = pFrame->m_PxScene->sweep(sphere, physx::PxTransform((physx::PxVec3&)(model_view_camera->m_LookAt + model_view_camera->m_View.getColumn<2>().xyz * m_Controller->GetStepOffset())),
		(physx::PxVec3&)model_view_camera->m_View.getColumn<2>().xyz, theApp.default_player_look_distance, hit, physx::PxHitFlag::eDEFAULT, filterData);
	if (status && hit.block.distance > 0)
	{
		model_view_camera->m_Distance = m_Controller->GetStepOffset() + hit.block.distance;
	}
	else
	{
		model_view_camera->m_Distance = Lerp(model_view_camera->m_Distance, m_Controller->GetStepOffset() + theApp.default_player_look_distance, 1.0 - pow(0.5f, 30 * fElapsedTime));
	}
	model_view_camera->UpdateViewProj();

	if (theApp.m_keyboard->IsKeyPress(KeyCode::KC_SPACE))
	{
		Vector3 right = model_view_camera->m_View.getColumn<0>().xyz;
		Vector3 forward = right.cross(Vector3(0, 1, 0));
		float HorizontalSpeed = theApp.m_keyboard->IsKeyDown(KeyCode::KC_LSHIFT) ? theApp.default_player_run_speed * 4 : theApp.default_player_run_speed;
		Vector3 HorizontalVel;
		if (lensq > 0)
		{
			HorizontalVel = (forward * dir.y + right * dir.x) * HorizontalSpeed;
		}
		else if (node_run_blend_list->GetTargetWeight(3) >= 0.5f)
		{
			HorizontalVel = Vector3(up.xz(), 0).normalize(Vector3(0, 0, 0)) * HorizontalSpeed;
		}
		else
		{
			HorizontalVel = Vector3(m_Steering->m_Forward.xz(), 0).normalize(Vector3(0, 0, 0)) * m_Steering->m_Speed;
		}
		boost::dynamic_pointer_cast<ActionTrackVelocity>(ActionTbl::getSingleton().Jump->m_TrackList[0])->m_ParamVelocity =
			HorizontalVel + Vector3(0, sqrt(-1.0f * 2.0f * theApp.default_physx_scene_gravity.y), 0);
		m_Actor->PlayAction(ActionTbl::getSingleton().Jump.get(), 1.0f);
		m_Suspending = 0.0f;
		m_Controller->SetUpDirection(Vector3(0, 1, 0));
	}

	for (int i = 0; i < theApp.default_player_mesh_list.size(); i++)
	{
		if (i >= m_Meshes.size())
		{
			boost::regex reg("([^;]+);(\\d+)");
			boost::match_results<std::string::iterator> what;
			std::string::iterator start = theApp.default_player_mesh_list[i].begin();
			std::string::iterator end = theApp.default_player_mesh_list[i].end();
			if (boost::regex_search(start, end, what, reg, boost::match_default))
			{
				MaterialPtr mtl(new Material());
				mtl->m_Shader = "shader/mtl_BlinnPhong.fx";
				mtl->m_PassMask = Material::PassMaskShadowNormalOpaque;
				start = what[0].second;
				boost::regex reg_param(";(\\w+),([^;]+)");
				boost::match_results<std::string::iterator> what2;
				for (; boost::regex_search(start, end, what2, reg_param, boost::match_default); start = what2[0].second)
				{
					mtl->AddParameter(what2[1].str().c_str(), what2[2].str());
				}

				m_Meshes.push_back(MeshComponentPtr(new MeshComponent(NULL)));
				m_Meshes[i]->m_MeshPath = what[1];
				m_Meshes[i]->m_MeshSubMeshId = boost::lexical_cast<int>(what[2]);
				m_Meshes[i]->SetMaterial(mtl);
				m_Actor->InsertComponent(m_Meshes[i]);
			}
		}
	}
}

void PlayerAgent::OnPxThreadSubstep(float dtime)
{
	if (m_Suspending > 0.0f)
	{
		m_Suspending -= dtime;
	}

	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);

	physx::PxRaycastBuffer hit;
	physx::PxQueryFilterData filterData = physx::PxQueryFilterData(physx::PxFilterData(theApp.default_player_water_filterword0, 0, 0, 0),
		physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC /*| physx::PxQueryFlag::ePREFILTER*/ | physx::PxQueryFlag::eANY_HIT);
	if (pFrame->m_PxScene->raycast((physx::PxVec3&)(m_Controller->GetPosition() + Vector3(0, 1000, 0)),
		physx::PxVec3(0, -1, 0), 1000, hit, physx::PxHitFlag::eDISTANCE, filterData, NULL, NULL))
	{
		m_Submergence = my::Clamp((1000 - hit.block.distance) / theApp.default_player_swim_depth, 0.0f, 1.0f);
	}
	else
	{
		m_Submergence = 0;
	}

	Vector3 disp;
	const float gravity = theApp.default_physx_scene_gravity.y * (1.0f - theApp.default_player_water_buoyancy * m_Submergence);
	if (m_Actor->TickActionAndGetDisplacement(dtime, disp))
	{
		Vector3 vel = disp / dtime;
		m_VerticalSpeed = vel.dot(m_Controller->GetUpDirection());
		vel -= m_Controller->GetUpDirection() * m_VerticalSpeed;
		m_Steering->m_Speed = vel.magnitude();
		if (m_Steering->m_Speed > 0)
		{
			m_Steering->m_Forward = vel / m_Steering->m_Speed;
		}
	}
	else if (m_Suspending <= 0.0f)
	{
		//_ASSERT(m_Controller->GetUpDirection() == Vector3(0, 1, 0));
		m_VerticalSpeed += gravity * dtime;
		m_VerticalSpeed *= 1.0f - theApp.default_player_water_drag * m_Submergence * dtime;
		Vector3 vel = m_Steering->m_Forward * m_Steering->m_Speed +
			m_MoveDir * (theApp.m_keyboard->IsKeyDown(KeyCode::KC_LSHIFT) ? theApp.default_player_swim_force * 4 : theApp.default_player_swim_force) * m_Submergence * dtime;
		vel *= 1.0f - Lerp(theApp.default_player_air_drag, theApp.default_player_water_drag, m_Submergence) * dtime;
		m_Steering->m_Speed = vel.magnitude();
		if (m_Steering->m_Speed > 0)
		{
			m_Steering->m_Forward = vel / m_Steering->m_Speed;
		}
		disp = (vel + m_Controller->GetUpDirection() * m_VerticalSpeed) * dtime;
	}
	else
	{
		m_VerticalSpeed += gravity * dtime;
		m_Steering->m_MaxSpeed = theApp.m_keyboard->IsKeyDown(KeyCode::KC_LSHIFT) ? theApp.default_player_run_speed * 4 : theApp.default_player_run_speed;
		Vector3 vel = m_Steering->SeekDir(m_MoveDir * theApp.default_player_seek_force, dtime) + m_Controller->GetUpDirection() * m_VerticalSpeed;
		disp = vel * dtime;
	}

	unsigned int moveFlags = m_Controller->Move(disp, 0.001f, dtime, theApp.default_physx_shape_filterword0);
	if (moveFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
	{
		m_VerticalSpeed = Lerp(m_VerticalSpeed, 0.0f, 1.0f - pow(0.5f, 30 * dtime));
		m_Suspending = 0.2f;
		if (moveFlags & physx::PxControllerCollisionFlag::eCOLLISION_SIDES
			&& m_Controller->GetTouchedComponent() && m_Controller->GetContactNormalSidePass().y > m_Controller->GetSlopeLimit())
		{
			m_Controller->SetUpDirection(m_Controller->GetContactNormalSidePass());
		}
		else if(m_Controller->GetTouchedComponent())
		{
			m_Controller->SetUpDirection(m_Controller->GetContactNormalDownPass());
		}
	}
	else if (moveFlags & physx::PxControllerCollisionFlag::eCOLLISION_SIDES)
	{
		if (m_Suspending <= 0.0f)
		{
			m_VerticalSpeed = 0;
			m_Suspending = 0.2f;
			m_Controller->SetUpDirection(m_Controller->GetContactNormalSidePass());
		}
	}
	else if (!moveFlags && m_LastMoveFlags)
	{
		const Vector3 & up = m_Controller->GetUpDirection();
		if (up.y <= m_Controller->GetSlopeLimit())
		{
			Vector3 vel = m_Steering->m_Forward * m_Steering->m_Speed + m_Controller->GetUpDirection() * m_VerticalSpeed;
			m_VerticalSpeed = vel.y;
			m_Steering->m_Speed = m_Steering->m_MaxSpeed;
			m_Steering->m_Forward = -up;
		}
		m_Controller->SetUpDirection(Vector3(0, 1, 0));
	}
	m_LastMoveFlags = moveFlags;
}

void PlayerAgent::OnPxThreadShapeHit(my::EventArg* arg)
{
	ShapeHitEventArg* hit = static_cast<ShapeHitEventArg*>(arg);
	if (hit->flag & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
	{
	}
}
