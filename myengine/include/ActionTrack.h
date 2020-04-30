#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <vector>
#include <map>

class ActionTrack;

typedef boost::shared_ptr<ActionTrack> ActionTrackPtr;

class Action
{
public:
	float m_Length;

	typedef std::vector<ActionTrackPtr> ActionTrackPtrList;

	ActionTrackPtrList m_TrackList;

public:
	Action(void)
		: m_Length(1.0f)
	{
	}

	virtual ~Action(void)
	{
	}

	void AddTrack(ActionTrackPtr track);

	void RemoveTrack(ActionTrackPtr track);
};

class ActionTrackInst;

typedef boost::shared_ptr<ActionTrackInst> ActionTrackInstPtr;

class Actor;

class ActionTrack : public boost::intrusive_ref_counter<ActionTrack, boost::sp_adl_block::thread_unsafe_counter>
{
public:
	ActionTrack(void)
	{
	}

	virtual ~ActionTrack(void)
	{
		_ASSERT(0 == use_count());
	}

	virtual ActionTrackInstPtr CreateInstance(Actor * actor) const = 0;
};

class ActionTrackInst
{
public:
	Actor * m_Actor;

public:
	ActionTrackInst(Actor * _Actor)
		: m_Actor(_Actor)
	{
	}

	virtual ~ActionTrackInst(void)
	{
	}

	virtual void UpdateTime(float Time, float fElapsedTime) = 0;
};

class ActionTrackAnimation : public ActionTrack
{
public:
	struct KeyFrame
	{
		std::string Name;
		std::string RootList;
		float Rate;
		float BlendTime;
		float BlendOutTime;
		bool Loop;
		int Prority;
		float StartTime;
		std::string Group;
	};

	typedef std::map<float, KeyFrame> KeyFrameMap;

	KeyFrameMap m_Keys;

public:
	ActionTrackAnimation(void)
	{
	}

	virtual ActionTrackInstPtr CreateInstance(Actor * _Actor) const;

	void AddKeyFrame(float Time, const char * Name, const char * RootList, float Rate, float BlendTime, float BlendOutTime, bool Loop, int Prority, float StartTime, const char * Group);
};

class ActionTrackAnimationInst : public ActionTrackInst
{
protected:
	boost::intrusive_ptr<const ActionTrackAnimation> m_Template;

public:
	ActionTrackAnimationInst(Actor * _Actor, const ActionTrackAnimation * Template)
		: ActionTrackInst(_Actor)
		, m_Template(Template)
	{
	}

	virtual void UpdateTime(float Time, float fElapsedTime);
};
