// Copyright (c) 2011-2024 tangyin025
// License: MIT
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

Keyboard::Keyboard(void)
{
	ZeroMemory(&m_State, sizeof(m_State));
}

const char * Keyboard::TranslateKeyCode(KeyCode kc)
{
	switch (kc)
	{
	case KC_UNASSIGNED: return "KC_UNASSIGNED";
	case KC_ESCAPE: return "KC_ESCAPE";
	case KC_1: return "KC_1";
	case KC_2: return "KC_2";
	case KC_3: return "KC_3";
	case KC_4: return "KC_4";
	case KC_5: return "KC_5";
	case KC_6: return "KC_6";
	case KC_7: return "KC_7";
	case KC_8: return "KC_8";
	case KC_9: return "KC_9";
	case KC_0: return "KC_0";
	case KC_MINUS: return "KC_MINUS";
	case KC_EQUALS: return "KC_EQUALS";
	case KC_BACK: return "KC_BACK";
	case KC_TAB: return "KC_TAB";
	case KC_Q: return "KC_Q";
	case KC_W: return "KC_W";
	case KC_E: return "KC_E";
	case KC_R: return "KC_R";
	case KC_T: return "KC_T";
	case KC_Y: return "KC_Y";
	case KC_U: return "KC_U";
	case KC_I: return "KC_I";
	case KC_O: return "KC_O";
	case KC_P: return "KC_P";
	case KC_LBRACKET: return "KC_LBRACKET";
	case KC_RBRACKET: return "KC_RBRACKET";
	case KC_RETURN: return "KC_RETURN";
	case KC_LCONTROL: return "KC_LCONTROL";
	case KC_A: return "KC_A";
	case KC_S: return "KC_S";
	case KC_D: return "KC_D";
	case KC_F: return "KC_F";
	case KC_G: return "KC_G";
	case KC_H: return "KC_H";
	case KC_J: return "KC_J";
	case KC_K: return "KC_K";
	case KC_L: return "KC_L";
	case KC_SEMICOLON: return "KC_SEMICOLON";
	case KC_APOSTROPHE: return "KC_APOSTROPHE";
	case KC_GRAVE: return "KC_GRAVE";
	case KC_LSHIFT: return "KC_LSHIFT";
	case KC_BACKSLASH: return "KC_BACKSLASH";
	case KC_Z: return "KC_Z";
	case KC_X: return "KC_X";
	case KC_C: return "KC_C";
	case KC_V: return "KC_V";
	case KC_B: return "KC_B";
	case KC_N: return "KC_N";
	case KC_M: return "KC_M";
	case KC_COMMA: return "KC_COMMA";
	case KC_PERIOD: return "KC_PERIOD";
	case KC_SLASH: return "KC_SLASH";
	case KC_RSHIFT: return "KC_RSHIFT";
	case KC_MULTIPLY: return "KC_MULTIPLY";
	case KC_LMENU: return "KC_LMENU";
	case KC_SPACE: return "KC_SPACE";
	case KC_CAPITAL: return "KC_CAPITAL";
	case KC_F1: return "KC_F1";
	case KC_F2: return "KC_F2";
	case KC_F3: return "KC_F3";
	case KC_F4: return "KC_F4";
	case KC_F5: return "KC_F5";
	case KC_F6: return "KC_F6";
	case KC_F7: return "KC_F7";
	case KC_F8: return "KC_F8";
	case KC_F9: return "KC_F9";
	case KC_F10: return "KC_F10";
	case KC_NUMLOCK: return "KC_NUMLOCK";
	case KC_SCROLL: return "KC_SCROLL";
	case KC_NUMPAD7: return "KC_NUMPAD7";
	case KC_NUMPAD8: return "KC_NUMPAD8";
	case KC_NUMPAD9: return "KC_NUMPAD9";
	case KC_SUBTRACT: return "KC_SUBTRACT";
	case KC_NUMPAD4: return "KC_NUMPAD4";
	case KC_NUMPAD5: return "KC_NUMPAD5";
	case KC_NUMPAD6: return "KC_NUMPAD6";
	case KC_ADD: return "KC_ADD";
	case KC_NUMPAD1: return "KC_NUMPAD1";
	case KC_NUMPAD2: return "KC_NUMPAD2";
	case KC_NUMPAD3: return "KC_NUMPAD3";
	case KC_NUMPAD0: return "KC_NUMPAD0";
	case KC_DECIMAL: return "KC_DECIMAL";
	case KC_OEM_102: return "KC_OEM_102";
	case KC_F11: return "KC_F11";
	case KC_F12: return "KC_F12";
	case KC_F13: return "KC_F13";
	case KC_F14: return "KC_F14";
	case KC_F15: return "KC_F15";
	case KC_KANA: return "KC_KANA";
	case KC_ABNT_C1: return "KC_ABNT_C1";
	case KC_CONVERT: return "KC_CONVERT";
	case KC_NOCONVERT: return "KC_NOCONVERT";
	case KC_YEN: return "KC_YEN";
	case KC_ABNT_C2: return "KC_ABNT_C2";
	case KC_NUMPADEQUALS: return "KC_NUMPADEQUALS";
	case KC_PREVTRACK: return "KC_PREVTRACK";
	case KC_AT: return "KC_AT";
	case KC_COLON: return "KC_COLON";
	case KC_UNDERLINE: return "KC_UNDERLINE";
	case KC_KANJI: return "KC_KANJI";
	case KC_STOP: return "KC_STOP";
	case KC_AX: return "KC_AX";
	case KC_UNLABELED: return "KC_UNLABELED";
	case KC_NEXTTRACK: return "KC_NEXTTRACK";
	case KC_NUMPADENTER: return "KC_NUMPADENTER";
	case KC_RCONTROL: return "KC_RCONTROL";
	case KC_MUTE: return "KC_MUTE";
	case KC_CALCULATOR: return "KC_CALCULATOR";
	case KC_PLAYPAUSE: return "KC_PLAYPAUSE";
	case KC_MEDIASTOP: return "KC_MEDIASTOP";
	case KC_VOLUMEDOWN: return "KC_VOLUMEDOWN";
	case KC_VOLUMEUP: return "KC_VOLUMEUP";
	case KC_WEBHOME: return "KC_WEBHOME";
	case KC_NUMPADCOMMA: return "KC_NUMPADCOMMA";
	case KC_DIVIDE: return "KC_DIVIDE";
	case KC_SYSRQ: return "KC_SYSRQ";
	case KC_RMENU: return "KC_RMENU";
	case KC_PAUSE: return "KC_PAUSE";
	case KC_HOME: return "KC_HOME";
	case KC_UP: return "KC_UP";
	case KC_PGUP: return "KC_PGUP";
	case KC_LEFT: return "KC_LEFT";
	case KC_RIGHT: return "KC_RIGHT";
	case KC_END: return "KC_END";
	case KC_DOWN: return "KC_DOWN";
	case KC_PGDOWN: return "KC_PGDOWN";
	case KC_INSERT: return "KC_INSERT";
	case KC_DELETE: return "KC_DELETE";
	case KC_LWIN: return "KC_LWIN";
	case KC_RWIN: return "KC_RWIN";
	case KC_APPS: return "KC_APPS";
	case KC_POWER: return "KC_POWER";
	case KC_SLEEP: return "KC_SLEEP";
	case KC_WAKE: return "KC_WAKE";
	case KC_WEBSEARCH: return "KC_WEBSEARCH";
	case KC_WEBFAVORITES: return "KC_WEBFAVORITES";
	case KC_WEBREFRESH: return "KC_WEBREFRESH";
	case KC_WEBSTOP: return "KC_WEBSTOP";
	case KC_WEBFORWARD: return "KC_WEBFORWARD";
	case KC_WEBBACK: return "KC_WEBBACK";
	case KC_MYCOMPUTER: return "KC_MYCOMPUTER";
	case KC_MAIL: return "KC_MAIL";
	case KC_MEDIASELECT: return "KC_MEDIASELECT";
	}
	return "unknown key code";
}

const char * Keyboard::TranslateVirtualKey(DWORD vk)
{
	switch(vk)
	{
	case VK_LBUTTON: return "VK_LBUTTON";
	case VK_RBUTTON: return "VK_RBUTTON";
	case VK_CANCEL: return "VK_CANCEL";
	case VK_MBUTTON: return "VK_MBUTTON";
#if(_WIN32_WINNT >= 0x0500)
	case VK_XBUTTON1: return "VK_XBUTTON1";
	case VK_XBUTTON2: return "VK_XBUTTON2";
#endif /* _WIN32_WINNT >= 0x0500 */
	case VK_BACK: return "VK_BACK";
	case VK_TAB: return "VK_TAB";
	case VK_CLEAR: return "VK_CLEAR";
	case VK_RETURN: return "VK_RETURN";
	case VK_SHIFT: return "VK_SHIFT";
	case VK_CONTROL: return "VK_CONTROL";
	case VK_MENU: return "VK_MENU";
	case VK_PAUSE: return "VK_PAUSE";
	case VK_CAPITAL: return "VK_CAPITAL";
	case VK_KANA: return "VK_KANA";
	//case VK_HANGEUL: return "VK_HANGEUL";
	//case VK_HANGUL: return "VK_HANGUL";
	case VK_JUNJA: return "VK_JUNJA";
	case VK_FINAL: return "VK_FINAL";
	case VK_HANJA: return "VK_HANJA";
	//case VK_KANJI: return "VK_KANJI";
	case VK_ESCAPE: return "VK_ESCAPE";
	case VK_CONVERT: return "VK_CONVERT";
	case VK_NONCONVERT: return "VK_NONCONVERT";
	case VK_ACCEPT: return "VK_ACCEPT";
	case VK_MODECHANGE: return "VK_MODECHANGE";
	case VK_SPACE: return "VK_SPACE";
	case VK_PRIOR: return "VK_PRIOR";
	case VK_NEXT: return "VK_NEXT";
	case VK_END: return "VK_END";
	case VK_HOME: return "VK_HOME";
	case VK_LEFT: return "VK_LEFT";
	case VK_UP: return "VK_UP";
	case VK_RIGHT: return "VK_RIGHT";
	case VK_DOWN: return "VK_DOWN";
	case VK_SELECT: return "VK_SELECT";
	case VK_PRINT: return "VK_PRINT";
	case VK_EXECUTE: return "VK_EXECUTE";
	case VK_SNAPSHOT: return "VK_SNAPSHOT";
	case VK_INSERT: return "VK_INSERT";
	case VK_DELETE: return "VK_DELETE";
	case VK_HELP: return "VK_HELP";
	case VK_LWIN: return "VK_LWIN";
	case VK_RWIN: return "VK_RWIN";
	case VK_APPS: return "VK_APPS";
	case VK_SLEEP: return "VK_SLEEP";
	case VK_NUMPAD0: return "VK_NUMPAD0";
	case VK_NUMPAD1: return "VK_NUMPAD1";
	case VK_NUMPAD2: return "VK_NUMPAD2";
	case VK_NUMPAD3: return "VK_NUMPAD3";
	case VK_NUMPAD4: return "VK_NUMPAD4";
	case VK_NUMPAD5: return "VK_NUMPAD5";
	case VK_NUMPAD6: return "VK_NUMPAD6";
	case VK_NUMPAD7: return "VK_NUMPAD7";
	case VK_NUMPAD8: return "VK_NUMPAD8";
	case VK_NUMPAD9: return "VK_NUMPAD9";
	case VK_MULTIPLY: return "VK_MULTIPLY";
	case VK_ADD: return "VK_ADD";
	case VK_SEPARATOR: return "VK_SEPARATOR";
	case VK_SUBTRACT: return "VK_SUBTRACT";
	case VK_DECIMAL: return "VK_DECIMAL";
	case VK_DIVIDE: return "VK_DIVIDE";
	case VK_F1: return "VK_F1";
	case VK_F2: return "VK_F2";
	case VK_F3: return "VK_F3";
	case VK_F4: return "VK_F4";
	case VK_F5: return "VK_F5";
	case VK_F6: return "VK_F6";
	case VK_F7: return "VK_F7";
	case VK_F8: return "VK_F8";
	case VK_F9: return "VK_F9";
	case VK_F10: return "VK_F10";
	case VK_F11: return "VK_F11";
	case VK_F12: return "VK_F12";
	case VK_F13: return "VK_F13";
	case VK_F14: return "VK_F14";
	case VK_F15: return "VK_F15";
	case VK_F16: return "VK_F16";
	case VK_F17: return "VK_F17";
	case VK_F18: return "VK_F18";
	case VK_F19: return "VK_F19";
	case VK_F20: return "VK_F20";
	case VK_F21: return "VK_F21";
	case VK_F22: return "VK_F22";
	case VK_F23: return "VK_F23";
	case VK_F24: return "VK_F24";
	case VK_NUMLOCK: return "VK_NUMLOCK";
	case VK_SCROLL: return "VK_SCROLL";
	case VK_OEM_NEC_EQUAL: return "VK_OEM_NEC_EQUAL";
	//case VK_OEM_FJ_JISHO: return "VK_OEM_FJ_JISHO";
	case VK_OEM_FJ_MASSHOU: return "VK_OEM_FJ_MASSHOU";
	case VK_OEM_FJ_TOUROKU: return "VK_OEM_FJ_TOUROKU";
	case VK_OEM_FJ_LOYA: return "VK_OEM_FJ_LOYA";
	case VK_OEM_FJ_ROYA: return "VK_OEM_FJ_ROYA";
	case VK_LSHIFT: return "VK_LSHIFT";
	case VK_RSHIFT: return "VK_RSHIFT";
	case VK_LCONTROL: return "VK_LCONTROL";
	case VK_RCONTROL: return "VK_RCONTROL";
	case VK_LMENU: return "VK_LMENU";
	case VK_RMENU: return "VK_RMENU";
#if(_WIN32_WINNT >= 0x0500)
	case VK_BROWSER_BACK: return "VK_BROWSER_BACK";
	case VK_BROWSER_FORWARD: return "VK_BROWSER_FORWARD";
	case VK_BROWSER_REFRESH: return "VK_BROWSER_REFRESH";
	case VK_BROWSER_STOP: return "VK_BROWSER_STOP";
	case VK_BROWSER_SEARCH: return "VK_BROWSER_SEARCH";
	case VK_BROWSER_FAVORITES: return "VK_BROWSER_FAVORITES";
	case VK_BROWSER_HOME: return "VK_BROWSER_HOME";
	case VK_VOLUME_MUTE: return "VK_VOLUME_MUTE";
	case VK_VOLUME_DOWN: return "VK_VOLUME_DOWN";
	case VK_VOLUME_UP: return "VK_VOLUME_UP";
	case VK_MEDIA_NEXT_TRACK: return "VK_MEDIA_NEXT_TRACK";
	case VK_MEDIA_PREV_TRACK: return "VK_MEDIA_PREV_TRACK";
	case VK_MEDIA_STOP: return "VK_MEDIA_STOP";
	case VK_MEDIA_PLAY_PAUSE: return "VK_MEDIA_PLAY_PAUSE";
	case VK_LAUNCH_MAIL: return "VK_LAUNCH_MAIL";
	case VK_LAUNCH_MEDIA_SELECT: return "VK_LAUNCH_MEDIA_SELECT";
	case VK_LAUNCH_APP1: return "VK_LAUNCH_APP1";
	case VK_LAUNCH_APP2: return "VK_LAUNCH_APP2";
#endif /* _WIN32_WINNT >= 0x0500 */
	case VK_OEM_1: return "VK_OEM_1";
	case VK_OEM_PLUS: return "VK_OEM_PLUS";
	case VK_OEM_COMMA: return "VK_OEM_COMMA";
	case VK_OEM_MINUS: return "VK_OEM_MINUS";
	case VK_OEM_PERIOD: return "VK_OEM_PERIOD";
	case VK_OEM_2: return "VK_OEM_2";
	case VK_OEM_3: return "VK_OEM_3";
	case VK_OEM_4: return "VK_OEM_4";
	case VK_OEM_5: return "VK_OEM_5";
	case VK_OEM_6: return "VK_OEM_6";
	case VK_OEM_7: return "VK_OEM_7";
	case VK_OEM_8: return "VK_OEM_8";
	case VK_OEM_AX: return "VK_OEM_AX";
	case VK_OEM_102: return "VK_OEM_102";
	case VK_ICO_HELP: return "VK_ICO_HELP";
	case VK_ICO_00: return "VK_ICO_00";
#if(WINVER >= 0x0400)
	case VK_PROCESSKEY: return "VK_PROCESSKEY";
#endif /* WINVER >= 0x0400 */
	case VK_ICO_CLEAR: return "VK_ICO_CLEAR";
#if(_WIN32_WINNT >= 0x0500)
	case VK_PACKET: return "VK_PACKET";
#endif /* _WIN32_WINNT >= 0x0500 */
	case VK_OEM_RESET: return "VK_OEM_RESET";
	case VK_OEM_JUMP: return "VK_OEM_JUMP";
	case VK_OEM_PA1: return "VK_OEM_PA1";
	case VK_OEM_PA2: return "VK_OEM_PA2";
	case VK_OEM_PA3: return "VK_OEM_PA3";
	case VK_OEM_WSCTRL: return "VK_OEM_WSCTRL";
	case VK_OEM_CUSEL: return "VK_OEM_CUSEL";
	case VK_OEM_ATTN: return "VK_OEM_ATTN";
	case VK_OEM_FINISH: return "VK_OEM_FINISH";
	case VK_OEM_COPY: return "VK_OEM_COPY";
	case VK_OEM_AUTO: return "VK_OEM_AUTO";
	case VK_OEM_ENLW: return "VK_OEM_ENLW";
	case VK_OEM_BACKTAB: return "VK_OEM_BACKTAB";
	case VK_ATTN: return "VK_ATTN";
	case VK_CRSEL: return "VK_CRSEL";
	case VK_EXSEL: return "VK_EXSEL";
	case VK_EREOF: return "VK_EREOF";
	case VK_PLAY: return "VK_PLAY";
	case VK_ZOOM: return "VK_ZOOM";
	case VK_NONAME: return "VK_NONAME";
	case VK_PA1: return "VK_PA1";
	case VK_OEM_CLEAR: return "VK_OEM_CLEAR";
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
	float dead_zone)
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

	//_ASSERT(dead_zone >= 0 && dead_zone <= 100);

	//DIPROPDWORD dipd  = {sizeof(dipd), sizeof(dipd.diph)};
	//dipd.diph.dwObj = DIJOFS_X;
	//dipd.diph.dwHow = DIPH_BYOFFSET;
	//dipd.dwData = (DWORD)(dead_zone * 100);
	//SetProperty(DIPROP_DEADZONE, &dipd.diph);

	//dipd.diph.dwObj = DIJOFS_Y;
	//dipd.diph.dwHow = DIPH_BYOFFSET;
	//dipd.dwData = (DWORD)(dead_zone * 100);
	//SetProperty(DIPROP_DEADZONE, &dipd.diph);

	//dipd.diph.dwObj = DIJOFS_Z;
	//dipd.diph.dwHow = DIPH_BYOFFSET;
	//dipd.dwData = (DWORD)(dead_zone * 100);
	//SetProperty(DIPROP_DEADZONE, &dipd.diph);

	//dipd.diph.dwObj = DIJOFS_RZ;
	//dipd.diph.dwHow = DIPH_BYOFFSET;
	//dipd.dwData = (DWORD)(dead_zone * 100);
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

	JoystickEnumDesc desc;
	desc.input = m_input->m_ptr;
	desc.hwnd = hwnd;
	desc.min_x = -255;
	desc.max_x =  255;
	desc.min_y = -255;
	desc.max_y =  255;
	desc.min_z = -255;
	desc.max_z =  255;
	desc.dead_zone = 10;
	m_input->EnumDevices(DI8DEVCLASS_GAMECTRL, JoystickFinderCallback, &desc, DIEDFL_ATTACHEDONLY);
	if (desc.joystick)
	{
		m_joystick = desc.joystick;
		m_joystick->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
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

BOOL CALLBACK InputMgr::JoystickFinderCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
	_ASSERT(lpddi && pvRef);

	if(lpddi->dwDevType & DI8DEVTYPE_JOYSTICK)
	{
		JoystickEnumDesc * desc = static_cast<JoystickEnumDesc *>(pvRef);
		JoystickPtr joystick(new Joystick);
		joystick->CreateJoystick(
			desc->input, desc->hwnd, lpddi->guidInstance, desc->min_x, desc->max_x, desc->min_y, desc->max_y, desc->min_z, desc->max_z, desc->dead_zone);
		_ASSERT(!desc->joystick);
		desc->joystick = joystick;
		return DIENUM_STOP;
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
						if (m_joystick->GetX() < 32767 - m_JoystickAxisDeadZone || m_joystick->GetX() > 32767 + m_JoystickAxisDeadZone)
							return m_joystick->GetX();
						break;
					case 1:
						if (m_joystick->GetY() < 32767 - m_JoystickAxisDeadZone || m_joystick->GetY() > 32767 + m_JoystickAxisDeadZone)
							return m_joystick->GetY();
						break;
					case 2:
						if (m_joystick->GetZ() < 32767 - m_JoystickAxisDeadZone || m_joystick->GetZ() > 32767 + m_JoystickAxisDeadZone)
							return m_joystick->GetZ();
						break;
					case 3:
						if (m_joystick->GetRx() < 32767 - m_JoystickAxisDeadZone || m_joystick->GetRx() > 32767 + m_JoystickAxisDeadZone)
							return m_joystick->GetRx();
						break;
					case 4:
						if (m_joystick->GetRy() < 32767 - m_JoystickAxisDeadZone || m_joystick->GetRy() > 32767 + m_JoystickAxisDeadZone)
							return m_joystick->GetRy();
						break;
					case 5:
						if (m_joystick->GetRz() < 32767 - m_JoystickAxisDeadZone || m_joystick->GetRz() > 32767 + m_JoystickAxisDeadZone)
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
						if (m_joystick->GetX() < 32767 - m_JoystickAxisDeadZone || m_joystick->GetX() > 32767 + m_JoystickAxisDeadZone)
							return m_joystick->GetX();
						break;
					case 1:
						if (m_joystick->GetY() < 32767 - m_JoystickAxisDeadZone || m_joystick->GetY() > 32767 + m_JoystickAxisDeadZone)
							return m_joystick->GetY();
						break;
					case 2:
						if (m_joystick->GetZ() < 32767 - m_JoystickAxisDeadZone || m_joystick->GetZ() > 32767 + m_JoystickAxisDeadZone)
							return m_joystick->GetZ();
						break;
					case 3:
						if (m_joystick->GetRx() < 32767 - m_JoystickAxisDeadZone || m_joystick->GetRx() > 32767 + m_JoystickAxisDeadZone)
							return m_joystick->GetRx();
						break;
					case 4:
						if (m_joystick->GetRy() < 32767 - m_JoystickAxisDeadZone || m_joystick->GetRy() > 32767 + m_JoystickAxisDeadZone)
							return m_joystick->GetRy();
						break;
					case 5:
						if (m_joystick->GetRz() < 32767 - m_JoystickAxisDeadZone || m_joystick->GetRz() > 32767 + m_JoystickAxisDeadZone)
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
