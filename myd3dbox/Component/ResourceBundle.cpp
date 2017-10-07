#include "stdafx.h"
#include "ResourceBundle.h"

using namespace my;

template<>
void ResourceBundle<my::BaseTexture>::RequestResource(void)
{
	_ASSERT(!m_Path.empty());
	my::ResourceMgr::getSingleton().LoadTextureAsync(m_Path.c_str(), this);
}

template<>
void ResourceBundle<my::OgreMesh>::RequestResource(void)
{
	_ASSERT(!m_Path.empty());
	my::ResourceMgr::getSingleton().LoadMeshAsync(m_Path.c_str(), this);
}

template<>
void ResourceBundle<my::OgreSkeletonAnimation>::RequestResource(void)
{
	_ASSERT(!m_Path.empty());
	my::ResourceMgr::getSingleton().LoadSkeletonAsync(m_Path.c_str(), this);
}
