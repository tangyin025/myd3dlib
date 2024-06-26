// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "ActionTrack.h"
#include "Actor.h"
#include "Animator.h"
#include "libc.h"
#include "myOctree.h"
#include "myResource.h"
#include "Material.h"
#include "PhysxContext.h"
#include "Controller.h"
#include "myDxutApp.h"

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
	return ActionInstPtr(new ActionInst(_Actor, shared_from_this()));
}

ActionInst::ActionInst(Actor * _Actor, boost::shared_ptr<const Action> Template)
	: m_Template(Template)
	, m_LastTime(0.0f)
	, m_Time(0.0f)
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
		(*track_inst_iter)->UpdateTime(m_LastTime, m_Time);
	}

	if (m_Time > m_LastTime)
	{
		m_LastTime = m_Time;
	}
}

void ActionInst::StopAllTrack(void)
{
	ActionTrackInstPtrList::iterator track_inst_iter = m_TrackInstList.begin();
	for (; track_inst_iter != m_TrackInstList.end(); track_inst_iter++)
	{
		(*track_inst_iter)->Stop();
	}
}

ActionTrackInstPtr ActionTrackAnimation::CreateInstance(Actor * _Actor) const
{
	return ActionTrackInstPtr(new ActionTrackAnimationInst(_Actor, boost::static_pointer_cast<const ActionTrackAnimation>(shared_from_this())));
}

void ActionTrackAnimation::AddKeyFrame(float Time, const char * SlotName, const char * Name, float Rate, float Weight, float BlendTime, float BlendOutTime, const char * Group, int Prority)
{
	KeyFrameMap::iterator key_iter = m_Keys.insert(std::make_pair(Time, KeyFrame()));
	_ASSERT(key_iter != m_Keys.end());
	key_iter->second.SlotName = SlotName;
	key_iter->second.Name = Name;
	key_iter->second.Rate = Rate;
	key_iter->second.Weight = Weight;
	key_iter->second.BlendTime = BlendTime;
	key_iter->second.BlendOutTime = BlendOutTime;
	key_iter->second.Group = Group;
	key_iter->second.Prority = Prority;
}

void ActionTrackAnimationInst::UpdateTime(float LastTime, float Time)
{
	_ASSERT(m_Template);

	_ASSERT(m_Actor);

	Animator* animator = m_Actor->GetFirstComponent<Animator>();
	if (animator)
	{
		ActionTrackAnimation::KeyFrameMap::const_iterator key_iter = m_Template->m_Keys.lower_bound(LastTime);
		ActionTrackAnimation::KeyFrameMap::const_iterator key_end = m_Template->m_Keys.lower_bound(Time);
		for (; key_iter != key_end; key_iter++)
		{
			AnimationNodeSlot * slot = dynamic_cast<AnimationNodeSlot *>(animator->FindSubNode(key_iter->second.SlotName));
			if (slot)
			{
				slot->Play(
					key_iter->second.Name,
					key_iter->second.Rate,
					key_iter->second.Weight,
					key_iter->second.BlendTime,
					key_iter->second.BlendOutTime,
					key_iter->second.Group,
					key_iter->second.Prority,
					(DWORD_PTR)this);
			}
		}
	}
}

void ActionTrackAnimationInst::Stop(void)
{
	Animator* animator = m_Actor->GetFirstComponent<Animator>();
	if (animator)
	{
		ActionTrackAnimation::KeyFrameMap::const_iterator key_iter = m_Template->m_Keys.begin();
		for (; key_iter != m_Template->m_Keys.end(); key_iter++)
		{
			AnimationNodeSlot* slot = dynamic_cast<AnimationNodeSlot*>(animator->FindSubNode(key_iter->second.SlotName));
			if (slot)
			{
				slot->StopSlotByUserData((DWORD_PTR)this, 0.1f);
			}
		}
	}
}

void ActionTrackSound::KeyFrame::OnSoundReady(my::DeviceResourceBasePtr res)
{
	Sound = boost::dynamic_pointer_cast<my::Wav>(res);
}

ActionTrackSound::~ActionTrackSound(void)
{
	KeyFrameMap::iterator key_iter = m_Keys.begin();
	for (; key_iter != m_Keys.end(); key_iter++)
	{
		if (!key_iter->second.SoundPath.empty())
		{
			my::ResourceMgr::getSingleton().RemoveIORequestCallback(key_iter->second.SoundPath, boost::bind(&ActionTrackSound::KeyFrame::OnSoundReady, &key_iter->second, boost::placeholders::_1));

			key_iter->second.Sound.reset();
		}
	}
}

ActionTrackInstPtr ActionTrackSound::CreateInstance(Actor * _Actor) const
{
	return ActionTrackInstPtr(new ActionTrackSoundInst(_Actor, boost::static_pointer_cast<const ActionTrackSound>(shared_from_this())));
}

void ActionTrackSound::AddKeyFrame(float Time, const char * SoundPath, float StartSec, float EndSec, bool Loop, float MinDistance, float MaxDistance)
{
	KeyFrameMap::iterator key_iter = m_Keys.insert(std::make_pair(Time, KeyFrame()));
	_ASSERT(key_iter != m_Keys.end());
	key_iter->second.SoundPath = SoundPath;
	key_iter->second.StartSec = StartSec;
	key_iter->second.EndSec = EndSec;
	key_iter->second.Loop = Loop;
	key_iter->second.MinDistance = MinDistance;
	key_iter->second.MaxDistance = MaxDistance;

	if (!key_iter->second.SoundPath.empty())
	{
		_ASSERT(!key_iter->second.Sound);

		my::ResourceMgr::getSingleton().LoadWavAsync(SoundPath, boost::bind(&ActionTrackSound::KeyFrame::OnSoundReady, &key_iter->second, boost::placeholders::_1), 0);
	}
}

ActionTrackSoundInst::~ActionTrackSoundInst(void)
{
#ifdef _DEBUG
	SoundEventList::iterator event_iter = m_Events.begin();
	for (; event_iter != m_Events.end(); event_iter++)
	{
		if ((*event_iter)->m_sbuffer)
		{
			_ASSERT(!(*event_iter)->m_sbuffer->GetStatus() & DSBSTATUS_PLAYING);
		}
	}
#endif
}

void ActionTrackSoundInst::UpdateTime(float LastTime, float Time)
{
	_ASSERT(m_Template);

	_ASSERT(m_Actor);

	SoundEventList::iterator event_iter = m_Events.begin();
	for (; event_iter != m_Events.end() && !(*event_iter)->m_sbuffer; )
	{
		event_iter = m_Events.erase(event_iter);
	}

	const Vector3 listener_pos = SoundContext::getSingleton().m_listener->GetPosition();

	ActionTrackSound::KeyFrameMap::const_iterator key_iter = m_Template->m_Keys.lower_bound(LastTime);
	ActionTrackSound::KeyFrameMap::const_iterator key_end = m_Template->m_Keys.lower_bound(Time);
	for (; key_iter != key_end; key_iter++)
	{
		if (key_iter->second.Sound)
		{
			const Vector3& pos = m_Actor->m_World.getRow<3>().xyz;

			if (key_iter->second.Loop || listener_pos.distanceSq(pos) < key_iter->second.MaxDistance * key_iter->second.MaxDistance)
			{
				m_Events.push_back(SoundContext::getSingleton().Play(
					key_iter->second.Sound, key_iter->second.StartSec, key_iter->second.EndSec, key_iter->second.Loop, pos, Vector3(0, 0, 0), key_iter->second.MinDistance, key_iter->second.MaxDistance));
			}
		}
	}
}

void ActionTrackSoundInst::Stop(void)
{
	SoundEventList::iterator event_iter = m_Events.begin();
	for (; event_iter != m_Events.end(); event_iter++)
	{
		if ((*event_iter)->m_sbuffer)
		{
			(*event_iter)->m_sbuffer->Stop();
		}
	}
}

ActionTrackInstPtr ActionTrackEmitter::CreateInstance(Actor * _Actor) const
{
	return ActionTrackInstPtr(new ActionTrackEmitterInst(_Actor, boost::static_pointer_cast<const ActionTrackEmitter>(shared_from_this())));
}

void ActionTrackEmitter::AddKeyFrame(float Time, int SpawnCount, float SpawnInterval)
{
	KeyFrameMap::iterator key_iter = m_Keys.insert(std::make_pair(Time, KeyFrame()));
	_ASSERT(key_iter != m_Keys.end());
	key_iter->second.SpawnCount = SpawnCount;
	key_iter->second.SpawnInterval = SpawnInterval;
}

ActionTrackEmitterInst::ActionTrackEmitterInst(Actor * _Actor, boost::shared_ptr<const ActionTrackEmitter> Template)
	: ActionTrackInst(_Actor)
	, m_Template(Template)
{
}

ActionTrackEmitterInst::~ActionTrackEmitterInst(void)
{
	_ASSERT(!m_EmitterCmp);
}

void ActionTrackEmitterInst::UpdateTime(float LastTime, float Time)
{
	ActionTrackEmitter::KeyFrameMap::const_iterator key_iter = m_Template->m_Keys.lower_bound(LastTime);
	ActionTrackEmitter::KeyFrameMap::const_iterator key_end = m_Template->m_Keys.lower_bound(Time);
	for (; key_iter != key_end; key_iter++)
	{
		m_KeyInsts.push_back(KeyFrameInst(key_iter->second.SpawnCount, key_iter->second.SpawnInterval));
	}

	KeyFrameInstList::iterator key_inst_iter = m_KeyInsts.begin();
	for (; key_inst_iter != m_KeyInsts.end(); )
	{
		key_inst_iter->m_Time += my::D3DContext::getSingleton().m_fElapsedTime;
		for (; key_inst_iter->m_Time >= key_inst_iter->m_SpawnInterval && key_inst_iter->m_SpawnCount > 0;
			key_inst_iter->m_Time -= key_inst_iter->m_SpawnInterval, key_inst_iter->m_SpawnCount--)
		{
			if (!m_EmitterCmp)
			{
				m_EmitterCmp.reset(new SphericalEmitter(NamedObject::MakeUniqueName("ActionTrackEmitterInst_cmp").c_str(),
					m_Template->m_EmitterCapacity, (EmitterComponent::FaceType)m_Template->m_EmitterFaceType, (EmitterComponent::SpaceType)m_Template->m_EmitterSpaceType));
				m_EmitterCmp->m_SpawnInterval = 0;
				m_EmitterCmp->m_ParticleLifeTime = m_Template->m_ParticleLifeTime;
				m_EmitterCmp->m_ParticleDamping = m_Template->m_ParticleDamping;
				m_EmitterCmp->m_ParticleColorR = m_Template->m_ParticleColorR;
				m_EmitterCmp->m_ParticleColorG = m_Template->m_ParticleColorG;
				m_EmitterCmp->m_ParticleColorB = m_Template->m_ParticleColorB;
				m_EmitterCmp->m_ParticleColorA = m_Template->m_ParticleColorA;
				m_EmitterCmp->m_ParticleSizeX = m_Template->m_ParticleSizeX;
				m_EmitterCmp->m_ParticleSizeY = m_Template->m_ParticleSizeY;
				m_EmitterCmp->m_ParticleAngle = m_Template->m_ParticleAngle;
				m_EmitterCmp->m_DelayRemoveTime = 0;
				m_EmitterCmp->SetMaterial(m_Template->m_EmitterMaterial->Clone());

				m_Actor->InsertComponent(m_EmitterCmp);
			}

			const Bone pose = m_EmitterCmp->m_EmitterSpaceType == EmitterComponent::SpaceTypeWorld ?
				m_Actor->GetAttachPose(m_Template->m_SpawnBoneId, m_Template->m_SpawnLocalPose.m_position, m_Template->m_SpawnLocalPose.m_rotation) : m_Template->m_SpawnLocalPose;

			m_EmitterCmp->Spawn(
				Vector4(Vector3(
					Random(-m_Template->m_HalfSpawnArea.x, m_Template->m_HalfSpawnArea.x),
					Random(-m_Template->m_HalfSpawnArea.y, m_Template->m_HalfSpawnArea.y),
					Random(-m_Template->m_HalfSpawnArea.z, m_Template->m_HalfSpawnArea.z)) + pose.m_position, 1.0f),
				Vector4(Vector3::PolarToCartesian(
					m_Template->m_SpawnSpeed,
					Random(m_Template->m_SpawnInclination.x, m_Template->m_SpawnInclination.y),
					Random(m_Template->m_SpawnAzimuth.x, m_Template->m_SpawnAzimuth.y)), 1),
				Vector4(1, 1, 1, 1), Vector2(1, 1), 0, 0);
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
}

void ActionTrackEmitterInst::Stop(void)
{
	_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId || PhysxSdk::getSingleton().m_RenderTickMuted);

	if (m_EmitterCmp)
	{
		_ASSERT(m_EmitterCmp->m_Actor && m_EmitterCmp->m_DelayRemoveTime <= 0);
		m_EmitterCmp->m_DelayRemoveTime = m_Template->m_ParticleLifeTime;
		m_EmitterCmp.reset();
	}
}

ActionTrackInstPtr ActionTrackVelocity::CreateInstance(Actor * _Actor) const
{
	return ActionTrackInstPtr(new ActionTrackVelocityInst(_Actor, boost::static_pointer_cast<const ActionTrackVelocity>(shared_from_this())));
}

void ActionTrackVelocity::AddKeyFrame(float Time, float Length)
{
	KeyFrameMap::iterator key_iter = m_Keys.insert(std::make_pair(Time, KeyFrame()));
	_ASSERT(key_iter != m_Keys.end());
	key_iter->second.Length = Length;
}

ActionTrackVelocityInst::ActionTrackVelocityInst(Actor * _Actor, boost::shared_ptr<const ActionTrackVelocity> Template)
	: ActionTrackInst(_Actor)
	, m_Template(Template)
	, m_Velocity(Template->m_ParamVelocity)
{
}

void ActionTrackVelocityInst::UpdateTime(float LastTime, float Time)
{
}

void ActionTrackVelocityInst::Stop(void)
{
}

bool ActionTrackVelocityInst::GetDisplacement(float LastTime, float dtime, my::Vector3 & disp)
{
	ActionTrackVelocity::KeyFrameMap::const_iterator key_iter = m_Template->m_Keys.upper_bound(LastTime);
	if (key_iter != m_Template->m_Keys.begin() && LastTime < (--key_iter)->first + key_iter->second.Length)
	{
		disp = m_Velocity * dtime;
		return true;
	}
	return false;
}

ActionTrackInstPtr ActionTrackPose::CreateInstance(Actor * _Actor) const
{
	return ActionTrackInstPtr(new ActionTrackPoseInst(_Actor, boost::static_pointer_cast<const ActionTrackPose>(shared_from_this())));
}

void ActionTrackPose::AddKeyFrame(float Time, float Length)
{
	KeyFrameMap::iterator key_iter = m_Keys.insert(std::make_pair(Time, KeyFrame()));
	_ASSERT(key_iter != m_Keys.end());
	key_iter->second.Length = Length;
}

ActionTrackPoseInst::ActionTrackPoseInst(Actor * _Actor, boost::shared_ptr<const ActionTrackPose> Template)
	: ActionTrackInst(_Actor)
	, m_Template(Template)
	, m_Pose(Template->m_ParamPose)
{
}

void ActionTrackPoseInst::UpdateTime(float LastTime, float Time)
{
	ActionTrackPose::KeyFrameMap::const_iterator key_iter = m_Template->m_Keys.lower_bound(LastTime);
	ActionTrackPose::KeyFrameMap::const_iterator key_end = m_Template->m_Keys.lower_bound(Time);
	for (; key_iter != key_end; key_iter++)
	{
		m_KeyInsts.push_back(KeyFrameInst(key_iter->second.Length, Bone(m_Actor->m_Position, m_Actor->m_Rotation)));
	}

	KeyFrameInstList::reverse_iterator key_inst_iter = m_KeyInsts.rbegin();
	if (key_inst_iter != m_KeyInsts.rend())
	{
		key_inst_iter->m_Time += my::D3DContext::getSingleton().m_fElapsedTime;

		const Bone pose = key_inst_iter->m_StartPose.Lerp(m_Pose, m_Template->m_Interpolation.Interpolate(key_inst_iter->m_Time / key_inst_iter->m_Length, 0));

		m_Actor->SetPose(pose);

		// ! Actor::Update, m_Base->GetAttachPose
		if (!m_Actor->m_Base || (m_Actor->m_PxActor && !m_Actor->GetRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC)))
		{
			m_Actor->SetPxPoseOrbyPxThread(pose);
		}

		if (key_inst_iter->m_Time >= key_inst_iter->m_Length)
		{
			m_KeyInsts.erase(m_KeyInsts.begin(), key_inst_iter.base());
		}
	}
}

void ActionTrackPoseInst::Stop(void)
{
}

bool ActionTrackPoseInst::GetDisplacement(float LastTime, float dtime, my::Vector3 & disp)
{
	if (!m_KeyInsts.empty())
	{
		disp = Vector3(0, 0, 0);
		return true;
	}
	return false;
}
