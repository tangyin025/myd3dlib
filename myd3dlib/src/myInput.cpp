#include "stdafx.h"
#include "myInput.h"
#include "myException.h"

using namespace my;

Input::Input(void)
: m_ptr(NULL)
{
}

Input::~Input(void)
{
	Destroy();
}

void Input::Create(IDirectInput8 * dinput)
{
	_ASSERT(!m_ptr);

	m_ptr = dinput;
}

void Input::CreateInput(HINSTANCE hinst)
{
	LPDIRECTINPUT8 input;
	if(FAILED(hr = DirectInput8Create(hinst, DIRECTINPUT_HEADER_VERSION, IID_IDirectInput8, (LPVOID *)&input, NULL)))
	{
		THROW_DINPUTEXCEPTION(hr);
	}

	Create(input);
}

void Input::Destroy(void)
{
	SAFE_RELEASE(m_ptr);
}

CComPtr<IDirectInputDevice8> Input::CreateDevice(REFGUID rguid)
{
	CComPtr<IDirectInputDevice8> ret;
	if(FAILED(hr = m_ptr->CreateDevice(rguid, &ret, NULL)))
	{
		THROW_DINPUTEXCEPTION(hr);
	}
	return ret;
}

void Input::ConfigureDevices(
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

void Input::EnumDevices(
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

void Input::EnumDevicesBySemantics(
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

void Input::FindDevice(
	REFGUID rguidClass,
	LPCTSTR ptszName,
	LPGUID pguidInstance)
{
	if(FAILED(hr = m_ptr->FindDevice(rguidClass, ptszName, pguidInstance)))
	{
		THROW_DINPUTEXCEPTION(hr);
	}
}

void Input::GetDeviceStatus(REFGUID rguidInstance)
{
	if(FAILED(hr = m_ptr->GetDeviceStatus(rguidInstance)))
	{
		THROW_DINPUTEXCEPTION(hr);
	}
}

void Input::RunControlPanel(HWND hwndOwner)
{
	V(m_ptr->RunControlPanel(hwndOwner, 0));
}

InputDevice::InputDevice(void)
	: m_ptr(NULL)
{
}

InputDevice::~InputDevice(void)
{
	Destroy();
}

void InputDevice::Create(LPDIRECTINPUTDEVICE8 device)
{
	_ASSERT(!m_ptr);

	m_ptr = device;
}

void InputDevice::Destroy(void)
{
	SAFE_RELEASE(m_ptr);
}

void InputDevice::SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
{
	V(m_ptr->SetCooperativeLevel(hwnd, dwFlags));
}

void InputDevice::SetDataFormat(LPCDIDATAFORMAT lpdf)
{
	V(m_ptr->SetDataFormat(lpdf));
}

void InputDevice::SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
{
	V(m_ptr->SetProperty(rguidProp, pdiph));
}

void InputDevice::Acquire(void)
{
	if(FAILED(hr = m_ptr->Acquire()))
	{
		THROW_DINPUTEXCEPTION(hr);
	}
}

void InputDevice::Unacquire(void)
{
	V(m_ptr->Unacquire());
}

void InputDevice::GetDeviceState(DWORD cbData, LPVOID lpvData)
{
	if(FAILED(hr = m_ptr->GetDeviceState(cbData, lpvData)))
	{
		THROW_DINPUTEXCEPTION(hr);
	}
}

void Keyboard::CreateKeyboard(LPDIRECTINPUT8 input)
{
	LPDIRECTINPUTDEVICE8 device;
	if(FAILED(hr = input->CreateDevice(GUID_SysKeyboard, &device, NULL)))
	{
		THROW_DINPUTEXCEPTION(hr);
	}

	Create(device);

	SetDataFormat(&c_dfDIKeyboard);

	ZeroMemory(&m_CurState, sizeof(m_CurState));
}

void Keyboard::Capture(void)
{
	memcpy(m_PreState, m_CurState, sizeof(m_PreState));

	if(FAILED(hr = m_ptr->GetDeviceState(sizeof(m_CurState), m_CurState)))
	{
		hr = m_ptr->Acquire();
		if(SUCCEEDED(hr))
		{
			GetDeviceState(sizeof(m_CurState), m_CurState);
		}
	}
}

BYTE Keyboard::IsKeyDown(DWORD dwIndex)
{
	_ASSERT(dwIndex < _countof(m_CurState));

	return m_CurState[dwIndex];
}

bool Keyboard::IsKeyPressed(DWORD dwIndex)
{
	_ASSERT(dwIndex < _countof(m_CurState));

	return !m_PreState[dwIndex] && m_CurState[dwIndex];
}

bool Keyboard::IsKeyReleased(DWORD dwIndex)
{
	_ASSERT(dwIndex < _countof(m_CurState));

	return m_PreState[dwIndex] && !m_CurState[dwIndex];
}

void Mouse::CreateMouse(LPDIRECTINPUT8 input)
{
	LPDIRECTINPUTDEVICE8 device;
	if(FAILED(hr = input->CreateDevice(GUID_SysMouse, &device, NULL)))
	{
		THROW_DINPUTEXCEPTION(hr);
	}

	Create(device);

	SetDataFormat(&c_dfDIMouse);

	ZeroMemory(&m_CurState, sizeof(m_CurState));
}

void Mouse::Capture(void)
{
	memcpy(&m_PreState, &m_CurState, sizeof(m_PreState));

	if(FAILED(hr = m_ptr->GetDeviceState(sizeof(m_CurState), &m_CurState)))
	{
		hr = m_ptr->Acquire();
		if(SUCCEEDED(hr))
		{
			GetDeviceState(sizeof(m_CurState), &m_CurState);
		}
	}
}

LONG Mouse::GetX(void)
{
	return m_CurState.lX;
}

LONG Mouse::GetY(void)
{
	return m_CurState.lY;
}

LONG Mouse::GetZ(void)
{
	return m_CurState.lZ;
}

unsigned char Mouse::IsButtonDown(DWORD dwIndex)
{
	return m_CurState.rgbButtons[dwIndex];
}

bool Mouse::IsButtonPressed(DWORD dwIndex)
{
	return !m_PreState.rgbButtons[dwIndex] && m_CurState.rgbButtons[dwIndex];
}

bool Mouse::IsButtonReleased(DWORD dwIndex)
{
	return m_PreState.rgbButtons[dwIndex] && !m_CurState.rgbButtons[dwIndex];
}

void Joystick::CreateJoystick(
	LPDIRECTINPUT8 input,
	REFGUID rguid,
	LONG min_x,
	LONG max_x,
	LONG min_y,
	LONG max_y,
	LONG min_z,
	LONG max_z,
	float dead_zone)
{
	LPDIRECTINPUTDEVICE8 device;
	if(FAILED(hr = input->CreateDevice(rguid, &device, NULL)))
	{
		THROW_DINPUTEXCEPTION(hr);
	}

	Create(device);

	SetDataFormat(&c_dfDIJoystick);

	_ASSERT(min_x <= max_x && min_y <= max_y);

	DIPROPRANGE dipr = {sizeof(dipr), sizeof(dipr.diph)};
	dipr.diph.dwObj = DIJOFS_X;
	dipr.diph.dwHow = DIPH_BYOFFSET;
	dipr.lMin = min_x;
	dipr.lMax = max_x;
	SetProperty(DIPROP_RANGE, &dipr.diph);

	dipr.diph.dwObj = DIJOFS_Y;
	dipr.diph.dwHow = DIPH_BYOFFSET;
	dipr.lMin = min_y;
	dipr.lMax = max_y;
	SetProperty(DIPROP_RANGE, &dipr.diph);

	dipr.diph.dwObj = DIJOFS_Z;
	dipr.diph.dwHow = DIPH_BYOFFSET;
	dipr.lMin = min_z;
	dipr.lMax = max_z;
	SetProperty(DIPROP_RANGE, &dipr.diph);

	dipr.diph.dwObj = DIJOFS_RZ;
	dipr.diph.dwHow = DIPH_BYOFFSET;
	dipr.lMin = min_z;
	dipr.lMax = max_z;
	SetProperty(DIPROP_RANGE, &dipr.diph);

	_ASSERT(dead_zone >= 0 && dead_zone <= 100);

	DIPROPDWORD dipd  = {sizeof(dipd), sizeof(dipd.diph)};
	dipd.diph.dwObj = DIJOFS_X;
	dipd.diph.dwHow = DIPH_BYOFFSET;
	dipd.dwData = (DWORD)(dead_zone * 100);
	SetProperty(DIPROP_DEADZONE, &dipd.diph);

	dipd.diph.dwObj = DIJOFS_Y;
	dipd.diph.dwHow = DIPH_BYOFFSET;
	dipd.dwData = (DWORD)(dead_zone * 100);
	SetProperty(DIPROP_DEADZONE, &dipd.diph);

	dipd.diph.dwObj = DIJOFS_Z;
	dipd.diph.dwHow = DIPH_BYOFFSET;
	dipd.dwData = (DWORD)(dead_zone * 100);
	SetProperty(DIPROP_DEADZONE, &dipd.diph);

	dipd.diph.dwObj = DIJOFS_RZ;
	dipd.diph.dwHow = DIPH_BYOFFSET;
	dipd.dwData = (DWORD)(dead_zone * 100);
	SetProperty(DIPROP_DEADZONE, &dipd.diph);
}

void Joystick::Capture(void)
{
	memcpy(&m_PreState, &m_CurState, sizeof(m_PreState));

	if(FAILED(hr = m_ptr->GetDeviceState(sizeof(m_CurState), &m_CurState)))
	{
		hr = m_ptr->Acquire();
		if(SUCCEEDED(hr))
		{
			GetDeviceState(sizeof(m_CurState), &m_CurState);
		}
	}
}

LONG Joystick::GetX(void) const
{
	return m_CurState.lX;
}

LONG Joystick::GetY(void) const
{
	return m_CurState.lY;
}

LONG Joystick::GetZ(void) const
{
	return m_CurState.lZ;
}

LONG Joystick::GetRx(void) const
{
	return m_CurState.lRx;
}

LONG Joystick::GetRy(void) const
{
	return m_CurState.lRy;
}

LONG Joystick::GetRz(void) const
{
	return m_CurState.lRz;
}

LONG Joystick::GetU(void) const
{
	return m_CurState.rglSlider[0];
}

LONG Joystick::GetV(void) const
{
	return m_CurState.rglSlider[1];
}

DWORD Joystick::GetPOV(DWORD dwIndex) const
{
	_ASSERT(dwIndex < _countof(m_CurState.rgdwPOV));

	return m_CurState.rgdwPOV[dwIndex];
}

BYTE Joystick::IsButtonDown(DWORD dwIndex) const
{
	_ASSERT(dwIndex < _countof(m_CurState.rgbButtons));

	return m_CurState.rgbButtons[dwIndex];
}
