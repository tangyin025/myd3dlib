#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/circular_buffer.hpp>
#include <vector>
#include <map>
#include <set>
#include "myMath.h"
#include "mySkeleton.h"
#include "mySpline.h"
#include "myTask.h"

class ActionTrack;

typedef boost::intrusive_ptr<ActionTrack> ActionTrackPtr;

class ActionInst;

typedef boost::shared_ptr<ActionInst> ActionInstPtr;

class Actor;

class Action : public boost::intrusive_ref_counter<Action>
{
public:
	typedef std::vector<ActionTrackPtr> ActionTrackPtrList;

	ActionTrackPtrList m_TrackList;

public:
	Action(void)
	{
	}

	virtual ~Action(void)
	{
	}

	void AddTrack(ActionTrackPtr track);

	void RemoveTrack(ActionTrackPtr track);

	ActionInstPtr CreateInstance(Actor * _Actor);
};

typedef boost::intrusive_ptr<Action> ActionPtr;

class ActionTrackInst;

typedef boost::shared_ptr<ActionTrackInst> ActionTrackInstPtr;

class ActionInst
{
public:
	boost::intrusive_ptr<const Action> m_Template;

	float m_Time;

	typedef std::vector<ActionTrackInstPtr> ActionTrackInstPtrList;

	ActionTrackInstPtrList m_TrackInstList;

public:
	ActionInst(Actor * _Actor, const Action * Template);

	void Update(float fElapsedTime);

	void Stop(void);
};

class ActionTrack : public boost::intrusive_ref_counter<ActionTrack>
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

typedef boost::intrusive_ptr<ActionTrack> ActionTrackPtr;

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

	virtual void Stop(void) = 0;
};

class ActionTrackAnimation : public ActionTrack
{
public:
	struct KeyFrame
	{
		std::string Name;
		float Rate;
		float Weight;
		float BlendTime;
		float BlendOutTime;
		bool Loop;
		int Prority;
		std::string Group;
		int RootId;
	};

	typedef std::multimap<float, KeyFrame> KeyFrameMap;

	KeyFrameMap m_Keys;

public:
	ActionTrackAnimation(void)
	{
	}

	virtual ActionTrackInstPtr CreateInstance(Actor * _Actor) const;

	void AddKeyFrame(float Time, const char * Name, float Rate, float Weight, float BlendTime, float BlendOutTime, bool Loop, int Prority, const char * Group, int RootId);
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

	virtual void Stop(void);
};

class ActionTrackSound : public ActionTrack
{
public:
	struct KeyFrame
	{
		std::string Name;
	};

	typedef std::multimap<float, KeyFrame> KeyFrameMap;

	KeyFrameMap m_Keys;

public:
	ActionTrackSound(void)
	{
	}

	virtual ActionTrackInstPtr CreateInstance(Actor * _Actor) const;

	void AddKeyFrame(float Time, const char * Name);
};

namespace FMOD
{
	class Event;
};

class ActionTrackSoundInst : public ActionTrackInst
{
protected:
	boost::intrusive_ptr<const ActionTrackSound> m_Template;

	typedef std::set<FMOD::Event *> FmodEventSet;

	FmodEventSet m_evts;

	friend class ActionTrackSoundInstCallback;

public:
	ActionTrackSoundInst(Actor * _Actor, const ActionTrackSound * Template)
		: ActionTrackInst(_Actor)
		, m_Template(Template)
	{
	}

	~ActionTrackSoundInst(void);

	virtual void UpdateTime(float Time, float fElapsedTime);

	virtual void Stop(void);

	void StopAllEvent(bool immediate);
};

class Material;

class ActionTrackEmitter : public ActionTrack
{
public:
	boost::shared_ptr<Material> m_EmitterMaterial;

	unsigned int m_EmitterCapacity;

	DWORD m_EmitterFaceType;

	float m_ParticleLifeTime;

	my::Spline m_ParticlePositionX;

	my::Spline m_ParticlePositionY;

	my::Spline m_ParticlePositionZ;

	my::Spline m_ParticleColorR;

	my::Spline m_ParticleColorG;

	my::Spline m_ParticleColorB;

	my::Spline m_ParticleColorA;

	my::Spline m_ParticleSizeX;

	my::Spline m_ParticleSizeY;

	my::Spline m_ParticleAngle;

	int m_AttachBoneId;

	struct KeyFrame
	{
		int SpawnCount;

		float SpawnInterval;
	};

	typedef std::multimap<float, KeyFrame> KeyFrameMap;

	KeyFrameMap m_Keys;

public:
	ActionTrackEmitter(void)
		: m_EmitterCapacity(1024)
		, m_EmitterFaceType(3) // FaceTypeCamera = 3
		, m_ParticleLifeTime(FLT_MAX)
		, m_AttachBoneId(-1)
	{
	}

	virtual ActionTrackInstPtr CreateInstance(Actor * _Actor) const;

	void AddKeyFrame(float Time, int SpawnCount, float SpawnInterval);
};

class CircularEmitter;

class ActionTrackEmitterInst : public ActionTrackInst, public my::ParallelTask
{
protected:
	boost::intrusive_ptr<const ActionTrackEmitter> m_Template;

	//boost::shared_ptr<Actor> m_WorldEmitterActor;

	boost::shared_ptr<CircularEmitter> m_WorldEmitterCmp;

	boost::circular_buffer<my::Bone> m_SpawnPos;

	float m_ActionTime;

	struct KeyFrameInst
	{
		float m_Time;

		int m_SpawnCount;

		float m_SpawnInterval;

		KeyFrameInst(float Time, int SpawnCount, float SpawnInterval)
			: m_Time(Time)
			, m_SpawnCount(SpawnCount)
			, m_SpawnInterval(SpawnInterval)
		{
		}
	};

	typedef std::vector<KeyFrameInst> KeyFrameInstList;

	KeyFrameInstList m_KeyInsts;

	my::Event m_TaskEvent;

public:
	ActionTrackEmitterInst(Actor * _Actor, const ActionTrackEmitter * Template);

	virtual ~ActionTrackEmitterInst(void);

	virtual void UpdateTime(float Time, float fElapsedTime);

	virtual void Stop(void);

	virtual void DoTask(void);
};

class ActionTrackPose : public ActionTrack
{
public:
	float m_Length;

	my::Spline m_InterpolateX;

	my::Spline m_InterpolateY;

	my::Spline m_InterpolateZ;

	my::Vector3 m_ParamStartPos;

	my::Vector3 m_ParamEndPos;

	struct KeyFrame
	{
	};

	typedef std::multimap<float, KeyFrame> KeyFrameMap;

	KeyFrameMap m_Keys;

public:
	ActionTrackPose(float Length)
		: m_Length(Length)
		, m_ParamStartPos(0,0,0)
		, m_ParamEndPos(0,0,0)
	{
	}

	virtual ActionTrackInstPtr CreateInstance(Actor * _Actor) const;

	void AddKeyFrame(float Time);
};

class ActionTrackPoseInst : public ActionTrackInst
{
protected:
	boost::intrusive_ptr<const ActionTrackPose> m_Template;

	my::Vector3 m_StartPos;

	my::Vector3 m_EndPos;

	struct KeyFrameInst
	{
		float m_Time;

		KeyFrameInst(void)
			: m_Time(0)
		{
		}
	};

	typedef std::vector<KeyFrameInst> KeyFrameInstList;

	KeyFrameInstList m_KeyInsts;

public:
	ActionTrackPoseInst(Actor * _Actor, const ActionTrackPose * Template, const my::Vector3 & StartPos, const my::Vector3 & EndPos)
		: ActionTrackInst(_Actor)
		, m_Template(Template)
		, m_StartPos(StartPos)
		, m_EndPos(EndPos)
	{
	}

	virtual ~ActionTrackPoseInst(void);

	virtual void UpdateTime(float Time, float fElapsedTime);

	virtual void Stop(void);
};
