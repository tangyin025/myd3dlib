// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "myInput.h"
#include "myException.h"
#include <dinputd.h>

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

Keyboard::Keyboard(void)
{
	ZeroMemory(&m_State, sizeof(m_State));
}

const char * Keyboard::TranslateKeyCode(KeyCode kc)
{
	switch (kc)
	{
	case KC_UNASSIGNED: return "UNASSIGNED";
	case KC_ESCAPE: return "ESCAPE";
	case KC_1: return "1";
	case KC_2: return "2";
	case KC_3: return "3";
	case KC_4: return "4";
	case KC_5: return "5";
	case KC_6: return "6";
	case KC_7: return "7";
	case KC_8: return "8";
	case KC_9: return "9";
	case KC_0: return "0";
	case KC_MINUS: return "MINUS";
	case KC_EQUALS: return "EQUALS";
	case KC_BACK: return "BACK";
	case KC_TAB: return "TAB";
	case KC_Q: return "Q";
	case KC_W: return "W";
	case KC_E: return "E";
	case KC_R: return "R";
	case KC_T: return "T";
	case KC_Y: return "Y";
	case KC_U: return "U";
	case KC_I: return "I";
	case KC_O: return "O";
	case KC_P: return "P";
	case KC_LBRACKET: return "LBRACKET";
	case KC_RBRACKET: return "RBRACKET";
	case KC_RETURN: return "RETURN";
	case KC_LCONTROL: return "LCONTROL";
	case KC_A: return "A";
	case KC_S: return "S";
	case KC_D: return "D";
	case KC_F: return "F";
	case KC_G: return "G";
	case KC_H: return "H";
	case KC_J: return "J";
	case KC_K: return "K";
	case KC_L: return "L";
	case KC_SEMICOLON: return "SEMICOLON";
	case KC_APOSTROPHE: return "APOSTROPHE";
	case KC_GRAVE: return "GRAVE";
	case KC_LSHIFT: return "LSHIFT";
	case KC_BACKSLASH: return "BACKSLASH";
	case KC_Z: return "Z";
	case KC_X: return "X";
	case KC_C: return "C";
	case KC_V: return "V";
	case KC_B: return "B";
	case KC_N: return "N";
	case KC_M: return "M";
	case KC_COMMA: return "COMMA";
	case KC_PERIOD: return "PERIOD";
	case KC_SLASH: return "SLASH";
	case KC_RSHIFT: return "RSHIFT";
	case KC_MULTIPLY: return "MULTIPLY";
	case KC_LMENU: return "LMENU";
	case KC_SPACE: return "SPACE";
	case KC_CAPITAL: return "CAPITAL";
	case KC_F1: return "F1";
	case KC_F2: return "F2";
	case KC_F3: return "F3";
	case KC_F4: return "F4";
	case KC_F5: return "F5";
	case KC_F6: return "F6";
	case KC_F7: return "F7";
	case KC_F8: return "F8";
	case KC_F9: return "F9";
	case KC_F10: return "F10";
	case KC_NUMLOCK: return "NUMLOCK";
	case KC_SCROLL: return "SCROLL";
	case KC_NUMPAD7: return "NUMPAD7";
	case KC_NUMPAD8: return "NUMPAD8";
	case KC_NUMPAD9: return "NUMPAD9";
	case KC_SUBTRACT: return "SUBTRACT";
	case KC_NUMPAD4: return "NUMPAD4";
	case KC_NUMPAD5: return "NUMPAD5";
	case KC_NUMPAD6: return "NUMPAD6";
	case KC_ADD: return "ADD";
	case KC_NUMPAD1: return "NUMPAD1";
	case KC_NUMPAD2: return "NUMPAD2";
	case KC_NUMPAD3: return "NUMPAD3";
	case KC_NUMPAD0: return "NUMPAD0";
	case KC_DECIMAL: return "DECIMAL";
	case KC_OEM_102: return "OEM_102";
	case KC_F11: return "F11";
	case KC_F12: return "F12";
	case KC_F13: return "F13";
	case KC_F14: return "F14";
	case KC_F15: return "F15";
	case KC_KANA: return "KANA";
	case KC_ABNT_C1: return "ABNT_C1";
	case KC_CONVERT: return "CONVERT";
	case KC_NOCONVERT: return "NOCONVERT";
	case KC_YEN: return "YEN";
	case KC_ABNT_C2: return "ABNT_C2";
	case KC_NUMPADEQUALS: return "NUMPADEQUALS";
	case KC_PREVTRACK: return "PREVTRACK";
	case KC_AT: return "AT";
	case KC_COLON: return "COLON";
	case KC_UNDERLINE: return "UNDERLINE";
	case KC_KANJI: return "KANJI";
	case KC_STOP: return "STOP";
	case KC_AX: return "AX";
	case KC_UNLABELED: return "UNLABELED";
	case KC_NEXTTRACK: return "NEXTTRACK";
	case KC_NUMPADENTER: return "NUMPADENTER";
	case KC_RCONTROL: return "RCONTROL";
	case KC_MUTE: return "MUTE";
	case KC_CALCULATOR: return "CALCULATOR";
	case KC_PLAYPAUSE: return "PLAYPAUSE";
	case KC_MEDIASTOP: return "MEDIASTOP";
	case KC_VOLUMEDOWN: return "VOLUMEDOWN";
	case KC_VOLUMEUP: return "VOLUMEUP";
	case KC_WEBHOME: return "WEBHOME";
	case KC_NUMPADCOMMA: return "NUMPADCOMMA";
	case KC_DIVIDE: return "DIVIDE";
	case KC_SYSRQ: return "SYSRQ";
	case KC_RMENU: return "RMENU";
	case KC_PAUSE: return "PAUSE";
	case KC_HOME: return "HOME";
	case KC_UP: return "UP";
	case KC_PGUP: return "PGUP";
	case KC_LEFT: return "LEFT";
	case KC_RIGHT: return "RIGHT";
	case KC_END: return "END";
	case KC_DOWN: return "DOWN";
	case KC_PGDOWN: return "PGDOWN";
	case KC_INSERT: return "INSERT";
	case KC_DELETE: return "DELETE";
	case KC_LWIN: return "LWIN";
	case KC_RWIN: return "RWIN";
	case KC_APPS: return "APPS";
	case KC_POWER: return "POWER";
	case KC_SLEEP: return "SLEEP";
	case KC_WAKE: return "WAKE";
	case KC_WEBSEARCH: return "WEBSEARCH";
	case KC_WEBFAVORITES: return "WEBFAVORITES";
	case KC_WEBREFRESH: return "WEBREFRESH";
	case KC_WEBSTOP: return "WEBSTOP";
	case KC_WEBFORWARD: return "WEBFORWARD";
	case KC_WEBBACK: return "WEBBACK";
	case KC_MYCOMPUTER: return "MYCOMPUTER";
	case KC_MAIL: return "MAIL";
	case KC_MEDIASELECT: return "MEDIASELECT";
	}
	return "unknown key code";
}

const char * Keyboard::TranslateVirtualKey(DWORD vk)
{
	switch(vk)
	{
	case VK_LBUTTON: return "LBUTTON";
	case VK_RBUTTON: return "RBUTTON";
	case VK_CANCEL: return "CANCEL";
	case VK_MBUTTON: return "MBUTTON";
#if(_WIN32_WINNT >= 0x0500)
	case VK_XBUTTON1: return "XBUTTON1";
	case VK_XBUTTON2: return "XBUTTON2";
#endif /* _WIN32_WINNT >= 0x0500 */
	case VK_BACK: return "BACK";
	case VK_TAB: return "TAB";
	case VK_CLEAR: return "CLEAR";
	case VK_RETURN: return "RETURN";
	case VK_SHIFT: return "SHIFT";
	case VK_CONTROL: return "CONTROL";
	case VK_MENU: return "MENU";
	case VK_PAUSE: return "PAUSE";
	case VK_CAPITAL: return "CAPITAL";
	case VK_KANA: return "KANA";
	//case VK_HANGEUL: return "HANGEUL";
	//case VK_HANGUL: return "HANGUL";
	case VK_JUNJA: return "JUNJA";
	case VK_FINAL: return "FINAL";
	case VK_HANJA: return "HANJA";
	//case VK_KANJI: return "KANJI";
	case VK_ESCAPE: return "ESCAPE";
	case VK_CONVERT: return "CONVERT";
	case VK_NONCONVERT: return "NONCONVERT";
	case VK_ACCEPT: return "ACCEPT";
	case VK_MODECHANGE: return "MODECHANGE";
	case VK_SPACE: return "SPACE";
	case VK_PRIOR: return "PRIOR";
	case VK_NEXT: return "NEXT";
	case VK_END: return "END";
	case VK_HOME: return "HOME";
	case VK_LEFT: return "LEFT";
	case VK_UP: return "UP";
	case VK_RIGHT: return "RIGHT";
	case VK_DOWN: return "DOWN";
	case VK_SELECT: return "SELECT";
	case VK_PRINT: return "PRINT";
	case VK_EXECUTE: return "EXECUTE";
	case VK_SNAPSHOT: return "SNAPSHOT";
	case VK_INSERT: return "INSERT";
	case VK_DELETE: return "DELETE";
	case VK_HELP: return "HELP";
	case VK_LWIN: return "LWIN";
	case VK_RWIN: return "RWIN";
	case VK_APPS: return "APPS";
	case VK_SLEEP: return "SLEEP";
	case VK_NUMPAD0: return "NUMPAD0";
	case VK_NUMPAD1: return "NUMPAD1";
	case VK_NUMPAD2: return "NUMPAD2";
	case VK_NUMPAD3: return "NUMPAD3";
	case VK_NUMPAD4: return "NUMPAD4";
	case VK_NUMPAD5: return "NUMPAD5";
	case VK_NUMPAD6: return "NUMPAD6";
	case VK_NUMPAD7: return "NUMPAD7";
	case VK_NUMPAD8: return "NUMPAD8";
	case VK_NUMPAD9: return "NUMPAD9";
	case VK_MULTIPLY: return "MULTIPLY";
	case VK_ADD: return "ADD";
	case VK_SEPARATOR: return "SEPARATOR";
	case VK_SUBTRACT: return "SUBTRACT";
	case VK_DECIMAL: return "DECIMAL";
	case VK_DIVIDE: return "DIVIDE";
	case VK_F1: return "F1";
	case VK_F2: return "F2";
	case VK_F3: return "F3";
	case VK_F4: return "F4";
	case VK_F5: return "F5";
	case VK_F6: return "F6";
	case VK_F7: return "F7";
	case VK_F8: return "F8";
	case VK_F9: return "F9";
	case VK_F10: return "F10";
	case VK_F11: return "F11";
	case VK_F12: return "F12";
	case VK_F13: return "F13";
	case VK_F14: return "F14";
	case VK_F15: return "F15";
	case VK_F16: return "F16";
	case VK_F17: return "F17";
	case VK_F18: return "F18";
	case VK_F19: return "F19";
	case VK_F20: return "F20";
	case VK_F21: return "F21";
	case VK_F22: return "F22";
	case VK_F23: return "F23";
	case VK_F24: return "F24";
	case VK_NUMLOCK: return "NUMLOCK";
	case VK_SCROLL: return "SCROLL";
	case VK_OEM_NEC_EQUAL: return "OEM_NEC_EQUAL";
	//case VK_OEM_FJ_JISHO: return "OEM_FJ_JISHO";
	case VK_OEM_FJ_MASSHOU: return "OEM_FJ_MASSHOU";
	case VK_OEM_FJ_TOUROKU: return "OEM_FJ_TOUROKU";
	case VK_OEM_FJ_LOYA: return "OEM_FJ_LOYA";
	case VK_OEM_FJ_ROYA: return "OEM_FJ_ROYA";
	case VK_LSHIFT: return "LSHIFT";
	case VK_RSHIFT: return "RSHIFT";
	case VK_LCONTROL: return "LCONTROL";
	case VK_RCONTROL: return "RCONTROL";
	case VK_LMENU: return "LMENU";
	case VK_RMENU: return "RMENU";
#if(_WIN32_WINNT >= 0x0500)
	case VK_BROWSER_BACK: return "BROWSER_BACK";
	case VK_BROWSER_FORWARD: return "BROWSER_FORWARD";
	case VK_BROWSER_REFRESH: return "BROWSER_REFRESH";
	case VK_BROWSER_STOP: return "BROWSER_STOP";
	case VK_BROWSER_SEARCH: return "BROWSER_SEARCH";
	case VK_BROWSER_FAVORITES: return "BROWSER_FAVORITES";
	case VK_BROWSER_HOME: return "BROWSER_HOME";
	case VK_VOLUME_MUTE: return "VOLUME_MUTE";
	case VK_VOLUME_DOWN: return "VOLUME_DOWN";
	case VK_VOLUME_UP: return "VOLUME_UP";
	case VK_MEDIA_NEXT_TRACK: return "MEDIA_NEXT_TRACK";
	case VK_MEDIA_PREV_TRACK: return "MEDIA_PREV_TRACK";
	case VK_MEDIA_STOP: return "MEDIA_STOP";
	case VK_MEDIA_PLAY_PAUSE: return "MEDIA_PLAY_PAUSE";
	case VK_LAUNCH_MAIL: return "LAUNCH_MAIL";
	case VK_LAUNCH_MEDIA_SELECT: return "LAUNCH_MEDIA_SELECT";
	case VK_LAUNCH_APP1: return "LAUNCH_APP1";
	case VK_LAUNCH_APP2: return "LAUNCH_APP2";
#endif /* _WIN32_WINNT >= 0x0500 */
	case VK_OEM_1: return "OEM_1";
	case VK_OEM_PLUS: return "OEM_PLUS";
	case VK_OEM_COMMA: return "OEM_COMMA";
	case VK_OEM_MINUS: return "OEM_MINUS";
	case VK_OEM_PERIOD: return "OEM_PERIOD";
	case VK_OEM_2: return "OEM_2";
	case VK_OEM_3: return "OEM_3";
	case VK_OEM_4: return "OEM_4";
	case VK_OEM_5: return "OEM_5";
	case VK_OEM_6: return "OEM_6";
	case VK_OEM_7: return "OEM_7";
	case VK_OEM_8: return "OEM_8";
	case VK_OEM_AX: return "OEM_AX";
	case VK_OEM_102: return "OEM_102";
	case VK_ICO_HELP: return "ICO_HELP";
	case VK_ICO_00: return "ICO_00";
#if(WINVER >= 0x0400)
	case VK_PROCESSKEY: return "PROCESSKEY";
#endif /* WINVER >= 0x0400 */
	case VK_ICO_CLEAR: return "ICO_CLEAR";
#if(_WIN32_WINNT >= 0x0500)
	case VK_PACKET: return "PACKET";
#endif /* _WIN32_WINNT >= 0x0500 */
	case VK_OEM_RESET: return "OEM_RESET";
	case VK_OEM_JUMP: return "OEM_JUMP";
	case VK_OEM_PA1: return "OEM_PA1";
	case VK_OEM_PA2: return "OEM_PA2";
	case VK_OEM_PA3: return "OEM_PA3";
	case VK_OEM_WSCTRL: return "OEM_WSCTRL";
	case VK_OEM_CUSEL: return "OEM_CUSEL";
	case VK_OEM_ATTN: return "OEM_ATTN";
	case VK_OEM_FINISH: return "OEM_FINISH";
	case VK_OEM_COPY: return "OEM_COPY";
	case VK_OEM_AUTO: return "OEM_AUTO";
	case VK_OEM_ENLW: return "OEM_ENLW";
	case VK_OEM_BACKTAB: return "OEM_BACKTAB";
	case VK_ATTN: return "ATTN";
	case VK_CRSEL: return "CRSEL";
	case VK_EXSEL: return "EXSEL";
	case VK_EREOF: return "EREOF";
	case VK_PLAY: return "PLAY";
	case VK_ZOOM: return "ZOOM";
	case VK_NONAME: return "NONAME";
	case VK_PA1: return "PA1";
	case VK_OEM_CLEAR: return "OEM_CLEAR";
	/*
	 * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
	 * 0x40 : unassigned
	 * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
	 */
	case 0x30: return "\x30";
	case 0x31: return "\x31";
	case 0x32: return "\x32";
	case 0x33: return "\x33";
	case 0x34: return "\x34";
	case 0x35: return "\x35";
	case 0x36: return "\x36";
	case 0x37: return "\x37";
	case 0x38: return "\x38";
	case 0x39: return "\x39";
	case 0x41: return "\x41";
	case 0x42: return "\x42";
	case 0x43: return "\x43";
	case 0x44: return "\x44";
	case 0x45: return "\x45";
	case 0x46: return "\x46";
	case 0x47: return "\x47";
	case 0x48: return "\x48";
	case 0x49: return "\x49";
	case 0x4A: return "\x4A";
	case 0x4B: return "\x4B";
	case 0x4C: return "\x4C";
	case 0x4D: return "\x4D";
	case 0x4E: return "\x4E";
	case 0x4F: return "\x4F";
	case 0x50: return "\x50";
	case 0x51: return "\x51";
	case 0x52: return "\x52";
	case 0x53: return "\x53";
	case 0x54: return "\x54";
	case 0x55: return "\x55";
	case 0x56: return "\x56";
	case 0x57: return "\x57";
	case 0x58: return "\x58";
	case 0x59: return "\x59";
	case 0x5A: return "\x5A";
	}
	return "reserved vritual key";
}

void Keyboard::CreateKeyboard(LPDIRECTINPUT8 input, HWND hwnd)
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
}

bool Keyboard::Capture(void)
{
	memcpy(m_LastFrameState, m_State, sizeof(m_LastFrameState));

	DIDEVICEOBJECTDATA diBuff[KEYBOARD_DX_BUFFERSIZE];
	DWORD entries = KEYBOARD_DX_BUFFERSIZE;
	if(FAILED(hr = m_ptr->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &entries, 0)))
	{
		if (FAILED(hr = m_ptr->Acquire()))
		{
			return false;
		}

		GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &entries, 0);
	}

	for (DWORD i = 0; i < entries; i++)
	{
		KeyCode kc = (KeyCode)diBuff[i].dwOfs;
		m_State[kc] = static_cast<BYTE>(diBuff[i].dwData);
	}
	return true;
}

Mouse::Mouse(void)
{
	ZeroMemory(&m_State, sizeof(m_State));
}

void Mouse::CreateMouse(LPDIRECTINPUT8 input, HWND hwnd)
{
	LPDIRECTINPUTDEVICE8 device;
	if(FAILED(hr = input->CreateDevice(GUID_SysMouse, &device, NULL)))
	{
		THROW_DINPUTEXCEPTION(hr);
	}

	Create(device);

	SetDataFormat(&c_dfDIMouse);
}

bool Mouse::Capture(void)
{
	memcpy(&m_LastFrameState, &m_State, sizeof(m_LastFrameState));

	if(FAILED(hr = m_ptr->GetDeviceState(sizeof(m_State), &m_State)))
	{
		if(FAILED(hr = m_ptr->Acquire()))
		{
			return false;
		}

		GetDeviceState(sizeof(m_State), &m_State);
	}
	return true;
}

const char * Joystick::TranslatePov(DWORD pov)
{
	switch(pov)
	{
	case JP_None: return "JP_None";
	case JP_North: return "JP_North";
	case JP_NorthEast: return "JP_NorthEast";
	case JP_East: return "JP_East";
	case JP_SouthEast: return "JP_SouthEast";
	case JP_South: return "JP_South";
	case JP_SouthWest: return "JP_SouthWest";
	case JP_West: return "JP_West";
	case JP_NorthWest: return "JP_NorthWest";
	}
	return "unknown pov";
}

void Joystick::CreateJoystick(
	LPDIRECTINPUT8 input,
	HWND hwnd,
	REFGUID rguid,
	LONG min_x,
	LONG max_x,
	LONG min_y,
	LONG max_y,
	LONG min_z,
	LONG max_z,
	DWORD dead_zone)
{
	LPDIRECTINPUTDEVICE8 device;
	if(FAILED(hr = input->CreateDevice(rguid, &device, NULL)))
	{
		THROW_DINPUTEXCEPTION(hr);
	}

	Create(device);

	SetDataFormat(&c_dfDIJoystick);

	//_ASSERT(min_x <= max_x && min_y <= max_y);

	//DIPROPRANGE dipr = {sizeof(dipr), sizeof(dipr.diph)};
	//dipr.diph.dwObj = DIJOFS_X;
	//dipr.diph.dwHow = DIPH_BYOFFSET;
	//dipr.lMin = min_x;
	//dipr.lMax = max_x;
	//SetProperty(DIPROP_RANGE, &dipr.diph);

	//dipr.diph.dwObj = DIJOFS_Y;
	//dipr.diph.dwHow = DIPH_BYOFFSET;
	//dipr.lMin = min_y;
	//dipr.lMax = max_y;
	//SetProperty(DIPROP_RANGE, &dipr.diph);

	//dipr.diph.dwObj = DIJOFS_Z;
	//dipr.diph.dwHow = DIPH_BYOFFSET;
	//dipr.lMin = min_z;
	//dipr.lMax = max_z;
	//SetProperty(DIPROP_RANGE, &dipr.diph);

	//dipr.diph.dwObj = DIJOFS_RZ;
	//dipr.diph.dwHow = DIPH_BYOFFSET;
	//dipr.lMin = min_z;
	//dipr.lMax = max_z;
	//SetProperty(DIPROP_RANGE, &dipr.diph);

	//_ASSERT(dead_zone >= 0 && dead_zone <= 10000);

	//DIPROPDWORD dipd  = {sizeof(dipd), sizeof(dipd.diph)};
	//dipd.diph.dwObj = DIJOFS_X;
	//dipd.diph.dwHow = DIPH_BYOFFSET;
	//dipd.dwData = dead_zone;
	//SetProperty(DIPROP_DEADZONE, &dipd.diph);

	//dipd.diph.dwObj = DIJOFS_Y;
	//dipd.diph.dwHow = DIPH_BYOFFSET;
	//dipd.dwData = dead_zone;
	//SetProperty(DIPROP_DEADZONE, &dipd.diph);

	//dipd.diph.dwObj = DIJOFS_Z;
	//dipd.diph.dwHow = DIPH_BYOFFSET;
	//dipd.dwData = dead_zone;
	//SetProperty(DIPROP_DEADZONE, &dipd.diph);

	//dipd.diph.dwObj = DIJOFS_RX;
	//dipd.diph.dwHow = DIPH_BYOFFSET;
	//dipd.dwData = dead_zone;
	//SetProperty(DIPROP_DEADZONE, &dipd.diph);

	//dipd.diph.dwObj = DIJOFS_RY;
	//dipd.diph.dwHow = DIPH_BYOFFSET;
	//dipd.dwData = dead_zone;
	//SetProperty(DIPROP_DEADZONE, &dipd.diph);

	//dipd.diph.dwObj = DIJOFS_RZ;
	//dipd.diph.dwHow = DIPH_BYOFFSET;
	//dipd.dwData = dead_zone;
	//SetProperty(DIPROP_DEADZONE, &dipd.diph);

	ZeroMemory(&m_State, sizeof(m_State));
}

static LONG _subsection(LONG lastval, LONG val)
{
	switch (lastval)
	{
	case 0:
		if (val < 21845)
			return 0;
		else if (val < 54612)
			return 32767;
		else
			return 65535;
	case 32767:
		if (val < 10922)
			return 0;
		else if (val < 54612)
			return 32767;
		else
			return 65535;
	case 65535:
		if (val < 10922)
			return 0;
		else if (val < 43690)
			return 32767;
		else
			return 65535;
	}
	return 32767;
}

bool Joystick::Capture(void)
{
	m_LastFrameState.lX = _subsection(m_LastFrameState.lX, m_State.lX);
	m_LastFrameState.lY = _subsection(m_LastFrameState.lY, m_State.lY);
	m_LastFrameState.lZ = _subsection(m_LastFrameState.lZ, m_State.lZ);
	m_LastFrameState.lRx = _subsection(m_LastFrameState.lRx, m_State.lRx);
	m_LastFrameState.lRy = _subsection(m_LastFrameState.lRy, m_State.lRy);
	m_LastFrameState.lRz = _subsection(m_LastFrameState.lRz, m_State.lRz);

	memcpy(&m_LastFrameState.rglSlider, &m_State.rglSlider, sizeof(m_LastFrameState.rglSlider));

	for (int i = 0; i < _countof(m_LastFrameState.rgdwPOV); i++)
	{
		if (LOWORD(m_LastFrameState.rgdwPOV[i]) != 0xFFFF && LOWORD(m_State.rgdwPOV[i]) == 0xFFFF
			|| LOWORD(m_LastFrameState.rgdwPOV[i]) == 0xFFFF && LOWORD(m_State.rgdwPOV[i]) != 0xFFFF)
		{
			m_LastFrameState.rgdwPOV[i] = m_State.rgdwPOV[i];
		}
	}

	memcpy(&m_LastFrameState.rgbButtons, &m_State.rgbButtons, sizeof(m_LastFrameState.rgbButtons));

	if(FAILED(hr = m_ptr->GetDeviceState(sizeof(m_State), &m_State)))
	{
		if(FAILED(hr = m_ptr->Acquire()))
		{
			return false;
		}

		GetDeviceState(sizeof(m_State), &m_State);
	}
	return true;
}

struct DI_ENUM_CONTEXT
{
	DIJOYCONFIG* pPreferredJoyCfg;
	bool bPreferredJoyCfgValid;
	LPDIRECTINPUT8 input;
	HWND hwnd;
	LONG min_x;
	LONG max_x;
	LONG min_y;
	LONG max_y;
	LONG min_z;
	LONG max_z;
	DWORD dead_zone;
	JoystickPtr joystick;
};

void InputMgr::Create(HINSTANCE hinst, HWND hwnd)
{
	m_input.reset(new Input);
	m_input->CreateInput(hinst);

	m_keyboard.reset(new Keyboard);
	m_keyboard->CreateKeyboard(m_input->m_ptr, hwnd);
	m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE /*| DISCL_NOWINKEY*/);

	m_mouse.reset(new Mouse);
	m_mouse->CreateMouse(m_input->m_ptr, hwnd);
	m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	DIJOYCONFIG PreferredJoyCfg = { 0 };
	DI_ENUM_CONTEXT enumContext;
	enumContext.pPreferredJoyCfg = &PreferredJoyCfg;
	enumContext.bPreferredJoyCfgValid = false;

	HRESULT hr;
	CComPtr<IDirectInputJoyConfig8> pJoyConfig;
	if (FAILED(hr = m_input->m_ptr->QueryInterface(IID_IDirectInputJoyConfig8, (void**)&pJoyConfig)))
	{
		THROW_DINPUTEXCEPTION(hr);
	}

	PreferredJoyCfg.dwSize = sizeof(PreferredJoyCfg);
	if (SUCCEEDED(pJoyConfig->GetConfig(0, &PreferredJoyCfg, DIJC_GUIDINSTANCE))) // This function is expected to fail if no Joystick is attached
		enumContext.bPreferredJoyCfgValid = true;
	pJoyConfig.Release();

	enumContext.input = m_input->m_ptr;
	enumContext.hwnd = hwnd;
	enumContext.min_x = -255;
	enumContext.max_x =  255;
	enumContext.min_y = -255;
	enumContext.max_y =  255;
	enumContext.min_z = -255;
	enumContext.max_z =  255;
	enumContext.dead_zone = m_JoystickAxisDeadZone;
	_ASSERT(m_JoystickAxisDeadZone >= 0 && m_JoystickAxisDeadZone <= 10000);
	m_input->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, &enumContext, DIEDFL_ATTACHEDONLY);
	if (enumContext.joystick)
	{
		m_joystick = enumContext.joystick;
		m_joystick->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

		// Enumerate the Joystick objects. The callback function enabled user
		// interface elements for objects that are found, and sets the min/max
		// values property for discovered axes.
		if (FAILED(hr = m_joystick->m_ptr->EnumObjects(EnumObjectsCallback,
			&enumContext, DIDFT_ALL)))
			THROW_DINPUTEXCEPTION(hr);
	}

	//::GetCursorPos(&m_MousePos);
	//::ScreenToClient(hwnd, &m_MousePos);
}

void InputMgr::Destroy(void)
{
	m_keyboard.reset();
	m_mouse.reset();
	m_joystick.reset();
	m_input.reset();
}

bool InputMgr::Capture(double fTime, float fElapsedTime)
{
	if (!m_keyboard->Capture())
	{
		//return false;
	}

	if (!m_mouse->Capture())
	{
		//return false;
	}

	if (m_joystick)
	{
		if (!m_joystick->Capture())
		{
			//return false;
		}
	}
	return true;
}
//
//bool InputMgr::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	switch(uMsg)
//	{
//	case WM_KEYDOWN:
//		if (!(0x40000000 & lParam) && m_KeyPressedEvent)
//		{
//			KeyboardEventArg arg(wParam);
//			m_KeyPressedEvent(&arg);
//			return arg.handled;
//		}
//		break;
//	case WM_SYSKEYDOWN:
//		if (!(0x40000000 & lParam) && m_KeyPressedEvent)
//		{
//			KeyboardEventArg arg(wParam);
//			m_KeyPressedEvent(&arg);
//			return arg.handled;
//		}
//		break;
//	case WM_KEYUP:
//		if (m_KeyReleasedEvent)
//		{
//			KeyboardEventArg arg(wParam);
//			m_KeyReleasedEvent(&arg);
//			return arg.handled;
//		}
//		break;
//	case WM_SYSKEYUP:
//		if (m_KeyReleasedEvent)
//		{
//			KeyboardEventArg arg(wParam);
//			m_KeyReleasedEvent(&arg);
//			return arg.handled;
//		}
//		break;
//	case WM_MOUSEMOVE:
//		if (m_MouseMovedEvent)
//		{
//			CPoint OldMousePos(m_MousePos);
//			m_MousePos.SetPoint(LOWORD(lParam), HIWORD(lParam));
//			MouseMoveEventArg arg(m_MousePos.x - OldMousePos.x, m_MousePos.y - OldMousePos.y, 0);
//			m_MouseMovedEvent(&arg);
//			return arg.handled;
//		}
//		break;
//	case WM_LBUTTONDOWN:
//		if (m_MousePressedEvent)
//		{
//			MouseBtnEventArg arg(0);
//			m_MousePressedEvent(&arg);
//			return arg.handled;
//		}
//		break;
//	case WM_LBUTTONUP:
//		if (m_MouseReleasedEvent)
//		{
//			MouseBtnEventArg arg(0);
//			m_MouseReleasedEvent(&arg);
//			return arg.handled;
//		}
//		break;
//	case WM_MBUTTONDOWN:
//		if (m_MousePressedEvent)
//		{
//			MouseBtnEventArg arg(2);
//			m_MousePressedEvent(&arg);
//			return arg.handled;
//		}
//		break;
//	case WM_MBUTTONUP:
//		if (m_MouseReleasedEvent)
//		{
//			MouseBtnEventArg arg(2);
//			m_MouseReleasedEvent(&arg);
//			return arg.handled;
//		}
//		break;
//	case WM_RBUTTONDOWN:
//		if (m_MousePressedEvent)
//		{
//			MouseBtnEventArg arg(1);
//			m_MousePressedEvent(&arg);
//			return arg.handled;
//		}
//		break;
//	case WM_RBUTTONUP:
//		if (m_MouseReleasedEvent)
//		{
//			MouseBtnEventArg arg(1);
//			m_MouseReleasedEvent(&arg);
//			return arg.handled;
//		}
//		break;
//	case WM_MOUSEWHEEL:
//		if (m_MouseMovedEvent)
//		{
//			MouseMoveEventArg arg(0, 0, (short)HIWORD(wParam) / WHEEL_DELTA);
//			m_MouseMovedEvent(&arg);
//			return arg.handled;
//		}
//		break;
//	}
//	return false;
//}

BOOL CALLBACK InputMgr::EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
	DI_ENUM_CONTEXT* pEnumContext = static_cast<DI_ENUM_CONTEXT*>(pContext);
	_ASSERT(pEnumContext);

	// Skip anything other than the perferred Joystick device as defined by the control panel.  
	// Instead you could store all the enumerated Joysticks and let the user pick.
	if (pEnumContext->bPreferredJoyCfgValid &&
		!IsEqualGUID(pdidInstance->guidInstance, pEnumContext->pPreferredJoyCfg->guidInstance))
		return DIENUM_CONTINUE;

	// Obtain an interface to the enumerated Joystick.
	JoystickPtr joystick(new Joystick);
	joystick->CreateJoystick(
		pEnumContext->input, pEnumContext->hwnd, pdidInstance->guidInstance, pEnumContext->min_x, pEnumContext->max_x, pEnumContext->min_y, pEnumContext->max_y, pEnumContext->min_z, pEnumContext->max_z, pEnumContext->dead_zone);

	// Stop enumeration. Note: we're just taking the first Joystick we get. You
	// could store all the enumerated Joysticks and let the user pick.
	_ASSERT(!pEnumContext->joystick);
	pEnumContext->joystick = joystick;
	return DIENUM_STOP;
}

BOOL CALLBACK InputMgr::EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext)
{
	DI_ENUM_CONTEXT* pEnumContext = static_cast<DI_ENUM_CONTEXT*>(pContext);
	_ASSERT(pEnumContext);

	// For axes that are returned, set the DIPROP_RANGE property for the
	// enumerated axis in order to scale min/max values.
	if (pdidoi->dwType & DIDFT_AXIS)
	{
		DIPROPDWORD diprg;
		diprg.diph.dwSize = sizeof(DIPROPDWORD);
		diprg.diph.dwHeaderSize = sizeof(diprg.diph);
		diprg.diph.dwHow = DIPH_BYID;
		diprg.diph.dwObj = pdidoi->dwType; // Specify the enumerated axis
		diprg.dwData = pEnumContext->dead_zone;

		// Set the range for the axis
		pEnumContext->joystick->SetProperty(DIPROP_DEADZONE, &diprg.diph);
	}
	return DIENUM_CONTINUE;
}

void InputMgr::BindKey(DWORD Key, KeyType type, int id)
{
	if (Key < m_BindKeys.size())
	{
		KeyPairListList::value_type::value_type value = std::make_pair(type, id);
		_ASSERT(std::find(m_BindKeys[Key].begin(), m_BindKeys[Key].end(), value) == m_BindKeys[Key].end());
		m_BindKeys[Key].push_back(value);
	}
}

void InputMgr::UnbindKey(DWORD Key, KeyType type, int id)
{
	if (Key < m_BindKeys.size())
	{
		KeyPairListList::value_type::iterator value_iter = std::find(m_BindKeys[Key].begin(), m_BindKeys[Key].end(), std::make_pair(type, id));
		_ASSERT(value_iter != m_BindKeys[Key].end());
		m_BindKeys[Key].erase(value_iter);
	}
}

LONG InputMgr::GetKeyAxisRaw(DWORD Key) const
{
	if (Key < m_BindKeys.size())
	{
		KeyPairListList::value_type::const_iterator value_iter = m_BindKeys[Key].begin();
		for (; value_iter != m_BindKeys[Key].end(); value_iter++)
		{
			switch (value_iter->first)
			{
			case KeyboardButton:
				if (value_iter->second >= 0 && value_iter->second < _countof(m_keyboard->m_State))
				{
					if (m_keyboard->IsKeyDown(value_iter->second))
						return 65536;
				}
				break;
			case KeyboardNegativeButton:
				if (value_iter->second >= 0 && value_iter->second < _countof(m_keyboard->m_State))
				{
					if (m_keyboard->IsKeyDown(value_iter->second))
						return 0;
				}
				break;
			case MouseMove:
				switch (value_iter->second)
				{
				case 0:
					if (m_mouse->GetX() != 0)
						return 32767 + m_mouse->GetX() * m_MouseMoveAxisCoef;
					break;
				case 1:
					if (m_mouse->GetY() != 0)
						return 32767 + m_mouse->GetY() * m_MouseMoveAxisCoef;
					break;
				case 2:
					if (m_mouse->GetZ() != 0)
						return 32767 + m_mouse->GetZ();
					break;
				}
				break;
			case MouseButton:
				if (value_iter->second >= 0 && value_iter->second < _countof(m_mouse->m_State.rgbButtons))
				{
					if (m_mouse->IsButtonDown(value_iter->second))
						return 65535;
				}
				break;
			case JoystickAxis:
				if (m_joystick)
				{
					switch (value_iter->second)
					{
					case 0:
						if (m_joystick->GetX() < 32767 || m_joystick->GetX() > 32767)
							return m_joystick->GetX();
						break;
					case 1:
						if (m_joystick->GetY() < 32767 || m_joystick->GetY() > 32767)
							return m_joystick->GetY();
						break;
					case 2:
						if (m_joystick->GetZ() < 32767 || m_joystick->GetZ() > 32767)
							return m_joystick->GetZ();
						break;
					case 3:
						if (m_joystick->GetRx() < 32767 || m_joystick->GetRx() > 32767)
							return m_joystick->GetRx();
						break;
					case 4:
						if (m_joystick->GetRy() < 32767 || m_joystick->GetRy() > 32767)
							return m_joystick->GetRy();
						break;
					case 5:
						if (m_joystick->GetRz() < 32767 || m_joystick->GetRz() > 32767)
							return m_joystick->GetRz();
						break;
					}
				}
				break;
			case JoystickNegativeAxis:
				if (m_joystick)
				{
					switch (value_iter->second)
					{
					case 0:
						if (m_joystick->GetX() < 32767 || m_joystick->GetX() > 32767)
							return m_joystick->GetX();
						break;
					case 1:
						if (m_joystick->GetY() < 32767 || m_joystick->GetY() > 32767)
							return m_joystick->GetY();
						break;
					case 2:
						if (m_joystick->GetZ() < 32767 || m_joystick->GetZ() > 32767)
							return m_joystick->GetZ();
						break;
					case 3:
						if (m_joystick->GetRx() < 32767 || m_joystick->GetRx() > 32767)
							return m_joystick->GetRx();
						break;
					case 4:
						if (m_joystick->GetRy() < 32767 || m_joystick->GetRy() > 32767)
							return m_joystick->GetRy();
						break;
					case 5:
						if (m_joystick->GetRz() < 32767 || m_joystick->GetRz() > 32767)
							return m_joystick->GetRz();
						break;
					}
				}
				break;
			case JoystickPov:
				if (m_joystick)
				{
					switch (value_iter->second)
					{
					case 0: // Horizontal
						switch (m_joystick->m_State.rgdwPOV[0])
						{
						case JP_North:
							return 32767;
						case JP_NorthEast:
							return 54612;
						case JP_East:
							return 65535;
						case JP_SouthEast:
							return 54612;
						case JP_South:
							return 32767;
						case JP_SouthWest:
							return 10922;
						case JP_West:
							return 0;
						case JP_NorthWest:
							return 10922;
						}
						break;
					case 1: // Vertical
						switch (m_joystick->m_State.rgdwPOV[0])
						{
						case JP_North:
							return 0;
						case JP_NorthEast:
							return 10922;
						case JP_East:
							return 32767;
						case JP_SouthEast:
							return 54612;
						case JP_South:
							return 65535;
						case JP_SouthWest:
							return 54612;
						case JP_West:
							return 32767;
						case JP_NorthWest:
							return 10922;
						}
						break;
					}
				}
				break;
			case JoystickNegativePov:
				if (m_joystick)
				{
					switch (value_iter->second)
					{
					case 0: // Horizontal
						switch (m_joystick->m_State.rgdwPOV[0])
						{
						case JP_North:
							return 32767;
						case JP_NorthEast:
							return 54612;
						case JP_East:
							return 65535;
						case JP_SouthEast:
							return 54612;
						case JP_South:
							return 32767;
						case JP_SouthWest:
							return 10922;
						case JP_West:
							return 0;
						case JP_NorthWest:
							return 10922;
						}
						break;
					case 1: // Vertical
						switch (m_joystick->m_State.rgdwPOV[0])
						{
						case JP_North:
							return 0;
						case JP_NorthEast:
							return 10922;
						case JP_East:
							return 32767;
						case JP_SouthEast:
							return 54612;
						case JP_South:
							return 65535;
						case JP_SouthWest:
							return 54612;
						case JP_West:
							return 32767;
						case JP_NorthWest:
							return 10922;
						}
						break;
					}
				}
				break;
			case JoystickButton:
				if (m_joystick && value_iter->second >= 0 && value_iter->second < _countof(m_joystick->m_State.rgbButtons))
				{
					if (m_joystick->IsButtonDown(value_iter->second))
						return 65535;
				}
				break;
			}
		}
	}
	return 32767;
}

static bool _isaxisdown(LONG val)
{
	return val > 54612;
}

static bool _isnegativeaxisdown(LONG val)
{
	return val < 10922;
}

static bool _ispovnorthdown(DWORD val)
{
	switch (val)
	{
	case JP_North:
		return true;
	case JP_NorthEast:
		return true;
	case JP_East:
		return false;
	case JP_SouthEast:
		return false;
	case JP_South:
		return false;
	case JP_SouthWest:
		return false;
	case JP_West:
		return false;
	case JP_NorthWest:
		return true;
	}
	return false;
}

static bool _ispoveastdown(DWORD val)
{
	switch (val)
	{
	case JP_North:
		return false;
	case JP_NorthEast:
		return true;
	case JP_East:
		return true;
	case JP_SouthEast:
		return true;
	case JP_South:
		return false;
	case JP_SouthWest:
		return false;
	case JP_West:
		return false;
	case JP_NorthWest:
		return false;
	}
	return false;
}

static bool _ispovsouthdown(DWORD val)
{
	switch (val)
	{
	case JP_North:
		return false;
	case JP_NorthEast:
		return false;
	case JP_East:
		return false;
	case JP_SouthEast:
		return true;
	case JP_South:
		return true;
	case JP_SouthWest:
		return true;
	case JP_West:
		return false;
	case JP_NorthWest:
		return false;
	}
	return false;
}

static bool _ispovwestdown(DWORD val)
{
	switch (val)
	{
	case JP_North:
		return false;
	case JP_NorthEast:
		return false;
	case JP_East:
		return false;
	case JP_SouthEast:
		return false;
	case JP_South:
		return false;
	case JP_SouthWest:
		return true;
	case JP_West:
		return true;
	case JP_NorthWest:
		return true;
	}
	return false;
}

bool InputMgr::IsKeyDown(DWORD Key) const
{
	if (Key < m_BindKeys.size())
	{
		KeyPairListList::value_type::const_iterator value_iter = m_BindKeys[Key].begin();
		for (; value_iter != m_BindKeys[Key].end(); value_iter++)
		{
			switch (value_iter->first)
			{
			case KeyboardButton:
				if (value_iter->second >= 0 && value_iter->second < _countof(m_keyboard->m_State))
				{
					if (m_keyboard->IsKeyDown(value_iter->second))
						return true;
				}
				break;
			case KeyboardNegativeButton:
				if (value_iter->second >= 0 && value_iter->second < _countof(m_keyboard->m_State))
				{
					if (m_keyboard->IsKeyDown(value_iter->second))
						return true;
				}
				break;
			case MouseMove:
				switch (value_iter->second)
				{
				case 0:
					if (m_mouse->GetX() != 0)
						return true;
					break;
				case 1:
					if (m_mouse->GetY() != 0)
						return true;
					break;
				case 2:
					if (m_mouse->GetZ() != 0)
						return true;
					break;
				}
				break;
			case MouseButton:
				if (value_iter->second >= 0 && value_iter->second < _countof(m_mouse->m_State.rgbButtons))
				{
					if (m_mouse->IsButtonDown(value_iter->second))
						return true;
				}
				break;
			case JoystickAxis:
				if (m_joystick)
				{
					switch (value_iter->second)
					{
					case 0:
						if (_isaxisdown(m_joystick->GetX()))
							return true;
						break;
					case 1:
						if (_isaxisdown(m_joystick->GetY()))
							return true;
						break;
					case 2:
						if (_isaxisdown(m_joystick->GetZ()))
							return true;
						break;
					case 3:
						if (_isaxisdown(m_joystick->GetRx()))
							return true;
						break;
					case 4:
						if (_isaxisdown(m_joystick->GetRy()))
							return true;
						break;
					case 5:
						if (_isaxisdown(m_joystick->GetRz()))
							return true;
						break;
					}
				}
				break;
			case JoystickNegativeAxis:
				if (m_joystick)
				{
					switch (value_iter->second)
					{
					case 0:
						if (_isnegativeaxisdown(m_joystick->GetX()))
							return true;
						break;
					case 1:
						if (_isnegativeaxisdown(m_joystick->GetY()))
							return true;
						break;
					case 2:
						if (_isnegativeaxisdown(m_joystick->GetZ()))
							return true;
						break;
					case 3:
						if (_isnegativeaxisdown(m_joystick->GetRx()))
							return true;
						break;
					case 4:
						if (_isnegativeaxisdown(m_joystick->GetRy()))
							return true;
						break;
					case 5:
						if (_isnegativeaxisdown(m_joystick->GetRz()))
							return true;
						break;
					}
				}
				break;
			case JoystickPov:
				if (m_joystick)
				{
					switch (value_iter->second)
					{
					case 0: // Horizontal
						if (_ispoveastdown(m_joystick->m_State.rgdwPOV[0]))
							return true;
						break;
					case 1: // Vertical
						if (_ispovsouthdown(m_joystick->m_State.rgdwPOV[0]))
							return true;
						break;
					}
				}
				break;
			case JoystickNegativePov:
				if (m_joystick)
				{
					switch (value_iter->second)
					{
					case 0: // Horizontal
						if (_ispovwestdown(m_joystick->m_State.rgdwPOV[0]))
							return true;
						break;
					case 1: // Vertical
						if (_ispovnorthdown(m_joystick->m_State.rgdwPOV[0]))
							return true;
						break;
					}
				}
			break;
			case JoystickButton:
				if (m_joystick && value_iter->second >= 0 && value_iter->second < _countof(m_joystick->m_State.rgbButtons))
				{
					if (m_joystick->IsButtonDown(value_iter->second))
						return true;
				}
				break;
			}
		}
	}
	return false;
}

static bool _isaxispress(LONG lastval, LONG val)
{
	if (lastval == 32767 && _isaxisdown(val))
	{
		return true;
	}
	return false;
}

static bool _isnegativeaxispress(LONG lastval, LONG val)
{
	if (lastval == 32767 && _isnegativeaxisdown(val))
	{
		return true;
	}
	return false;
}

bool InputMgr::IsKeyPressRaw(DWORD Key) const
{
	if (Key < m_BindKeys.size())
	{
		KeyPairListList::value_type::const_iterator value_iter = m_BindKeys[Key].begin();
		for (; value_iter != m_BindKeys[Key].end(); value_iter++)
		{
			switch (value_iter->first)
			{
			case KeyboardButton:
				if (value_iter->second >= 0 && value_iter->second < _countof(m_keyboard->m_State))
				{
					if (m_keyboard->IsKeyPress(value_iter->second))
						return true;
				}
				break;
			case KeyboardNegativeButton:
				if (value_iter->second >= 0 && value_iter->second < _countof(m_keyboard->m_State))
				{
					if (m_keyboard->IsKeyPress(value_iter->second))
						return true;
				}
				break;
			case MouseMove:
				switch (value_iter->second)
				{
				case 0:
					if (m_mouse->m_LastFrameState.lX == 0 && m_mouse->GetX() != 0)
						return true;
					break;
				case 1:
					if (m_mouse->m_LastFrameState.lY == 0 && m_mouse->GetY() != 0)
						return true;
					break;
				case 2:
					if (m_mouse->m_LastFrameState.lZ == 0 && m_mouse->GetZ() != 0)
						return true;
					break;
				}
				break;
			case MouseButton:
				if (value_iter->second >= 0 && value_iter->second < _countof(m_mouse->m_State.rgbButtons))
				{
					if (m_mouse->IsButtonPress(value_iter->second))
						return true;
				}
				break;
			case JoystickAxis:
				if (m_joystick)
				{
					switch (value_iter->second)
					{
					case 0:
						if (_isaxispress(m_joystick->m_LastFrameState.lX, m_joystick->GetX()))
							return true;
						break;
					case 1:
						if (_isaxispress(m_joystick->m_LastFrameState.lY, m_joystick->GetY()))
							return true;
						break;
					case 2:
						if (_isaxispress(m_joystick->m_LastFrameState.lZ, m_joystick->GetZ()))
							return true;
						break;
					case 3:
						if (_isaxispress(m_joystick->m_LastFrameState.lRx, m_joystick->GetRx()))
							return true;
						break;
					case 4:
						if (_isaxispress(m_joystick->m_LastFrameState.lRy, m_joystick->GetRy()))
							return true;
						break;
					case 5:
						if (_isaxispress(m_joystick->m_LastFrameState.lRz, m_joystick->GetRz()))
							return true;
						break;
					}
				}
				break;
			case JoystickNegativeAxis:
				if (m_joystick)
				{
					switch (value_iter->second)
					{
					case 0:
						if (_isnegativeaxispress(m_joystick->m_LastFrameState.lX, m_joystick->GetX()))
							return true;
						break;
					case 1:
						if (_isnegativeaxispress(m_joystick->m_LastFrameState.lY, m_joystick->GetY()))
							return true;
						break;
					case 2:
						if (_isnegativeaxispress(m_joystick->m_LastFrameState.lZ, m_joystick->GetZ()))
							return true;
						break;
					case 3:
						if (_isnegativeaxispress(m_joystick->m_LastFrameState.lRx, m_joystick->GetRx()))
							return true;
						break;
					case 4:
						if (_isnegativeaxispress(m_joystick->m_LastFrameState.lRy, m_joystick->GetRy()))
							return true;
						break;
					case 5:
						if (_isnegativeaxispress(m_joystick->m_LastFrameState.lRz, m_joystick->GetRz()))
							return true;
						break;
					}
				}
				break;
			case JoystickPov:
				if (m_joystick)
				{
					switch (value_iter->second)
					{
					case 0: // Horizontal
						if (!_ispoveastdown(m_joystick->m_LastFrameState.rgdwPOV[0]) && _ispoveastdown(m_joystick->m_State.rgdwPOV[0]))
							return true;
						break;
					case 1: // Vertical
						if (!_ispovsouthdown(m_joystick->m_LastFrameState.rgdwPOV[0]) && _ispovsouthdown(m_joystick->m_State.rgdwPOV[0]))
							return true;
						break;
					}
				}
				break;
			case JoystickNegativePov:
				if (m_joystick)
				{
					switch (value_iter->second)
					{
					case 0: // Horizontal
						if (!_ispovwestdown(m_joystick->m_LastFrameState.rgdwPOV[0]) && _ispovwestdown(m_joystick->m_State.rgdwPOV[0]))
							return true;
						break;
					case 1: // Vertical
						if (!_ispovnorthdown(m_joystick->m_LastFrameState.rgdwPOV[0]) && _ispovnorthdown(m_joystick->m_State.rgdwPOV[0]))
							return true;
						break;
					}
				}
				break;
			case JoystickButton:
				if (m_joystick && value_iter->second >= 0 && value_iter->second < _countof(m_joystick->m_State.rgbButtons))
				{
					if (m_joystick->IsButtonPress(value_iter->second))
						return true;
				}
				break;
			}
		}
	}
	return false;
}

bool InputMgr::IsKeyPress(DWORD Key)
{
	if (IsKeyPressRaw(Key))
	{
		m_LastPressKey = Key;
		m_LastPressTime = timeGetTime();
		m_LastPressCount = 1;
		return true;
	}
	else if (m_LastPressKey == Key && IsKeyDown(Key))
	{
		if (m_LastPressCount <= 1)
		{
			DWORD dwAbsoluteTime = timeGetTime();
			if (330 < dwAbsoluteTime - m_LastPressTime)
			{
				m_LastPressTime = dwAbsoluteTime;
				m_LastPressCount++;
				return true;
			}
		}
		else
		{
			DWORD dwAbsoluteTime = timeGetTime();
			if (50 < dwAbsoluteTime - m_LastPressTime)
			{
				m_LastPressTime = dwAbsoluteTime;
				m_LastPressCount++;
				return true;
			}
		}
	}
	return false;
}

static bool _isaxisrelease(LONG lastval, LONG val)
{
	if (lastval == 65535 && val < 43690)
	{
		return true;
	}
	return true;
}

static bool _isnegativeaxisrelease(LONG lastval, LONG val)
{
	if (lastval == 0 && val > 21845)
	{
		return true;
	}
	return false;
}

bool InputMgr::IsKeyRelease(DWORD Key) const
{
	if (Key < m_BindKeys.size())
	{
		KeyPairListList::value_type::const_iterator value_iter = m_BindKeys[Key].begin();
		for (; value_iter != m_BindKeys[Key].end(); value_iter++)
		{
			switch (value_iter->first)
			{
			case KeyboardButton:
				if (value_iter->second >= 0 && value_iter->second < _countof(m_keyboard->m_State))
				{
					if (m_keyboard->IsKeyRelease(value_iter->second))
						return true;
				}
				break;
			case KeyboardNegativeButton:
				if (value_iter->second >= 0 && value_iter->second < _countof(m_keyboard->m_State))
				{
					if (m_keyboard->IsKeyRelease(value_iter->second))
						return true;
				}
				break;
			case MouseMove:
				switch (value_iter->second)
				{
				case 0:
					if (m_mouse->m_LastFrameState.lX != 0 && m_mouse->GetX() == 0)
						return true;
					break;
				case 1:
					if (m_mouse->m_LastFrameState.lY != 0 && m_mouse->GetY() == 0)
						return true;
					break;
				case 2:
					if (m_mouse->m_LastFrameState.lZ != 0 && m_mouse->GetZ() == 0)
						return true;
					break;
				}
				break;
			case MouseButton:
				if (value_iter->second >= 0 && value_iter->second < _countof(m_mouse->m_State.rgbButtons))
				{
					if (m_mouse->IsButtonRelease(value_iter->second))
						return true;
				}
				break;
			case JoystickAxis:
				if (m_joystick)
				{
					switch (value_iter->second)
					{
					case 0:
						if (_isaxisrelease(m_joystick->m_LastFrameState.lX, m_joystick->GetX()))
							return true;
						break;
					case 1:
						if (_isaxisrelease(m_joystick->m_LastFrameState.lY, m_joystick->GetY()))
							return true;
						break;
					case 2:
						if (_isaxisrelease(m_joystick->m_LastFrameState.lZ, m_joystick->GetZ()))
							return true;
						break;
					case 3:
						if (_isaxisrelease(m_joystick->m_LastFrameState.lRx, m_joystick->GetRx()))
							return true;
						break;
					case 4:
						if (_isaxisrelease(m_joystick->m_LastFrameState.lRy, m_joystick->GetRy()))
							return true;
						break;
					case 5:
						if (_isaxisrelease(m_joystick->m_LastFrameState.lRz, m_joystick->GetRz()))
							return true;
						break;
					}
				}
				break;
			case JoystickNegativeAxis:
				if (m_joystick)
				{
					switch (value_iter->second)
					{
					case 0:
						if (_isnegativeaxisrelease(m_joystick->m_LastFrameState.lX, m_joystick->GetX()))
							return true;
						break;
					case 1:
						if (_isnegativeaxisrelease(m_joystick->m_LastFrameState.lY, m_joystick->GetY()))
							return true;
						break;
					case 2:
						if (_isnegativeaxisrelease(m_joystick->m_LastFrameState.lZ, m_joystick->GetZ()))
							return true;
						break;
					case 3:
						if (_isnegativeaxisrelease(m_joystick->m_LastFrameState.lRx, m_joystick->GetRx()))
							return true;
						break;
					case 4:
						if (_isnegativeaxisrelease(m_joystick->m_LastFrameState.lRy, m_joystick->GetRy()))
							return true;
						break;
					case 5:
						if (_isnegativeaxisrelease(m_joystick->m_LastFrameState.lRz, m_joystick->GetRz()))
							return true;
						break;
					}
				}
				break;
			case JoystickPov:
				if (m_joystick)
				{
					switch (value_iter->second)
					{
					case 0: // Horizontal
						if (_ispoveastdown(m_joystick->m_LastFrameState.rgdwPOV[0]) && !_ispoveastdown(m_joystick->m_State.rgdwPOV[0]))
							return true;
						break;
					case 1: // Vertical
						if (_ispovsouthdown(m_joystick->m_LastFrameState.rgdwPOV[0]) && !_ispovsouthdown(m_joystick->m_State.rgdwPOV[0]))
							return true;
						break;
					}
				}
				break;
			case JoystickNegativePov:
				if (m_joystick)
				{
					switch (value_iter->second)
					{
					case 0: // Horizontal
						if (_ispovwestdown(m_joystick->m_LastFrameState.rgdwPOV[0]) && !_ispovwestdown(m_joystick->m_State.rgdwPOV[0]))
							return true;
						break;
					case 1: // Vertical
						if (_ispovnorthdown(m_joystick->m_LastFrameState.rgdwPOV[0]) && !_ispovnorthdown(m_joystick->m_State.rgdwPOV[0]))
							return true;
						break;
					}
				}
				break;
			case JoystickButton:
				if (m_joystick && value_iter->second >= 0 && value_iter->second < _countof(m_joystick->m_State.rgbButtons))
				{
					if (m_joystick->IsButtonRelease(value_iter->second))
						return true;
				}
				break;
			}
		}
	}
	return false;
}
