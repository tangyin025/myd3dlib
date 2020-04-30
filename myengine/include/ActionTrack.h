#pragma once

#include <boost/shared_ptr.hpp>
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

class ActionTrack
{
public:
	ActionTrack(void)
	{
	}

	virtual ~ActionTrack(void)
	{
	}

	virtual ActionTrackInstPtr CreateInstance(Actor * actor) const = 0;
};

class ActionTrackInst
{
public:
	ActionTrackInst(void)
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

	virtual ActionTrackInstPtr CreateInstance(Actor * actor) const;

	void AddKeyFrame(float time, const char * Name, const char * RootList, float Rate, float BlendTime, float BlendOutTime, bool Loop, int Prority, float StartTime, const char * Group);
};

class ActionTrackAnimationInst : public ActionTrackInst
{
protected:
	const ActionTrackAnimation * m_Template;

	Actor * m_Actor;

public:
	ActionTrackAnimationInst(const ActionTrackAnimation * Template, Actor * Actor)
		: m_Template(Template)
		, m_Actor(Actor)
	{
	}

	virtual void UpdateTime(float Time, float fElapsedTime);
};
