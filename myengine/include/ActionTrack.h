#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/circular_buffer.hpp>
#include <vector>
#include <map>
#include <list>
#include "myMath.h"
#include "mySpline.h"
#include "myTask.h"
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

	ActionInstPtr CreateInstance(Actor * _Actor, float Rate);
};

class ActionTrackInst;

typedef boost::shared_ptr<ActionTrackInst> ActionTrackInstPtr;

class ActionInst
{
public:
	boost::shared_ptr<const Action> m_Template;

	float m_LastTime;

	float m_Time;

	float m_Rate;

	typedef std::vector<ActionTrackInstPtr> ActionTrackInstPtrList;

	ActionTrackInstPtrList m_TrackInstList;

public:
	ActionInst(Actor * _Actor, boost::shared_ptr<const Action> Template, float Rate);

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

	virtual ActionTrackInstPtr CreateInstance(Actor * _Actor, float Rate) const = 0;
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
	}

	virtual void UpdateTime(float LastTime, float Time) = 0;

	virtual void Stop(void) = 0;

	virtual bool GetDisplacement(float LastTime, float dtime, my::Vector3 & disp) { return false; }
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
		bool Loop;
		int Prority;
	};

	typedef std::multimap<float, KeyFrame> KeyFrameMap;

	KeyFrameMap m_Keys;

	float m_ParamWeight;

public:
	ActionTrackAnimation(void)
		: m_ParamWeight(1.0f)
	{
	}

	virtual ActionTrackInstPtr CreateInstance(Actor * _Actor, float Rate) const;

	void AddKeyFrame(float Time, const char * SlotName, const char * Name, float Rate, float Weight, float BlendTime, float BlendOutTime, bool Loop, int Prority);
};

class ActionTrackAnimationInst : public ActionTrackInst
{
protected:
	boost::shared_ptr<const ActionTrackAnimation> m_Template;

	float m_Rate;

	float m_Weight;

public:
	ActionTrackAnimationInst(Actor * _Actor, boost::shared_ptr<const ActionTrackAnimation> Template, float Rate)
		: ActionTrackInst(_Actor)
		, m_Template(Template)
		, m_Rate(Rate)
		, m_Weight(Template->m_ParamWeight)
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

	virtual ActionTrackInstPtr CreateInstance(Actor * _Actor, float Rate) const;

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

class ActionTrackEmitter : public ActionTrack
{
public:
	boost::shared_ptr<Material> m_EmitterMaterial;

	unsigned int m_EmitterCapacity;

	DWORD m_EmitterFaceType;

	DWORD m_EmitterSpaceType;

	my::AABB m_SpawnArea;

	my::Vector2 m_SpawnInclination;

	my::Vector2 m_SpawnAzimuth;

	float m_SpawnSpeed;

	int m_SpawnBoneId;

	my::Bone m_SpawnLocalPose;

	float m_ParticleLifeTime;

	float m_ParticleDamping;

	my::Spline m_ParticleColorR;

	my::Spline m_ParticleColorG;

	my::Spline m_ParticleColorB;

	my::Spline m_ParticleColorA;

	my::Spline m_ParticleSizeX;

	my::Spline m_ParticleSizeY;

	my::Spline m_ParticleAngle;

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
		, m_EmitterSpaceType(0) // SpaceTypeWorld = 0
		, m_SpawnArea(-0.0f,0.0f)
		, m_SpawnInclination(D3DXToRadian(-90), D3DXToRadian(90))
		, m_SpawnAzimuth(D3DXToRadian(0), D3DXToRadian(360))
		, m_SpawnSpeed(0.0f)
		, m_SpawnBoneId(-1)
		, m_SpawnLocalPose(my::Vector3(0, 0, 0))
		, m_ParticleLifeTime(1.0f)
		, m_ParticleDamping(1.0f)
	{
	}

	virtual ActionTrackInstPtr CreateInstance(Actor * _Actor, float Rate) const;

	void AddKeyFrame(float Time, int SpawnCount, float SpawnInterval);
};

class CircularEmitter;

class ActionTrackEmitterInst : public ActionTrackInst, public my::ParallelTask
{
protected:
	boost::shared_ptr<const ActionTrackEmitter> m_Template;

	//boost::shared_ptr<Actor> m_WorldEmitterActor;

	boost::shared_ptr<CircularEmitter> m_WorldEmitterCmp;

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

	float m_TaskDuration;

public:
	ActionTrackEmitterInst(Actor * _Actor, boost::shared_ptr<const ActionTrackEmitter> Template);

	virtual ~ActionTrackEmitterInst(void);

	virtual void UpdateTime(float LastTime, float Time);

	virtual void Stop(void);

	virtual void DoTask(void);
};

class ActionTrackVelocity : public ActionTrack
{
public:
	struct KeyFrame
	{
		float Length;
	};

	typedef std::multimap<float, KeyFrame> KeyFrameMap;

	KeyFrameMap m_Keys;

	my::Vector3 m_ParamVelocity;

public:
	ActionTrackVelocity(void)
		: m_ParamVelocity(0,0,0)
	{
	}

	virtual ActionTrackInstPtr CreateInstance(Actor * _Actor, float Rate) const;

	void AddKeyFrame(float Time, float Length);
};

class ActionTrackVelocityInst : public ActionTrackInst
{
protected:
	boost::shared_ptr<const ActionTrackVelocity> m_Template;

	my::Vector3 m_Velocity;

public:
	ActionTrackVelocityInst(Actor * _Actor, boost::shared_ptr<const ActionTrackVelocity> Template);

	virtual void UpdateTime(float LastTime, float Time);

	virtual void Stop(void);

	virtual bool GetDisplacement(float LastTime, float dtime, my::Vector3 & disp);
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

	virtual ActionTrackInstPtr CreateInstance(Actor * _Actor, float Rate) const;

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

		KeyFrameInst(float Time, float Length, Actor * actor);
	};

	typedef std::vector<KeyFrameInst> KeyFrameInstList;

	KeyFrameInstList m_KeyInsts;

	my::Bone m_Pose;

public:
	ActionTrackPoseInst(Actor * _Actor, boost::shared_ptr<const ActionTrackPose> Template);

	virtual void UpdateTime(float LastTime, float Time);

	virtual void Stop(void);

	virtual bool GetDisplacement(float LastTime, float dtime, my::Vector3 & disp);
};
