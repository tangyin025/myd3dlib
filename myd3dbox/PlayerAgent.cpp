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

class ActionTbl : public my::Singleton<ActionTbl>
{
public:
	boost::shared_ptr<Action> Jump;

	ActionTbl(void)
		: Jump(new Action)
	{
		boost::shared_ptr<ActionTrackVelocity> JumpVel(new ActionTrackVelocity);
		JumpVel->AddKeyFrame(0.0f, 0.1f);
		Jump->AddTrack(JumpVel);
	}
};

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

DWORD PlayerAgent::GetComponentType(void) const
{
	return ComponentTypeScript;
}

void PlayerAgent::RequestResource(void)
{
	Component::RequestResource();

	PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Actor->m_Node->GetTopNode());
	scene->m_EventPxThreadSubstep.connect(boost::bind(&PlayerAgent::OnPxThreadSubstep, this, boost::placeholders::_1));

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

	if (!m_Skel)
	{
		// ! TODO: do it offline
		m_Skel = theApp.LoadSkeleton(theApp.default_player_skeleton.c_str());
		m_Skel->AddOgreSkeletonAnimationFromFile("character/jack_run.skeleton.xml");
		m_Skel->AddOgreSkeletonAnimationFromFile("character/jack_jump.skeleton.xml");
		m_Skel->AddOgreSkeletonAnimationFromFile("character/jack_drop.skeleton.xml");
		m_Skel->AddOgreSkeletonAnimationFromFile("character/jack_land.skeleton.xml");
		m_Skel->AddOgreSkeletonAnimationFromFile("character/jack_lock_stand.skeleton.xml");
		m_Skel->AddOgreSkeletonAnimationFromFile("character/jack_lock_run_back.skeleton.xml");
		m_Skel->AddOgreSkeletonAnimationFromFile("character/jack_lock_run_left.skeleton.xml");
		m_Skel->AddOgreSkeletonAnimationFromFile("character/jack_lock_run_right.skeleton.xml");
		m_Skel->AddOgreSkeletonAnimationFromFile("character/jack_lock_stand_aim.skeleton.xml");
		m_Skel->AddOgreSkeletonAnimationFromFile("character/jack_aim.skeleton.xml");
		m_Skel->AddOgreSkeletonAnimationFromFile("character/jack_aim_attack.skeleton.xml");
		m_Skel->AddOgreSkeletonAnimationFromFile("character/jack_aim_up.skeleton.xml");
		m_Skel->AddOgreSkeletonAnimationFromFile("character/jack_aim_attack_up.skeleton.xml");
		m_Skel->AddOgreSkeletonAnimationFromFile("character/jack_aim_down.skeleton.xml");
		m_Skel->AddOgreSkeletonAnimationFromFile("character/jack_aim_attack_down.skeleton.xml");
		m_Skel->AddOgreSkeletonAnimationFromFile("character/jack_attack_sword.skeleton.xml");
		m_Skel->AddOgreSkeletonAnimationFromFile("character/jack_attack2_sword.skeleton.xml");
		m_Skel->Transform(Matrix4::Compose(Vector3(1, 1, 1), Quaternion::Identity(), Vector3(0, -(0.65 + 0.1 + 0.1) / theApp.default_player_scale, 0)));
	}
}

void PlayerAgent::ReleaseResource(void)
{
	Component::ReleaseResource();

	PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Actor->m_Node->GetTopNode());
	scene->m_EventPxThreadSubstep.disconnect(boost::bind(&PlayerAgent::OnPxThreadSubstep, this, boost::placeholders::_1));

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
	const Quaternion rot = m_Steering->m_Speed > 0 ?
		Quaternion::RotationFromToSafe(Vector3(0, 0, 1), m_Steering->m_Forward) : m_Actor->m_Rotation;
	if (pos.y > m_Actor->m_Position.y + EPSILON_E3)
	{
		m_Actor->SetPose(Vector3(pos.x, Lerp(m_Actor->m_Position.y, pos.y, 1.0f - pow(0.7, 30 * fElapsedTime)), pos.z), rot);
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
		float angle = atan2(dir.x, dir.y) + model_view_camera->m_Euler.y;
		m_MoveDir = Quaternion::RotationEulerAngles(0, angle, 0) * Vector3(0, 0, 1);
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
			boost::regex reg("([^;]+);(\\w*);(\\d+)");
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
				m_Meshes[i]->m_MeshSubMeshName = what[2];
				m_Meshes[i]->m_MeshSubMeshId = boost::lexical_cast<int>(what[3]);
				m_Meshes[i]->SetMaterial(mtl);
				m_Actor->InsertComponent(m_Meshes[i]);
			}
		}
	}

	m_Animator->Tick(fElapsedTime, 1.0f);
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
		m_VerticalSpeed += theApp.default_physx_scene_gravity.y * dtime;
		Vector3 vel((m_Steering->m_Forward * m_Steering->m_Speed).xz(), m_VerticalSpeed);
		disp = vel * dtime;
	}
	else
	{
		m_VerticalSpeed += theApp.default_physx_scene_gravity.y * dtime;
		Vector3 vel(m_Steering->SeekDir(m_MoveDir * theApp.default_player_seek_force, dtime).xz(), m_VerticalSpeed);
		disp = vel * dtime;
	}

	physx::PxControllerCollisionFlags moveFlags =
		(physx::PxControllerCollisionFlags)m_Controller->Move(disp, 0.001f, dtime, 0x01);
	if (moveFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
	{
		m_VerticalSpeed = 0;
		m_Suspending = 0.2f;
	}
}
