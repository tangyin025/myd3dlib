
#pragma once

#include <boost/shared_ptr.hpp>
#include <dsound.h>
#include <DXUT.h>
#include "myMath.h"

namespace my
{
	class Sound;

	typedef boost::shared_ptr<Sound> SoundPtr;

	class SoundBuffer;

	typedef boost::shared_ptr<SoundBuffer> SoundBufferPtr;

	class Sound
	{
	public:
		IDirectSound8 * m_ptr;

		HRESULT hr;

		Sound(IDirectSound8 * ptr)
			: m_ptr(ptr)
		{
		}

	public:
		virtual ~Sound(void)
		{
			SAFE_RELEASE(m_ptr);
		}

		static SoundPtr CreateSound(void);

		SoundBufferPtr CreateSoundBuffer(
			LPCDSBUFFERDESC pcDSBufferDesc);

		DSCAPS GetCaps(void)
		{
			DSCAPS res;
			V(m_ptr->GetCaps(&res));
			return res;
		}

		void SetCooperativeLevel(
			HWND hwnd,
			DWORD dwLevel = DSSCL_PRIORITY)
		{
			V(m_ptr->SetCooperativeLevel(hwnd, dwLevel));
		}
	};

	class Sound3DBuffer;

	typedef boost::shared_ptr<Sound3DBuffer> Sound3DBufferPtr;

	class Sound3DListener;

	typedef boost::shared_ptr<Sound3DListener> Sound3DListenerPtr;

	class SoundBuffer
	{
		friend Sound;

	public:
		IDirectSoundBuffer * m_ptr;

		HRESULT hr;

		SoundBuffer(IDirectSoundBuffer * ptr)
			: m_ptr(ptr)
		{
		}

		static SoundBufferPtr CreateSoundBufferFromMmio(
			LPDIRECTSOUND8 pDSound,
			HMMIO hmmio,
			DWORD flags);

	public:
		virtual ~SoundBuffer(void)
		{
			SAFE_RELEASE(m_ptr);
		}

		static SoundBufferPtr CreateSoundBufferFromFile(
			LPDIRECTSOUND8 pDSound,
			LPCSTR pSrcFile,
			DWORD flags = DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_STATIC | DSBCAPS_LOCSOFTWARE);

		static SoundBufferPtr CreateSoundBufferFromFileInMemory(
			LPDIRECTSOUND8 pDSound,
			LPCVOID pSrcData,
			UINT SrcDataLen,
			DWORD flags = DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_STATIC | DSBCAPS_LOCSOFTWARE);

		void GetCurrentPosition(
			LPDWORD pdwCurrentPlayCursor,
			LPDWORD pdwCurrentWriteCursor)
		{
			V(m_ptr->GetCurrentPosition(pdwCurrentPlayCursor, pdwCurrentWriteCursor));
		}

		void Lock(
			DWORD dwOffset,
			DWORD dwBytes,
			LPVOID * ppvAudioPtr1,
			LPDWORD  pdwAudioBytes1,
			LPVOID * ppvAudioPtr2,
			LPDWORD pdwAudioBytes2,
			DWORD dwFlags)
		{
			V(m_ptr->Lock(dwOffset, dwBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags));
		}

		void Play(
			DWORD dwPriority = 0,
			DWORD dwFlags = 0)
		{
			V(m_ptr->Play(0, dwPriority, dwFlags));
		}

		void SetCurrentPosition(
			DWORD dwNewPosition)
		{
			V(m_ptr->SetCurrentPosition(dwNewPosition));
		}

		void Stop(void)
		{
			V(m_ptr->Stop());
		}

		void Unlock(
			LPVOID pvAudioPtr1,
			DWORD dwAudioBytes1,
			LPVOID pvAudioPtr2,
			DWORD dwAudioBytes2)
		{
			V(m_ptr->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2));
		}

		DWORD GetFrequency(void)
		{
			DWORD res;
			V(m_ptr->GetFrequency(&res));
			return res;
		}

		LONG GetPan(void)
		{
			LONG res;
			V(m_ptr->GetPan(&res));
			return res;
		}

		LONG GetVolume(void)
		{
			LONG res;
			V(m_ptr->GetVolume(&res));
			return res;
		}

		void SetFrequency(
			DWORD dwFrequency)
		{
			V(m_ptr->SetFrequency(dwFrequency));
		}

		void SetPan(
			LONG lPan)
		{
			V(m_ptr->SetPan(lPan));
		}

		void SetVolume(
			LONG lVolume)
		{
			V(m_ptr->SetVolume(lVolume));
		}

		DSBCAPS GetCaps(void)
		{
			DSBCAPS res;
			V(m_ptr->GetCaps(&res));
			return res;
		}

		void GetFormat(
			LPWAVEFORMATEX pwfxFormat,
			DWORD dwSizeAllocated,
			LPDWORD pdwSizeWritten)
		{
			V(m_ptr->GetFormat(pwfxFormat, dwSizeAllocated, pdwSizeWritten));
		}

		DWORD GetStatus(void)
		{
			DWORD res;
			V(m_ptr->GetStatus(&res));
			return res;
		}

		void SetFormat(
			LPCWAVEFORMATEX pcfxFormat)
		{
			V(m_ptr->SetFormat(pcfxFormat));
		}

	public:
		Sound3DBufferPtr Get3DBuffer(void);

		Sound3DListenerPtr Get3DListener(void);
	};

	class Sound3DBuffer
	{
		friend SoundBuffer;

	public:
		IDirectSound3DBuffer * m_ptr;

		HRESULT hr;

		Sound3DBuffer(IDirectSound3DBuffer * ptr)
			: m_ptr(ptr)
		{
		}

	public:
		virtual ~Sound3DBuffer(void)
		{
			SAFE_RELEASE(m_ptr);
		}

		DWORD GetMode(void)
		{
			DWORD res;
			V(m_ptr->GetMode(&res));
			return res;
		}

		void SetMode(
			DWORD dwMode,
			DWORD dwApply = DS3D_IMMEDIATE)
		{
			V(m_ptr->SetMode(dwMode, dwApply));
		}

		DS3DBUFFER GetAllParameters(void)
		{
			DS3DBUFFER res;
			V(m_ptr->GetAllParameters(&res));
			return res;
		}

		void SetAllParameters(
			LPCDS3DBUFFER pcDs3dBuffer,
			DWORD dwApply = DS3D_IMMEDIATE)
		{
			V(m_ptr->SetAllParameters(pcDs3dBuffer, dwApply));
		}

		D3DVALUE GetMaxDistance(void)
		{
			D3DVALUE res;
			V(m_ptr->GetMaxDistance(&res));
			return res;
		}

		D3DVALUE GetMinDistance(void)
		{
			D3DVALUE res;
			V(m_ptr->GetMinDistance(&res));
			return res;
		}

		void SetMaxDistance(
			D3DVALUE flMaxDistance,
			DWORD dwApply = DS3D_IMMEDIATE)
		{
			V(m_ptr->SetMaxDistance(flMaxDistance, dwApply));
		}

		void SetMinDistance(
			D3DVALUE flMinDistance,
			DWORD dwApply = DS3D_IMMEDIATE)
		{
			V(m_ptr->SetMinDistance(flMinDistance, dwApply));
		}

		Vector3 GetPosition(void)
		{
			Vector3 res;
			V(m_ptr->GetPosition((D3DVECTOR *)&res));
			return res;
		}

		void SetPosition(
			const Vector3 & pos,
			DWORD dwApply = DS3D_IMMEDIATE)
		{
			V(m_ptr->SetPosition(pos.x, pos.y, pos.z, dwApply));
		}

		void GetConeAngles(
			LPDWORD pdwInsideConeAngle,
			LPDWORD pdwOutsideConeAngle)
		{
			V(m_ptr->GetConeAngles(pdwInsideConeAngle, pdwOutsideConeAngle));
		}

		Vector3 GetConeOrientation(void)
		{
			Vector3 res;
			V(m_ptr->GetConeOrientation((D3DVECTOR *)&res));
			return res;
		}

		LONG GetConeOutsideVolume(void)
		{
			LONG res;
			V(m_ptr->GetConeOutsideVolume(&res));
			return res;
		}

		void SetConeAngles(
			DWORD dwInsideConeAngle,
			DWORD dwOutsideConeAngle,
			DWORD dwApply = DS3D_IMMEDIATE)
		{
			V(m_ptr->SetConeAngles(dwInsideConeAngle, dwOutsideConeAngle, dwApply));
		}

		void SetConeOrientation(
			const Vector3 & ori,
			DWORD dwApply = DS3D_IMMEDIATE)
		{
			V(m_ptr->SetConeOrientation(ori.x, ori.y, ori.z, dwApply));
		}

		void SetConeOutsideVolume(
			LONG lConeOutsideVolume,
			DWORD dwApply = DS3D_IMMEDIATE)
		{
			V(m_ptr->SetConeOutsideVolume(lConeOutsideVolume, dwApply));
		}

		Vector3 GetVelocity(void)
		{
			Vector3 res;
			V(m_ptr->GetVelocity((D3DVECTOR *)&res));
			return res;
		}

		void SetVelocity(
			const Vector3 & vel,
			DWORD dwApply = DS3D_IMMEDIATE)
		{
			V(m_ptr->SetVelocity(vel.x, vel.y, vel.z, dwApply));
		}
	};

	class Sound3DListener
	{
	public:
		IDirectSound3DListener * m_ptr;

		HRESULT hr;

		Sound3DListener(IDirectSound3DListener * ptr)
			: m_ptr(ptr)
		{
		}

	public:
		virtual ~Sound3DListener(void)
		{
			SAFE_RELEASE(m_ptr);
		}

		void CommitDeferredSettings(void)
		{
			V(m_ptr->CommitDeferredSettings());
		}

		DS3DLISTENER GetAllParameters(void)
		{
			DS3DLISTENER res;
			V(m_ptr->GetAllParameters(&res));
			return res;
		}

		void SetAllParameters(
			LPCDS3DLISTENER pcListener,
			DWORD dwApply = DS3D_IMMEDIATE)
		{
			V(m_ptr->SetAllParameters(pcListener, dwApply));
		}

		D3DVALUE GetDistanceFactor(void)
		{
			D3DVALUE res;
			V(m_ptr->GetDistanceFactor(&res));
			return res;
		}

		D3DVALUE GetDopplerFactor(void)
		{
			D3DVALUE res;
			V(m_ptr->GetDopplerFactor(&res));
			return res;
		}

		D3DVALUE GetRolloffFactor(void)
		{
			D3DVALUE res;
			V(m_ptr->GetRolloffFactor(&res));
			return res;
		}

		void SetDistanceFactor(
			D3DVALUE flDistanceFactor,
			DWORD dwApply = DS3D_IMMEDIATE)
		{
			V(m_ptr->SetDistanceFactor(flDistanceFactor, dwApply));
		}

		void SetDopplerFactor(
			D3DVALUE flDopplerFactor,
			DWORD dwApply = DS3D_IMMEDIATE)
		{
			V(m_ptr->SetDopplerFactor(flDopplerFactor, dwApply));
		}

		void SetRolloffFactor(
			D3DVALUE flRolloffFactor,
			DWORD dwApply = DS3D_IMMEDIATE)
		{
			V(m_ptr->SetRolloffFactor(flRolloffFactor, dwApply));
		}

		void GetOrientation(
			D3DVECTOR * pvOrientFront,
			D3DVECTOR * pvOrientTop)
		{
			V(m_ptr->GetOrientation(pvOrientFront, pvOrientTop));
		}

		Vector3 GetPosition(void)
		{
			Vector3 res;
			V(m_ptr->GetPosition((D3DVECTOR *)&res));
			return res;
		}

		Vector3 GetVelocity(void)
		{
			Vector3 res;
			V(m_ptr->GetVelocity((D3DVECTOR *)&res));
			return res;
		}

		void SetOrientation(
			const Vector3 & Front,
			const Vector3 & Top,
			DWORD dwApply = DS3D_IMMEDIATE)
		{
			V(m_ptr->SetOrientation(Front.x, Front.y, Front.z, Top.x, Top.y, Top.z, dwApply));
		}

		void SetPosition(
			const Vector3 & pos,
			DWORD dwApply = DS3D_IMMEDIATE)
		{
			V(m_ptr->SetPosition(pos.x, pos.y, pos.z, dwApply));
		}

		void SetVelocity(
			const Vector3 & vel,
			DWORD dwApply = DS3D_IMMEDIATE)
		{
			V(m_ptr->SetVelocity(vel.x, vel.y, vel.z, dwApply));
		}
	};
}
