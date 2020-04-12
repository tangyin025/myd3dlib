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

LPCTSTR Keyboard::TranslateKeyCode(KeyCode kc)
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

LPCTSTR Keyboard::TranslateVirtualKey(DWORD vk)
{
	switch(vk)
	{
	case VK_LBUTTON: return _T("VK_LBUTTON");
	case VK_RBUTTON: return _T("VK_RBUTTON");
	case VK_CANCEL: return _T("VK_CANCEL");
	case VK_MBUTTON: return _T("VK_MBUTTON");
#if(_WIN32_WINNT >= 0x0500)
	case VK_XBUTTON1: return _T("VK_XBUTTON1");
	case VK_XBUTTON2: return _T("VK_XBUTTON2");
#endif /* _WIN32_WINNT >= 0x0500 */
	case VK_BACK: return _T("VK_BACK");
	case VK_TAB: return _T("VK_TAB");
	case VK_CLEAR: return _T("VK_CLEAR");
	case VK_RETURN: return _T("VK_RETURN");
	case VK_SHIFT: return _T("VK_SHIFT");
	case VK_CONTROL: return _T("VK_CONTROL");
	case VK_MENU: return _T("VK_MENU");
	case VK_PAUSE: return _T("VK_PAUSE");
	case VK_CAPITAL: return _T("VK_CAPITAL");
	case VK_KANA: return _T("VK_KANA");
	//case VK_HANGEUL: return _T("VK_HANGEUL");
	//case VK_HANGUL: return _T("VK_HANGUL");
	case VK_JUNJA: return _T("VK_JUNJA");
	case VK_FINAL: return _T("VK_FINAL");
	case VK_HANJA: return _T("VK_HANJA");
	//case VK_KANJI: return _T("VK_KANJI");
	case VK_ESCAPE: return _T("VK_ESCAPE");
	case VK_CONVERT: return _T("VK_CONVERT");
	case VK_NONCONVERT: return _T("VK_NONCONVERT");
	case VK_ACCEPT: return _T("VK_ACCEPT");
	case VK_MODECHANGE: return _T("VK_MODECHANGE");
	case VK_SPACE: return _T("VK_SPACE");
	case VK_PRIOR: return _T("VK_PRIOR");
	case VK_NEXT: return _T("VK_NEXT");
	case VK_END: return _T("VK_END");
	case VK_HOME: return _T("VK_HOME");
	case VK_LEFT: return _T("VK_LEFT");
	case VK_UP: return _T("VK_UP");
	case VK_RIGHT: return _T("VK_RIGHT");
	case VK_DOWN: return _T("VK_DOWN");
	case VK_SELECT: return _T("VK_SELECT");
	case VK_PRINT: return _T("VK_PRINT");
	case VK_EXECUTE: return _T("VK_EXECUTE");
	case VK_SNAPSHOT: return _T("VK_SNAPSHOT");
	case VK_INSERT: return _T("VK_INSERT");
	case VK_DELETE: return _T("VK_DELETE");
	case VK_HELP: return _T("VK_HELP");
	case VK_LWIN: return _T("VK_LWIN");
	case VK_RWIN: return _T("VK_RWIN");
	case VK_APPS: return _T("VK_APPS");
	case VK_SLEEP: return _T("VK_SLEEP");
	case VK_NUMPAD0: return _T("VK_NUMPAD0");
	case VK_NUMPAD1: return _T("VK_NUMPAD1");
	case VK_NUMPAD2: return _T("VK_NUMPAD2");
	case VK_NUMPAD3: return _T("VK_NUMPAD3");
	case VK_NUMPAD4: return _T("VK_NUMPAD4");
	case VK_NUMPAD5: return _T("VK_NUMPAD5");
	case VK_NUMPAD6: return _T("VK_NUMPAD6");
	case VK_NUMPAD7: return _T("VK_NUMPAD7");
	case VK_NUMPAD8: return _T("VK_NUMPAD8");
	case VK_NUMPAD9: return _T("VK_NUMPAD9");
	case VK_MULTIPLY: return _T("VK_MULTIPLY");
	case VK_ADD: return _T("VK_ADD");
	case VK_SEPARATOR: return _T("VK_SEPARATOR");
	case VK_SUBTRACT: return _T("VK_SUBTRACT");
	case VK_DECIMAL: return _T("VK_DECIMAL");
	case VK_DIVIDE: return _T("VK_DIVIDE");
	case VK_F1: return _T("VK_F1");
	case VK_F2: return _T("VK_F2");
	case VK_F3: return _T("VK_F3");
	case VK_F4: return _T("VK_F4");
	case VK_F5: return _T("VK_F5");
	case VK_F6: return _T("VK_F6");
	case VK_F7: return _T("VK_F7");
	case VK_F8: return _T("VK_F8");
	case VK_F9: return _T("VK_F9");
	case VK_F10: return _T("VK_F10");
	case VK_F11: return _T("VK_F11");
	case VK_F12: return _T("VK_F12");
	case VK_F13: return _T("VK_F13");
	case VK_F14: return _T("VK_F14");
	case VK_F15: return _T("VK_F15");
	case VK_F16: return _T("VK_F16");
	case VK_F17: return _T("VK_F17");
	case VK_F18: return _T("VK_F18");
	case VK_F19: return _T("VK_F19");
	case VK_F20: return _T("VK_F20");
	case VK_F21: return _T("VK_F21");
	case VK_F22: return _T("VK_F22");
	case VK_F23: return _T("VK_F23");
	case VK_F24: return _T("VK_F24");
	case VK_NUMLOCK: return _T("VK_NUMLOCK");
	case VK_SCROLL: return _T("VK_SCROLL");
	case VK_OEM_NEC_EQUAL: return _T("VK_OEM_NEC_EQUAL");
	//case VK_OEM_FJ_JISHO: return _T("VK_OEM_FJ_JISHO");
	case VK_OEM_FJ_MASSHOU: return _T("VK_OEM_FJ_MASSHOU");
	case VK_OEM_FJ_TOUROKU: return _T("VK_OEM_FJ_TOUROKU");
	case VK_OEM_FJ_LOYA: return _T("VK_OEM_FJ_LOYA");
	case VK_OEM_FJ_ROYA: return _T("VK_OEM_FJ_ROYA");
	case VK_LSHIFT: return _T("VK_LSHIFT");
	case VK_RSHIFT: return _T("VK_RSHIFT");
	case VK_LCONTROL: return _T("VK_LCONTROL");
	case VK_RCONTROL: return _T("VK_RCONTROL");
	case VK_LMENU: return _T("VK_LMENU");
	case VK_RMENU: return _T("VK_RMENU");
#if(_WIN32_WINNT >= 0x0500)
	case VK_BROWSER_BACK: return _T("VK_BROWSER_BACK");
	case VK_BROWSER_FORWARD: return _T("VK_BROWSER_FORWARD");
	case VK_BROWSER_REFRESH: return _T("VK_BROWSER_REFRESH");
	case VK_BROWSER_STOP: return _T("VK_BROWSER_STOP");
	case VK_BROWSER_SEARCH: return _T("VK_BROWSER_SEARCH");
	case VK_BROWSER_FAVORITES: return _T("VK_BROWSER_FAVORITES");
	case VK_BROWSER_HOME: return _T("VK_BROWSER_HOME");
	case VK_VOLUME_MUTE: return _T("VK_VOLUME_MUTE");
	case VK_VOLUME_DOWN: return _T("VK_VOLUME_DOWN");
	case VK_VOLUME_UP: return _T("VK_VOLUME_UP");
	case VK_MEDIA_NEXT_TRACK: return _T("VK_MEDIA_NEXT_TRACK");
	case VK_MEDIA_PREV_TRACK: return _T("VK_MEDIA_PREV_TRACK");
	case VK_MEDIA_STOP: return _T("VK_MEDIA_STOP");
	case VK_MEDIA_PLAY_PAUSE: return _T("VK_MEDIA_PLAY_PAUSE");
	case VK_LAUNCH_MAIL: return _T("VK_LAUNCH_MAIL");
	case VK_LAUNCH_MEDIA_SELECT: return _T("VK_LAUNCH_MEDIA_SELECT");
	case VK_LAUNCH_APP1: return _T("VK_LAUNCH_APP1");
	case VK_LAUNCH_APP2: return _T("VK_LAUNCH_APP2");
#endif /* _WIN32_WINNT >= 0x0500 */
	case VK_OEM_1: return _T("VK_OEM_1");
	case VK_OEM_PLUS: return _T("VK_OEM_PLUS");
	case VK_OEM_COMMA: return _T("VK_OEM_COMMA");
	case VK_OEM_MINUS: return _T("VK_OEM_MINUS");
	case VK_OEM_PERIOD: return _T("VK_OEM_PERIOD");
	case VK_OEM_2: return _T("VK_OEM_2");
	case VK_OEM_3: return _T("VK_OEM_3");
	case VK_OEM_4: return _T("VK_OEM_4");
	case VK_OEM_5: return _T("VK_OEM_5");
	case VK_OEM_6: return _T("VK_OEM_6");
	case VK_OEM_7: return _T("VK_OEM_7");
	case VK_OEM_8: return _T("VK_OEM_8");
	case VK_OEM_AX: return _T("VK_OEM_AX");
	case VK_OEM_102: return _T("VK_OEM_102");
	case VK_ICO_HELP: return _T("VK_ICO_HELP");
	case VK_ICO_00: return _T("VK_ICO_00");
#if(WINVER >= 0x0400)
	case VK_PROCESSKEY: return _T("VK_PROCESSKEY");
#endif /* WINVER >= 0x0400 */
	case VK_ICO_CLEAR: return _T("VK_ICO_CLEAR");
#if(_WIN32_WINNT >= 0x0500)
	case VK_PACKET: return _T("VK_PACKET");
#endif /* _WIN32_WINNT >= 0x0500 */
	case VK_OEM_RESET: return _T("VK_OEM_RESET");
	case VK_OEM_JUMP: return _T("VK_OEM_JUMP");
	case VK_OEM_PA1: return _T("VK_OEM_PA1");
	case VK_OEM_PA2: return _T("VK_OEM_PA2");
	case VK_OEM_PA3: return _T("VK_OEM_PA3");
	case VK_OEM_WSCTRL: return _T("VK_OEM_WSCTRL");
	case VK_OEM_CUSEL: return _T("VK_OEM_CUSEL");
	case VK_OEM_ATTN: return _T("VK_OEM_ATTN");
	case VK_OEM_FINISH: return _T("VK_OEM_FINISH");
	case VK_OEM_COPY: return _T("VK_OEM_COPY");
	case VK_OEM_AUTO: return _T("VK_OEM_AUTO");
	case VK_OEM_ENLW: return _T("VK_OEM_ENLW");
	case VK_OEM_BACKTAB: return _T("VK_OEM_BACKTAB");
	case VK_ATTN: return _T("VK_ATTN");
	case VK_CRSEL: return _T("VK_CRSEL");
	case VK_EXSEL: return _T("VK_EXSEL");
	case VK_EREOF: return _T("VK_EREOF");
	case VK_PLAY: return _T("VK_PLAY");
	case VK_ZOOM: return _T("VK_ZOOM");
	case VK_NONAME: return _T("VK_NONAME");
	case VK_PA1: return _T("VK_PA1");
	case VK_OEM_CLEAR: return _T("VK_OEM_CLEAR");
	/*
	 * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
	 * 0x40 : unassigned
	 * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
	 */
	case 0x30: return _T("\x30");
	case 0x31: return _T("\x31");
	case 0x32: return _T("\x32");
	case 0x33: return _T("\x33");
	case 0x34: return _T("\x34");
	case 0x35: return _T("\x35");
	case 0x36: return _T("\x36");
	case 0x37: return _T("\x37");
	case 0x38: return _T("\x38");
	case 0x39: return _T("\x39");
	case 0x41: return _T("\x41");
	case 0x42: return _T("\x42");
	case 0x43: return _T("\x43");
	case 0x44: return _T("\x44");
	case 0x45: return _T("\x45");
	case 0x46: return _T("\x46");
	case 0x47: return _T("\x47");
	case 0x48: return _T("\x48");
	case 0x49: return _T("\x49");
	case 0x4A: return _T("\x4A");
	case 0x4B: return _T("\x4B");
	case 0x4C: return _T("\x4C");
	case 0x4D: return _T("\x4D");
	case 0x4E: return _T("\x4E");
	case 0x4F: return _T("\x4F");
	case 0x50: return _T("\x50");
	case 0x51: return _T("\x51");
	case 0x52: return _T("\x52");
	case 0x53: return _T("\x53");
	case 0x54: return _T("\x54");
	case 0x55: return _T("\x55");
	case 0x56: return _T("\x56");
	case 0x57: return _T("\x57");
	case 0x58: return _T("\x58");
	case 0x59: return _T("\x59");
	case 0x5A: return _T("\x5A");
	}
	return _T("reserved vritual key");
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
	BYTE OldState[256];
	memcpy(OldState, m_State, sizeof(OldState));

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
		if (diBuff[i].dwData & 0x80)
		{
			KeyboardEventArg arg(kc);
			m_PressedEvent(&arg);
		}
		else
		{
			KeyboardEventArg arg(kc);
			m_ReleasedEvent(&arg);
		}
	}
	return true;
}

bool Keyboard::IsKeyDown(KeyCode kc)
{
	return m_State[kc] & 0x80;
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
	DIMOUSESTATE OldState;
	memcpy(&OldState, &m_State, sizeof(OldState));

	if(FAILED(hr = m_ptr->GetDeviceState(sizeof(m_State), &m_State)))
	{
		if(FAILED(hr = m_ptr->Acquire()))
		{
			return false;
		}

		GetDeviceState(sizeof(m_State), &m_State);
	}

	for (DWORD i = 0; i < _countof(m_State.rgbButtons); i++)
	{
		if (m_State.rgbButtons[i] & 0x80)
		{
			if (!(OldState.rgbButtons[i] & 0x80))
			{
				MouseBtnEventArg arg(i);
				m_PressedEvent(&arg);
			}
		}
		else
		{
			if (OldState.rgbButtons[i] & 0x80)
			{
				MouseBtnEventArg arg(i);
				m_ReleasedEvent(&arg);
			}
		}
	}

	if (m_State.lX || m_State.lY || m_State.lZ)
	{
		MouseMoveEventArg arg(m_State.lX, m_State.lY, m_State.lZ);
		m_MovedEvent(&arg);
	}
	return true;
}

LPCTSTR Joystick::TranslateAxis(DWORD axis)
{
	switch(axis)
	{
	case JA_X: return _T("JA_X");
	case JA_Y: return _T("JA_Y");
	case JA_Z: return _T("JA_Z");
	case JA_Rx: return _T("JA_Rx");
	case JA_Ry: return _T("JA_Ry");
	case JA_Rz: return _T("JA_Rz");
	case JA_S0: return _T("JA_S0");
	case JA_S1: return _T("JA_S1");
	}
	return _T("unknown axis");
}

LPCTSTR Joystick::TranslatePov(DWORD pov)
{
	switch(pov)
	{
	case JP_North: return _T("JP_North");
	case JP_NorthEast: return _T("JP_NorthEast");
	case JP_East: return _T("JP_East");
	case JP_SouthEast: return _T("JP_SouthEast");
	case JP_South: return _T("JP_South");
	case JP_SouthWest: return _T("JP_SouthWest");
	case JP_West: return _T("JP_West");
	case JP_NorthWest: return _T("JP_NorthWest");
	}
	return _T("unknown pov");
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

void Joystick::CheckAxis(LONG value, JoystickAxis axis)
{
	if (value)
	{
		JoystickAxisEventArg arg(axis, value);
		m_AxisMovedEvent(&arg);
	}
}

bool Joystick::Capture(void)
{
	DIJOYSTATE OldState;
	memcpy(&OldState, &m_State, sizeof(OldState));

	if(FAILED(hr = m_ptr->GetDeviceState(sizeof(m_State), &m_State)))
	{
		if(FAILED(hr = m_ptr->Acquire()))
		{
			return false;
		}

		GetDeviceState(sizeof(m_State), &m_State);
	}

	CheckAxis(m_State.lX, JA_X);
	CheckAxis(m_State.lY, JA_Y);
	CheckAxis(m_State.lZ, JA_Z);
	CheckAxis(m_State.lRx, JA_Rx);
	CheckAxis(m_State.lRy, JA_Ry);
	CheckAxis(m_State.lRz, JA_Rz);
	CheckAxis(m_State.rglSlider[0], JA_S0);
	CheckAxis(m_State.rglSlider[1], JA_S1);

	for (DWORD i = 0; i < _countof(m_State.rgdwPOV); i++)
	{
		if (LOWORD(m_State.rgdwPOV[i]) != 0xFFFF)
		{
			JoystickPovEventArg arg(i, (JoystickPov)m_State.rgdwPOV[i]);
			m_PovMovedEvent(&arg);
		}
	}

	for (DWORD i = 0; i < _countof(m_State.rgbButtons); i++)
	{
		if (m_State.rgbButtons[i] & 0x80)
		{
			if (!(OldState.rgbButtons[i] & 0x80))
			{
				JoystickBtnEventArg arg(i);
				m_BtnPressedEvent(&arg);
			}
		}
		else
		{
			if (OldState.rgbButtons[i] & 0x80)
			{
				JoystickBtnEventArg arg(i);
				m_BtnReleasedEvent(&arg);
			}
		}
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
		m_joystick->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
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
		return false;
	}

	if (!m_mouse->Capture())
	{
		return false;
	}

	if (m_joystick)
	{
		if (!m_joystick->Capture())
		{
			return false;
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
