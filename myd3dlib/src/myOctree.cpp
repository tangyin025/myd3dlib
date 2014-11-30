#include "StdAfx.h"
#include "myOctree.h"

using namespace my;

void OctRoot::ClearComponent(void)
{
	m_ComponentList.clear();

	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		m_Childs[i].reset();
	}
}
