#include "StreamNode.h"
#include "Actor.h"
#include "myResource.h"
#include "libc.h"
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/map.hpp>
#include <fstream>
#include "Material.h"

using namespace my;

class StreamNodeIORequest : public IORequest
{
public:
	std::string m_path;

	OctNode::OctActorMap m_Actors;

public:
	StreamNodeIORequest(const std::string & path)
		: m_path(path)
	{
	}

	virtual void LoadResource(void)
	{
		IStreamBuff buff(my::ResourceMgr::getSingleton().OpenIStream(m_path.c_str()));
		std::istream istr(&buff);
		boost::archive::polymorphic_xml_iarchive ia(istr);
		ia.template register_type<MaterialParameterTexture>();
		ia >> BOOST_SERIALIZATION_NVP(m_Actors);
	}

	virtual void CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		m_res.reset(new DeviceResourceBase());
	}
};

//
//BOOST_CLASS_EXPORT(StreamNode)

void StreamNode::AddToChild(ChildArray::reference & child, const my::AABB & child_aabb, my::OctActorPtr actor, const my::AABB & aabb)
{
	if (!child)
	{
		child.reset(new StreamNode(this, child_aabb));
	}
	child->AddActor(actor, aabb);
}

void StreamNode::RequestResource(void)
{
	StreamRoot * Root = dynamic_cast<StreamRoot *>(GetTopNode());
	_ASSERT(Root);
	std::string Path = BuildPath(Root->m_Path.c_str());
	if (!Path.empty())
	{
		_ASSERT(!m_Ready);

		IORequestPtr request(new StreamNodeIORequest(Path));
		request->m_callbacks.insert(this);
		my::ResourceMgr::getSingleton().LoadIORequestAsync(Path, request, true);
	}
}

void StreamNode::ReleaseResource(void)
{
	StreamRoot * Root = dynamic_cast<StreamRoot *>(GetTopNode());
	_ASSERT(Root);
	std::string Path = BuildPath(Root->m_Path.c_str());
	if (!Path.empty() && !m_Ready)
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(Path, this);
	}

	m_Ready = false;
}

std::string StreamNode::BuildPath(const char * RootPath)
{
	return str_printf("%s.%+05.0f%+05.0f%+05.0f%+05.0f%+05.0f%+05.0f",
		RootPath, m_aabb.m_min.x, m_aabb.m_min.y, m_aabb.m_min.z, m_aabb.m_max.x, m_aabb.m_max.y, m_aabb.m_max.z);
}

void StreamNode::OnReady(my::IORequest * request)
{
	StreamNodeIORequest * node_request = dynamic_cast<StreamNodeIORequest *>(request);

	_ASSERT(node_request);

	_ASSERT(!m_Ready);

	m_Actors.insert(node_request->m_Actors.begin(), node_request->m_Actors.end());
	OctActorMap::iterator actor_iter = m_Actors.begin();
	for (; actor_iter != m_Actors.end(); actor_iter++)
	{
		actor_iter->first->m_Node = this;
	}

	m_Ready = true;
}

void StreamNode::SaveAllActor(const char * RootPath)
{
	if (m_Ready)
	{
		StreamRoot * Root = dynamic_cast<StreamRoot *>(GetTopNode());
		_ASSERT(Root);
		std::string Path = BuildPath(RootPath);
		std::basic_ofstream<char> ofs(Path);
		boost::archive::polymorphic_xml_oarchive oa(ofs);
		// ! solve the strange runtime error, ref: https://www.boost.org/doc/libs/1_63_0/libs/serialization/doc/serialization.html#registration
		oa.template register_type<MaterialParameterTexture>();
		oa << BOOST_SERIALIZATION_NVP(m_Actors);
	}

	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		if (m_Childs[i])
		{
			boost::dynamic_pointer_cast<StreamNode>(m_Childs[i])->SaveAllActor(RootPath);
		}
	}
}

//
//BOOST_CLASS_EXPORT(StreamRoot)

StreamRoot::StreamRoot(void)
{
}

StreamRoot::StreamRoot(const my::AABB & aabb)
	: StreamNode(NULL, aabb)
{
}

StreamRoot::~StreamRoot()
{
}

bool StreamRoot::CheckViewedActor(PhysXSceneContext * Scene, const my::AABB & In, const my::AABB & Out)
{
	struct Callback : public OctNode::QueryNodeCallback
	{
		StreamNodeSet & m_ViewedNodes;

		WeakActorMap & m_ViewedActors;

		PhysXSceneContext * m_Scene;

		AABB m_aabb;

		bool AllLoaded;

		Callback(StreamNodeSet & ViewedNodes, WeakActorMap & ViewedActors, PhysXSceneContext * Scene, const AABB & aabb)
			: m_ViewedNodes(ViewedNodes)
			, m_ViewedActors(ViewedActors)
			, m_Scene(Scene)
			, m_aabb(aabb)
			, AllLoaded(true)
		{
		}

		void operator() (OctNode * oct_node, IntersectionTests::IntersectionType intersect_type)
		{
			StreamNode * node = dynamic_cast<StreamNode *>(oct_node);
			_ASSERT(node);
			if (!node->m_Ready && !node->IsRequested())
			{
				node->RequestResource();
				m_ViewedNodes.insert(node);
				AllLoaded = false;
				return;
			}

			OctActorMap::const_iterator actor_iter = node->m_Actors.begin();
			for (; actor_iter != node->m_Actors.end(); actor_iter++)
			{
				IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndAABB(actor_iter->second, m_aabb);
				switch (intersect_type)
				{
				case IntersectionTests::IntersectionTypeInside:
				case IntersectionTests::IntersectionTypeIntersect:
				{
					Actor * actor = static_cast<Actor *>(actor_iter->first.get());
					if (!actor->IsRequested())
					{
						actor->RequestResource();
						actor->OnEnterPxScene(m_Scene);
						m_ViewedActors.insert(std::make_pair(actor, boost::dynamic_pointer_cast<Actor>(actor_iter->first)));
					}
					break;
				}
				}
			}
		}
	};

	Callback cb(m_ViewedNodes, m_ViewedActors, Scene, In);
	QueryNode(In, &cb);

	// todo: OnLeavePxScene, ReleaseResource

	return cb.AllLoaded;
}
