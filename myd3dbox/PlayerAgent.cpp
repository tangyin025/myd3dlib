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
	: Jump(new Action)
	, Climb(new Action)
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
		: AnimationNodeBlendList(Name, 3)
		, m_Agent(Agent)
		, m_Rate(1.0f)
	{
	}


	virtual void Tick(float fElapsedTime, float fTotalWeight)
	{
		if (m_Agent->m_Suspending <= 0.0f)
		{
			if (GetTargetWeight(0) < 0.5f)
			{
				SetActiveChild(0, 0.5f);
				m_Rate = 1.0f;
			}
		}
		else if (m_Agent->m_Steering->m_Speed < 0.1f)
		{
			if (GetTargetWeight(1) < 0.5f)
			{
				SetActiveChild(1, 0.1f);
				m_Rate = 1.0f;
			}
		}
		else
		{
			if (GetTargetWeight(2) < 0.5f)
			{
				SetActiveChild(2, 0.1f);
			}
			m_Rate = m_Agent->m_Steering->m_Speed / 2.6f;
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

	PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Actor->m_Node->GetTopNode());
	scene->m_EventPxThreadSubstep.connect(boost::bind(&PlayerAgent::OnPxThreadSubstep, this, boost::placeholders::_1));

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
		m_Skel->AdjustAnimationRoot(Bone(-m_Controller->GetFootOffset() / theApp.default_player_scale));

		m_Animator->AddIK(m_Skel->GetBoneIndex("joint1"), m_Skel->m_boneHierarchy, 0.08f, theApp.default_physx_shape_filterword0);
		m_Animator->AddIK(m_Skel->GetBoneIndex("joint82"), m_Skel->m_boneHierarchy, 0.08f, theApp.default_physx_shape_filterword0);
	}
}

void PlayerAgent::ReleaseResource(void)
{
	Component::ReleaseResource();

	PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Actor->m_Node->GetTopNode());
	scene->m_EventPxThreadSubstep.disconnect(boost::bind(&PlayerAgent::OnPxThreadSubstep, this, boost::placeholders::_1));

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

	const Vector3 pos = m_Controller->GetPosition();
	const Quaternion rot = m_Steering->m_Speed > 0 ? Quaternion::RotationFromToSafe(Vector3(0, 0, 1), Vector3(m_Steering->m_Forward.xz(), 0)) : m_Actor->m_Rotation;
	if (m_Suspending > 0.0f && pos.y > m_Actor->m_Position.y + EPSILON_E3)
	{
		m_Actor->SetPose(Vector3(pos.x, Lerp(Max(m_Actor->m_Position.y, pos.y - m_Controller->GetStepOffset()), pos.y, 1.0f - pow(0.7f, 30 * fElapsedTime)), pos.z), rot);
	}
	else
	{
		m_Actor->SetPose(pos, rot);
	}

	if (m_Suspending > 0.0f)
	{
		m_Suspending -= fElapsedTime;
	}

	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CChildView* pView = DYNAMIC_DOWNCAST(CChildView, pFrame->GetActiveView());
	ASSERT_VALID(pView);

	ModelViewerCamera* model_view_camera = dynamic_cast<ModelViewerCamera*>(pView->m_Camera.get());
	model_view_camera->m_Euler.y -= D3DXToRadian(theApp.m_mouse->GetX()) * 0.5;
	model_view_camera->m_Euler.x -= D3DXToRadian(theApp.m_mouse->GetY()) * 0.25;

	Vector2 dir;
	dir.x = (theApp.m_keyboard->IsKeyDown(KeyCode::KC_A) ? -1.0f : 0.0f) + (theApp.m_keyboard->IsKeyDown(KeyCode::KC_D) ? 1.0f : 0.0f);
	dir.y = (theApp.m_keyboard->IsKeyDown(KeyCode::KC_W) ? -1.0f : 0.0f) + (theApp.m_keyboard->IsKeyDown(KeyCode::KC_S) ? 1.0f : 0.0f);
	float lensq = dir.magnitudeSq();
	if (lensq > 0)
	{
		dir /= sqrt(lensq);
		float angle = atan2(dir.x, dir.y);
		Vector3 up = m_Controller->GetUpDirection();
		if (up.y > 0.5f)
		{
			m_MoveDir = Quaternion::RotationAxis(up, angle) * up.perpendicular(model_view_camera->m_View.getColumn<2>().xyz).normalize(Vector3(0, 0, 0));
		}
		else
		{
			m_MoveDir = Quaternion::RotationAxis(up, angle) * up.perpendicular(Vector3(0, -1, 0)).normalize(Vector3(0, 0, 0));
		}
	}
	else
	{
		m_MoveDir = Vector3(0, 0, 0);
	}

	model_view_camera->m_LookAt = m_Actor->m_World.getRow<3>().xyz + Vector3(0, 0.85, 0);
	model_view_camera->m_Distance = Lerp(model_view_camera->m_Distance, theApp.default_player_look_distance, 1.0 - pow(0.5f, 30 * fElapsedTime));
	model_view_camera->UpdateViewProj();

	if (theApp.m_keyboard->IsKeyPress(KeyCode::KC_SPACE))
	{
		boost::dynamic_pointer_cast<ActionTrackVelocity>(ActionTbl::getSingleton().Jump->m_TrackList[0])->m_ParamVelocity =
			Vector3((lensq > 0 ? m_MoveDir * m_Steering->m_MaxSpeed : m_Steering->m_Forward * m_Steering->m_Speed).xz(), sqrt(-1.0f * 2.0f * theApp.default_physx_scene_gravity.y));
		m_Actor->PlayAction(ActionTbl::getSingleton().Jump.get(), 0.5f);
		m_Suspending = 0.0f;
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

	m_Animator->Tick(fElapsedTime, 1.0f);

	m_Steering->m_MaxSpeed = theApp.m_keyboard->IsKeyDown(KeyCode::KC_LSHIFT) ? theApp.default_player_max_speed * 10 : theApp.default_player_max_speed;
}

void PlayerAgent::OnPxThreadSubstep(float dtime)
{
	Vector3 disp;
	if (m_Actor->TickActionAndGetDisplacement(dtime, disp))
	{
		Vector3 vel = disp / dtime;
		m_VerticalSpeed = vel.y;
		m_Steering->m_Speed = vel.magnitude2D();
		if (m_Steering->m_Speed > 0)
		{
			m_Steering->m_Forward = Vector3(vel.xz(), 0) / m_Steering->m_Speed;
		}
	}
	else if (m_Suspending <= 0.0f)
	{
		Vector3 vel = m_Steering->m_Forward * m_Steering->m_Speed + m_Controller->GetUpDirection() * m_VerticalSpeed;
		vel.y += theApp.default_physx_scene_gravity.y * dtime;
		m_VerticalSpeed = vel.y;
		m_Steering->m_Speed = vel.magnitude2D();
		if (m_Steering->m_Speed > 0)
		{
			m_Steering->m_Forward = Vector3(vel.xz(), 0) / m_Steering->m_Speed;
		}
		m_Controller->SetUpDirection(Vector3(0, 1, 0));
		disp = vel * dtime;
	}
	else
	{
		m_VerticalSpeed += theApp.default_physx_scene_gravity.y * dtime;
		Vector3 vel = m_Steering->SeekDir(m_MoveDir * theApp.default_player_seek_force, dtime) + m_Controller->GetUpDirection() * m_VerticalSpeed;
		disp = vel * dtime;
	}

	physx::PxControllerCollisionFlags moveFlags =
		(physx::PxControllerCollisionFlags)m_Controller->Move(disp, 0.001f, dtime, theApp.default_physx_shape_filterword0);
	if (moveFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
	{
		m_VerticalSpeed = Lerp(m_VerticalSpeed, 0.0f, 1.0f - pow(0.5f, 30 * dtime));
		m_Suspending = 0.2f;
		m_Controller->SetUpDirection(m_Controller->GetContactNormalDownPass());
	}
	else if (moveFlags & physx::PxControllerCollisionFlag::eCOLLISION_SIDES)
	{
		m_VerticalSpeed = Lerp(m_VerticalSpeed, 0.0f, 1.0f - pow(0.5f, 30 * dtime));
		m_Suspending = 0.2f;
		m_Controller->SetUpDirection(m_Controller->GetContactNormalSidePass());
	}
}

void PlayerAgent::OnPxThreadShapeHit(my::EventArg* arg)
{
	ShapeHitEventArg* hit = static_cast<ShapeHitEventArg*>(arg);
	if (hit->flag & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
	{
		//m_DownTouchedCmp = hit->other_cmp;
		//m_DownTouchedPos = hit->worldPos;
		//m_DownTouchedNormal = hit->worldNormal;

		//if (m_Steering->m_Speed < 0.001f && (hit->worldPos.xz() - m_Actor->m_Position.xz()).magnitudeSq() > 0.09f * 0.09f)
		//{
		//	hit->behaviorflags = physx::PxControllerBehaviorFlag::eCCT_SLIDE;
		//}
	}
	else if (hit->flag & physx::PxControllerCollisionFlag::eCOLLISION_SIDES)
	{
		//m_SideTouchedCmp = hit->other_cmp;
		//m_SideTouchedPos = hit->worldPos;
		//m_SideTouchedNormal = hit->worldNormal;
	}
	else if (hit->flag & physx::PxControllerCollisionFlag::eCOLLISION_UP)
	{

	}
}
