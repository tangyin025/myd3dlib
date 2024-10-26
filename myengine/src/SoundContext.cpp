// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "SoundContext.h"
#include "myResource.h"
#include "myDxutApp.h"
//#define MINIMP3_IMPLEMENTATION 
//#include "minimp3.h"
#include <vorbis/codec.h>
#include <libc.h>

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

SoundContext::BufferEventPairList::iterator SoundContext::GetIdleBuffer(my::WavPtr wav, size_t startbyte, DWORD bytes, DWORD flags)
{
	_ASSERT(startbyte % wav->wavfmt->nBlockAlign == 0 && startbyte < wav->child.cksize);

	_ASSERT(bytes % wav->wavfmt->nBlockAlign == 0 && startbyte + bytes <= wav->child.cksize);

	_ASSERT(!(flags & DSBCAPS_CTRL3D) || wav->wavfmt->nChannels == 1);

	DSBUFFERDESC dsbd;
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = flags;
	dsbd.dwBufferBytes = bytes;
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
	buff_event_iter->first.Lock(0, bytes, (LPVOID*)&buffer1, &bytes1, (LPVOID*)&buffer2, &bytes2, DSBLOCK_ENTIREBUFFER);
	_ASSERT(bytes1 + bytes2 == bytes);
	if (buffer1)
	{
		memcpy(buffer1, &wav->buffer[startbyte], bytes1);
	}
	if (buffer2)
	{
		memcpy(buffer2, &wav->buffer[startbyte + bytes1], bytes2);
	}
	buff_event_iter->first.Unlock(buffer1, bytes1, buffer2, bytes2);

	return buff_event_iter;
}

SoundEventPtr SoundContext::Play(my::WavPtr wav, float StartSec, float EndSec, bool Loop)
{
	const size_t startbyte = Wav::SecToBlockByte(*wav->wavfmt, StartSec);

	const size_t endbyte = Min((size_t)wav->child.cksize, Wav::SecToBlockByte(*wav->wavfmt, EndSec));

	BufferEventPairList::iterator buff_event_iter = GetIdleBuffer(wav, startbyte, endbyte - startbyte, DSBCAPS_CTRLVOLUME | DSBCAPS_LOCDEFER);

	if (buff_event_iter != m_pool.end())
	{
		buff_event_iter->second.reset(new SoundEvent());
		buff_event_iter->second->m_sbuffer = &buff_event_iter->first;
		buff_event_iter->first.Play(0, DSBPLAY_TERMINATEBY_TIME | (Loop ? DSBPLAY_LOOPING : 0));
		return buff_event_iter->second;
	}

	return SoundEventPtr();
}

SoundEventPtr SoundContext::Play(my::WavPtr wav, float StartSec, float EndSec, bool Loop, const my::Vector3 & pos, const my::Vector3 & vel, float min_dist, float max_dist)
{
	const size_t startbyte = Wav::SecToBlockByte(*wav->wavfmt, StartSec);

	const size_t endbyte = Min((size_t)wav->child.cksize, Wav::SecToBlockByte(*wav->wavfmt, EndSec));

	BufferEventPairList::iterator buff_event_iter = GetIdleBuffer(wav, startbyte, endbyte - startbyte, DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_LOCDEFER | DSBCAPS_MUTE3DATMAXDISTANCE);

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
	// initialize dsound position notifies
	for (size_t i = 0; i < _countof(m_dsnp); i++)
	{
		BOOST_VERIFY(::ResetEvent(m_dsnp[i].hEventNotify));
	}

	// set the default block which to begin playing
	BOOST_VERIFY(::SetEvent(m_dsnp[_countof(m_dsnp) - 1].hEventNotify));

	//CachePtr cache = my::ResourceMgr::getSingleton().OpenIStream(m_Mp3Path.c_str())->GetWholeCache();

	//bool ret = false;
	//mp3dec_t mp3d;
	//mp3dec_init(&mp3d);
	//mp3dec_frame_info_t info;
	//short pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
	//std::vector<unsigned char> sbuffer;
	//for (int inLen = 0; true; inLen += info.frame_bytes)
	//{
	//	// decode audio frame
	//	int samples = mp3dec_decode_frame(&mp3d, cache->data() + inLen, cache->size() - inLen, pcm, &info);
	//	if (samples <= 0)
	//	{
	//		if (NULL != m_dsbuffer)
	//		{
	//			// wait for dsound buffer block playing
	//			_ASSERT(sizeof(m_events) == sizeof(HANDLE) * _countof(m_events));
	//			if (WAIT_OBJECT_0 != ::WaitForMultipleObjects(_countof(m_events), reinterpret_cast<HANDLE*>(m_events), FALSE, INFINITE))
	//			{
	//				// normal play end, need to continue if looped
	//				ret = true;
	//			}
	//			m_dsbuffer->Stop();
	//		}
	//		break;
	//	}

	//	// parse dither linear pcm data to compatible format
	//	if (2 == info.channels)
	//	{
	//		for (int i = 0; i < samples; i++)
	//		{
	//			sbuffer.push_back(pcm[i * 2 + 0] >> 0);
	//			sbuffer.push_back(pcm[i * 2 + 0] >> 8);
	//			sbuffer.push_back(pcm[i * 2 + 1] >> 0);
	//			sbuffer.push_back(pcm[i * 2 + 1] >> 8);
	//		}
	//	}
	//	else
	//	{
	//		for (int i = 0; i < samples; i++)
	//		{
	//			sbuffer.push_back(pcm[i] >> 0);
	//			sbuffer.push_back(pcm[i] >> 8);
	//		}
	//	}

	//	// create platform sound devices if needed
	//	if (m_wavfmt.nChannels != info.channels || m_wavfmt.nSamplesPerSec != info.hz)
	//	{
	//		// dsound buffer should only be create once
	//		_ASSERT(NULL == m_dsnotify);
	//		_ASSERT(NULL == m_dsbuffer);

	//		_ASSERT(WAVE_FORMAT_PCM == m_wavfmt.wFormatTag);
	//		m_wavfmt.nChannels = info.channels;
	//		m_wavfmt.nSamplesPerSec = info.hz;
	//		m_wavfmt.wBitsPerSample = 16;
	//		m_wavfmt.nBlockAlign = m_wavfmt.nChannels * m_wavfmt.wBitsPerSample / 8;
	//		m_wavfmt.nAvgBytesPerSec = m_wavfmt.nSamplesPerSec * m_wavfmt.nBlockAlign;
	//		m_wavfmt.cbSize = 0;

	//		DSBUFFERDESC dsbd;
	//		dsbd.dwSize = sizeof(dsbd);
	//		dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_LOCSOFTWARE;
	//		dsbd.dwBufferBytes = m_wavfmt.nAvgBytesPerSec * BLOCK_COUNT;
	//		dsbd.dwReserved = 0;
	//		dsbd.lpwfxFormat = &m_wavfmt;
	//		dsbd.guid3DAlgorithm = DS3DALG_DEFAULT;

	//		// recalculate notify position for each block
	//		for (int i = 0; i < _countof(m_dsnp); i++)
	//		{
	//			m_dsnp[i].dwOffset = i * m_wavfmt.nAvgBytesPerSec;
	//		}

	//		// create dsound buffer & notify
	//		my::CriticalSectionLock lock(SoundContext::getSingleton().m_soundsec);
	//		m_dsbuffer = SoundContext::getSingleton().m_sound.CreateSoundBuffer(&dsbd);
	//		lock.Unlock();
	//		m_dsnotify = m_dsbuffer->GetNotify();
	//		m_dsnotify->setNotificationPositions(_countof(m_dsnp), m_dsnp);
	//	}

	//	// fill pcm data to dsbuffer
	//	_ASSERT(NULL != m_dsbuffer);
	//	if (sbuffer.size() > m_wavfmt.nAvgBytesPerSec)
	//	{
	//		// wait for notify event even with stop event
	//		_ASSERT(sizeof(m_events) == sizeof(HANDLE) * _countof(m_events));
	//		DWORD wait_res = ::WaitForMultipleObjects(_countof(m_events), reinterpret_cast<HANDLE*>(m_events), FALSE, INFINITE);
	//		_ASSERT(WAIT_TIMEOUT != wait_res);
	//		if (wait_res == WAIT_OBJECT_0)
	//		{
	//			// out if stop event occured
	//			m_dsbuffer->Stop();
	//			break;
	//		}

	//		// calculate current block
	//		DWORD curr_block = wait_res - WAIT_OBJECT_0 - 1;
	//		_ASSERT(curr_block < _countof(m_dsnp));

	//		// calculate next block which need to be update
	//		DWORD next_block = (curr_block + 1) % _countof(m_dsnp);

	//		// decoded sound buffer copying
	//		unsigned char* audioPtr1, * audioPtr2;
	//		DWORD audioBytes1, audioBytes2;
	//		m_dsbuffer->Lock(m_dsnp[next_block].dwOffset, m_wavfmt.nAvgBytesPerSec, (LPVOID*)&audioPtr1, &audioBytes1, (LPVOID*)&audioPtr2, &audioBytes2, 0);
	//		_ASSERT(audioBytes1 + audioBytes2 <= m_wavfmt.nAvgBytesPerSec);
	//		if (audioPtr1 != NULL)
	//		{
	//			memcpy(audioPtr1, &sbuffer[0], audioBytes1);
	//		}
	//		if (audioPtr2 != NULL)
	//		{
	//			memcpy(audioPtr2, &sbuffer[0 + audioBytes1], audioBytes2);
	//		}
	//		m_dsbuffer->Unlock(audioPtr1, audioBytes1, audioPtr2, audioBytes2);

	//		// begin play
	//		if (!(m_dsbuffer->GetStatus() & DSBSTATUS_PLAYING))
	//		{
	//			// reset play position
	//			m_dsbuffer->SetCurrentPosition(m_dsnp[next_block].dwOffset);
	//			m_dsbuffer->Play(0, DSBPLAY_LOOPING);
	//		}

	//		// move remain buffer which havent been pushed into dsound buffer to header
	//		size_t remain = sbuffer.size() - m_wavfmt.nAvgBytesPerSec;
	//		memmove(&sbuffer[0], &sbuffer[m_wavfmt.nAvgBytesPerSec], remain);
	//		sbuffer.resize(remain);
	//	}
	//}
	//return ret;

std::vector<unsigned char> convbuffer; /* take 8k out of the data segment, not the stack */
int convsize = 4096;

	ogg_sync_state   oy; /* sync and verify incoming physical bitstream */
	ogg_stream_state os; /* take physical pages, weld into a logical
							stream of packets */
	ogg_page         og; /* one Ogg bitstream page. Vorbis packets are inside */
	ogg_packet       op; /* one raw packet of data for decode */

	vorbis_info      vi; /* struct that stores all the static vorbis bitstream
							settings */
	vorbis_comment   vc; /* struct that stores all the bitstream user comments */
	vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
	vorbis_block     vb; /* local working space for packet->PCM decode */

	char* buffer;
	int  bytes;

	/********** Decode setup ************/

	ogg_sync_init(&oy); /* Now we can read pages */

	my::IStreamPtr istr = my::ResourceMgr::getSingleton().OpenIStream(m_Mp3Path.c_str());

	while (1) { /* we repeat if the bitstream is chained */
		int eos = 0;
		int i;

		/* grab some data at the head of the stream. We want the first page
		   (which is guaranteed to be small and only contain the Vorbis
		   stream initial header) We need the first page to get the stream
		   serialno. */

		   /* submit a 4k block to libvorbis' Ogg layer */
		buffer = ogg_sync_buffer(&oy, 4096);
		bytes = istr->read(buffer, 4096);
		ogg_sync_wrote(&oy, bytes);

		/* Get the first page. */
		if (ogg_sync_pageout(&oy, &og) != 1) {
			/* have we simply run out of data?  If so, we're done. */
			if (bytes < 4096)break;

			/* error case.  Must not be Vorbis data */
			D3DContext::getSingleton().m_EventLog("Input does not appear to be an Ogg bitstream.\n");
			return false;
		}

		/* Get the serial number and set up the rest of decode. */
		/* serialno first; use it to set up a logical stream */
		ogg_stream_init(&os, ogg_page_serialno(&og));

		/* extract the initial header from the first page and verify that the
		   Ogg bitstream is in fact Vorbis data */

		   /* I handle the initial header first instead of just having the code
			  read all three Vorbis headers at once because reading the initial
			  header is an easy way to identify a Vorbis bitstream and it's
			  useful to see that functionality seperated out. */

		vorbis_info_init(&vi);
		vorbis_comment_init(&vc);
		if (ogg_stream_pagein(&os, &og) < 0) {
			/* error; stream version mismatch perhaps */
			D3DContext::getSingleton().m_EventLog("Error reading first page of Ogg bitstream data.\n");
			return false;
		}

		if (ogg_stream_packetout(&os, &op) != 1) {
			/* no page? must not be vorbis */
			D3DContext::getSingleton().m_EventLog("Error reading initial header packet.\n");
			return false;
		}

		if (vorbis_synthesis_headerin(&vi, &vc, &op) < 0) {
			/* error case; not a vorbis header */
			D3DContext::getSingleton().m_EventLog("This Ogg bitstream does not contain Vorbis "
				"audio data.\n");
			return false;
		}

		/* At this point, we're sure we're Vorbis. We've set up the logical
		   (Ogg) bitstream decoder. Get the comment and codebook headers and
		   set up the Vorbis decoder */

		   /* The next two packets in order are the comment and codebook headers.
			  They're likely large and may span multiple pages. Thus we read
			  and submit data until we get our two packets, watching that no
			  pages are missing. If a page is missing, error out; losing a
			  header page is the only place where missing data is fatal. */

		i = 0;
		while (i < 2) {
			while (i < 2) {
				int result = ogg_sync_pageout(&oy, &og);
				if (result == 0)break; /* Need more data */
				/* Don't complain about missing or corrupt data yet. We'll
				   catch it at the packet output phase */
				if (result == 1) {
					ogg_stream_pagein(&os, &og); /* we can ignore any errors here
												   as they'll also become apparent
												   at packetout */
					while (i < 2) {
						result = ogg_stream_packetout(&os, &op);
						if (result == 0)break;
						if (result < 0) {
							/* Uh oh; data at some point was corrupted or missing!
							   We can't tolerate that in a header.  Die. */
							D3DContext::getSingleton().m_EventLog("Corrupt secondary header.  Exiting.\n");
							return false;
						}
						result = vorbis_synthesis_headerin(&vi, &vc, &op);
						if (result < 0) {
							D3DContext::getSingleton().m_EventLog("Corrupt secondary header.  Exiting.\n");
							return false;
						}
						i++;
					}
				}
			}
			/* no harm in not checking before adding more */
			buffer = ogg_sync_buffer(&oy, 4096);
			bytes = istr->read(buffer, 4096);
			if (bytes == 0 && i < 2) {
				D3DContext::getSingleton().m_EventLog("End of file before finding all Vorbis headers!\n");
				return false;
			}
			ogg_sync_wrote(&oy, bytes);
		}

		/* Throw the comments plus a few lines about the bitstream we're
		   decoding */
		//{
		//	char** ptr = vc.user_comments;
		//	while (*ptr) {
		//		D3DContext::getSingleton().m_EventLog(str_printf("%s\n", *ptr).c_str());
		//		++ptr;
		//	}
		//	D3DContext::getSingleton().m_EventLog(str_printf("\nBitstream is %d channel, %ldHz\n", vi.channels, vi.rate).c_str());
		//	D3DContext::getSingleton().m_EventLog(str_printf("Encoded by: %s\n\n", vc.vendor).c_str());
		//}
		if (m_wavfmt.nChannels != vi.channels || m_wavfmt.nSamplesPerSec != vi.rate)
		{
			// dsound buffer should only be create once
			_ASSERT(NULL == m_dsnotify);
			_ASSERT(NULL == m_dsbuffer);

			_ASSERT(WAVE_FORMAT_PCM == m_wavfmt.wFormatTag);
			m_wavfmt.nChannels = vi.channels;
			m_wavfmt.nSamplesPerSec = vi.rate;
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
			my::CriticalSectionLock lock2(m_buffersec);
			m_dsbuffer = SoundContext::getSingleton().m_sound.CreateSoundBuffer(&dsbd);
			lock.Unlock();
			m_dsnotify = m_dsbuffer->GetNotify();
			m_dsnotify->setNotificationPositions(_countof(m_dsnp), m_dsnp);
		}

		convsize = 4096 / vi.channels;

		/* OK, got and parsed all three headers. Initialize the Vorbis
		   packet->PCM decoder. */
		if (vorbis_synthesis_init(&vd, &vi) == 0) { /* central decode state */
			vorbis_block_init(&vd, &vb);          /* local state for most of the decode
													so multiple block decodes can
													proceed in parallel. We could init
													multiple vorbis_block structures
													for vd here */

			/* The rest is just a straight decode loop until end of stream */
			while (!eos) {
				while (!eos) {
					int result = ogg_sync_pageout(&oy, &og);
					if (result == 0)break; /* need more data */
					if (result < 0) { /* missing or corrupt data at this page position */
						D3DContext::getSingleton().m_EventLog("Corrupt or missing data in bitstream; "
							"continuing...\n");
					}
					else {
						ogg_stream_pagein(&os, &og); /* can safely ignore errors at
													   this point */
						while (1) {
							result = ogg_stream_packetout(&os, &op);

							if (result == 0)break; /* need more data */
							if (result < 0) { /* missing or corrupt data at this page position */
							  /* no reason to complain; already complained above */
							}
							else {
								/* we have a packet.  Decode it */
								float** pcm;
								int samples;

								if (vorbis_synthesis(&vb, &op) == 0) /* test for success! */
									vorbis_synthesis_blockin(&vd, &vb);
								/*

								**pcm is a multichannel float vector.  In stereo, for
								example, pcm[0] is left, and pcm[1] is right.  samples is
								the size of each channel.  Convert the float values
								(-1.<=range<=1.) to whatever PCM format and write it out */

								while ((samples = vorbis_synthesis_pcmout(&vd, &pcm)) > 0) {
									int j;
									int clipflag = 0;
									int bout = (samples < convsize ? samples : convsize);

									/* convert floats to 16 bit signed ints (host order) and
									   interleave */
									for (j = 0; j < bout; j++) {
										for (i = 0; i < vi.channels; i++) {
											float* mono = pcm[i];
#if 1
											int val = floor(mono[j] * 32767.f + .5f);
#else /* optional dither */
											int val = mono[j] * 32767.f + drand48() - 0.5f;
#endif
											/* might as well guard against clipping */
											if (val > 32767) {
												val = 32767;
												clipflag = 1;
											}
											if (val < -32768) {
												val = -32768;
												clipflag = 1;
											}
											convbuffer.push_back(LOBYTE(val >> 0));
											convbuffer.push_back(LOBYTE(val >> 8));
											//ptr += vi.channels;
										}
									}

									//if (clipflag)
									//	D3DContext::getSingleton().m_EventLog(str_printf("Clipping in frame %ld\n", (long)(vd.sequence)).c_str());

									//FILE* fout = fopen("aaa.pcm", "ab");
									//fwrite(convbuffer.data(), 2 * vi.channels, bout, fout);
									//fclose(fout);
									//convbuffer.clear();

									// fill pcm data to dsbuffer
									_ASSERT(NULL != m_dsbuffer);
									if (convbuffer.size() > m_wavfmt.nAvgBytesPerSec)
									{
										// wait for notify event even with stop event
										_ASSERT(sizeof(m_events) == sizeof(HANDLE) * _countof(m_events));
										DWORD wait_res = ::WaitForMultipleObjects(_countof(m_events), reinterpret_cast<HANDLE*>(m_events), FALSE, INFINITE);
										_ASSERT(WAIT_TIMEOUT != wait_res);
										if (wait_res == WAIT_OBJECT_0)
										{
											// out if stop event occured
											my::CriticalSectionLock lock(m_buffersec);
											m_dsbuffer->Stop();
											lock.Unlock();
											vorbis_block_clear(&vb);
											vorbis_dsp_clear(&vd);
											ogg_stream_clear(&os);
											vorbis_comment_clear(&vc);
											vorbis_info_clear(&vi);  /* must be called last */
											ogg_sync_clear(&oy);
											return false;
										}

										// calculate current block
										DWORD curr_block = wait_res - WAIT_OBJECT_0 - 1;
										_ASSERT(curr_block < _countof(m_dsnp));

										// calculate next block which need to be update
										DWORD next_block = (curr_block + 1) % _countof(m_dsnp);

										// decoded sound buffer copying
										unsigned char* audioPtr1, * audioPtr2;
										DWORD audioBytes1, audioBytes2;
										my::CriticalSectionLock lock(m_buffersec);
										m_dsbuffer->Lock(m_dsnp[next_block].dwOffset, m_wavfmt.nAvgBytesPerSec, (LPVOID*)&audioPtr1, &audioBytes1, (LPVOID*)&audioPtr2, &audioBytes2, 0);
										_ASSERT(audioBytes1 + audioBytes2 <= m_wavfmt.nAvgBytesPerSec);
										if (audioPtr1 != NULL)
										{
											memcpy(audioPtr1, &convbuffer[0], audioBytes1);
										}
										if (audioPtr2 != NULL)
										{
											memcpy(audioPtr2, &convbuffer[0 + audioBytes1], audioBytes2);
										}
										m_dsbuffer->Unlock(audioPtr1, audioBytes1, audioPtr2, audioBytes2);

										// begin play
										if (!(m_dsbuffer->GetStatus() & DSBSTATUS_PLAYING))
										{
											// reset play position
											m_dsbuffer->SetCurrentPosition(m_dsnp[next_block].dwOffset);
											m_dsbuffer->SetVolume(m_Volume);
											m_dsbuffer->Play(0, DSBPLAY_LOOPING);
										}
										lock.Unlock();

										// move remain buffer which havent been pushed into dsound buffer to header
										size_t remain = convbuffer.size() - m_wavfmt.nAvgBytesPerSec;
										memmove(&convbuffer[0], &convbuffer[m_wavfmt.nAvgBytesPerSec], remain);
										convbuffer.resize(remain);
									}

									vorbis_synthesis_read(&vd, bout); /* tell libvorbis how
																		many samples we
																		actually consumed */
								}
							}
						}
						if (ogg_page_eos(&og))eos = 1;
					}
				}
				if (!eos) {
					buffer = ogg_sync_buffer(&oy, 4096);
					bytes = istr->read(buffer, 4096);
					ogg_sync_wrote(&oy, bytes);
					if (bytes == 0)eos = 1;
				}
			}

			/* ogg_page and ogg_packet structs always point to storage in
			   libvorbis.  They're never freed or manipulated directly */

			vorbis_block_clear(&vb);
			vorbis_dsp_clear(&vd);
		}
		else {
			D3DContext::getSingleton().m_EventLog("Error: Corrupt header during playback initialization.\n");
		}

		/* clean up this logical bitstream; before exit we see if we're
		   followed by another [chained] */

		ogg_stream_clear(&os);
		vorbis_comment_clear(&vc);
		vorbis_info_clear(&vi);  /* must be called last */
	}

	/* OK, clean up the framer */
	ogg_sync_clear(&oy);

	//D3DContext::getSingleton().m_EventLog("Done.\n");
	return true;
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

LONG Mp3::GetVolume(void)
{
	return m_Volume;
}

void Mp3::SetVolume(LONG lVolume)
{
	m_Volume = my::Clamp(lVolume, (LONG)DSBVOLUME_MIN, (LONG)DSBVOLUME_MAX);
	my::CriticalSectionLock lock(m_buffersec);
	if (m_dsbuffer)
	{
		m_dsbuffer->SetVolume(m_Volume);
	}
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

	m_Mp3Path.clear();
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
