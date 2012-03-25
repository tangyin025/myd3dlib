
#pragma once

namespace my
{
	class Input;

	typedef boost::shared_ptr<Input> InputPtr;

	class Input
	{
	public:
		IDirectInput8W * m_ptr;

		HRESULT hr;

		Input(IDirectInput8W * dinput)
			: m_ptr(dinput)
		{
		}

	public:
		virtual ~Input(void)
		{
			SAFE_RELEASE(m_ptr);
		}

		static InputPtr CreateInput(HINSTANCE hinst);

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

	class InputDevice
	{
	public:
		friend Input;

		IDirectInputDevice8W * m_ptr;

		HRESULT hr;

		InputDevice(LPDIRECTINPUTDEVICE8W device)
			: m_ptr(device)
		{
		}

	public:
		virtual ~InputDevice(void)
		{
			SAFE_RELEASE(m_ptr);
		}

		virtual void Capture(void) = 0;

		void SetCooperativeLevel(HWND hwnd, DWORD dwFlags = DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)
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

	class Keyboard;

	typedef boost::shared_ptr<Keyboard> KeyboardPtr;

	class Keyboard : public InputDevice
	{
	protected:
		BYTE m_PreState[256];

		BYTE m_CurState[256];

		Keyboard(LPDIRECTINPUTDEVICE8W device);

	public:
		static KeyboardPtr CreateKeyboard(LPDIRECTINPUT8W input);

		virtual void Capture(void);

		BYTE IsKeyDown(DWORD dwIndex);

		bool IsKeyPressed(DWORD dwIndex);

		bool IsKeyReleased(DWORD dwIndex);
	};

	class Mouse;

	typedef boost::shared_ptr<Mouse> MousePtr;

	class Mouse : public InputDevice
	{
	protected:
		DIMOUSESTATE m_PreState;

		DIMOUSESTATE m_CurState;

		Mouse(LPDIRECTINPUTDEVICE8W device);

	public:
		static MousePtr CreateMouse(LPDIRECTINPUT8W input);

		virtual void Capture(void);

		unsigned char IsButtonDown(DWORD dwIndex);

		bool IsButtonPressed(DWORD dwIndex);

		bool IsButtonReleased(DWORD dwIndex);
	};
}
