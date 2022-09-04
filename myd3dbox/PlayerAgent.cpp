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
	model_view_camera->m_Distance = Lerp(model_view_camera->m_Distance, 4.0f, 1.0 - pow(0.5f, 30 * fElapsedTime));
	model_view_camera->UpdateViewProj();

	if (theApp.m_keyboard->IsKeyPress(KeyCode::KC_SPACE))
	{
		boost::dynamic_pointer_cast<ActionTrackVelocity>(ActionTbl::getSingleton().Jump->m_TrackList[0])->m_ParamVelocity =
			Vector3((lensq > 0 ? m_MoveDir * m_Steering->m_MaxSpeed : m_Steering->m_Forward * m_Steering->m_Speed).xz(), sqrt(-1.0f * 2.0f * theApp.default_physx_scene_gravity.y));
		m_Actor->PlayAction(ActionTbl::getSingleton().Jump.get(), 0.5f);
	}

	for (int i = 0; i < theApp.default_player_mesh_list.size(); i++)
	{
		if (i >= m_Meshes.size())
		{
			MaterialPtr mtl(new Material());
			mtl->m_Shader = "shader/mtl_BlinnPhong.fx";
			mtl->m_PassMask = Material::PassMaskShadowNormalOpaque;
			mtl->SetParameter("g_DiffuseTexture", std::string("texture/Checker.bmp"));
			mtl->SetParameter("g_NormalTexture", std::string("texture/Normal.dds"));
			mtl->SetParameter("g_SpecularTexture", std::string("texture/White.dds"));

			m_Meshes.push_back(MeshComponentPtr(new MeshComponent(NULL)));
			m_Meshes[i]->m_MeshPath = theApp.default_player_mesh_list[i];
			m_Meshes[i]->SetMaterial(mtl);
			m_Actor->InsertComponent(m_Meshes[i]);
		}
	}
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
	else if (m_Jumping)
	{
		m_VerticalSpeed += theApp.default_physx_scene_gravity.y * dtime;
		Vector3 vel((m_Steering->m_Forward * m_Steering->m_Speed).xz(), m_VerticalSpeed);
		disp = vel * dtime;
	}
	else
	{
		Vector3 vel = m_Steering->SeekDir(m_MoveDir * 50.0f, dtime);
		disp = vel * dtime;
	}

	physx::PxControllerCollisionFlags moveFlags =
		(physx::PxControllerCollisionFlags)m_Controller->Move(disp, 0.001f, dtime, 0x01);
	if (moveFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
	{
		m_VerticalSpeed = 0;
		if (m_Jumping)
		{
			m_Jumping = false;
		}
	}
	else
	{
		m_Jumping = true;
	}
}
