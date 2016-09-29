#include "StdAfx.h"
#include "Logic.h"
#include "../Game.h"
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/vector.hpp>
#include <fstream>

using namespace my;

Logic::Logic(void)
{
}

Logic::~Logic(void)
{
}

void Logic::Create(void)
{
	std::ifstream istr("aaa.xml");
	boost::archive::polymorphic_xml_iarchive ia(istr);
	ia >> boost::serialization::make_nvp("level", m_cmps);
	for (unsigned int i = 0; i < m_cmps.size(); i++)
	{
		Game::getSingleton().m_Root.AddComponent(m_cmps[i].get(), m_cmps[i]->m_aabb.transform(Component::GetComponentWorld(m_cmps[i].get())), 0.1f);
	}

	m_player.reset(new LocalPlayer());
}

void Logic::Update(float fElapsedTime)
{
	m_player->Update(fElapsedTime);

	FirstPersonCamera * camera = dynamic_cast<FirstPersonCamera *>(Game::getSingleton().m_Camera.get());
	camera->m_Eye = m_player->getPosition() + m_player->m_LookDir * m_player->m_LookDist;
	camera->m_Eular = m_player->m_LookAngles; // ����ϵ�ռ��������-z��
	Game::getSingleton().ResetViewedCmps(camera->m_Eye, m_player->getPosition());
}

void Logic::Destroy(void)
{
	m_player.reset();
	m_cmps.clear();
}
