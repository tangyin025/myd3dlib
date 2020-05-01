#include "ActionTrack.h"
#include "Actor.h"
#include "Animation.h"
#include "FModContext.h"
#include <fmod_errors.h>
#include "libc.h"

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
				key_iter->second.Group);
		}
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

void ActionTrackSoundInst::UpdateTime(float Time, float fElapsedTime)
{
	_ASSERT(m_Template);

	_ASSERT(m_Actor);

	ActionTrackSound::KeyFrameMap::const_iterator key_iter = m_Template->m_Keys.lower_bound(Time);
	ActionTrackSound::KeyFrameMap::const_iterator key_end = m_Template->m_Keys.upper_bound(Time + fElapsedTime);
	for (; key_iter != key_end; key_iter++)
	{
		FMOD_RESULT result;
		FMOD::Event       *event;
		ERRCHECK(FModContext::getSingleton().m_EventSystem->getEvent(key_iter->second.Name.c_str(), FMOD_EVENT_INFOONLY, &event));
		ERRCHECK(event->set3DAttributes((FMOD_VECTOR *)&m_Actor->m_Position, NULL, NULL));
		result = FModContext::getSingleton().m_EventSystem->getEvent(key_iter->second.Name.c_str(), FMOD_EVENT_DEFAULT, &event);
		if (FMOD_OK == result)
		{
			ERRCHECK(event->start());
		}
	}
}
