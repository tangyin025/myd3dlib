// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include <boost/shared_ptr.hpp>
#include <WTypes.h>
#include <Mmsystem.h>
#include <dsound.h>
#include "myMath.h"
#include "mySingleton.h"

namespace my
{
	class SoundBuffer;

	typedef boost::shared_ptr<SoundBuffer> SoundBufferPtr;

	class Sound
	{
	public:
		HRESULT hr;

		IDirectSound8 * m_ptr;

	public:
		Sound(void);

		virtual ~Sound(void);

		void Create(IDirectSound8 * ptr);

		void CreateSound(void);

		SoundBufferPtr CreateSoundBuffer(
			LPCDSBUFFERDESC pcSoundBufferDesc);

		DSCAPS GetCaps(void);

		void SetCooperativeLevel(
			HWND hwnd,
			DWORD dwLevel = DSSCL_PRIORITY);
	};

	typedef boost::shared_ptr<Sound> SoundPtr;

	class SoundNotify;

	typedef boost::shared_ptr<SoundNotify> SoundNotifyPtr;

	class Sound3DBuffer;

	typedef boost::shared_ptr<Sound3DBuffer> Sound3DBufferPtr;

	class Sound3DListener;

	typedef boost::shared_ptr<Sound3DListener> Sound3DListenerPtr;

	class SoundBuffer
	{
	public:
		HRESULT hr;

		IDirectSoundBuffer * m_ptr;

	public:
		SoundBuffer(void);

		virtual ~SoundBuffer(void);

		void Create(IDirectSoundBuffer * ptr);

		void GetCurrentPosition(
			LPDWORD pdwCurrentPlayCursor,
			LPDWORD pdwCurrentWriteCursor);

		void Lock(
			DWORD dwOffset,
			DWORD dwBytes,
			LPVOID * ppvAudioPtr1,
			LPDWORD  pdwAudioBytes1,
			LPVOID * ppvAudioPtr2,
			LPDWORD pdwAudioBytes2,
			DWORD dwFlags);

		void Play(
			DWORD dwPriority = 0,
			DWORD dwFlags = 0);

		void SetCurrentPosition(
			DWORD dwNewPosition);

		void Stop(void);

		void Unlock(
			LPVOID pvAudioPtr1,
			DWORD dwAudioBytes1,
			LPVOID pvAudioPtr2,
			DWORD dwAudioBytes2);

		DWORD GetFrequency(void);

		LONG GetPan(void);

		LONG GetVolume(void);

		void SetFrequency(
			DWORD dwFrequency);

		void SetPan(
			LONG lPan);

		void SetVolume(
			LONG lVolume);

		DSBCAPS GetCaps(void);

		void GetFormat(
			LPWAVEFORMATEX pwfxFormat,
			DWORD dwSizeAllocated,
			LPDWORD pdwSizeWritten);

		DWORD GetStatus(void);

		void SetFormat(
			LPCWAVEFORMATEX pcfxFormat);

	public:
		SoundNotifyPtr GetNotify(void);

		Sound3DBufferPtr Get3DBuffer(void);

		Sound3DListenerPtr Get3DListener(void);
	};

	class SoundNotify
	{
		friend SoundBuffer;

	public:
		IDirectSoundNotify * m_ptr;

		HRESULT hr;

		SoundNotify(IDirectSoundNotify * ptr);

	public:
		virtual ~SoundNotify(void);

		void setNotificationPositions(DWORD dwPositionNotifies, LPCDSBPOSITIONNOTIFY pcPositionNotifies);
	};

	class Sound3DBuffer
	{
		friend SoundBuffer;

	public:
		IDirectSound3DBuffer * m_ptr;

		HRESULT hr;

		Sound3DBuffer(IDirectSound3DBuffer * ptr);

	public:
		virtual ~Sound3DBuffer(void);

		DWORD GetMode(void);

		void SetMode(
			DWORD dwMode,
			DWORD dwApply = DS3D_DEFERRED);

		DS3DBUFFER GetAllParameters(void);

		void SetAllParameters(
			LPCDS3DBUFFER pcDs3dBuffer,
			DWORD dwApply = DS3D_DEFERRED);

		D3DVALUE GetMaxDistance(void);

		D3DVALUE GetMinDistance(void);

		void SetMaxDistance(
			D3DVALUE flMaxDistance,
			DWORD dwApply = DS3D_DEFERRED);

		void SetMinDistance(
			D3DVALUE flMinDistance,
			DWORD dwApply = DS3D_DEFERRED);

		Vector3 GetPosition(void);

		void SetPosition(
			const Vector3 & pos,
			DWORD dwApply = DS3D_DEFERRED);

		void GetConeAngles(
			LPDWORD pdwInsideConeAngle,
			LPDWORD pdwOutsideConeAngle);

		Vector3 GetConeOrientation(void);

		LONG GetConeOutsideVolume(void);

		void SetConeAngles(
			DWORD dwInsideConeAngle,
			DWORD dwOutsideConeAngle,
			DWORD dwApply = DS3D_DEFERRED);

		void SetConeOrientation(
			const Vector3 & ori,
			DWORD dwApply = DS3D_DEFERRED);

		void SetConeOutsideVolume(
			LONG lConeOutsideVolume,
			DWORD dwApply = DS3D_DEFERRED);

		Vector3 GetVelocity(void);

		void SetVelocity(
			const Vector3 & vel,
			DWORD dwApply = DS3D_DEFERRED);
	};

	class Sound3DListener
	{
	public:
		IDirectSound3DListener * m_ptr;

		HRESULT hr;

		Sound3DListener(IDirectSound3DListener * ptr);

	public:
		virtual ~Sound3DListener(void);

		void CommitDeferredSettings(void);

		DS3DLISTENER GetAllParameters(void);

		void SetAllParameters(
			LPCDS3DLISTENER pcListener,
			DWORD dwApply = DS3D_DEFERRED);

		D3DVALUE GetDistanceFactor(void);

		D3DVALUE GetDopplerFactor(void);

		D3DVALUE GetRolloffFactor(void);

		void SetDistanceFactor(
			D3DVALUE flDistanceFactor,
			DWORD dwApply = DS3D_DEFERRED);

		void SetDopplerFactor(
			D3DVALUE flDopplerFactor,
			DWORD dwApply = DS3D_DEFERRED);

		void SetRolloffFactor(
			D3DVALUE flRolloffFactor,
			DWORD dwApply = DS3D_DEFERRED);

		void GetOrientation(
			Vector3 & Front,
			Vector3 & Top);

		Vector3 GetPosition(void);

		Vector3 GetVelocity(void);

		void SetOrientation(
			const Vector3 & Front,
			const Vector3 & Top,
			DWORD dwApply = DS3D_DEFERRED);

		void SetPosition(
			const Vector3 & pos,
			DWORD dwApply = DS3D_DEFERRED);

		void SetVelocity(
			const Vector3 & vel,
			DWORD dwApply = DS3D_DEFERRED);
	};

	class Wav : public DeviceResourceBase
	{
	public:
		MMCKINFO parent;

		MMCKINFO child;

		boost::shared_ptr<WAVEFORMATEX> wavfmt;

		std::vector<unsigned char> buffer;

	public:
		Wav(void)
		{
		}

		void OnResetDevice(void)
		{
		}

		void OnLostDevice(void)
		{
		}

		void OnDestroyDevice(void)
		{
		}

		void CreateWavFromMmio(
			HMMIO hmmio);

		void CreateWavFromFile(
			LPCTSTR pFilename);

		void CreateWavFromFileInStream(
			my::IStreamPtr istr);

		void CreateWavFromFileInMemory(
			LPCVOID Memory,
			DWORD SizeOfMemory);

		static size_t SecToBlockByte(const WAVEFORMATEX & fmt, float sec);
	};

	typedef boost::shared_ptr<Wav> WavPtr;
}
