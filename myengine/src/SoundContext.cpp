#include "SoundContext.h"
#include "myResource.h"
#include "myDxutApp.h"
#define MINIMP3_IMPLEMENTATION 
#include "minimp3.h"

using namespace my;

bool SoundContext::Init(HWND hwnd)
{
	m_sound.CreateSound();

	m_sound.SetCooperativeLevel(hwnd, DSSCL_PRIORITY);

	DSBUFFERDESC dsbd;
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;

	SoundBufferPtr dsbPrimary = m_sound.CreateSoundBuffer(&dsbd);
	m_listener = dsbPrimary->Get3DListener();

	return true;
}

void SoundContext::Shutdown(void)
{
	m_listener.reset();

	BufferEventPairList::iterator buff_event_iter = m_pool.begin();
	for (; buff_event_iter != m_pool.end(); buff_event_iter++)
	{
		buff_event_iter->second->m_sbuffer = NULL;
		buff_event_iter->second->m_3dbuffer.reset();
	}

	m_pool.clear();
}

void SoundContext::ReleaseIdleBuffer(float fElapsedTime)
{
	BufferEventPairList::iterator buff_event_iter = m_pool.begin();
	for (; buff_event_iter != m_pool.end(); )
	{
		DWORD status = buff_event_iter->first.GetStatus();
		if (status & DSBSTATUS_PLAYING)
		{
			buff_event_iter++;
		}
		else
		{
			buff_event_iter->second->m_sbuffer = NULL;
			buff_event_iter->second->m_3dbuffer.reset();
			buff_event_iter = m_pool.erase(buff_event_iter);
		}
	}
}

SoundContext::BufferEventPairList::iterator SoundContext::GetIdleBuffer(my::WavPtr wav, DWORD flags)
{
	_ASSERT(!(flags & DSBCAPS_CTRL3D) || wav->wavfmt->nChannels == 1);

	DSBUFFERDESC dsbd;
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = flags;
	dsbd.dwBufferBytes = wav->child.cksize;
	dsbd.dwReserved = 0;
	dsbd.lpwfxFormat = wav->wavfmt.get();
	dsbd.guid3DAlgorithm = DS3DALG_DEFAULT;

	LPDIRECTSOUNDBUFFER pDsb = NULL;
	my::CriticalSectionLock lock(m_soundsec);
	HRESULT hr = m_sound.m_ptr->CreateSoundBuffer(&dsbd, &pDsb, NULL);
	if (FAILED(hr))
	{
		THROW_DSOUNDEXCEPTION(hr);
	}
	lock.Unlock();

	BufferEventPairList::iterator buff_event_iter = m_pool.insert(m_pool.begin(), BufferEventPair());
	buff_event_iter->first.Create(pDsb);

	unsigned char* buffer1, * buffer2;
	DWORD bytes1, bytes2;
	buff_event_iter->first.Lock(0, wav->child.cksize, (LPVOID*)&buffer1, &bytes1, (LPVOID*)&buffer2, &bytes2, DSBLOCK_ENTIREBUFFER);
	_ASSERT(bytes1 + bytes2 == wav->child.cksize);
	if (buffer1)
	{
		memcpy(buffer1, &wav->buffer[0], bytes1);
	}
	if (buffer2)
	{
		memcpy(buffer2, &wav->buffer[bytes1], bytes2);
	}
	buff_event_iter->first.Unlock(buffer1, bytes1, buffer2, bytes2);

	return buff_event_iter;
}

SoundEventPtr SoundContext::Play(my::WavPtr wav, bool Loop)
{
	BufferEventPairList::iterator buff_event_iter = GetIdleBuffer(wav, DSBCAPS_CTRLVOLUME | DSBCAPS_LOCDEFER);

	if (buff_event_iter != m_pool.end())
	{
		buff_event_iter->second.reset(new SoundEvent());
		buff_event_iter->second->m_sbuffer = &buff_event_iter->first;
		buff_event_iter->first.Play(0, DSBPLAY_TERMINATEBY_TIME | (Loop ? DSBPLAY_LOOPING : 0));
		return buff_event_iter->second;
	}

	return SoundEventPtr();
}

SoundEventPtr SoundContext::Play(my::WavPtr wav, bool Loop, const my::Vector3 & pos, const my::Vector3 & vel, float min_dist, float max_dist)
{
	BufferEventPairList::iterator buff_event_iter = GetIdleBuffer(wav, DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_LOCDEFER | DSBCAPS_MUTE3DATMAXDISTANCE);

	if (buff_event_iter != m_pool.end())
	{
		buff_event_iter->second.reset(new SoundEvent());
		buff_event_iter->second->m_sbuffer = &buff_event_iter->first;
		buff_event_iter->second->m_3dbuffer = buff_event_iter->first.Get3DBuffer();
		DS3DBUFFER ds3dbuff = buff_event_iter->second->m_3dbuffer->GetAllParameters();
		ds3dbuff.vPosition = (D3DVECTOR&)pos;
		ds3dbuff.vVelocity = (D3DVECTOR&)vel;
		ds3dbuff.flMinDistance = min_dist;
		ds3dbuff.flMaxDistance = max_dist;
		ds3dbuff.dwMode = DS3DMODE_NORMAL;
		buff_event_iter->second->m_3dbuffer->SetAllParameters(&ds3dbuff, DS3D_IMMEDIATE);
		buff_event_iter->first.Play(0, DSBPLAY_TERMINATEBY_TIME | (Loop ? DSBPLAY_LOOPING : 0));
		return buff_event_iter->second;
	}

	return SoundEventPtr();
}

bool Mp3::PlayOnceByThread(void)
{
	CachePtr cache = my::ResourceMgr::getSingleton().OpenIStream(m_Mp3Path.c_str())->GetWholeCache();

	// initialize dsound position notifies
	for (size_t i = 0; i < _countof(m_dsnp); i++)
	{
		BOOST_VERIFY(::ResetEvent(m_dsnp[i].hEventNotify));
	}

	// set the default block which to begin playing
	BOOST_VERIFY(::SetEvent(m_dsnp[0].hEventNotify));

	bool ret = false;
	mp3dec_t mp3d;
	mp3dec_init(&mp3d);
	mp3dec_frame_info_t info;
	short pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
	std::vector<unsigned char> sbuffer;
	for (int inLen = 0; true; inLen += info.frame_bytes)
	{
		// decode audio frame
		int samples = mp3dec_decode_frame(&mp3d, cache->data() + inLen, cache->size() - inLen, pcm, &info);
		if (samples <= 0)
		{
			if (NULL != m_dsbuffer)
			{
				// wait for dsound buffer block playing
				_ASSERT(sizeof(m_events) == sizeof(HANDLE) * _countof(m_events));
				if (WAIT_OBJECT_0 != ::WaitForMultipleObjects(_countof(m_events), reinterpret_cast<HANDLE*>(m_events), FALSE, INFINITE))
				{
					// normal play end, need to continue if looped
					ret = true;
				}
				m_dsbuffer->Stop();
			}
			break;
		}

		// parse dither linear pcm data to compatible format
		if (2 == info.channels)
		{
			for (int i = 0; i < samples; i++)
			{
				sbuffer.push_back(pcm[i * 2 + 0] >> 0);
				sbuffer.push_back(pcm[i * 2 + 0] >> 8);
				sbuffer.push_back(pcm[i * 2 + 1] >> 0);
				sbuffer.push_back(pcm[i * 2 + 1] >> 8);
			}
		}
		else
		{
			for (int i = 0; i < samples; i++)
			{
				sbuffer.push_back(pcm[i] >> 0);
				sbuffer.push_back(pcm[i] >> 8);
			}
		}

		// create platform sound devices if needed
		if (m_wavfmt.nChannels != info.channels || m_wavfmt.nSamplesPerSec != info.hz)
		{
			// dsound buffer should only be create once
			_ASSERT(NULL == m_dsnotify);
			_ASSERT(NULL == m_dsbuffer);

			_ASSERT(WAVE_FORMAT_PCM == m_wavfmt.wFormatTag);
			m_wavfmt.nChannels = info.channels;
			m_wavfmt.nSamplesPerSec = info.hz;
			m_wavfmt.wBitsPerSample = 16;
			m_wavfmt.nBlockAlign = m_wavfmt.nChannels * m_wavfmt.wBitsPerSample / 8;
			m_wavfmt.nAvgBytesPerSec = m_wavfmt.nSamplesPerSec * m_wavfmt.nBlockAlign;
			m_wavfmt.cbSize = 0;

			DSBUFFERDESC dsbd;
			dsbd.dwSize = sizeof(dsbd);
			dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_LOCSOFTWARE;
			dsbd.dwBufferBytes = m_wavfmt.nAvgBytesPerSec * BLOCK_COUNT;
			dsbd.dwReserved = 0;
			dsbd.lpwfxFormat = &m_wavfmt;
			dsbd.guid3DAlgorithm = DS3DALG_DEFAULT;

			// recalculate notify position for each block
			for (int i = 0; i < _countof(m_dsnp); i++)
			{
				m_dsnp[i].dwOffset = i * m_wavfmt.nAvgBytesPerSec;
			}

			// create dsound buffer & notify
			my::CriticalSectionLock lock(SoundContext::getSingleton().m_soundsec);
			m_dsbuffer = SoundContext::getSingleton().m_sound.CreateSoundBuffer(&dsbd);
			lock.Unlock();
			m_dsnotify = m_dsbuffer->GetNotify();
			m_dsnotify->setNotificationPositions(_countof(m_dsnp), m_dsnp);
		}

		// fill pcm data to dsbuffer
		_ASSERT(NULL != m_dsbuffer);
		if (sbuffer.size() > m_wavfmt.nAvgBytesPerSec)
		{
			// wait for notify event even with stop event
			_ASSERT(sizeof(m_events) == sizeof(HANDLE) * _countof(m_events));
			DWORD wait_res = ::WaitForMultipleObjects(_countof(m_events), reinterpret_cast<HANDLE*>(m_events), FALSE, INFINITE);
			_ASSERT(WAIT_TIMEOUT != wait_res);
			if (wait_res == WAIT_OBJECT_0)
			{
				// out if stop event occured
				m_dsbuffer->Stop();
				break;
			}

			// calculate current block
			DWORD curr_block = wait_res - WAIT_OBJECT_0 - 1;
			_ASSERT(curr_block < _countof(m_dsnp));

			// calculate next block which need to be update
			DWORD next_block = (curr_block + 1) % _countof(m_dsnp);

			// decoded sound buffer copying
			unsigned char* audioPtr1, * audioPtr2;
			DWORD audioBytes1, audioBytes2;
			m_dsbuffer->Lock(m_dsnp[next_block].dwOffset, m_wavfmt.nAvgBytesPerSec, (LPVOID*)&audioPtr1, &audioBytes1, (LPVOID*)&audioPtr2, &audioBytes2, 0);
			_ASSERT(audioBytes1 + audioBytes2 <= m_wavfmt.nAvgBytesPerSec);
			if (audioPtr1 != NULL)
			{
				memcpy(audioPtr1, &sbuffer[0], audioBytes1);
			}
			if (audioPtr2 != NULL)
			{
				memcpy(audioPtr2, &sbuffer[0 + audioBytes1], audioBytes2);
			}
			m_dsbuffer->Unlock(audioPtr1, audioBytes1, audioPtr2, audioBytes2);

			// begin play
			if (!(m_dsbuffer->GetStatus() & DSBSTATUS_PLAYING))
			{
				// reset play position
				m_dsbuffer->SetCurrentPosition(m_dsnp[next_block].dwOffset);
				m_dsbuffer->Play(0, DSBPLAY_LOOPING);
			}

			// move remain buffer which havent been pushed into dsound buffer to header
			size_t remain = sbuffer.size() - m_wavfmt.nAvgBytesPerSec;
			memmove(&sbuffer[0], &sbuffer[m_wavfmt.nAvgBytesPerSec], remain);
			sbuffer.resize(remain);
		}
	}
	return ret;
}

Mp3::Mp3(void)
	: Thread(boost::bind(&Mp3::OnThreadProc, this))
{
	m_wavfmt.wFormatTag = WAVE_FORMAT_PCM;
	m_wavfmt.nChannels = 0;
	m_wavfmt.nSamplesPerSec = 0;

	// NOTE: the first event of m_events array is stop event, position event is following the next
	for (int i = 0; i < _countof(m_dsnp); i++)
	{
		m_dsnp[i].dwOffset = 0;
		m_dsnp[i].hEventNotify = m_events[i + 1].m_handle;
	}
}

Mp3::~Mp3(void)
{
	if (NULL != m_handle)
	{
		Stop();
	}
}

void Mp3::Play(const char* path, bool Loop)
{
	if (NULL != m_handle)
	{
		Stop();
	}

	m_Mp3Path = path;

	m_Loop = Loop;

	m_events[0].ResetEvent();

	CreateThread();

	ResumeThread();
}

void Mp3::StopAsync(void)
{
	m_events[0].SetEvent();
}

void Mp3::Stop(void)
{
	StopAsync();

	WaitForThreadStopped(INFINITE);

	CloseThread();
}

DWORD Mp3::OnThreadProc(void)
{
	try
	{
		while (PlayOnceByThread() && m_Loop)
		{
		}
	}
	catch (const Exception & e)
	{
		D3DContext::getSingleton().m_EventLog(e.what().c_str());
	}

	return 0;
}
