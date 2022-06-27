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

bool ActionInst::GetDisplacement(float LastTime, float dtime, my::Vector3 & disp)
{
	bool ret = false;
	ActionTrackInstPtrList::iterator track_inst_iter = m_TrackInstList.begin();
	for (; track_inst_iter != m_TrackInstList.end(); track_inst_iter++)
	{
		my::Vector3 local_disp;
		if ((*track_inst_iter)->GetDisplacement(LastTime, dtime, local_disp))
		{
			if (ret)
			{
				disp += local_disp;
			}
			else
			{
				disp = local_disp;
				ret = true;
			}
		}
	}
	return ret;
}

ActionTrackInstPtr ActionTrackAnimation::CreateInstance(Actor * _Actor) const
{
	return ActionTrackInstPtr(new ActionTrackAnimationInst(_Actor, boost::static_pointer_cast<const ActionTrackAnimation>(shared_from_this())));
}

void ActionTrackAnimation::AddKeyFrame(float Time, const char * Name, float Rate, float Weight, float BlendTime, float BlendOutTime, bool Loop, int Prority, const char * Group, int RootId)
{
	KeyFrameMap::iterator key_iter = m_Keys.insert(std::make_pair(Time, KeyFrame()));
	_ASSERT(key_iter != m_Keys.end());
	key_iter->second.Name = Name;
	key_iter->second.Rate = Rate;
	key_iter->second.Weight = Weight;
	key_iter->second.BlendTime = BlendTime;
	key_iter->second.BlendOutTime = BlendOutTime;
	key_iter->second.Loop = Loop;
	key_iter->second.Prority = Prority;
	key_iter->second.Group = Group;
	key_iter->second.RootId = RootId;
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
			animator->Play(
				key_iter->second.Name,
				key_iter->second.Rate,
				key_iter->second.Weight,
				key_iter->second.BlendTime,
				key_iter->second.BlendOutTime,
				key_iter->second.Loop,
				key_iter->second.Prority,
				key_iter->second.Group,
				key_iter->second.RootId,
				(DWORD_PTR)this);
		}
	}
}

void ActionTrackAnimationInst::Stop(void)
{
	Animator* animator = m_Actor->GetFirstComponent<Animator>();
	if (animator)
	{
		animator->Stop((DWORD_PTR)this);
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
	, m_SpawnPose(m_Template->m_EmitterCapacity)
	, m_TaskEvent(NULL, TRUE, TRUE, NULL)
{
	m_WorldEmitterCmp.reset(new CircularEmitter(NamedObject::MakeUniqueName("ActionTrackEmitterInst_cmp").c_str(),
		m_Template->m_EmitterCapacity, (EmitterComponent::FaceType)m_Template->m_EmitterFaceType, EmitterComponent::SpaceTypeWorld,
			m_Template->m_EmitterFaceType == EmitterComponent::FaceTypeCamera || m_Template->m_EmitterFaceType == EmitterComponent::FaceTypeAngleCamera ? EmitterComponent::VelocityTypeNone : EmitterComponent::VelocityTypeQuat, EmitterComponent::PrimitiveTypeQuad));
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
	_ASSERT(m_TaskEvent.Wait(0));

	//if (m_WorldEmitterActor->m_Node)
	//{
	//	my::OctNode * Root = m_WorldEmitterActor->m_Node->GetTopNode();
	//	Root->RemoveEntity(m_WorldEmitterActor.get());
	//}

	m_Actor->RemoveComponent(m_WorldEmitterCmp->GetSiblingId());
}

void ActionTrackEmitterInst::UpdateTime(float LastTime, float Time)
{
	m_WorldEmitterCmp->RemoveParticleBefore(Time - m_Template->m_ParticleLifeTime);

	m_SpawnPose.rresize(m_WorldEmitterCmp->m_ParticleList.size());

	ActionTrackEmitter::KeyFrameMap::const_iterator key_iter = m_Template->m_Keys.lower_bound(LastTime);
	ActionTrackEmitter::KeyFrameMap::const_iterator key_end = m_Template->m_Keys.lower_bound(Time);
	for (; key_iter != key_end; key_iter++)
	{
		m_KeyInsts.push_back(KeyFrameInst(
			key_iter->first, key_iter->second.SpawnCount, key_iter->second.SpawnInterval));
	}

	Animator* animator = m_Actor->GetFirstComponent<Animator>();
	KeyFrameInstList::iterator key_inst_iter = m_KeyInsts.begin();
	for (; key_inst_iter != m_KeyInsts.end(); )
	{
		for (; key_inst_iter->m_Time < Time && key_inst_iter->m_SpawnCount > 0;
			key_inst_iter->m_Time += key_inst_iter->m_SpawnInterval, key_inst_iter->m_SpawnCount--)
		{
			m_SpawnPose.push_back(m_Actor->GetAttachPose(
				m_Template->m_AttachBoneId, Vector3(0, 0, 0), Quaternion::Identity()));

			m_WorldEmitterCmp->Spawn(
				Vector4(0, 0, 0, 1), Vector4(0, 0, 0, 1), Vector4(1, 1, 1, 1), Vector2(1, 1), 0, key_inst_iter->m_Time);
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

	m_TaskTime = Time;

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
	_ASSERT(m_WorldEmitterCmp->m_ParticleList.size() == m_SpawnPose.size());
	for (; particle_iter != m_WorldEmitterCmp->m_ParticleList.end(); particle_iter++)
	{
		const float ParticleTime = m_TaskTime - particle_iter->m_Time;
		const my::Bone & SpawnPose = m_SpawnPose[std::distance(m_WorldEmitterCmp->m_ParticleList.begin(), particle_iter)];
		particle_iter->m_Position.xyz = SpawnPose.m_position + SpawnPose.m_rotation * my::Vector3(
			m_Template->m_ParticlePositionX.Interpolate(ParticleTime, 0),
			m_Template->m_ParticlePositionY.Interpolate(ParticleTime, 0),
			m_Template->m_ParticlePositionZ.Interpolate(ParticleTime, 0));
		particle_iter->m_Velocity = (Vector4&)(Quaternion::RotationEulerAngles(
			m_Template->m_ParticleEulerX.Interpolate(ParticleTime, 0),
			m_Template->m_ParticleEulerY.Interpolate(ParticleTime, 0),
			m_Template->m_ParticleEulerZ.Interpolate(ParticleTime, 0)) * SpawnPose.m_rotation);
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

		if (Time < key_inst_iter->m_Time + key_inst_iter->m_Length)
		{
			const my::Bone pose = key_inst_iter->m_StartPose.Lerp(m_Pose, m_Template->m_Interpolation.Interpolate((Time - key_inst_iter->m_Time) / key_inst_iter->m_Length, 0));

			m_Actor->SetPose(pose);

			if (!m_Actor->m_Base) // ! Actor::Update, m_Base->GetAttachPose
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

ActionTrackInstPtr ActionTrackEvent::CreateInstance(Actor* _Actor) const
{
	return ActionTrackInstPtr(new ActionTrackEventInst(_Actor, boost::static_pointer_cast<const ActionTrackEvent>(shared_from_this())));
}

void ActionTrackEvent::AddKeyFrame(float Time)
{
	KeyFrameMap::iterator key_iter = m_Keys.insert(Time);
	_ASSERT(key_iter != m_Keys.end());
}

void ActionTrackEventInst::UpdateTime(float LastTime, float Time)
{
	if (m_Event)
	{
		ActionTrackEvent::KeyFrameMap::const_iterator key_iter = m_Template->m_Keys.lower_bound(LastTime);
		ActionTrackEvent::KeyFrameMap::const_iterator key_end = m_Template->m_Keys.lower_bound(Time);
		for (; key_iter != key_end; key_iter++)
		{
			ActionTrackEventArg arg(*key_iter);
			m_Event(&arg);
		}
	}
}

void ActionTrackEventInst::Stop(void)
{
}
