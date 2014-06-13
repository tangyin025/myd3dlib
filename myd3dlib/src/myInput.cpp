#include "stdafx.h"
#include "myInput.h"
#include "myException.h"

using namespace my;

#define KEYBOARD_DX_BUFFERSIZE 17
#define MOUSE_DX_BUFFERSIZE 128
#define JOYSTICK_DX_BUFFERSIZE 129

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

void InputDevice::GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
	if (FAILED(hr = m_ptr->GetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags)))
	{
		THROW_DINPUTEXCEPTION(hr);
	}
}

LPCTSTR Keyboard::Translate(KeyCode kc)
{
	switch (kc)
	{
	case KC_UNASSIGNED: return _T("KC_UNASSIGNED");
	case KC_ESCAPE: return _T("KC_ESCAPE");
	case KC_1: return _T("KC_1");
	case KC_2: return _T("KC_2");
	case KC_3: return _T("KC_3");
	case KC_4: return _T("KC_4");
	case KC_5: return _T("KC_5");
	case KC_6: return _T("KC_6");
	case KC_7: return _T("KC_7");
	case KC_8: return _T("KC_8");
	case KC_9: return _T("KC_9");
	case KC_0: return _T("KC_0");
	case KC_MINUS: return _T("KC_MINUS");
	case KC_EQUALS: return _T("KC_EQUALS");
	case KC_BACK: return _T("KC_BACK");
	case KC_TAB: return _T("KC_TAB");
	case KC_Q: return _T("KC_Q");
	case KC_W: return _T("KC_W");
	case KC_E: return _T("KC_E");
	case KC_R: return _T("KC_R");
	case KC_T: return _T("KC_T");
	case KC_Y: return _T("KC_Y");
	case KC_U: return _T("KC_U");
	case KC_I: return _T("KC_I");
	case KC_O: return _T("KC_O");
	case KC_P: return _T("KC_P");
	case KC_LBRACKET: return _T("KC_LBRACKET");
	case KC_RBRACKET: return _T("KC_RBRACKET");
	case KC_RETURN: return _T("KC_RETURN");
	case KC_LCONTROL: return _T("KC_LCONTROL");
	case KC_A: return _T("KC_A");
	case KC_S: return _T("KC_S");
	case KC_D: return _T("KC_D");
	case KC_F: return _T("KC_F");
	case KC_G: return _T("KC_G");
	case KC_H: return _T("KC_H");
	case KC_J: return _T("KC_J");
	case KC_K: return _T("KC_K");
	case KC_L: return _T("KC_L");
	case KC_SEMICOLON: return _T("KC_SEMICOLON");
	case KC_APOSTROPHE: return _T("KC_APOSTROPHE");
	case KC_GRAVE: return _T("KC_GRAVE");
	case KC_LSHIFT: return _T("KC_LSHIFT");
	case KC_BACKSLASH: return _T("KC_BACKSLASH");
	case KC_Z: return _T("KC_Z");
	case KC_X: return _T("KC_X");
	case KC_C: return _T("KC_C");
	case KC_V: return _T("KC_V");
	case KC_B: return _T("KC_B");
	case KC_N: return _T("KC_N");
	case KC_M: return _T("KC_M");
	case KC_COMMA: return _T("KC_COMMA");
	case KC_PERIOD: return _T("KC_PERIOD");
	case KC_SLASH: return _T("KC_SLASH");
	case KC_RSHIFT: return _T("KC_RSHIFT");
	case KC_MULTIPLY: return _T("KC_MULTIPLY");
	case KC_LMENU: return _T("KC_LMENU");
	case KC_SPACE: return _T("KC_SPACE");
	case KC_CAPITAL: return _T("KC_CAPITAL");
	case KC_F1: return _T("KC_F1");
	case KC_F2: return _T("KC_F2");
	case KC_F3: return _T("KC_F3");
	case KC_F4: return _T("KC_F4");
	case KC_F5: return _T("KC_F5");
	case KC_F6: return _T("KC_F6");
	case KC_F7: return _T("KC_F7");
	case KC_F8: return _T("KC_F8");
	case KC_F9: return _T("KC_F9");
	case KC_F10: return _T("KC_F10");
	case KC_NUMLOCK: return _T("KC_NUMLOCK");
	case KC_SCROLL: return _T("KC_SCROLL");
	case KC_NUMPAD7: return _T("KC_NUMPAD7");
	case KC_NUMPAD8: return _T("KC_NUMPAD8");
	case KC_NUMPAD9: return _T("KC_NUMPAD9");
	case KC_SUBTRACT: return _T("KC_SUBTRACT");
	case KC_NUMPAD4: return _T("KC_NUMPAD4");
	case KC_NUMPAD5: return _T("KC_NUMPAD5");
	case KC_NUMPAD6: return _T("KC_NUMPAD6");
	case KC_ADD: return _T("KC_ADD");
	case KC_NUMPAD1: return _T("KC_NUMPAD1");
	case KC_NUMPAD2: return _T("KC_NUMPAD2");
	case KC_NUMPAD3: return _T("KC_NUMPAD3");
	case KC_NUMPAD0: return _T("KC_NUMPAD0");
	case KC_DECIMAL: return _T("KC_DECIMAL");
	case KC_OEM_102: return _T("KC_OEM_102");
	case KC_F11: return _T("KC_F11");
	case KC_F12: return _T("KC_F12");
	case KC_F13: return _T("KC_F13");
	case KC_F14: return _T("KC_F14");
	case KC_F15: return _T("KC_F15");
	case KC_KANA: return _T("KC_KANA");
	case KC_ABNT_C1: return _T("KC_ABNT_C1");
	case KC_CONVERT: return _T("KC_CONVERT");
	case KC_NOCONVERT: return _T("KC_NOCONVERT");
	case KC_YEN: return _T("KC_YEN");
	case KC_ABNT_C2: return _T("KC_ABNT_C2");
	case KC_NUMPADEQUALS: return _T("KC_NUMPADEQUALS");
	case KC_PREVTRACK: return _T("KC_PREVTRACK");
	case KC_AT: return _T("KC_AT");
	case KC_COLON: return _T("KC_COLON");
	case KC_UNDERLINE: return _T("KC_UNDERLINE");
	case KC_KANJI: return _T("KC_KANJI");
	case KC_STOP: return _T("KC_STOP");
	case KC_AX: return _T("KC_AX");
	case KC_UNLABELED: return _T("KC_UNLABELED");
	case KC_NEXTTRACK: return _T("KC_NEXTTRACK");
	case KC_NUMPADENTER: return _T("KC_NUMPADENTER");
	case KC_RCONTROL: return _T("KC_RCONTROL");
	case KC_MUTE: return _T("KC_MUTE");
	case KC_CALCULATOR: return _T("KC_CALCULATOR");
	case KC_PLAYPAUSE: return _T("KC_PLAYPAUSE");
	case KC_MEDIASTOP: return _T("KC_MEDIASTOP");
	case KC_VOLUMEDOWN: return _T("KC_VOLUMEDOWN");
	case KC_VOLUMEUP: return _T("KC_VOLUMEUP");
	case KC_WEBHOME: return _T("KC_WEBHOME");
	case KC_NUMPADCOMMA: return _T("KC_NUMPADCOMMA");
	case KC_DIVIDE: return _T("KC_DIVIDE");
	case KC_SYSRQ: return _T("KC_SYSRQ");
	case KC_RMENU: return _T("KC_RMENU");
	case KC_PAUSE: return _T("KC_PAUSE");
	case KC_HOME: return _T("KC_HOME");
	case KC_UP: return _T("KC_UP");
	case KC_PGUP: return _T("KC_PGUP");
	case KC_LEFT: return _T("KC_LEFT");
	case KC_RIGHT: return _T("KC_RIGHT");
	case KC_END: return _T("KC_END");
	case KC_DOWN: return _T("KC_DOWN");
	case KC_PGDOWN: return _T("KC_PGDOWN");
	case KC_INSERT: return _T("KC_INSERT");
	case KC_DELETE: return _T("KC_DELETE");
	case KC_LWIN: return _T("KC_LWIN");
	case KC_RWIN: return _T("KC_RWIN");
	case KC_APPS: return _T("KC_APPS");
	case KC_POWER: return _T("KC_POWER");
	case KC_SLEEP: return _T("KC_SLEEP");
	case KC_WAKE: return _T("KC_WAKE");
	case KC_WEBSEARCH: return _T("KC_WEBSEARCH");
	case KC_WEBFAVORITES: return _T("KC_WEBFAVORITES");
	case KC_WEBREFRESH: return _T("KC_WEBREFRESH");
	case KC_WEBSTOP: return _T("KC_WEBSTOP");
	case KC_WEBFORWARD: return _T("KC_WEBFORWARD");
	case KC_WEBBACK: return _T("KC_WEBBACK");
	case KC_MYCOMPUTER: return _T("KC_MYCOMPUTER");
	case KC_MAIL: return _T("KC_MAIL");
	case KC_MEDIASELECT: return _T("KC_MEDIASELECT");
	}
	return _T("unknown key code");
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

	DIPROPDWORD dipdw;
	dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj        = 0;
	dipdw.diph.dwHow        = DIPH_DEVICE;
	dipdw.dwData            = KEYBOARD_DX_BUFFERSIZE;
	SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

	ZeroMemory(&m_State, sizeof(m_State));
}

void Keyboard::Capture(void)
{
	BYTE OldState[256];

	memcpy(OldState, m_State, sizeof(OldState));

	DIDEVICEOBJECTDATA diBuff[KEYBOARD_DX_BUFFERSIZE];
	DWORD entries = KEYBOARD_DX_BUFFERSIZE;
	if(FAILED(hr = m_ptr->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &entries, 0)))
	{
		if (FAILED(hr = m_ptr->Acquire()))
		{
			return;
		}

		GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &entries, 0);
	}

	for (DWORD i = 0; i < entries; i++)
	{
		KeyCode kc = (KeyCode)diBuff[i].dwOfs;
		m_State[kc] = static_cast<BYTE>(diBuff[i].dwData);
		if (diBuff[i].dwData & 0x80)
		{
			if (m_PressedEvent)
			{
				m_PressedEvent(kc);
			}
		}
		else
		{
			if (m_ReleasedEvent)
			{
				m_ReleasedEvent(kc);
			}
		}
	}
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

	ZeroMemory(&m_State, sizeof(m_State));
}

void Mouse::Capture(void)
{
	memcpy(&OldState, &m_State, sizeof(OldState));

	if(FAILED(hr = m_ptr->GetDeviceState(sizeof(m_State), &m_State)))
	{
		hr = m_ptr->Acquire();
		if(SUCCEEDED(hr))
		{
			GetDeviceState(sizeof(m_State), &m_State);
		}
	}
}

LONG Mouse::GetX(void)
{
	return m_State.lX;
}

LONG Mouse::GetY(void)
{
	return m_State.lY;
}

LONG Mouse::GetZ(void)
{
	return m_State.lZ;
}

unsigned char Mouse::IsButtonDown(DWORD dwIndex)
{
	return m_State.rgbButtons[dwIndex];
}

bool Mouse::IsButtonPressed(DWORD dwIndex)
{
	return !OldState.rgbButtons[dwIndex] && m_State.rgbButtons[dwIndex];
}

bool Mouse::IsButtonReleased(DWORD dwIndex)
{
	return OldState.rgbButtons[dwIndex] && !m_State.rgbButtons[dwIndex];
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
	memcpy(&OldState, &m_State, sizeof(OldState));

	if(FAILED(hr = m_ptr->GetDeviceState(sizeof(m_State), &m_State)))
	{
		hr = m_ptr->Acquire();
		if(SUCCEEDED(hr))
		{
			GetDeviceState(sizeof(m_State), &m_State);
		}
	}
}

LONG Joystick::GetX(void) const
{
	return m_State.lX;
}

LONG Joystick::GetY(void) const
{
	return m_State.lY;
}

LONG Joystick::GetZ(void) const
{
	return m_State.lZ;
}

LONG Joystick::GetRx(void) const
{
	return m_State.lRx;
}

LONG Joystick::GetRy(void) const
{
	return m_State.lRy;
}

LONG Joystick::GetRz(void) const
{
	return m_State.lRz;
}

LONG Joystick::GetU(void) const
{
	return m_State.rglSlider[0];
}

LONG Joystick::GetV(void) const
{
	return m_State.rglSlider[1];
}

DWORD Joystick::GetPOV(DWORD dwIndex) const
{
	_ASSERT(dwIndex < _countof(m_State.rgdwPOV));

	return m_State.rgdwPOV[dwIndex];
}

BYTE Joystick::IsButtonDown(DWORD dwIndex) const
{
	_ASSERT(dwIndex < _countof(m_State.rgbButtons));

	return m_State.rgbButtons[dwIndex];
}
