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
	ia >> boost::serialization::make_nvp("m_Root", Game::getSingleton().m_Root);

	m_player.reset(new LocalPlayer());
}

void Logic::Update(float fElapsedTime)
{
	m_player->Update(fElapsedTime);

	FirstPersonCamera * camera = dynamic_cast<FirstPersonCamera *>(Game::getSingleton().m_Camera.get());
	camera->m_Eye = m_player->getPosition() + m_player->m_LookDir * m_player->m_LookDist;
	camera->m_Eular = m_player->m_LookAngles; // 右手系空间相机朝向-z轴
	Game::getSingleton().ResetViewedCmps(camera->m_Eye, m_player->getPosition());
}

void Logic::Destroy(void)
{
	m_player.reset();
	m_cmps.clear();
}
