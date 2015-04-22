#pragma once

#include "RenderPipeline.h"

class Animator;

class RenderComponent;

class ClothComponent;

class Actor
	: public my::OctRoot
{
public:
	class Attacher
	{
	public:
		Actor * m_Owner;

		unsigned int m_AnimId;

		unsigned int m_SlotId;

		my::Matrix4 m_World;

	public:
		Attacher(Actor * Owner)
			: m_Owner(Owner)
			, m_AnimId(0)
			, m_SlotId(0)
			, m_World(my::Matrix4::Identity())
		{
		}

		void UpdateWorld(void);
	};

	my::Matrix4 m_World;

	typedef std::vector<boost::shared_ptr<Animator> > AnimatorPtrList;

	AnimatorPtrList m_AnimatorList;

	typedef std::vector<boost::shared_ptr<ClothComponent> > ClothComponentPtrList;

	ClothComponentPtrList m_Clothes;

public:
	Actor(void)
		: OctRoot(my::AABB(-FLT_MAX,FLT_MAX), 1.0f)
		, m_World(my::Matrix4::Identity())
	{
	}

	virtual ~Actor(void)
	{
	}

	void OnResetDevice(void);

	void OnLostDevice(void);

	void OnDestroyDevice(void);

	void Update(float fElapsedTime);

	void OnPxThreadSubstep(float dtime);

	void QueryMesh(const my::Frustum & frustum, RenderPipeline * pipeline, Material::DrawStage stage);
};

typedef boost::shared_ptr<Actor> ActorPtr;
