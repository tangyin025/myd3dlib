#include "StreamRoot.h"
#include "Actor.h"
#include "myResource.h"
#include "libc.h"
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/map.hpp>
#include <fstream>

using namespace my;

class StreamNodeResource : public DeviceResourceBase
{
public:
	OctNode::OctActorMap m_Actors;

public:
	StreamNodeResource(void)
	{
	}

	~StreamNodeResource(void)
	{
	}
};

typedef boost::shared_ptr<StreamNodeResource> StreamNodeResourcePtr;

class StreamNodeIORequest : public IORequest
{
protected:
	std::string m_path;

	StreamNodeResourcePtr m_NodeRes;

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
		m_NodeRes.reset(new StreamNodeResource());
		ia >> boost::serialization::make_nvp("Actors", m_NodeRes->m_Actors);
	}

	virtual void CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if (!m_NodeRes)
		{
			THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
		}

		m_res = m_NodeRes;
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
		request->PushCallback(this);
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
	return str_printf("%s.%05.0f_%05.0f_%05.0f_%05.0f_%05.0f_%05.0f",
		RootPath, m_aabb.m_min.x, m_aabb.m_min.y, m_aabb.m_min.z, m_aabb.m_max.x, m_aabb.m_max.y, m_aabb.m_max.z);
}

void StreamNode::OnReady(my::DeviceResourceBasePtr res)
{
	StreamNodeResourcePtr node = boost::dynamic_pointer_cast<StreamNodeResource>(res);

	_ASSERT(node);

	_ASSERT(m_Actors.empty());

	m_Actors.insert(node->m_Actors.begin(), node->m_Actors.end());

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

void StreamRoot::CheckViewedActor(PhysXSceneContext * scene, const my::AABB & In, const my::AABB & Out)
{
	struct Callback : public my::OctNode::QueryCallback
	{
		WeakActorMap & ViewedActors;
		PhysXSceneContext * scene;
		typedef std::vector<ActorPtr> ActorList;
		ActorList actor_list;
		Callback(WeakActorMap & _ViewedActors, PhysXSceneContext * _scene)
			: ViewedActors(_ViewedActors)
			, scene(_scene)
		{
		}
		void operator() (OctActor * oct_actor, const AABB & aabb, IntersectionTests::IntersectionType)
		{
			_ASSERT(dynamic_cast<Actor *>(oct_actor));
			Actor * actor = static_cast<Actor *>(oct_actor);
			WeakActorMap::const_iterator actor_iter = ViewedActors.find(actor);
			if (actor_iter != ViewedActors.end())
			{
				ViewedActors.erase(actor);
			}
			else if (!actor->IsRequested())
			{
				actor->RequestResource();
				actor->OnEnterPxScene(scene);
			}
			actor_list.push_back(boost::static_pointer_cast<Actor>(actor->shared_from_this()));
		}
	};

	Callback cb(m_ViewedActors, scene);
	QueryActor(In, &cb);

	WeakActorMap::iterator weak_actor_iter = m_ViewedActors.begin();
	for (; weak_actor_iter != m_ViewedActors.end(); weak_actor_iter++)
	{
		ActorPtr actor = weak_actor_iter->second.lock();
		if (actor && actor->IsRequested() && !actor->GetOctAABB().Intersect(Out).IsValid())
		{
			actor->OnLeavePxScene(scene);
			actor->ReleaseResource();
		}
	}

	m_ViewedActors.clear();
	Callback::ActorList::iterator actor_iter = cb.actor_list.begin();
	for (; actor_iter != cb.actor_list.end(); actor_iter++)
	{
		m_ViewedActors.insert(std::make_pair(actor_iter->get(), *actor_iter));
	}
}
