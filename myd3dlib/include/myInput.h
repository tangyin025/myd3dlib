#pragma once

#include <boost/shared_ptr.hpp>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <atlbase.h>

namespace my
{
	class Input
	{
	public:
		HRESULT hr;

		IDirectInput8 * m_ptr;

	public:
		Input(void);

		virtual ~Input(void);

		void Create(IDirectInput8 * dinput);

		void CreateInput(HINSTANCE hinst);

		void ConfigureDevices(
			LPDICONFIGUREDEVICESCALLBACK lpdiCallback,
			LPDICONFIGUREDEVICESPARAMS lpdiCDParams,
			DWORD dwFlags,
			LPVOID pvRefData);

		CComPtr<IDirectInputDevice8> CreateDevice(REFGUID rguid);

		void EnumDevices(
			DWORD dwDevType,
			LPDIENUMDEVICESCALLBACK lpCallback,
			LPVOID pvRef,
			DWORD dwFlags);

		void EnumDevicesBySemantics(
			LPCTSTR ptszUserName,
			LPDIACTIONFORMAT lpdiActionFormat,
			LPDIENUMDEVICESBYSEMANTICSCB lpCallback,
			LPVOID pvRef,
			DWORD dwFlags);

		void FindDevice(
			REFGUID rguidClass,
			LPCTSTR ptszName,
			LPGUID pguidInstance);

		void GetDeviceStatus(REFGUID rguidInstance);

		void RunControlPanel(HWND hwndOwner);
	};

	typedef boost::shared_ptr<Input> InputPtr;

	class InputDevice
	{
	public:
		HRESULT hr;

		friend Input;

		IDirectInputDevice8 * m_ptr;

	public:
		InputDevice(void);

		virtual ~InputDevice(void);

		void Create(LPDIRECTINPUTDEVICE8 device);

		virtual void Capture(void) = 0;

		void SetCooperativeLevel(HWND hwnd, DWORD dwFlags = DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

		void SetDataFormat(LPCDIDATAFORMAT lpdf);

		void SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph);

		void Acquire(void);

		void Unacquire(void);

		void GetDeviceState(DWORD cbData, LPVOID lpvData);
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

		void CreateKeyboard(LPDIRECTINPUT8 input);

		virtual void Capture(void);

		BYTE IsKeyDown(DWORD dwIndex);

		bool IsKeyPressed(DWORD dwIndex);

		bool IsKeyReleased(DWORD dwIndex);
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

		void CreateMouse(LPDIRECTINPUT8 input);

		virtual void Capture(void);

		LONG GetX(void);

		LONG GetY(void);

		LONG GetZ(void);

		unsigned char IsButtonDown(DWORD dwIndex);

		bool IsButtonPressed(DWORD dwIndex);

		bool IsButtonReleased(DWORD dwIndex);
	};

	typedef boost::shared_ptr<Mouse> MousePtr;
}
