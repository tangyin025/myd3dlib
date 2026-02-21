// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <vector>
#include <map>
#include <list>
#include "myMath.h"
#include "mySpline.h"
#include "SoundContext.h"

class ActionTrack;

typedef boost::shared_ptr<ActionTrack> ActionTrackPtr;

class ActionInst;

typedef boost::shared_ptr<ActionInst> ActionInstPtr;

class Actor;

class Action : public boost::enable_shared_from_this<Action>
{
public:
	float m_Length;

	typedef std::vector<ActionTrackPtr> ActionTrackPtrList;

	ActionTrackPtrList m_TrackList;

public:
	Action(float Length)
		: m_Length(Length)
	{
	}

	virtual ~Action(void)
	{
	}

	bool operator ==(const Action & rhs) const
	{
		return this == &rhs;
	}

	void AddTrack(ActionTrackPtr track);

	void RemoveTrack(ActionTrackPtr track);

	ActionInstPtr CreateInstance(Actor * _Actor);
};

class ActionTrackInst;

typedef boost::shared_ptr<ActionTrackInst> ActionTrackInstPtr;

class ActionInst
{
public:
	boost::shared_ptr<const Action> m_Template;

	float m_LastTime;

	typedef std::vector<ActionTrackInstPtr> ActionTrackInstPtrList;

	ActionTrackInstPtrList m_TrackInstList;

public:
	ActionInst(Actor * _Actor, boost::shared_ptr<const Action> Template);

	void Update(float fElapsedTime);

	void StopAllTrack(void);
};

class ActionTrack : public boost::enable_shared_from_this<ActionTrack>
{
public:
	ActionTrack(void)
	{
	}

	virtual ~ActionTrack(void)
	{
	}

	virtual ActionTrackInstPtr CreateInstance(Actor * _Actor) const = 0;
};

typedef boost::shared_ptr<ActionTrack> ActionTrackPtr;

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
		_ASSERT(!m_Actor); // ! Actor::StopAllActionInst
	}

	virtual void UpdateTime(float LastTime, float Time) = 0;

	virtual void Stop(void) = 0;
};

class ActionTrackAnimation : public ActionTrack
{
public:
	struct KeyFrame
	{
		std::string SlotName;
		std::string Name;
		float Rate;
		float Weight;
		float BlendTime;
		float BlendOutTime;
		std::string Group;
		int Prority;
	};

	typedef std::multimap<float, KeyFrame> KeyFrameMap;

	KeyFrameMap m_Keys;

public:
	ActionTrackAnimation(void)
	{
	}

	virtual ActionTrackInstPtr CreateInstance(Actor * _Actor) const;

	void AddKeyFrame(float Time, const char * SlotName, const char * Name, float Rate, float Weight, float BlendTime, float BlendOutTime, const char * Group, int Prority);
};

class ActionTrackAnimationInst : public ActionTrackInst
{
protected:
	boost::shared_ptr<const ActionTrackAnimation> m_Template;

public:
	ActionTrackAnimationInst(Actor * _Actor, boost::shared_ptr<const ActionTrackAnimation> Template)
		: ActionTrackInst(_Actor)
		, m_Template(Template)
	{
	}

	virtual void UpdateTime(float LastTime, float Time);

	virtual void Stop(void);
};

class ActionTrackSound : public ActionTrack
{
public:
	struct KeyFrame
	{
		std::string SoundPath;
		my::WavPtr Sound;
		float StartSec;
		float EndSec;
		bool Loop;
		float MinDistance;
		float MaxDistance;

		void OnSoundReady(my::DeviceResourceBasePtr res);
	};

	typedef std::multimap<float, KeyFrame> KeyFrameMap;

	KeyFrameMap m_Keys;

public:
	ActionTrackSound(void)
	{
	}

	virtual ~ActionTrackSound(void);

	virtual ActionTrackInstPtr CreateInstance(Actor * _Actor) const;

	void AddKeyFrame(float Time, const char * SoundPath, float StartSec, float EndSec, bool Loop, float MinDistance, float MaxDistance);
};

class ActionTrackSoundInst : public ActionTrackInst
{
protected:
	boost::shared_ptr<const ActionTrackSound> m_Template;

	typedef std::list<SoundEventPtr> SoundEventList;
	
	SoundEventList m_Events;

public:
	ActionTrackSoundInst(Actor * _Actor, boost::shared_ptr<const ActionTrackSound> Template)
		: ActionTrackInst(_Actor)
		, m_Template(Template)
	{
	}

	~ActionTrackSoundInst(void);

	virtual void UpdateTime(float LastTime, float Time);

	virtual void Stop(void);
};

class Material;

class SphericalEmitter;

class ActionTrackEmitter : public ActionTrack
{
public:
	struct KeyFrame
	{
		float Length;

		std::string EmitterName;
	};

	typedef std::multimap<float, KeyFrame> KeyFrameMap;

	KeyFrameMap m_Keys;

public:
	ActionTrackEmitter(void)
	{
	}

	virtual ActionTrackInstPtr CreateInstance(Actor * _Actor) const;

	void AddKeyFrame(float Time, float Length, const char * EmitterName);
};

class ActionTrackEmitterInst : public ActionTrackInst
{
protected:
	boost::shared_ptr<const ActionTrackEmitter> m_Template;

	struct KeyFrameInst
	{
		float m_Time;

		float m_Length;

		float m_SpawnInterval;

		int m_SpawnCount;

		float m_SpawnTime;

		boost::shared_ptr<SphericalEmitter> m_EmitterCmp;

		KeyFrameInst(float Length, float SpawnInterval, int SpawnCount)
			: m_Time(0)
			, m_Length(Length)
			, m_SpawnInterval(SpawnInterval)
			, m_SpawnCount(SpawnCount)
			, m_SpawnTime(0)
		{
		}
	};

	typedef std::vector<KeyFrameInst> KeyFrameInstList;

	KeyFrameInstList m_KeyInsts;

public:
	ActionTrackEmitterInst(Actor * _Actor, boost::shared_ptr<const ActionTrackEmitter> Template);

	virtual ~ActionTrackEmitterInst(void);

	virtual void UpdateTime(float LastTime, float Time);

	virtual void Stop(void);
};

class ActionTrackPose : public ActionTrack
{
public:
	struct KeyFrame
	{
		float Length;
	};

	typedef std::multimap<float, KeyFrame> KeyFrameMap;

	KeyFrameMap m_Keys;

	my::Bone m_ParamPose;

	my::Spline m_Interpolation;

public:
	ActionTrackPose(void)
		: m_ParamPose(my::Vector3(0, 0, 0), my::Quaternion::Identity())
	{
		m_Interpolation.AddNode(0, 0, 0, 0);
		m_Interpolation.AddNode(1, 1, 0, 0);
	}

	virtual ActionTrackInstPtr CreateInstance(Actor * _Actor) const;

	void AddKeyFrame(float Time, float Length);
};

class ActionTrackPoseInst : public ActionTrackInst
{
protected:
	boost::shared_ptr<const ActionTrackPose> m_Template;

	struct KeyFrameInst
	{
		float m_Time;

		float m_Length;

		my::Bone m_StartPose;

		KeyFrameInst(float Length, const my::Bone& StartPose)
			: m_Time(0)
			, m_Length(Length)
			, m_StartPose(StartPose)
		{
		}
	};

	typedef std::vector<KeyFrameInst> KeyFrameInstList;

	KeyFrameInstList m_KeyInsts;

	my::Bone m_Pose;

public:
	ActionTrackPoseInst(Actor * _Actor, boost::shared_ptr<const ActionTrackPose> Template);

	virtual void UpdateTime(float LastTime, float Time);

	virtual void Stop(void);
};
