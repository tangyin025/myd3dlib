#include "ActionTrack.h"
#include "Actor.h"
#include "Animation.h"
#include "FModContext.h"
#include <fmod_errors.h>
#include "libc.h"
#include "myOctree.h"
#include "Material.h"

using namespace my;

#define ERRCHECK(result) if ((result) != FMOD_OK) { \
	throw my::CustomException(str_printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result)), __FILE__, __LINE__); }

void Action::AddTrack(ActionTrackPtr track)
{
	m_TrackList.push_back(track);
}

void Action::RemoveTrack(ActionTrackPtr track)
{
	ActionTrackPtrList::iterator track_iter = std::find(m_TrackList.begin(), m_TrackList.end(), track);
	if (track_iter != m_TrackList.end())
	{
		m_TrackList.erase(track_iter);
	}
}

ActionTrackInstPtr ActionTrackAnimation::CreateInstance(Actor * _Actor) const
{
	return ActionTrackInstPtr(new ActionTrackAnimationInst(_Actor, this));
}

void ActionTrackAnimation::AddKeyFrame(float Time, const char * Name, const char * RootList, float Rate, float BlendTime, float BlendOutTime, bool Loop, int Prority, float StartTime, const char * Group)
{
	KeyFrame & key = m_Keys[Time];
	key.Name = Name;
	key.RootList = RootList;
	key.Rate = Rate;
	key.BlendTime = BlendTime;
	key.BlendOutTime = BlendOutTime;
	key.Loop = Loop;
	key.Prority = Prority;
	key.StartTime = StartTime;
	key.Group = Group;
}

void ActionTrackAnimationInst::UpdateTime(float Time, float fElapsedTime)
{
	_ASSERT(m_Template);

	_ASSERT(m_Actor);

	ActionTrackAnimation::KeyFrameMap::const_iterator key_iter = m_Template->m_Keys.lower_bound(Time);
	ActionTrackAnimation::KeyFrameMap::const_iterator key_end = m_Template->m_Keys.upper_bound(Time + fElapsedTime);
	for (; key_iter != key_end; key_iter++)
	{
		if (m_Actor->m_Animation)
		{
			m_Actor->m_Animation->Play(
				key_iter->second.Name,
				key_iter->second.RootList,
				key_iter->second.Rate,
				key_iter->second.BlendTime,
				key_iter->second.BlendOutTime,
				key_iter->second.Loop,
				key_iter->second.Prority,
				key_iter->second.StartTime,
				key_iter->second.Group,
				(DWORD_PTR)this);
		}
	}
}

void ActionTrackAnimationInst::OnStop(void)
{
	if (m_Actor->m_Animation)
	{
		m_Actor->m_Animation->Stop((DWORD_PTR)this);
	}
}

ActionTrackInstPtr ActionTrackSound::CreateInstance(Actor * _Actor) const
{
	return ActionTrackInstPtr(new ActionTrackSoundInst(_Actor, this));
}

void ActionTrackSound::AddKeyFrame(float Time, const char * Name)
{
	KeyFrame & key = m_Keys[Time];
	key.Name = Name;
}

class ActionTrackSoundInstCallback
{
public:
	static FMOD_RESULT F_CALLBACK _FmodEventCallback(
		FMOD_EVENT * _event,
		FMOD_EVENT_CALLBACKTYPE type,
		void * param1,
		void * param2,
		void * userdata)
	{
		ActionTrackSoundInst * inst = (ActionTrackSoundInst *)userdata;
		FMOD::Event * _event_obj = (FMOD::Event *)_event;
		switch (type)
		{
		case FMOD_EVENT_CALLBACKTYPE_STOLEN:
		{
			ActionTrackSoundInst::FmodEventSet::iterator evt_iter = inst->m_evts.find(_event_obj);
			if (evt_iter != inst->m_evts.end())
			{
				inst->m_evts.erase(evt_iter);
			}
			else
			{
				_ASSERT(false);
			}
			break;
		}
		}
		return FMOD_OK;
	}
};

ActionTrackSoundInst::~ActionTrackSoundInst(void)
{
	StopAllEvent(true);
}

void ActionTrackSoundInst::UpdateTime(float Time, float fElapsedTime)
{
	_ASSERT(m_Template);

	_ASSERT(m_Actor);

	ActionTrackSound::KeyFrameMap::const_iterator key_iter = m_Template->m_Keys.lower_bound(Time);
	ActionTrackSound::KeyFrameMap::const_iterator key_end = m_Template->m_Keys.upper_bound(Time + fElapsedTime);
	for (; key_iter != key_end; key_iter++)
	{
		FMOD_RESULT result;
		FMOD::Event *_event;
		ERRCHECK(FModContext::getSingleton().m_EventSystem->getEvent(key_iter->second.Name.c_str(), FMOD_EVENT_INFOONLY, &_event));
		ERRCHECK(_event->set3DAttributes((FMOD_VECTOR *)&m_Actor->m_Position, NULL, NULL));
		result = FModContext::getSingleton().m_EventSystem->getEvent(key_iter->second.Name.c_str(), FMOD_EVENT_DEFAULT, &_event);
		if (FMOD_OK == result)
		{
			m_evts.insert(_event);
			ERRCHECK(_event->setCallback(&ActionTrackSoundInstCallback::_FmodEventCallback, this));
			ERRCHECK(_event->start());
		}
	}
}

void ActionTrackSoundInst::OnStop(void)
{
	StopAllEvent(false);
}

void ActionTrackSoundInst::StopAllEvent(bool immediate)
{
	FmodEventSet::iterator evt_iter = m_evts.begin();
	for (; evt_iter != m_evts.end(); evt_iter = m_evts.begin())
	{
		FMOD_EVENT_STATE state;
		ERRCHECK((*evt_iter)->getState(&state));
		if (state & FMOD_EVENT_STATE_PLAYING)
		{
			ERRCHECK((*evt_iter)->stop(immediate));
		}
		ERRCHECK((*evt_iter)->setCallback(NULL, NULL));
		m_evts.erase(evt_iter);
	}
}

ActionTrackInstPtr ActionTrackSphericalEmitter::CreateInstance(Actor * _Actor) const
{
	return ActionTrackInstPtr(new ActionTrackSphericalEmitterInst(_Actor, this));
}

void ActionTrackSphericalEmitter::AddKeyFrame(float Time)
{
	KeyFrame & key = m_Keys[Time];
}

ActionTrackSphericalEmitterInst::ActionTrackSphericalEmitterInst(Actor * _Actor, const ActionTrackSphericalEmitter * Template)
	: ActionTrackInst(_Actor)
	, m_Template(Template)
{
	m_WorldEmitterInst.reset(new EmitterComponent(Component::ComponentTypeEmitter, m_Template->m_ParticleCapacity));
	m_WorldEmitterInst->m_Material = m_Template->m_ParticleMaterial->Clone();

	my::OctNode * Root = m_Actor->m_Node->GetTopNode();
	m_WorldEmitterActor.reset(new Actor(Vector3(0, 0, 0), Quaternion::Identity(), Vector3(1, 1, 1), Root->m_aabb));
	m_WorldEmitterActor->AddComponent(m_WorldEmitterInst);
	Root->AddEntity(m_WorldEmitterActor.get(), m_WorldEmitterActor->m_aabb);
}

ActionTrackSphericalEmitterInst::~ActionTrackSphericalEmitterInst(void)
{
	my::OctNode * Root = m_Actor->m_Node->GetTopNode();
	Root->RemoveEntity(m_WorldEmitterActor.get());
}

void ActionTrackSphericalEmitterInst::UpdateTime(float Time, float fElapsedTime)
{
	m_WorldEmitterInst->RemoveParticleBefore(Time + fElapsedTime - m_Template->m_ParticleLifeTime);

	ActionTrackSphericalEmitter::KeyFrameMap::const_iterator key_iter = m_Template->m_Keys.lower_bound(Time - m_Template->m_SpawnLength);
	ActionTrackSphericalEmitter::KeyFrameMap::const_iterator key_end = m_Template->m_Keys.upper_bound(Time + fElapsedTime);
	for (; key_iter != key_end; key_iter++)
	{
		float SpawnTime = m_WorldEmitterInst->m_EmitterTime > m_Template->m_SpawnInterval ?
			(m_WorldEmitterInst->m_EmitterTime + fmod(m_WorldEmitterInst->m_EmitterTime, m_Template->m_SpawnInterval)) :
			(m_WorldEmitterInst->m_EmitterTime <= 0 ? m_WorldEmitterInst->m_EmitterTime : m_Template->m_SpawnInterval);
		for (; SpawnTime < Time + fElapsedTime; SpawnTime += m_Template->m_SpawnInterval)
		{
			m_WorldEmitterInst->Spawn(
				m_Actor->m_Position + Vector3(
					Random(m_Template->m_HalfSpawnArea.x, m_Template->m_HalfSpawnArea.x),
					Random(m_Template->m_HalfSpawnArea.y, m_Template->m_HalfSpawnArea.y),
					Random(m_Template->m_HalfSpawnArea.z, m_Template->m_HalfSpawnArea.z)),
				Vector3::SphericalToCartesian(
					m_Template->m_SpawnSpeed,
					m_Template->m_SpawnInclination.Interpolate(Time + fElapsedTime, 0),
					m_Template->m_SpawnAzimuth.Interpolate(Time + fElapsedTime, 0)),
				Vector4(
					m_Template->m_SpawnColorR.Interpolate(Time + fElapsedTime, 1),
					m_Template->m_SpawnColorG.Interpolate(Time + fElapsedTime, 1),
					m_Template->m_SpawnColorB.Interpolate(Time + fElapsedTime, 1),
					m_Template->m_SpawnColorA.Interpolate(Time + fElapsedTime, 1)),
				Vector2(m_Template->m_SpawnSizeX.Interpolate(Time + fElapsedTime, 1)),
				m_Template->m_SpawnAngle.Interpolate(Time + fElapsedTime, 0),
				SpawnTime);
		}
	}
}

void ActionTrackSphericalEmitterInst::OnStop(void)
{
	m_WorldEmitterInst->RemoveAllParticle();
}
