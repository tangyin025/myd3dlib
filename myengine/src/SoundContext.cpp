#include "SoundContext.h"
#include "myResource.h"
#ifdef _WIN64
#define FPM_64BIT
#endif
#include <mad.h>
#include <id3tag.h>

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
		if (!(status & DSBSTATUS_PLAYING) || (status & DSBSTATUS_TERMINATED))
		{
			buff_event_iter->second->m_sbuffer = NULL;
			buff_event_iter->second->m_3dbuffer.reset();
			buff_event_iter = m_pool.erase(buff_event_iter);
		}
		else
		{
			buff_event_iter++;
		}
	}
}

SoundContext::BufferEventPairList::iterator SoundContext::GetIdleBuffer(my::WavPtr wav, DWORD flags)
{
	m_pool.insert(m_pool.begin(), BufferEventPair());

	BufferEventPairList::iterator buff_event_iter = m_pool.begin();

	DSBUFFERDESC dsbd;
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = flags;
	dsbd.dwBufferBytes = wav->child.cksize;
	dsbd.dwReserved = 0;
	dsbd.lpwfxFormat = &wav->wavfmt;
	dsbd.guid3DAlgorithm = DS3DALG_DEFAULT;

	LPDIRECTSOUNDBUFFER pDsb = NULL;
	my::CriticalSectionLock lock(m_soundsec);
	HRESULT hr = m_sound.m_ptr->CreateSoundBuffer(&dsbd, &pDsb, NULL);
	if (FAILED(hr))
	{
		THROW_DSOUNDEXCEPTION(hr);
	}
	lock.Unlock();
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

SoundEventPtr SoundContext::Play(my::WavPtr wav)
{
	BufferEventPairList::iterator buff_event_iter = GetIdleBuffer(wav, DSBCAPS_CTRLVOLUME | DSBCAPS_LOCDEFER);

	if (buff_event_iter != m_pool.end())
	{
		buff_event_iter->second.reset(new SoundEvent());
		buff_event_iter->second->m_sbuffer = &buff_event_iter->first;
		buff_event_iter->first.Play(0, DSBPLAY_TERMINATEBY_TIME);
		return buff_event_iter->second;
	}

	return SoundEventPtr();
}

SoundEventPtr SoundContext::Play(my::WavPtr wav, const my::Vector3 & pos, const my::Vector3 & vel, float min_dist, float max_dist)
{
	BufferEventPairList::iterator buff_event_iter = GetIdleBuffer(wav, DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_LOCDEFER);

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
		buff_event_iter->first.Play(0, DSBPLAY_TERMINATEBY_TIME);
		return buff_event_iter->second;
	}

	return SoundEventPtr();
}

struct audio_dither
{
public:
	mad_fixed_t error[3];
	mad_fixed_t random;

public:
	audio_dither(void)
	{
		memset(this, 0, sizeof(*this));
	}
};

struct audio_stats
{
public:
	unsigned long clipped_samples;
	mad_fixed_t peak_clipping;
	mad_fixed_t peak_sample;

public:
	audio_stats(void)
	{
		memset(this, 0, sizeof(*this));
	}
};

static unsigned long prng(unsigned long state)
{
	return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}

static signed long audio_linear_dither(
	unsigned int bits,
	signed int sample,
	struct audio_dither* dither,
	struct audio_stats* stats)
{
	unsigned int scalebits;
	mad_fixed_t output, mask, random;

	enum {
		MIN = -MAD_F_ONE,
		MAX = MAD_F_ONE - 1
	};

	/* noise shape */
	sample += dither->error[0] - dither->error[1] + dither->error[2];

	dither->error[2] = dither->error[1];
	dither->error[1] = dither->error[0] / 2;

	/* bias */
	output = sample + (1L << (MAD_F_FRACBITS + 1 - bits - 1));

	scalebits = MAD_F_FRACBITS + 1 - bits;
	mask = (1L << scalebits) - 1;

	/* dither */
	random = prng(dither->random);
	output += (random & mask) - (dither->random & mask);

	dither->random = random;

	/* clip */
	if (output >= stats->peak_sample) {
		if (output > MAX) {
			++stats->clipped_samples;
			if (output - MAX > stats->peak_clipping)
				stats->peak_clipping = output - MAX;

			output = MAX;

			if (sample > MAX)
				sample = MAX;
		}
		stats->peak_sample = output;
	}
	else if (output < -stats->peak_sample) {
		if (output < MIN) {
			++stats->clipped_samples;
			if (MIN - output > stats->peak_clipping)
				stats->peak_clipping = MIN - output;

			output = MIN;

			if (sample < MIN)
				sample = MIN;
		}
		stats->peak_sample = -output;
	}

	/* quantize */
	output &= ~mask;

	/* error feedback */
	dither->error[0] = sample - output;

	/* scale */
	return output >> scalebits;
}

Mp3::Mp3(void)
	: Thread(boost::bind(&Mp3::OnProc, this))
	, m_buffer(MPEG_BUFSZ / sizeof(m_buffer[0]))
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

bool Mp3::PlayOnce(void)
{
	DSBUFFERDESC dsbd;
	audio_dither left_dither, right_dither;
	std::vector<unsigned char> sbuffer;

	m_stream->seek(0, SEEK_SET);

	mad_stream stream;
	mad_frame frame;
	mad_synth synth;

	mad_stream_init(&stream);
	mad_frame_init(&frame);
	mad_synth_init(&synth);

	// initialize dsound position notifies
	for (size_t i = 0; i < _countof(m_dsnp); i++)
	{
		BOOST_VERIFY(::ResetEvent(m_dsnp[i].hEventNotify));
	}

	// set the default block which to begin playing
	BOOST_VERIFY(::SetEvent(m_dsnp[0].hEventNotify));

	bool ret = false;
	do
	{
		// read original sound source from stream
		size_t remain = 0;
		if (NULL != stream.next_frame)
		{
			remain = &m_buffer[0] + MPEG_BUFSZ - stream.next_frame;
			memmove(&m_buffer[0], stream.next_frame, remain);
		}
		int read = m_stream->read(&m_buffer[0] + remain, (m_buffer.size() - remain) * sizeof(m_buffer[0]));

		// EOF of stream
		if (0 == read)
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
			goto play_once_end;
		}

		// fill remain buffer over MAD_BUFFER_GUARD to zero
		if (read < MAD_BUFFER_GUARD)
		{
			_ASSERT(MPEG_BUFSZ - remain > MAD_BUFFER_GUARD);
			memset(&m_buffer[remain + read], 0, MAD_BUFFER_GUARD - read);
			read = MAD_BUFFER_GUARD;
		}

		// attach buffer to mad stream
		mad_stream_buffer(&stream, &m_buffer[0], (remain + read) * sizeof(m_buffer[0]));

		while (true)
		{
			// decode audio frame
			if (-1 == mad_frame_decode(&frame, &stream))
			{
				if (!MAD_RECOVERABLE(stream.error))
				{
					break;
				}

				switch (stream.error)
				{
				case MAD_ERROR_BADDATAPTR:
					continue;

				case MAD_ERROR_LOSTSYNC:
				{
					// excute id3 tag frame skipping
					unsigned long tagsize = id3_tag_query(stream.this_frame, stream.bufend - stream.this_frame);
					if (tagsize > 0)
					{
						mad_stream_skip(&stream, tagsize);
					}
				}
				continue;

				default:
					continue;
				}
			}

			// convert frame data to pcm data
			mad_synth_frame(&synth, &frame);

			// parse dither linear pcm data to compatible format
			audio_stats stats;
			if (2 == synth.pcm.channels)
			{
				register signed int sample0, sample1;
				for (int i = 0; i < (int)synth.pcm.length; i++)
				{
					sample0 = audio_linear_dither(16, synth.pcm.samples[0][i], &left_dither, &stats);
					sample1 = audio_linear_dither(16, synth.pcm.samples[1][i], &right_dither, &stats);
					sbuffer.push_back(sample0 >> 0);
					sbuffer.push_back(sample0 >> 8);
					sbuffer.push_back(sample1 >> 0);
					sbuffer.push_back(sample1 >> 8);
				}
			}
			else
			{
				register int sample0;
				for (int i = 0; i < (int)synth.pcm.length; i++)
				{
					sample0 = audio_linear_dither(16, synth.pcm.samples[0][i], &left_dither, &stats);
					sbuffer.push_back(sample0 >> 0);
					sbuffer.push_back(sample0 >> 8);
				}
			}

			// create platform sound devices if needed
			if (m_wavfmt.nChannels != synth.pcm.channels || m_wavfmt.nSamplesPerSec != synth.pcm.samplerate)
			{
				// dsound buffer should only be create once
				_ASSERT(NULL == m_dsnotify);
				_ASSERT(NULL == m_dsbuffer);

				_ASSERT(WAVE_FORMAT_PCM == m_wavfmt.wFormatTag);
				m_wavfmt.nChannels = synth.pcm.channels;
				m_wavfmt.nSamplesPerSec = synth.pcm.samplerate;
				m_wavfmt.wBitsPerSample = 16;
				m_wavfmt.nBlockAlign = m_wavfmt.nChannels * m_wavfmt.wBitsPerSample / 8;
				m_wavfmt.nAvgBytesPerSec = m_wavfmt.nSamplesPerSec * m_wavfmt.nBlockAlign;
				m_wavfmt.cbSize = 0;

				dsbd.dwSize = sizeof(dsbd);
				dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_STATIC | DSBCAPS_LOCSOFTWARE;
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
					goto play_once_end;
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
	} while (stream.error == MAD_ERROR_BUFLEN);

	_ASSERT(false);

play_once_end:
	mad_synth_finish(&synth);
	mad_frame_finish(&frame);
	mad_stream_finish(&stream);
	return ret;
}

void Mp3::Play(my::IStreamPtr istr, bool Loop /*= false*/)
{
	if (NULL != m_handle)
	{
		Stop();
	}

	m_stream = istr;
	SetLoop(Loop);
	m_events[0].ResetEvent();
	CreateThread();
	ResumeThread();
}

void Mp3::Play(const char * path, bool Loop /*= false*/)
{
	Play(my::ResourceMgr::getSingleton().OpenIStream(path), Loop);
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

DWORD Mp3::OnProc(void)
{
	try
	{
		do
		{
			;
		} while (PlayOnce() && GetLoop());
	}
	catch (Exception& /*e*/)
	{
		//::my::Game::getSingleton().m_pwnd->sendMessage(WM_USER + 0, (WPARAM)&e);
	}

	return 0;
}
