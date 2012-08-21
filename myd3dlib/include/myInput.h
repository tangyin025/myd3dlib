#pragma once

#include <boost/shared_ptr.hpp>
#include <dinput.h>
#include <atlbase.h>
#include <DXUT.h>
#include "myException.h"

namespace my
{
	class Input
	{
	public:
		HRESULT hr;

		IDirectInput8W * m_ptr;

	public:
		Input(void)
			: m_ptr(NULL)
		{
		}

		virtual ~Input(void)
		{
			SAFE_RELEASE(m_ptr);
		}

		void Create(IDirectInput8W * dinput)
		{
			_ASSERT(!m_ptr);

			m_ptr = dinput;
		}

		void CreateInput(HINSTANCE hinst);

		void ConfigureDevices(
			LPDICONFIGUREDEVICESCALLBACK lpdiCallback,
			LPDICONFIGUREDEVICESPARAMS lpdiCDParams,
			DWORD dwFlags,
			LPVOID pvRefData)
		{
			if(FAILED(hr = m_ptr->ConfigureDevices(lpdiCallback, lpdiCDParams, dwFlags, pvRefData)))
			{
				THROW_DINPUTEXCEPTION(hr);
			}
		}

		CComPtr<IDirectInputDevice8> CreateDevice(REFGUID rguid)
		{
			CComPtr<IDirectInputDevice8> ret;
			if(FAILED(hr = m_ptr->CreateDevice(rguid, &ret, NULL)))
			{
				THROW_DINPUTEXCEPTION(hr);
			}
		}

		void EnumDevices(
			DWORD dwDevType,
			LPDIENUMDEVICESCALLBACK lpCallback,
			LPVOID pvRef,
			DWORD dwFlags)
		{
			if(FAILED(hr = m_ptr->EnumDevices(dwDevType, lpCallback, pvRef, dwFlags)))
			{
				THROW_DINPUTEXCEPTION(hr);
			}
		}

		void EnumDevicesBySemantics(
			LPCTSTR ptszUserName,
			LPDIACTIONFORMAT lpdiActionFormat,
			LPDIENUMDEVICESBYSEMANTICSCB lpCallback,
			LPVOID pvRef,
			DWORD dwFlags)
		{
			if(FAILED(hr = m_ptr->EnumDevicesBySemantics(ptszUserName, lpdiActionFormat, lpCallback, pvRef, dwFlags)))
			{
				THROW_DINPUTEXCEPTION(hr);
			}
		}

		void FindDevice(
			REFGUID rguidClass,
			LPCTSTR ptszName,
			LPGUID pguidInstance)
		{
			if(FAILED(hr = m_ptr->FindDevice(rguidClass, ptszName, pguidInstance)))
			{
				THROW_DINPUTEXCEPTION(hr);
			}
		}

		void GetDeviceStatus(REFGUID rguidInstance)
		{
			if(FAILED(hr = m_ptr->GetDeviceStatus(rguidInstance)))
			{
				THROW_DINPUTEXCEPTION(hr);
			}
		}

		void RunControlPanel(HWND hwndOwner)
		{
			V(m_ptr->RunControlPanel(hwndOwner, 0));
		}
	};

	typedef boost::shared_ptr<Input> InputPtr;

	class InputDevice
	{
	public:
		HRESULT hr;

		friend Input;

		IDirectInputDevice8W * m_ptr;

	public:
		InputDevice(void)
			: m_ptr(NULL)
		{
		}

		virtual ~InputDevice(void)
		{
			SAFE_RELEASE(m_ptr);
		}

		void Create(LPDIRECTINPUTDEVICE8W device)
		{
			_ASSERT(!m_ptr);

			m_ptr = device;
		}

		virtual void Capture(void) = 0;

		void SetCooperativeLevel(HWND hwnd, DWORD dwFlags = DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)
		{
			V(m_ptr->SetCooperativeLevel(hwnd, dwFlags));
		}

		void SetDataFormat(LPCDIDATAFORMAT lpdf)
		{
			V(m_ptr->SetDataFormat(lpdf));
		}

		void SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
		{
			V(m_ptr->SetProperty(rguidProp, pdiph));
		}

		void Acquire(void)
		{
			if(FAILED(hr = m_ptr->Acquire()))
			{
				THROW_DINPUTEXCEPTION(hr);
			}
		}

		void Unacquire(void)
		{
			V(m_ptr->Unacquire());
		}

		void GetDeviceState(DWORD cbData, LPVOID lpvData)
		{
			if(FAILED(hr = m_ptr->GetDeviceState(cbData, lpvData)))
			{
				THROW_DINPUTEXCEPTION(hr);
			}
		}
	};

	class Keyboard : public InputDevice
	{
	protected:
		BYTE m_PreState[256];

		BYTE m_CurState[256];

	public:
		Keyboard(void)
		{
		}

		void CreateKeyboard(LPDIRECTINPUT8W input);

		virtual void Capture(void);

		BYTE IsKeyDown(DWORD dwIndex)
		{
			return m_CurState[dwIndex];
		}

		bool IsKeyPressed(DWORD dwIndex)
		{
			return !m_PreState[dwIndex] && m_CurState[dwIndex];
		}

		bool IsKeyReleased(DWORD dwIndex)
		{
			return m_PreState[dwIndex] && !m_CurState[dwIndex];
		}
	};

	typedef boost::shared_ptr<Keyboard> KeyboardPtr;

	class Mouse : public InputDevice
	{
	protected:
		DIMOUSESTATE m_PreState;

		DIMOUSESTATE m_CurState;

	public:
		Mouse(void)
		{
		}

		void CreateMouse(LPDIRECTINPUT8W input);

		virtual void Capture(void);

		LONG GetX(void)
		{
			return m_CurState.lX;
		}

		LONG GetY(void)
		{
			return m_CurState.lY;
		}

		LONG GetZ(void)
		{
			return m_CurState.lZ;
		}

		unsigned char IsButtonDown(DWORD dwIndex)
		{
			return m_CurState.rgbButtons[dwIndex];
		}

		bool IsButtonPressed(DWORD dwIndex)
		{
			return !m_PreState.rgbButtons[dwIndex] && m_CurState.rgbButtons[dwIndex];
		}

		bool IsButtonReleased(DWORD dwIndex)
		{
			return m_PreState.rgbButtons[dwIndex] && !m_CurState.rgbButtons[dwIndex];
		}
	};

	typedef boost::shared_ptr<Mouse> MousePtr;
}
