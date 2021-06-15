#include "ActionTrack.h"
#include "Actor.h"
#include "Animation.h"
#include "FModContext.h"
#include <fmod_errors.h>
#include "libc.h"
#include "myOctree.h"
#include "Material.h"
#include "PhysxContext.h"

using namespace my;

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

ActionInstPtr Action::CreateInstance(Actor * _Actor)
{
	return ActionInstPtr(new ActionInst(_Actor, this));
}

ActionInst::ActionInst(Actor * _Actor, const Action * Template)
	: m_Template(Template)
	, m_Time(0)
{
	Action::ActionTrackPtrList::const_iterator track_iter = m_Template->m_TrackList.begin();
	for (; track_iter != m_Template->m_TrackList.end(); track_iter++)
	{
		m_TrackInstList.push_back((*track_iter)->CreateInstance(_Actor));
	}
}

void ActionInst::Update(float fElapsedTime)
{
	ActionTrackInstPtrList::iterator track_inst_iter = m_TrackInstList.begin();
	for (; track_inst_iter != m_TrackInstList.end(); track_inst_iter++)
	{
		(*track_inst_iter)->UpdateTime(m_Time, fElapsedTime);
	}
	m_Time += fElapsedTime;
}

void ActionInst::Stop(void)
{
	ActionTrackInstPtrList::iterator track_inst_iter = m_TrackInstList.begin();
	for (; track_inst_iter != m_TrackInstList.end(); track_inst_iter++)
	{
		(*track_inst_iter)->Stop();
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

	Animator* animator = m_Actor->GetAnimator();
	if (animator)
	{
		ActionTrackAnimation::KeyFrameMap::const_iterator key_iter = m_Template->m_Keys.lower_bound(Time);
		ActionTrackAnimation::KeyFrameMap::const_iterator key_end = m_Template->m_Keys.upper_bound(Time + fElapsedTime);
		for (; key_iter != key_end; key_iter++)
		{
			animator->Play(
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

void ActionTrackAnimationInst::Stop(void)
{
	Animator* animator = m_Actor->GetAnimator();
	if (animator)
	{
		animator->Stop((DWORD_PTR)this);
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

void ActionTrackSoundInst::Stop(void)
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

ActionTrackInstPtr ActionTrackEmitter::CreateInstance(Actor * _Actor) const
{
	return ActionTrackInstPtr(new ActionTrackEmitterInst(_Actor, this));
}

void ActionTrackEmitter::AddKeyFrame(float Time, int SpawnCount, float SpawnInterval)
{
	KeyFrame & key = m_Keys[Time];
	key.SpawnCount = SpawnCount;
	key.SpawnInterval = SpawnInterval;
}

ActionTrackEmitterInst::ActionTrackEmitterInst(Actor * _Actor, const ActionTrackEmitter * Template)
	: ActionTrackInst(_Actor)
	, m_Template(Template)
	, m_AttachBoneId(-1)
	, m_ActionTime(0)
	, m_TaskEvent(NULL, TRUE, TRUE, NULL)
{
	m_WorldEmitterCmp.reset(new EmitterComponent(Component::ComponentTypeEmitter,
		NamedObject::MakeUniqueName("ActionTrackEmitterInst_cmp").c_str(), m_Template->m_EmitterCapacity, (EmitterComponent::FaceType)m_Template->m_EmitterFaceType, EmitterComponent::SpaceTypeWorld, EmitterComponent::VelocityTypeNone));
	m_WorldEmitterCmp->SetMaterial(m_Template->m_EmitterMaterial->Clone());

	//if (!m_Actor->m_Node)
	//{
	//	THROW_CUSEXCEPTION("ActionTrackEmitterInst: !m_Actor->m_Node"); // ! Actor::PlayAction should be call after being AddEntity
	//}

	//my::OctNode * Root = m_Actor->m_Node->GetTopNode();
	//m_WorldEmitterActor.reset(new Actor(
	//	NamedObject::MakeUniqueName("ActionTrackEmitterInst_actor").c_str(), Vector3(0, 0, 0), Quaternion::Identity(), Vector3(1, 1, 1), *Root));
	//m_WorldEmitterActor->AddComponent(m_WorldEmitterCmp);
	//Root->AddEntity(m_WorldEmitterActor.get(), m_WorldEmitterActor->m_aabb, Actor::MinBlock, Actor::Threshold);

	m_Actor->AddComponent(m_WorldEmitterCmp);

	Animator* animator = m_Actor->GetAnimator();
	if (animator)
	{
		OgreSkeleton::BoneNameMap::const_iterator bone_name_iter = animator->m_Skeleton->m_boneNameMap.find(m_Template->m_AttachBoneName);
		if (bone_name_iter != animator->m_Skeleton->m_boneNameMap.end())
		{
			m_AttachBoneId = bone_name_iter->second;
		}
	}
}

ActionTrackEmitterInst::~ActionTrackEmitterInst(void)
{
	_ASSERT(m_TaskEvent.Wait(0));

	//if (m_WorldEmitterActor->m_Node)
	//{
	//	my::OctNode * Root = m_WorldEmitterActor->m_Node->GetTopNode();
	//	Root->RemoveEntity(m_WorldEmitterActor.get());
	//}

	m_Actor->RemoveComponent(m_WorldEmitterCmp);
}

void ActionTrackEmitterInst::UpdateTime(float Time, float fElapsedTime)
{
	m_ActionTime = Time + fElapsedTime;

	m_WorldEmitterCmp->RemoveParticleBefore(m_ActionTime - m_Template->m_ParticleLifeTime);

	ActionTrackEmitter::KeyFrameMap::const_iterator key_iter = m_Template->m_Keys.lower_bound(Time);
	ActionTrackEmitter::KeyFrameMap::const_iterator key_end = m_Template->m_Keys.upper_bound(m_ActionTime);
	for (; key_iter != key_end; key_iter++)
	{
		m_KeyInsts.push_back(KeyFrameInst(
			key_iter->first, key_iter->second.SpawnCount, key_iter->second.SpawnInterval));
	}

	Animator* animator = m_Actor->GetAnimator();
	KeyFrameInstList::iterator key_inst_iter = m_KeyInsts.begin();
	for (; key_inst_iter != m_KeyInsts.end(); )
	{
		for (; key_inst_iter->m_Time < m_ActionTime && key_inst_iter->m_SpawnCount > 0;
			key_inst_iter->m_Time += key_inst_iter->m_SpawnInterval, key_inst_iter->m_SpawnCount--)
		{
			Vector3 SpawnPos;
			if (m_AttachBoneId >= 0 && animator)
			{
				const Bone& bone = animator->anim_pose_hier[m_AttachBoneId];
				SpawnPos = bone.m_position.transformCoord(m_Actor->m_World);
			}
			else
			{
				SpawnPos = m_Actor->m_Position;
			}

			m_WorldEmitterCmp->Spawn(
				Vector3(0, 0, 0), SpawnPos, Vector4(1, 1, 1, 1), Vector2(1, 1), 0, key_inst_iter->m_Time);
		}

		if (key_inst_iter->m_SpawnCount <= 0)
		{
			key_inst_iter = m_KeyInsts.erase(key_inst_iter);
		}
		else
		{
			key_inst_iter++;
		}
	}

	m_TaskEvent.ResetEvent();

	ParallelTaskManager::getSingleton().PushTask(this);
}

void ActionTrackEmitterInst::Stop(void)
{
	m_TaskEvent.Wait(INFINITE);

	m_WorldEmitterCmp->RemoveAllParticle();
}

void ActionTrackEmitterInst::DoTask(void)
{
	// ! take care of thread safe
	Emitter::ParticleList::iterator particle_iter = m_WorldEmitterCmp->m_ParticleList.begin();
	for (; particle_iter != m_WorldEmitterCmp->m_ParticleList.end(); particle_iter++)
	{
		const float ParticleTime = m_ActionTime - particle_iter->m_Time;
		particle_iter->m_Position.x = particle_iter->m_Velocity.x + m_Template->m_ParticleVelocityX.Interpolate(ParticleTime, 0);
		particle_iter->m_Position.y = particle_iter->m_Velocity.y + m_Template->m_ParticleVelocityY.Interpolate(ParticleTime, 0);
		particle_iter->m_Position.z = particle_iter->m_Velocity.z + m_Template->m_ParticleVelocityZ.Interpolate(ParticleTime, 0);
		particle_iter->m_Color.x = m_Template->m_ParticleColorR.Interpolate(ParticleTime, 1);
		particle_iter->m_Color.y = m_Template->m_ParticleColorG.Interpolate(ParticleTime, 1);
		particle_iter->m_Color.z = m_Template->m_ParticleColorB.Interpolate(ParticleTime, 1);
		particle_iter->m_Color.w = m_Template->m_ParticleColorA.Interpolate(ParticleTime, 1);
		particle_iter->m_Size.x = m_Template->m_ParticleSizeX.Interpolate(ParticleTime, 1);
		particle_iter->m_Size.y = m_Template->m_ParticleSizeY.Interpolate(ParticleTime, 1);
		particle_iter->m_Angle = m_Template->m_ParticleAngle.Interpolate(ParticleTime, 0);
	}

	m_TaskEvent.SetEvent();
}

ActionTrackInstPtr ActionTrackPose::CreateInstance(Actor * _Actor) const
{
	return ActionTrackInstPtr(new ActionTrackPoseInst(_Actor, this, m_ParamStartPos, m_ParamEndPos));
}

void ActionTrackPose::AddKeyFrame(float Time)
{
	m_Keys.insert(std::make_pair(Time, KeyFrame()));
}

ActionTrackPoseInst::~ActionTrackPoseInst(void)
{
	//_ASSERT(m_Actor->m_ActionTrackPoseInstRef == 0);
}

void ActionTrackPoseInst::UpdateTime(float Time, float fElapsedTime)
{
	KeyFrameInstList::iterator key_inst_iter = m_KeyInsts.begin();
	if (key_inst_iter != m_KeyInsts.end())
	{
		key_inst_iter->m_Time += fElapsedTime;
		if (key_inst_iter->m_Time <= m_Template->m_Length)
		{
			my::Vector3 Pos(
				my::Lerp(m_StartPos.x, m_EndPos.x, m_Template->m_InterpolateX.Interpolate(key_inst_iter->m_Time, 0)),
				my::Lerp(m_StartPos.y, m_EndPos.y, m_Template->m_InterpolateY.Interpolate(key_inst_iter->m_Time, 0)),
				my::Lerp(m_StartPos.z, m_EndPos.z, m_Template->m_InterpolateZ.Interpolate(key_inst_iter->m_Time, 0)));
			m_Actor->SetPose(Pos, m_Actor->m_Rotation);
		}
		else
		{
			m_KeyInsts.erase(key_inst_iter);
			m_Actor->m_ActionTrackPoseInstRef--;
		}
	}
	else
	{
		ActionTrackPose::KeyFrameMap::const_iterator key_iter = m_Template->m_Keys.lower_bound(Time);
		ActionTrackPose::KeyFrameMap::const_iterator key_end = m_Template->m_Keys.upper_bound(Time + fElapsedTime);
		if (key_iter != key_end)
		{
			m_KeyInsts.push_back(KeyFrameInst());
			m_Actor->m_ActionTrackPoseInstRef++;
		}
	}
}

void ActionTrackPoseInst::Stop(void)
{
	KeyFrameInstList::iterator key_inst_iter = m_KeyInsts.begin();
	for (; key_inst_iter != m_KeyInsts.end(); key_inst_iter = m_KeyInsts.begin())
	{
		m_KeyInsts.erase(key_inst_iter);
		m_Actor->m_ActionTrackPoseInstRef--;
	}
}
