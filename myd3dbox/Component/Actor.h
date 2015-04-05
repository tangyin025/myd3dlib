#pragma once

class Animator;

class Actor
	: public my::OctRoot
{
public:
	class Attacher
	{
	public:
		Actor * m_Owner;

		enum AttachType
		{
			AttachTypeWorld,
			AttachTypeSlot,
			AttachTypeAnimation,
		};

		AttachType m_Type;

		my::Matrix4 m_Offset;

		unsigned int m_SlotId;

		my::Matrix4 m_World;

	public:
		Attacher(Actor * Owner)
			: m_Owner(Owner)
			, m_Type(AttachTypeWorld)
			, m_Offset(my::Matrix4::Identity())
			, m_SlotId(0)
		{
		}

		void UpdateWorld(void);

		void OnSetShader(my::Effect * shader, DWORD AttribId);
	};

	my::Matrix4 m_World;

	boost::shared_ptr<Animator> m_Animator;

public:
	Actor(void)
		: OctRoot(my::AABB(FLT_MIN,FLT_MAX), 1.1f)
	{
	}

	virtual ~Actor(void)
	{
	}
};

typedef boost::shared_ptr<Actor> ActorPtr;
