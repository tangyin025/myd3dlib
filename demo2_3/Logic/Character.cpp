#include "StdAfx.h"
#include "Character.h"
#include "../Game.h"

using namespace my;

Character::Character(void)
	: Particle(my::Vector3(0,0,0), my::Vector3(0,0,0), my::Vector3(0,0,0), my::Vector3(0,0,0), 1, 0.8f)
	, m_HitState(0)
{
	Game::getSingleton().m_EventPxThreadSubstep.connect(boost::bind(&Character::OnPxThreadSubstep, this, _1));
	CreateController();
}

Character::~Character(void)
{
	Game::getSingleton().m_EventPxThreadSubstep.disconnect(boost::bind(&Character::OnPxThreadSubstep, this, _1));
	m_controller.reset();
}

void Character::CreateController(void)
{
	PxCapsuleControllerDesc cDesc;
	cDesc.radius = 0.15f;
	cDesc.height = 1.0f;
	cDesc.position.y = 5;
	cDesc.material = Game::getSingleton().m_PxMaterial.get();
	cDesc.callback = this;
	cDesc.behaviorCallback = this;
	m_controller.reset(
		Game::getSingleton().m_ControllerMgr->createController(*Game::getSingleton().m_sdk, Game::getSingleton().m_PxScene.get(), cDesc));
}

void Character::Update(float fElapsedTime)
{
}

void Character::OnPxThreadSubstep(float dtime)
{
	// 注意，多线程调用，不要在这里更新模型渲染数据数据
	velocity.y = velocity.y + PhysXContext::Gravity.y * dtime;
	m_HitState = 0;
	m_controller->move((PxVec3&)(velocity * dtime), 0.001f, dtime, PxControllerFilters());
	setPosition((Vector3&)toVec3(m_controller->getPosition()));
}

void Character::onShapeHit(const PxControllerShapeHit& hit)
{
	if (hit.worldNormal.y > 0 && hit.worldNormal.y > fabs(hit.worldNormal.x) && hit.worldNormal.y > fabs(hit.worldNormal.z))
	{
		velocity.y = 0;
		m_HitState |= HitStateGround;
	}
}

void Character::onControllerHit(const PxControllersHit& hit)
{
}

void Character::onObstacleHit(const PxControllerObstacleHit& hit)
{
}

PxU32 Character::getBehaviorFlags(const PxShape& shape)
{
	return 0;
}

PxU32 Character::getBehaviorFlags(const PxController& controller)
{
	return 0;
}

PxU32 Character::getBehaviorFlags(const PxObstacle& obstacle)
{
	return 0;
}

Player::Player(void)
{
}

LocalPlayer::LocalPlayer(void)
	: m_LookAngles(0,0,0)
	, m_LookDir(0,0,1)
	, m_LookDist(5)
	, m_FaceAngle(0)
	, m_FaceAngleInerp(0)
	, m_InputLtRt(0)
	, m_InputUpDn(0)
{
	Game::getSingleton().m_MouseMovedEvent = boost::bind(&LocalPlayer::OnMouseMove, this, _1);
	Game::getSingleton().m_MousePressedEvent = boost::bind(&LocalPlayer::OnMouseBtnDown, this, _1);
	Game::getSingleton().m_MouseReleasedEvent = boost::bind(&LocalPlayer::OnMouseBtnUp, this, _1);
	Game::getSingleton().m_KeyPressedEvent = boost::bind(&LocalPlayer::OnKeyDown, this, _1);
	Game::getSingleton().m_KeyReleasedEvent = boost::bind(&LocalPlayer::OnKeyUp, this, _1);

	if (Game::getSingleton().m_joystick)
	{
		Game::getSingleton().m_joystick->m_AxisMovedEvent = boost::bind(&LocalPlayer::OnJoystickAxisMove, this, _1);
		Game::getSingleton().m_joystick->m_PovMovedEvent = boost::bind(&LocalPlayer::OnJoystickPovMove, this, _1);
		Game::getSingleton().m_joystick->m_BtnPressedEvent = boost::bind(&LocalPlayer::OnJoystickBtnDown, this, _1);
		Game::getSingleton().m_joystick->m_BtnReleasedEvent = boost::bind(&LocalPlayer::OnJoystickBtnUp, this, _1);
	}

	CreateMeshComponent();
}

LocalPlayer::~LocalPlayer(void)
{
	Game::getSingleton().m_MouseMovedEvent.clear();
	Game::getSingleton().m_MousePressedEvent.clear();
	Game::getSingleton().m_MouseReleasedEvent.clear();
	Game::getSingleton().m_KeyPressedEvent.clear();
	Game::getSingleton().m_KeyReleasedEvent.clear();

	if (Game::getSingleton().m_joystick)
	{
		Game::getSingleton().m_joystick->m_AxisMovedEvent.clear();
		Game::getSingleton().m_joystick->m_PovMovedEvent.clear();
		Game::getSingleton().m_joystick->m_BtnPressedEvent.clear();
		Game::getSingleton().m_joystick->m_BtnReleasedEvent.clear();
	}

	m_MeshCmp.reset();
}

void LocalPlayer::CreateMeshComponent(void)
{
	MaterialPtr lambert1(new Material());
	lambert1->m_Shader = "lambert1.fx";
	lambert1->m_PassMask = RenderPipeline::PassMaskOpaque;
	lambert1->m_MeshColor = Vector4(1,1,1,1);
	lambert1->m_MeshTexture.m_Path = "texture/casual19_m_35.jpg";
	lambert1->m_NormalTexture.m_Path = "texture/casual19_m_35_normal.png";
	lambert1->m_SpecularTexture.m_Path = "texture/casual19_m_35_spec.png";

	AnimationNodeSequencePtr node_walk(new AnimationNodeSequence());
	node_walk->m_Name = "walk";
	node_walk->m_Root = "Bip01";

	AnimationNodeSequencePtr node_idle(new AnimationNodeSequence());
	node_idle->m_Name = "idle";
	node_idle->m_Root = "Bip01";

	AnimationNodeBlendBySpeedPtr node_speed(new AnimationNodeBlendBySpeed());
	node_speed->m_Childs[0] = node_idle;
	node_speed->m_Childs[1] = node_walk;

	AnimatorPtr anim(new Animator());
	anim->m_SkeletonRes.m_Path = "mesh/casual19_m_highpoly.skeleton.xml";
	anim->m_Node = node_speed;
	anim->m_Node->SetOwner(anim.get());
	anim->m_Character = this;

	m_MeshCmp.reset(new MeshComponent(my::AABB(-100,100), my::Matrix4::Scaling(Vector3(0.01f)), false));
	m_MeshCmp->m_lods.resize(1);
	m_MeshCmp->m_lods[0].m_MeshRes.m_Path = "mesh/casual19_m_highpoly.mesh.xml";
	m_MeshCmp->m_MaterialList.push_back(lambert1);
	m_MeshCmp->m_Animator = anim;

	Game::getSingleton().m_Root.AddComponent(m_MeshCmp.get(), m_MeshCmp->m_aabb.transform(m_MeshCmp->m_World), 0.1f);
}

void LocalPlayer::Update(float fElapsedTime)
{
	Player::Update(fElapsedTime);

	if (m_InputLtRt != 0 || m_InputUpDn != 0)
	{
		const Vector3 Right(m_LookDir.z, 0, -m_LookDir.x);
		const Vector3 Final(Vector3(
			m_InputLtRt * Right.x + m_InputUpDn * m_LookDir.x, 0, m_InputLtRt * Right.z + m_InputUpDn * m_LookDir.z).normalize());
		const float Speed = 5.0f;
		velocity.x = Final.x * Speed;
		velocity.z = Final.z * Speed;
		m_FaceAngle = atan2(velocity.x, velocity.z);
		//swprintf_s(&Game::getSingleton().m_ScrInfos[6][0], 256, L"Angle: %f", D3DXToDegree(m_FaceAngle));
		if (m_FaceAngle > m_FaceAngleInerp + D3DX_PI)
		{
			m_FaceAngleInerp += 2 * D3DX_PI;
		}
		else if (m_FaceAngle < m_FaceAngleInerp - D3DX_PI)
		{
			m_FaceAngleInerp -= 2 * D3DX_PI;
		}
	}
	else
	{
		velocity.x = 0;
		velocity.z = 0;
	}

	m_FaceAngleInerp = Lerp(m_FaceAngleInerp, m_FaceAngle, 1.0f - powf(0.8f, 30 * fElapsedTime));

	const Matrix4 RotM = Matrix4::RotationYawPitchRoll(m_LookAngles.y, m_LookAngles.x, m_LookAngles.z);
	m_LookDir = RotM.row<2>().xyz;

	Quaternion rot = Quaternion::RotationYawPitchRoll(m_FaceAngleInerp,0,0);
	Vector3 pos = getPosition() + Vector3(0,-0.75f,0);
	m_MeshCmp->m_World = Matrix4::Compose(Vector3(0.01f,0.01f,0.01f), rot, pos);
	Game::getSingleton().m_Root.RemoveComponent(m_MeshCmp.get());
	Game::getSingleton().m_Root.AddComponent(m_MeshCmp.get(), m_MeshCmp->m_aabb.transform(m_MeshCmp->m_World), 0.1f);

	Game::getSingleton().m_SkyLightCam->m_Eye = pos;
}

void LocalPlayer::OnMouseMove(InputEventArg * arg)
{
	MouseMoveEventArg & mmarg = *dynamic_cast<MouseMoveEventArg *>(arg);
	if (mmarg.x != 0)
	{
		m_LookAngles.y += -D3DXToRadian(mmarg.x);
	}
	if (mmarg.y != 0)
	{
		m_LookAngles.x += -D3DXToRadian(mmarg.y);
	}
	if (mmarg.z != 0)
	{
		m_LookDist += -mmarg.z;
	}
}

void LocalPlayer::OnMouseBtnDown(my::InputEventArg * arg)
{
	MouseBtnEventArg & mbarg = *dynamic_cast<MouseBtnEventArg *>(arg);
}

void LocalPlayer::OnMouseBtnUp(my::InputEventArg * arg)
{
	MouseBtnEventArg & mbarg = *dynamic_cast<MouseBtnEventArg *>(arg);
}

void LocalPlayer::OnKeyDown(InputEventArg * arg)
{
	KeyboardEventArg & karg = *dynamic_cast<KeyboardEventArg *>(arg);
	switch (karg.kc)
	{
	case VK_SPACE:
		velocity.y = 5;
		karg.handled = true;
		break;
	case 'W':
		m_InputUpDn = -1;
		karg.handled = true;
		break;
	case 'A':
		m_InputLtRt = -1;
		karg.handled = true;
		break;
	case 'S':
		m_InputUpDn = 1;
		karg.handled = true;
		break;
	case 'D':
		m_InputLtRt = 1;
		karg.handled = true;
		break;
	}
}

void LocalPlayer::OnKeyUp(my::InputEventArg * arg)
{
	KeyboardEventArg & karg = *dynamic_cast<KeyboardEventArg *>(arg);
	switch (karg.kc)
	{
	case 'W':
		if (m_InputUpDn == -1)
		{
			m_InputUpDn = 0;
		}
		karg.handled = true;
		break;
	case 'A':
		if (m_InputLtRt == -1)
		{
			m_InputLtRt = 0;
		}
		karg.handled = true;
		break;
	case 'S':
		if (m_InputUpDn == 1)
		{
			m_InputUpDn = 0;
		}
		karg.handled = true;
		break;
	case 'D':
		if (m_InputLtRt == 1)
		{
			m_InputLtRt = 0;
		}
		karg.handled = true;
		break;
	}
}

void LocalPlayer::OnJoystickAxisMove(my::InputEventArg * arg)
{
	JoystickAxisEventArg & jaarg = *dynamic_cast<JoystickAxisEventArg *>(arg);
}

void LocalPlayer::OnJoystickPovMove(my::InputEventArg * arg)
{
	JoystickPovEventArg & jparg = *dynamic_cast<JoystickPovEventArg *>(arg);
}

void LocalPlayer::OnJoystickBtnDown(my::InputEventArg * arg)
{
	JoystickBtnEventArg & jbarg = *dynamic_cast<JoystickBtnEventArg *>(arg);
}

void LocalPlayer::OnJoystickBtnUp(my::InputEventArg * arg)
{
	JoystickBtnEventArg & jbarg = *dynamic_cast<JoystickBtnEventArg *>(arg);
}
