// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "stdafx.h"
#include "PlayerAgent.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "ChildView.h"
#include "Controller.h"
#include "Steering.h"
#include "ActionTrack.h"
#include "Material.h"
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include "rapidxml.hpp"

using namespace my;

ActionTbl::ActionTbl(void)
	: Jump(new Action(0.5f))
	, Climb(new Action(0.6f))
{
	boost::shared_ptr<ActionTrackVelocity> JumpVel(new ActionTrackVelocity);
	JumpVel->AddKeyFrame(0.0f, 0.1f);
	Jump->AddTrack(JumpVel);

	boost::shared_ptr<ActionTrackPose> ClimbPos(new ActionTrackPose);
	ClimbPos->AddKeyFrame(0.0f, 0.5f);
	Climb->AddTrack(ClimbPos);
}

NodeRunBlendList::NodeRunBlendList(const char* Name)
	: AnimationNodeBlendList(Name, 4)
{
}

void NodeRunBlendList::Tick(float fElapsedTime, float fTotalWeight)
{
	PlayerAgent* Agent = static_cast<Animator*>(GetTopNode())->m_Actor->GetFirstComponent<PlayerAgent>();
	if (Agent->m_Suspending <= 0.0f)
	{
		if (GetTargetWeight(2) < 0.5f)
		{
			SetActiveChild(2, 0.3f);
		}
		dynamic_cast<AnimationNodeSequence*>(m_Childs[2].get())->m_Rate = Agent->m_Steering->m_Speed / 5.2f;
	}
	else if (Agent->m_Steering->m_Speed > 0.1f)
	{
		if (GetTargetWeight(1) < 0.5f)
		{
			SetActiveChild(1, 0.1f);
		}
		dynamic_cast<AnimationNodeSequence*>(m_Childs[1].get())->m_Rate = Agent->m_Steering->m_Speed / 2.6f;
	}
	else
	{
		if (GetTargetWeight(0) < 0.5f)
		{
			SetActiveChild(0, 0.1f);
		}
	}
	AnimationNodeBlendList::Tick(fElapsedTime, fTotalWeight);
}

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

	//m_Animator->RemoveChild(0); // ! AnimationNodeSequence destructor _ASSERT(false)
}

void PlayerAgent::Update(float fElapsedTime)
{
	if (theApp.m_keyboard->IsKeyPress(KeyCode::KC_ESCAPE))
	{
		AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_TOOLS_PLAYING);
	}

	Vector3 pos = m_Controller->GetFootPosition();
	Quaternion rot;
	if (m_Steering->m_Speed > 0)
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
		Vector3 right = model_view_camera->m_View.getColumn<0>().xyz;
		Vector3 forward = right.cross(m_Controller->GetUpDirection()).normalize(Vector3(1, 0, 0));
		m_MoveDir = forward * dir.y + right * dir.x;
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
			HorizontalVel = m_MoveDir * HorizontalSpeed;
		}
		else
		{
			HorizontalVel = Vector3(m_Steering->m_Forward.xz(), 0).normalize(Vector3(0, 0, 0)) * m_Steering->m_Speed;
		}
		boost::dynamic_pointer_cast<ActionTrackVelocity>(ActionTbl::getSingleton().Jump->m_TrackList[0])->m_ParamVelocity =
			HorizontalVel + Vector3(0, sqrt(-1.0f * 2.0f * theApp.default_physx_scene_gravity.y), 0);
		m_Actor->PlayAction(ActionTbl::getSingleton().Jump.get());
		m_Suspending = 0.0f;
	}

	if (!theApp.default_player_mesh.empty() && m_Meshes.empty())
	{
		my::CachePtr cache = my::ResourceMgr::getSingleton().OpenIStream(theApp.default_player_mesh.c_str())->GetWholeCache();
		cache->push_back(0);
		rapidxml::xml_document<char> doc;
		doc.parse<0>((char*)&(*cache)[0]);

		const rapidxml::xml_node<char>* node_root = &doc;
		DEFINE_XML_NODE_SIMPLE(mesh, root);
		DEFINE_XML_NODE_SIMPLE(submeshes, mesh);
		DEFINE_XML_NODE_SIMPLE(submesh, submeshes);
		for (int submesh_i = 0; node_submesh != NULL; node_submesh = node_submesh->next_sibling(), submesh_i++)
		{
			MaterialPtr mtl(new Material());
			mtl->m_Shader = "shader/mtl_BlinnPhong.fx";
			mtl->m_PassMask = Material::PassMaskShadowNormalOpaque;
			mtl->SetParameter("g_DiffuseTexture", std::string("texture/Checker.bmp"));
			mtl->SetParameter("g_NormalTexture", std::string("texture/Normal.dds"));
			mtl->SetParameter("g_SpecularTexture", std::string("texture/Gray.dds"));

			MeshComponentPtr mesh(new MeshComponent(NULL));
			mesh->m_MeshPath = theApp.default_player_mesh;
			mesh->m_MeshSubMeshId = submesh_i;
			mesh->SetMaterial(mtl);
			m_Actor->InsertComponent(mesh);
			m_Meshes.push_back(mesh);
		}
	}
}

void PlayerAgent::OnPxThreadSubstep(float dtime)
{
	if (m_Suspending > 0.0f)
	{
		m_Suspending -= dtime;
	}

	Vector3 disp;
	if (m_Actor->TickActionAndGetDisplacement(dtime, disp))
	{
		Vector3 vel = disp / dtime;
		m_VerticalSpeed = vel.dot(m_Controller->GetUpDirection());
		m_Steering->SetVelocity(vel.slide(m_Controller->GetUpDirection()));
	}
	else if (m_Suspending <= 0.0f)
	{
		_ASSERT(m_Controller->GetUpDirection() == Vector3(0, 1, 0));
		m_VerticalSpeed += theApp.default_physx_scene_gravity.y * dtime;
		Vector3 vel = m_Steering->GetVelocity();
		vel *= 1.0f - theApp.default_player_air_drag * dtime;
		m_Steering->SetVelocity(vel);
		disp = (vel + m_Controller->GetUpDirection() * m_VerticalSpeed) * dtime;
	}
	else
	{
		m_VerticalSpeed += theApp.default_physx_scene_gravity.y * dtime;
		m_Steering->m_MaxSpeed = theApp.m_keyboard->IsKeyDown(KeyCode::KC_LSHIFT) ? theApp.default_player_run_speed * 4 : theApp.default_player_run_speed;
		Vector3 vel = m_Steering->SeekDir(m_MoveDir * theApp.default_player_seek_force, dtime) + m_Controller->GetUpDirection() * m_VerticalSpeed;
		disp = vel * dtime;
	}

	unsigned int moveFlags = m_Controller->Move(disp, 0.001f, dtime, theApp.default_physx_shape_filterword0);
	if (moveFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
	{
		m_VerticalSpeed = Lerp(m_VerticalSpeed, 0.0f, 1.0f - pow(0.5f, 30 * dtime));
		m_Suspending = 0.2f;
	}
}

void PlayerAgent::OnPxThreadShapeHit(my::EventArg* arg)
{
	ShapeHitEventArg* hit = static_cast<ShapeHitEventArg*>(arg);
	if (hit->flag & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
	{
	}
}
