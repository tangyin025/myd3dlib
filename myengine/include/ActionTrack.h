#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include <vector>
#include <map>
#include <set>
#include "myMath.h"
#include "mySpline.h"

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
protected:
	mutable int m_ref_counter;

	friend void intrusive_ptr_add_ref(const ActionTrack * p) BOOST_SP_NOEXCEPT;

	friend void intrusive_ptr_release(const ActionTrack * p) BOOST_SP_NOEXCEPT;

public:
	ActionTrack(void)
		: m_ref_counter(0)
	{
	}

	virtual ~ActionTrack(void)
	{
		// ! make sure Track obj should be define before Actor obj in script
		_ASSERT(0 == m_ref_counter);
	}

	virtual ActionTrackInstPtr CreateInstance(Actor * actor) const = 0;
};

inline void intrusive_ptr_add_ref(const ActionTrack * p) BOOST_SP_NOEXCEPT
{
	p->m_ref_counter++;
}

inline void intrusive_ptr_release(const ActionTrack * p) BOOST_SP_NOEXCEPT
{
	p->m_ref_counter--;
}

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

	float m_ParticleLifeTime;

	float m_SpawnInterval;

	float m_SpawnLength;

	my::Vector3 m_HalfSpawnArea;

	float m_SpawnSpeed;

	my::Spline m_SpawnInclination;

	my::Spline m_SpawnAzimuth;

	my::Spline m_SpawnColorR;

	my::Spline m_SpawnColorG;

	my::Spline m_SpawnColorB;

	my::Spline m_SpawnColorA;

	my::Spline m_SpawnSizeX;

	my::Spline m_SpawnSizeY;

	my::Spline m_SpawnAngle;

	struct KeyFrame
	{
	};

	typedef std::map<float, KeyFrame> KeyFrameMap;

	KeyFrameMap m_Keys;

public:
	ActionTrackSphericalEmitter(void)
		: m_ParticleCapacity(1024)
		, m_ParticleLifeTime(FLT_MAX)
		, m_SpawnInterval(FLT_MAX)
		, m_SpawnLength(0)
		, m_SpawnSpeed(0)
		, m_HalfSpawnArea(0, 0, 0)
	{
	}

	virtual ActionTrackInstPtr CreateInstance(Actor * _Actor) const;

	void AddKeyFrame(float Time);
};

class EmitterComponent;

class ActionTrackSphericalEmitterInst : public ActionTrackInst
{
protected:
	boost::intrusive_ptr<const ActionTrackSphericalEmitter> m_Template;

	boost::shared_ptr<Actor> m_WorldEmitterActor;

	boost::shared_ptr<EmitterComponent> m_WorldEmitterInst;

public:
	ActionTrackSphericalEmitterInst(Actor * _Actor, const ActionTrackSphericalEmitter * Template);

	virtual ~ActionTrackSphericalEmitterInst(void);

	virtual void UpdateTime(float Time, float fElapsedTime);

	virtual void OnStop(void);
};
