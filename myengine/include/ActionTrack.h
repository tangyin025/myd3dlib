#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <vector>
#include <map>
#include <set>
#include "myMath.h"
#include "mySpline.h"
#include "myTask.h"

class ActionTrack;

typedef boost::intrusive_ptr<ActionTrack> ActionTrackPtr;

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

	virtual void OnStop(void) = 0;
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

	virtual void OnStop(void);
};

class ActionTrackSound : public ActionTrack
{
public:
	struct KeyFrame
	{
		std::string Name;
	};

	typedef std::map<float, KeyFrame> KeyFrameMap;

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

	virtual void OnStop(void);

	void StopAllEvent(bool immediate);
};

class Material;

class ActionTrackSphericalEmitter : public ActionTrack
{
public:
	boost::shared_ptr<Material> m_ParticleMaterial;

	unsigned int m_ParticleCapacity;

	DWORD m_ParticleFaceType;

	float m_ParticleLifeTime;

	float m_SpawnInterval;

	float m_SpawnLength;

	my::Spline m_ParticlePosX;

	my::Spline m_ParticlePosY;

	my::Spline m_ParticlePosZ;

	my::Spline m_ParticleColorR;

	my::Spline m_ParticleColorG;

	my::Spline m_ParticleColorB;

	my::Spline m_ParticleColorA;

	my::Spline m_ParticleSizeX;

	my::Spline m_ParticleSizeY;

	my::Spline m_ParticleAngle;

	struct KeyFrame
	{
	};

	typedef std::map<float, KeyFrame> KeyFrameMap;

	KeyFrameMap m_Keys;

public:
	ActionTrackSphericalEmitter(void)
		: m_ParticleCapacity(1024)
		, m_ParticleFaceType(0)
		, m_ParticleLifeTime(FLT_MAX)
		, m_SpawnInterval(FLT_MAX)
		, m_SpawnLength(0)
	{
	}

	virtual ActionTrackInstPtr CreateInstance(Actor * _Actor) const;

	void AddKeyFrame(float Time);
};

class EmitterComponent;

class ActionTrackSphericalEmitterInst : public ActionTrackInst, public my::ParallelTask
{
protected:
	boost::intrusive_ptr<const ActionTrackSphericalEmitter> m_Template;

	boost::shared_ptr<Actor> m_WorldEmitterActor;

	boost::shared_ptr<EmitterComponent> m_WorldEmitterInst;

	float m_ActionTime;

public:
	ActionTrackSphericalEmitterInst(Actor * _Actor, const ActionTrackSphericalEmitter * Template);

	virtual ~ActionTrackSphericalEmitterInst(void);

	virtual void UpdateTime(float Time, float fElapsedTime);

	virtual void OnStop(void);

	virtual void DoTask(void);
};
