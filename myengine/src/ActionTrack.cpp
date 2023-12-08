#include "ActionTrack.h"
#include "Actor.h"
#include "Animator.h"
#include "libc.h"
#include "myOctree.h"
#include "myResource.h"
#include "Material.h"
#include "PhysxContext.h"
#include "Controller.h"

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

ActionInstPtr Action::CreateInstance(Actor * _Actor, float Rate)
{
	return ActionInstPtr(new ActionInst(_Actor, shared_from_this(), Rate));
}

ActionInst::ActionInst(Actor * _Actor, boost::shared_ptr<const Action> Template, float Rate)
	: m_Template(Template)
	, m_LastTime(0.0f)
	, m_Time(0.0f)
	, m_Rate(Rate)
{
	Action::ActionTrackPtrList::const_iterator track_iter = m_Template->m_TrackList.begin();
	for (; track_iter != m_Template->m_TrackList.end(); track_iter++)
	{
		m_TrackInstList.push_back((*track_iter)->CreateInstance(_Actor, Rate));
	}
}

void ActionInst::Update(float fElapsedTime)
{
	if (m_Time > m_LastTime)
	{
		ActionTrackInstPtrList::iterator track_inst_iter = m_TrackInstList.begin();
		for (; track_inst_iter != m_TrackInstList.end(); track_inst_iter++)
		{
			(*track_inst_iter)->UpdateTime(m_LastTime, m_Time);
		}
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

ActionTrackInstPtr ActionTrackAnimation::CreateInstance(Actor * _Actor, float Rate) const
{
	return ActionTrackInstPtr(new ActionTrackAnimationInst(_Actor, boost::static_pointer_cast<const ActionTrackAnimation>(shared_from_this()), Rate));
}

void ActionTrackAnimation::AddKeyFrame(float Time, const char * SlotName, const char * Name, float Rate, float Weight, float BlendTime, float BlendOutTime, bool Loop, int Prority)
{
	KeyFrameMap::iterator key_iter = m_Keys.insert(std::make_pair(Time, KeyFrame()));
	_ASSERT(key_iter != m_Keys.end());
	key_iter->second.SlotName = SlotName;
	key_iter->second.Name = Name;
	key_iter->second.Rate = Rate;
	key_iter->second.Weight = Weight;
	key_iter->second.BlendTime = BlendTime;
	key_iter->second.BlendOutTime = BlendOutTime;
	key_iter->second.Loop = Loop;
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
					key_iter->second.Rate * m_Rate,
					key_iter->second.Weight * m_Weight,
					key_iter->second.BlendTime,
					key_iter->second.BlendOutTime,
					key_iter->second.Loop,
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

ActionTrackInstPtr ActionTrackSound::CreateInstance(Actor * _Actor, float Rate) const
{
	return ActionTrackInstPtr(new ActionTrackSoundInst(_Actor, boost::static_pointer_cast<const ActionTrackSound>(shared_from_this())));
}

void ActionTrackSound::AddKeyFrame(float Time, const char * SoundPath, bool Loop, float MinDistance, float MaxDistance)
{
	KeyFrameMap::iterator key_iter = m_Keys.insert(std::make_pair(Time, KeyFrame()));
	_ASSERT(key_iter != m_Keys.end());
	key_iter->second.SoundPath = SoundPath;
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
	Stop();
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

	ActionTrackSound::KeyFrameMap::const_iterator key_iter = m_Template->m_Keys.lower_bound(LastTime);
	ActionTrackSound::KeyFrameMap::const_iterator key_end = m_Template->m_Keys.lower_bound(Time);
	for (; key_iter != key_end; key_iter++)
	{
		if (key_iter->second.Sound)
		{
			m_Events.push_back(SoundContext::getSingleton().Play(
				key_iter->second.Sound, key_iter->second.Loop, m_Actor->m_World.getRow<3>().xyz, my::Vector3(0, 0, 0), key_iter->second.MinDistance, key_iter->second.MaxDistance));
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

ActionTrackInstPtr ActionTrackEmitter::CreateInstance(Actor * _Actor, float Rate) const
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
	, m_TaskEvent(NULL, TRUE, TRUE, NULL)
{
	m_WorldEmitterCmp.reset(new CircularEmitter(NamedObject::MakeUniqueName("ActionTrackEmitterInst_cmp").c_str(),
		m_Template->m_EmitterCapacity, (EmitterComponent::FaceType)m_Template->m_EmitterFaceType, (EmitterComponent::SpaceType)m_Template->m_EmitterSpaceType, EmitterComponent::VelocityTypeNone));
	m_WorldEmitterCmp->SetMaterial(m_Template->m_EmitterMaterial->Clone());

	//if (!m_Actor->m_Node)
	//{
	//	THROW_CUSEXCEPTION("ActionTrackEmitterInst: !m_Actor->m_Node"); // ! Actor::PlayAction should be call after being AddEntity
	//}

	//my::OctNode * Root = m_Actor->m_Node->GetTopNode();
	//m_WorldEmitterActor.reset(new Actor(
	//	NamedObject::MakeUniqueName("ActionTrackEmitterInst_actor").c_str(), Vector3(0, 0, 0), Quaternion::Identity(), Vector3(1, 1, 1), *Root));
	//m_WorldEmitterActor->InsertComponent(m_WorldEmitterCmp);
	//Root->AddEntity(m_WorldEmitterActor.get(), m_WorldEmitterActor->m_aabb, Actor::MinBlock, Actor::Threshold);

	m_Actor->InsertComponent(m_WorldEmitterCmp);
}

ActionTrackEmitterInst::~ActionTrackEmitterInst(void)
{
	_ASSERT(m_TaskEvent.Wait(0) && !m_WorldEmitterCmp);
}

void ActionTrackEmitterInst::UpdateTime(float LastTime, float Time)
{
	my::Emitter::ParticleList::iterator first_part_iter = std::upper_bound(
		m_WorldEmitterCmp->m_ParticleList.begin(), m_WorldEmitterCmp->m_ParticleList.end(),	m_Template->m_ParticleLifeTime,
		boost::bind(std::greater<float>(), boost::placeholders::_1, boost::bind(&my::Emitter::Particle::m_Time, boost::placeholders::_2)));
	if (first_part_iter != m_WorldEmitterCmp->m_ParticleList.begin())
	{
		m_WorldEmitterCmp->m_ParticleList.rerase(m_WorldEmitterCmp->m_ParticleList.begin(), first_part_iter);
	}

	ActionTrackEmitter::KeyFrameMap::const_iterator key_iter = m_Template->m_Keys.lower_bound(LastTime);
	ActionTrackEmitter::KeyFrameMap::const_iterator key_end = m_Template->m_Keys.lower_bound(Time);
	for (; key_iter != key_end; key_iter++)
	{
		m_KeyInsts.push_back(KeyFrameInst(
			key_iter->first, key_iter->second.SpawnCount, key_iter->second.SpawnInterval));
	}

	const Bone pose = m_WorldEmitterCmp->m_EmitterSpaceType == EmitterComponent::SpaceTypeWorld ?
		m_Actor->GetAttachPose(m_Template->m_SpawnBoneId, m_Template->m_SpawnLocalPose.m_position, m_Template->m_SpawnLocalPose.m_rotation) : m_Template->m_SpawnLocalPose;

	KeyFrameInstList::iterator key_inst_iter = m_KeyInsts.begin();
	for (; key_inst_iter != m_KeyInsts.end(); )
	{
		for (; key_inst_iter->m_Time < Time && key_inst_iter->m_SpawnCount > 0;
			key_inst_iter->m_Time += key_inst_iter->m_SpawnInterval, key_inst_iter->m_SpawnCount--)
		{
			m_WorldEmitterCmp->Spawn(
				Vector4(Vector3(
					Random(m_Template->m_SpawnArea.m_min.x, m_Template->m_SpawnArea.m_max.x),
					Random(m_Template->m_SpawnArea.m_min.y, m_Template->m_SpawnArea.m_max.y),
					Random(m_Template->m_SpawnArea.m_min.z, m_Template->m_SpawnArea.m_max.z)) + pose.m_position, 1.0f),
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

	m_TaskEvent.ResetEvent();

	m_TaskDuration = Time - LastTime;

	ParallelTaskManager::getSingleton().PushTask(this);
}

void ActionTrackEmitterInst::Stop(void)
{
	m_TaskEvent.Wait(INFINITE);

	//if (m_WorldEmitterActor->m_Node)
	//{
	//	my::OctNode * Root = m_WorldEmitterActor->m_Node->GetTopNode();
	//	Root->RemoveEntity(m_WorldEmitterActor.get());
	//}

	m_Actor->RemoveComponent(m_WorldEmitterCmp->GetSiblingId());

	m_WorldEmitterCmp.reset();
}

void ActionTrackEmitterInst::DoTask(void)
{
	// ! take care of thread safe
	Emitter::ParticleList::iterator particle_iter = m_WorldEmitterCmp->m_ParticleList.begin();
	for (; particle_iter != m_WorldEmitterCmp->m_ParticleList.end(); particle_iter++)
	{
		const float ParticleTime = particle_iter->m_Time + m_TaskDuration;
		particle_iter->m_Velocity.xyz *= powf(m_Template->m_ParticleDamping, m_TaskDuration);
		particle_iter->m_Position.xyz += particle_iter->m_Velocity.xyz * m_TaskDuration;
		particle_iter->m_Color.x = m_Template->m_ParticleColorR.Interpolate(ParticleTime, 1);
		particle_iter->m_Color.y = m_Template->m_ParticleColorG.Interpolate(ParticleTime, 1);
		particle_iter->m_Color.z = m_Template->m_ParticleColorB.Interpolate(ParticleTime, 1);
		particle_iter->m_Color.w = m_Template->m_ParticleColorA.Interpolate(ParticleTime, 1);
		particle_iter->m_Size.x = m_Template->m_ParticleSizeX.Interpolate(ParticleTime, 1);
		particle_iter->m_Size.y = m_Template->m_ParticleSizeY.Interpolate(ParticleTime, 1);
		particle_iter->m_Angle = m_Template->m_ParticleAngle.Interpolate(ParticleTime, 0);
		particle_iter->m_Time = ParticleTime;
	}

	m_TaskEvent.SetEvent();
}

ActionTrackInstPtr ActionTrackVelocity::CreateInstance(Actor * _Actor, float Rate) const
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

ActionTrackInstPtr ActionTrackPose::CreateInstance(Actor * _Actor, float Rate) const
{
	return ActionTrackInstPtr(new ActionTrackPoseInst(_Actor, boost::static_pointer_cast<const ActionTrackPose>(shared_from_this())));
}

void ActionTrackPose::AddKeyFrame(float Time, float Length)
{
	KeyFrameMap::iterator key_iter = m_Keys.insert(std::make_pair(Time, KeyFrame()));
	_ASSERT(key_iter != m_Keys.end());
	key_iter->second.Length = Length;
}

ActionTrackPoseInst::KeyFrameInst::KeyFrameInst(float Time, float Length, Actor * actor)
	: m_Time(Time)
	, m_Length(Length)
	, m_StartPose(actor->m_Position, actor->m_Rotation)
{
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
		m_KeyInsts.push_back(KeyFrameInst(key_iter->first, key_iter->second.Length, m_Actor));
	}

	KeyFrameInstList::reverse_iterator key_inst_iter = m_KeyInsts.rbegin();
	for (; key_inst_iter != m_KeyInsts.rend(); )
	{
		_ASSERT(Time >= key_inst_iter->m_Time);

		if (LastTime < key_inst_iter->m_Time + key_inst_iter->m_Length)
		{
			const my::Bone pose = key_inst_iter->m_StartPose.Lerp(m_Pose,
				m_Template->m_Interpolation.Interpolate(Min(1.0f, (Time - key_inst_iter->m_Time) / key_inst_iter->m_Length), 0));

			m_Actor->SetPose(pose);

			if (!m_Actor->m_Base || (m_Actor->m_PxActor && !m_Actor->GetRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC))) // ! Actor::Update, m_Base->GetAttachPose
			{
				m_Actor->SetPxPoseOrbyPxThread(pose);
			}

			break;
		}
		else
		{
			key_inst_iter = KeyFrameInstList::reverse_iterator(m_KeyInsts.erase(m_KeyInsts.begin(), key_inst_iter.base()));
		}
	}
}

void ActionTrackPoseInst::Stop(void)
{
}

bool ActionTrackPoseInst::GetDisplacement(float LastTime, float dtime, my::Vector3 & disp)
{
	KeyFrameInstList::reverse_iterator key_inst_iter = m_KeyInsts.rbegin();
	for (; key_inst_iter != m_KeyInsts.rend(); key_inst_iter++)
	{
		if (LastTime >= key_inst_iter->m_Time && LastTime < key_inst_iter->m_Time + key_inst_iter->m_Length)
		{
			disp = Vector3(0, 0, 0);
			return true;
		}
	}
	return false;
}
