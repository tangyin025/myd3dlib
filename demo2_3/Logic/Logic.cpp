#include "StdAfx.h"
#include "Logic.h"
#include "../Game.h"

using namespace my;

Logic::Logic(void)
	: m_FixedTickTimer(1/60.0f)
{
	m_FixedTickTimer.m_EventTimer = boost::bind(&Logic::OnFixedTick, this, _1);
	m_FixedTickTimer.m_Managed = true;
}

Logic::~Logic(void)
{
}

void Logic::Create(void)
{
	Game::getSingleton().m_MouseMovedEvent = boost::bind(&Logic::OnMouseMove, this, _1);
	Game::getSingleton().m_MousePressedEvent = boost::bind(&Logic::OnMouseBtnDown, this, _1);
	Game::getSingleton().m_MouseReleasedEvent = boost::bind(&Logic::OnMouseBtnUp, this, _1);
	Game::getSingleton().m_KeyPressedEvent = boost::bind(&Logic::OnKeyDown, this, _1);
	Game::getSingleton().m_KeyReleasedEvent = boost::bind(&Logic::OnKeyUp, this, _1);

	if (Game::getSingleton().m_joystick)
	{
		Game::getSingleton().m_joystick->m_AxisMovedEvent = boost::bind(&Logic::OnJoystickAxisMove, this, _1);
		Game::getSingleton().m_joystick->m_PovMovedEvent = boost::bind(&Logic::OnJoystickPovMove, this, _1);
		Game::getSingleton().m_joystick->m_BtnPressedEvent = boost::bind(&Logic::OnJoystickBtnDown, this, _1);
		Game::getSingleton().m_joystick->m_BtnReleasedEvent = boost::bind(&Logic::OnJoystickBtnUp, this, _1);
	}

	Game::getSingleton().ExecuteCode("dofile \"StateMain.lua\"");

	OgreMeshSetPtr scene = Game::getSingleton().LoadMeshSet("mesh/scene.mesh.xml");
	PxRigidActor * scene_actor = Game::getSingleton().m_sdk->createRigidStatic(PxTransform::createIdentity());
	OgreMeshSet::const_iterator mesh_iter = scene->begin();
	for (; mesh_iter != scene->end(); mesh_iter++)
	{
		// ���볡����Ⱦģ��
		MeshComponentPtr comp(new MeshComponent((*mesh_iter)->m_aabb));
		comp->m_Mesh = *mesh_iter;
		std::vector<std::string>::const_iterator mat_name_iter = comp->m_Mesh->m_MaterialNameList.begin();
		for(; mat_name_iter != comp->m_Mesh->m_MaterialNameList.end(); mat_name_iter++)
		{
			comp->m_Materials.push_back(MeshComponent::MaterialPair(
				Game::getSingleton().LoadMaterial(str_printf("material/%s.xml", mat_name_iter->c_str())),
				Game::getSingleton().LoadEffect("shader/SimpleSample.fx", EffectMacroPairList())));
		}
		Game::getSingleton().m_OctScene->PushComponent(comp, 0.1f);

		// ���볡������ģ��
		MemoryOStreamPtr ostr(new MemoryOStream());
		Game::getSingleton().CookTriangleMesh(ostr, *mesh_iter);
		IStreamPtr istr(new MemoryIStream(&(*ostr->m_cache)[0], ostr->m_cache->size()));
		PxShape * shape = scene_actor->createShape(PxTriangleMeshGeometry(Game::getSingleton().CreateTriangleMesh(istr)), *Game::getSingleton().m_PxMaterial);
		shape->setFlag(PxShapeFlag::eVISUALIZATION, false);
	}
	Game::getSingleton().m_PxScene->addActor(*scene_actor);

	Game::getSingleton().m_PxScene->addActor(*PxCreateDynamic(
		*Game::getSingleton().m_sdk, PxTransform(PxVec3(0,3,0)), PxBoxGeometry(PxVec3(1,1,1)), *Game::getSingleton().m_PxMaterial, 1));

	m_LocalPlayer.reset(new Character());
	m_LocalPlayer->Create();
}

void Logic::Update(float fElapsedTime)
{
	ModelViewerCamera * camera = dynamic_cast<ModelViewerCamera *>(Game::getSingleton().m_Camera.get());
	camera->m_LookAt = (Vector3&)m_LocalPlayer->getPosition();
	camera->OnFrameMove(Game::getSingleton().m_fAbsoluteTime, fElapsedTime);
	m_LocalPlayer->m_LookDir = atan2f(-camera->m_View._13, -camera->m_View._33); // ! right hand inverse vector

	m_FixedTickTimer.Step(fElapsedTime, 4);

	Game::getSingleton().PushGrid();
}

void Logic::OnFixedTick(float fElapsedTime)
{
	m_LocalPlayer->Update(fElapsedTime);
}

void Logic::Destroy(void)
{
	m_StaticSceneActor.reset();
	m_LocalPlayer.reset();

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
}

void Logic::OnMouseMove(InputEventArg * arg)
{
	MouseMoveEventArg & mmarg = *dynamic_cast<MouseMoveEventArg *>(arg);
}

void Logic::OnMouseBtnDown(my::InputEventArg * arg)
{
	MouseBtnEventArg & mbarg = *dynamic_cast<MouseBtnEventArg *>(arg);
}

void Logic::OnMouseBtnUp(my::InputEventArg * arg)
{
	MouseBtnEventArg & mbarg = *dynamic_cast<MouseBtnEventArg *>(arg);
}

void Logic::OnKeyDown(InputEventArg * arg)
{
	KeyboardEventArg & karg = *dynamic_cast<KeyboardEventArg *>(arg);
	switch (karg.kc)
	{
	case VK_SPACE:
		m_LocalPlayer->Jump();
		karg.handled = true;
		break;
	case 'W':
		m_LocalPlayer->AddMoveState(Character::MoveStateFront);
		karg.handled = true;
		break;
	case 'A':
		m_LocalPlayer->AddMoveState(Character::MoveStateLeft);
		karg.handled = true;
		break;
	case 'S':
		m_LocalPlayer->AddMoveState(Character::MoveStateBack);
		karg.handled = true;
		break;
	case 'D':
		m_LocalPlayer->AddMoveState(Character::MoveStateRight);
		karg.handled = true;
		break;
	}
}

void Logic::OnKeyUp(my::InputEventArg * arg)
{
	KeyboardEventArg & karg = *dynamic_cast<KeyboardEventArg *>(arg);
	switch (karg.kc)
	{
	case 'W':
		m_LocalPlayer->RemoveMoveState(Character::MoveStateFront);
		karg.handled = true;
		break;
	case 'A':
		m_LocalPlayer->RemoveMoveState(Character::MoveStateLeft);
		karg.handled = true;
		break;
	case 'S':
		m_LocalPlayer->RemoveMoveState(Character::MoveStateBack);
		karg.handled = true;
		break;
	case 'D':
		m_LocalPlayer->RemoveMoveState(Character::MoveStateRight);
		karg.handled = true;
		break;
	}
}

void Logic::OnJoystickAxisMove(my::InputEventArg * arg)
{
	JoystickAxisEventArg & jaarg = *dynamic_cast<JoystickAxisEventArg *>(arg);
}

void Logic::OnJoystickPovMove(my::InputEventArg * arg)
{
	JoystickPovEventArg & jparg = *dynamic_cast<JoystickPovEventArg *>(arg);
}

void Logic::OnJoystickBtnDown(my::InputEventArg * arg)
{
	JoystickBtnEventArg & jbarg = *dynamic_cast<JoystickBtnEventArg *>(arg);
}

void Logic::OnJoystickBtnUp(my::InputEventArg * arg)
{
	JoystickBtnEventArg & jbarg = *dynamic_cast<JoystickBtnEventArg *>(arg);
}
