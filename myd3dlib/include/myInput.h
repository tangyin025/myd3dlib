// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include <boost/shared_ptr.hpp>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <atlbase.h>
#include "mySingleton.h"

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

		void Destroy(void);

		CComPtr<IDirectInputDevice8> CreateDevice(REFGUID rguid);

		void ConfigureDevices(
			LPDICONFIGUREDEVICESCALLBACK lpdiCallback,
			LPDICONFIGUREDEVICESPARAMS lpdiCDParams,
			DWORD dwFlags,
			LPVOID pvRefData);

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

		void Destroy(void);

		virtual bool Capture(void) = 0;

		void SetCooperativeLevel(HWND hwnd, DWORD dwFlags = DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

		void SetDataFormat(LPCDIDATAFORMAT lpdf);

		void SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph);

		void Acquire(void);

		void Unacquire(void);

		void GetDeviceState(DWORD cbData, LPVOID lpvData);

		void GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags = 0);
	};

	enum KeyCode
	{
		KC_UNASSIGNED  = 0x00,
		KC_ESCAPE      = 0x01,
		KC_1           = 0x02,
		KC_2           = 0x03,
		KC_3           = 0x04,
		KC_4           = 0x05,
		KC_5           = 0x06,
		KC_6           = 0x07,
		KC_7           = 0x08,
		KC_8           = 0x09,
		KC_9           = 0x0A,
		KC_0           = 0x0B,
		KC_MINUS       = 0x0C,    // - on main keyboard
		KC_EQUALS      = 0x0D,
		KC_BACK        = 0x0E,    // backspace
		KC_TAB         = 0x0F,
		KC_Q           = 0x10,
		KC_W           = 0x11,
		KC_E           = 0x12,
		KC_R           = 0x13,
		KC_T           = 0x14,
		KC_Y           = 0x15,
		KC_U           = 0x16,
		KC_I           = 0x17,
		KC_O           = 0x18,
		KC_P           = 0x19,
		KC_LBRACKET    = 0x1A,
		KC_RBRACKET    = 0x1B,
		KC_RETURN      = 0x1C,    // Enter on main keyboard
		KC_LCONTROL    = 0x1D,
		KC_A           = 0x1E,
		KC_S           = 0x1F,
		KC_D           = 0x20,
		KC_F           = 0x21,
		KC_G           = 0x22,
		KC_H           = 0x23,
		KC_J           = 0x24,
		KC_K           = 0x25,
		KC_L           = 0x26,
		KC_SEMICOLON   = 0x27,
		KC_APOSTROPHE  = 0x28,
		KC_GRAVE       = 0x29,    // accent
		KC_LSHIFT      = 0x2A,
		KC_BACKSLASH   = 0x2B,
		KC_Z           = 0x2C,
		KC_X           = 0x2D,
		KC_C           = 0x2E,
		KC_V           = 0x2F,
		KC_B           = 0x30,
		KC_N           = 0x31,
		KC_M           = 0x32,
		KC_COMMA       = 0x33,
		KC_PERIOD      = 0x34,    // . on main keyboard
		KC_SLASH       = 0x35,    // / on main keyboard
		KC_RSHIFT      = 0x36,
		KC_MULTIPLY    = 0x37,    // * on numeric keypad
		KC_LMENU       = 0x38,    // left Alt
		KC_SPACE       = 0x39,
		KC_CAPITAL     = 0x3A,
		KC_F1          = 0x3B,
		KC_F2          = 0x3C,
		KC_F3          = 0x3D,
		KC_F4          = 0x3E,
		KC_F5          = 0x3F,
		KC_F6          = 0x40,
		KC_F7          = 0x41,
		KC_F8          = 0x42,
		KC_F9          = 0x43,
		KC_F10         = 0x44,
		KC_NUMLOCK     = 0x45,
		KC_SCROLL      = 0x46,    // Scroll Lock
		KC_NUMPAD7     = 0x47,
		KC_NUMPAD8     = 0x48,
		KC_NUMPAD9     = 0x49,
		KC_SUBTRACT    = 0x4A,    // - on numeric keypad
		KC_NUMPAD4     = 0x4B,
		KC_NUMPAD5     = 0x4C,
		KC_NUMPAD6     = 0x4D,
		KC_ADD         = 0x4E,    // + on numeric keypad
		KC_NUMPAD1     = 0x4F,
		KC_NUMPAD2     = 0x50,
		KC_NUMPAD3     = 0x51,
		KC_NUMPAD0     = 0x52,
		KC_DECIMAL     = 0x53,    // . on numeric keypad
		KC_OEM_102     = 0x56,    // < > | on UK/Germany keyboards
		KC_F11         = 0x57,
		KC_F12         = 0x58,
		KC_F13         = 0x64,    //                     (NEC PC98)
		KC_F14         = 0x65,    //                     (NEC PC98)
		KC_F15         = 0x66,    //                     (NEC PC98)
		KC_KANA        = 0x70,    // (Japanese keyboard)
		KC_ABNT_C1     = 0x73,    // / ? on Portugese (Brazilian) keyboards
		KC_CONVERT     = 0x79,    // (Japanese keyboard)
		KC_NOCONVERT   = 0x7B,    // (Japanese keyboard)
		KC_YEN         = 0x7D,    // (Japanese keyboard)
		KC_ABNT_C2     = 0x7E,    // Numpad . on Portugese (Brazilian) keyboards
		KC_NUMPADEQUALS= 0x8D,    // = on numeric keypad (NEC PC98)
		KC_PREVTRACK   = 0x90,    // Previous Track (KC_CIRCUMFLEX on Japanese keyboard)
		KC_AT          = 0x91,    //                     (NEC PC98)
		KC_COLON       = 0x92,    //                     (NEC PC98)
		KC_UNDERLINE   = 0x93,    //                     (NEC PC98)
		KC_KANJI       = 0x94,    // (Japanese keyboard)
		KC_STOP        = 0x95,    //                     (NEC PC98)
		KC_AX          = 0x96,    //                     (Japan AX)
		KC_UNLABELED   = 0x97,    //                        (J3100)
		KC_NEXTTRACK   = 0x99,    // Next Track
		KC_NUMPADENTER = 0x9C,    // Enter on numeric keypad
		KC_RCONTROL    = 0x9D,
		KC_MUTE        = 0xA0,    // Mute
		KC_CALCULATOR  = 0xA1,    // Calculator
		KC_PLAYPAUSE   = 0xA2,    // Play / Pause
		KC_MEDIASTOP   = 0xA4,    // Media Stop
		KC_VOLUMEDOWN  = 0xAE,    // Volume -
		KC_VOLUMEUP    = 0xB0,    // Volume +
		KC_WEBHOME     = 0xB2,    // Web home
		KC_NUMPADCOMMA = 0xB3,    // , on numeric keypad (NEC PC98)
		KC_DIVIDE      = 0xB5,    // / on numeric keypad
		KC_SYSRQ       = 0xB7,
		KC_RMENU       = 0xB8,    // right Alt
		KC_PAUSE       = 0xC5,    // Pause
		KC_HOME        = 0xC7,    // Home on arrow keypad
		KC_UP          = 0xC8,    // UpArrow on arrow keypad
		KC_PGUP        = 0xC9,    // PgUp on arrow keypad
		KC_LEFT        = 0xCB,    // LeftArrow on arrow keypad
		KC_RIGHT       = 0xCD,    // RightArrow on arrow keypad
		KC_END         = 0xCF,    // End on arrow keypad
		KC_DOWN        = 0xD0,    // DownArrow on arrow keypad
		KC_PGDOWN      = 0xD1,    // PgDn on arrow keypad
		KC_INSERT      = 0xD2,    // Insert on arrow keypad
		KC_DELETE      = 0xD3,    // Delete on arrow keypad
		KC_LWIN        = 0xDB,    // Left Windows key
		KC_RWIN        = 0xDC,    // Right Windows key
		KC_APPS        = 0xDD,    // AppMenu key
		KC_POWER       = 0xDE,    // System Power
		KC_SLEEP       = 0xDF,    // System Sleep
		KC_WAKE        = 0xE3,    // System Wake
		KC_WEBSEARCH   = 0xE5,    // Web Search
		KC_WEBFAVORITES= 0xE6,    // Web Favorites
		KC_WEBREFRESH  = 0xE7,    // Web Refresh
		KC_WEBSTOP     = 0xE8,    // Web Stop
		KC_WEBFORWARD  = 0xE9,    // Web Forward
		KC_WEBBACK     = 0xEA,    // Web Back
		KC_MYCOMPUTER  = 0xEB,    // My Computer
		KC_MAIL        = 0xEC,    // Mail
		KC_MEDIASELECT = 0xED     // Media Select
	};

	class Keyboard : public InputDevice
	{
	public:
		BYTE m_State[256];

		BYTE m_LastFrameState[256];

	public:
		Keyboard(void);

		static const char * TranslateKeyCode(KeyCode kc);

		static const char * TranslateVirtualKey(DWORD vk);

		void CreateKeyboard(LPDIRECTINPUT8 input, HWND hwnd);

		virtual bool Capture(void);

		void SetKeyState(unsigned int kc, BYTE state, BYTE last_state)
		{
			_ASSERT(kc < _countof(m_State));
			m_State[kc] = state;
			m_LastFrameState[kc] = last_state;
		}

		bool IsKeyDown(unsigned int kc) const
		{
			_ASSERT(kc < _countof(m_State));
			return m_State[kc] & 0x80;
		}

		bool IsKeyPress(unsigned int kc) const
		{
			_ASSERT(kc < _countof(m_State));
			return !(m_LastFrameState[kc] & 0x80) && (m_State[kc] & 0x80);
		}

		bool IsKeyRelease(unsigned int kc) const
		{
			_ASSERT(kc < _countof(m_State));
			return (m_LastFrameState[kc] & 0x80) && !(m_State[kc] & 0x80);
		}
	};

	typedef boost::shared_ptr<Keyboard> KeyboardPtr;

	class Mouse : public InputDevice
	{
	public:
		DIMOUSESTATE m_State;

		DIMOUSESTATE m_LastFrameState;

	public:
		Mouse(void);

		void CreateMouse(LPDIRECTINPUT8 input, HWND hwnd);

		virtual bool Capture(void);

		LONG GetX(void) const
		{
			return m_State.lX;
		}

		LONG GetY(void) const
		{
			return m_State.lY;
		}

		LONG GetZ(void) const
		{
			return m_State.lZ;
		}

		bool IsButtonDown(unsigned int i) const
		{
			_ASSERT(i < _countof(m_State.rgbButtons));
			return m_State.rgbButtons[i] & 0x80;
		}

		bool IsButtonPress(unsigned int i) const
		{
			_ASSERT(i < _countof(m_State.rgbButtons));
			return !(m_LastFrameState.rgbButtons[i] & 0x80) && (m_State.rgbButtons[i] & 0x80);
		}

		bool IsButtonRelease(unsigned int i) const
		{
			_ASSERT(i < _countof(m_State.rgbButtons));
			return (m_LastFrameState.rgbButtons[i] & 0x80) && !(m_State.rgbButtons[i] & 0x80);
		}
	};

	typedef boost::shared_ptr<Mouse> MousePtr;

	enum JoystickPov
	{
		JP_None			= 0xFFFF,
		JP_North		= 0,
		JP_NorthEast	= 45 * DI_DEGREES,
		JP_East			= 90 * DI_DEGREES,
		JP_SouthEast	= 135 * DI_DEGREES,
		JP_South		= 180 * DI_DEGREES,
		JP_SouthWest	= 225 * DI_DEGREES,
		JP_West			= 270 * DI_DEGREES,
		JP_NorthWest	= 315 * DI_DEGREES,
	};

	class Joystick : public InputDevice
	{
	public:
		DIJOYSTATE m_State;

		DIJOYSTATE m_LastFrameState;

	public:
		Joystick(void)
		{
		}

		static const char * TranslatePov(DWORD pov);

		void CreateJoystick(
			LPDIRECTINPUT8 input,
			HWND hwnd,
			REFGUID rguid,
			LONG min_x,
			LONG max_x,
			LONG min_y,
			LONG max_y,
			LONG min_z,
			LONG max_z,
			DWORD dead_zone);

		bool Capture(void);

		LONG GetX(void) const
		{
			return m_State.lX;
		}

		LONG GetY(void) const
		{
			return m_State.lY;
		}

		LONG GetZ(void) const
		{
			return m_State.lZ;
		}

		LONG GetRx(void) const
		{
			return m_State.lRx;
		}

		LONG GetRy(void) const
		{
			return m_State.lRy;
		}

		LONG GetRz(void) const
		{
			return m_State.lRz;
		}

		LONG GetSlider(unsigned int i) const
		{
			_ASSERT(i < _countof(m_State.rglSlider));
			return m_State.rglSlider[i];
		}

		DWORD GetPov(unsigned int i) const
		{
			_ASSERT(i < _countof(m_State.rgdwPOV));
			return m_State.rgdwPOV[i];
		}

		bool IsButtonDown(unsigned int i) const
		{
			_ASSERT(i < _countof(m_State.rgbButtons));
			return m_State.rgbButtons[i] & 0x80;
		}

		bool IsButtonPress(unsigned int i) const
		{
			_ASSERT(i < _countof(m_State.rgbButtons));
			return !(m_LastFrameState.rgbButtons[i] & 0x80) && (m_State.rgbButtons[i] & 0x80);
		}

		bool IsButtonRelease(unsigned int i) const
		{
			_ASSERT(i < _countof(m_State.rgbButtons));
			return (m_LastFrameState.rgbButtons[i] & 0x80) && !(m_State.rgbButtons[i] & 0x80);
		}
	};

	typedef boost::shared_ptr<Joystick> JoystickPtr;

	class InputMgr
		: public SingletonInstance<InputMgr>
	{
	public:
		InputPtr m_input;

		KeyboardPtr m_keyboard;

		MousePtr m_mouse;

		JoystickPtr m_joystick;

		enum KeyType
		{
			KeyboardButton,
			KeyboardNegativeButton,
			MouseMove,
			MouseButton,
			JoystickAxis,
			JoystickNegativeAxis,
			JoystickPov,
			JoystickNegativePov,
			JoystickButton,
		};

		typedef std::vector<std::pair<KeyType, int> > KeyPairList;

		typedef std::vector<KeyPairList> KeyPairListList;

		KeyPairListList m_BindKeys;

		DWORD m_JoystickAxisDeadZone;

		DWORD m_LastPressKey;

		DWORD m_LastPressTime;

		int m_LastPressCount;

		LONG m_MouseMoveAxisCoef;

	public:
		InputMgr(size_t KeyCount)
			: m_BindKeys(KeyCount)
			, m_JoystickAxisDeadZone(1000)
			, m_LastPressKey(UINT32_MAX)
			, m_LastPressTime(0)
			, m_LastPressCount(0)
			, m_MouseMoveAxisCoef(5000)
		{
		}

		void Create(HINSTANCE hinst, HWND hwnd);

		void Destroy(void);

		bool Capture(double fTime, float fElapsedTime);

		//bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);

		static BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext);

		void BindKey(DWORD Key, KeyType type, int id);

		void UnbindKey(DWORD Key, KeyType type, int id);

		LONG GetKeyAxisRaw(DWORD Key) const;

		bool IsKeyDown(DWORD Key) const;

		bool IsKeyPressRaw(DWORD Key) const;

		bool IsKeyPress(DWORD Key);

		bool IsKeyRelease(DWORD Key) const;
	};
}
