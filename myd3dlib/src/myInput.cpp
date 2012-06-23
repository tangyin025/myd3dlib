
#include "stdafx.h"
#include "myInput.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

using namespace my;

InputPtr Input::CreateInput(HINSTANCE hinst)
{
	LPDIRECTINPUT8W input;
	HRESULT hres;
	if(FAILED(hres = DirectInput8Create(hinst, DIRECTINPUT_HEADER_VERSION, IID_IDirectInput8W, (LPVOID *)&input, NULL)))
	{
		THROW_DINPUTEXCEPTION(hres);
	}
	return InputPtr(new Input(input));
}

Keyboard::Keyboard(LPDIRECTINPUTDEVICE8W device)
	: InputDevice(device)
{
	SetDataFormat(&c_dfDIKeyboard);
}

KeyboardPtr Keyboard::CreateKeyboard(LPDIRECTINPUT8W input)
{
	HRESULT hres;
	LPDIRECTINPUTDEVICE8W device;
	if(FAILED(hres = input->CreateDevice(GUID_SysKeyboard, &device, NULL)))
	{
		THROW_DINPUTEXCEPTION(hres);
	}
	return KeyboardPtr(new Keyboard(device));
}

void Keyboard::Capture(void)
{
	hr = m_ptr->Acquire();
	if(SUCCEEDED(hr))
	{
		memcpy(m_PreState, m_CurState, sizeof(m_PreState));

		GetDeviceState(sizeof(m_CurState), m_CurState);
	}
}

BYTE Keyboard::IsKeyDown(DWORD dwIndex)
{
	return m_CurState[dwIndex];
}

bool Keyboard::IsKeyPressed(DWORD dwIndex)
{
	return !m_PreState[dwIndex] && m_CurState[dwIndex];
}

bool Keyboard::IsKeyReleased(DWORD dwIndex)
{
	return m_PreState[dwIndex] && !m_CurState[dwIndex];
}

Mouse::Mouse(LPDIRECTINPUTDEVICE8W device)
	: InputDevice(device)
{
	SetDataFormat(&c_dfDIMouse);
}

MousePtr Mouse::CreateMouse(LPDIRECTINPUT8W input)
{
	HRESULT hres;
	LPDIRECTINPUTDEVICE8W device;
	if(FAILED(hres = input->CreateDevice(GUID_SysMouse, &device, NULL)))
	{
		THROW_DINPUTEXCEPTION(hres);
	}
	return MousePtr(new Mouse(device));
}

void Mouse::Capture(void)
{
	hr = m_ptr->Acquire();
	if(SUCCEEDED(hr))
	{
		memcpy(&m_PreState, &m_CurState, sizeof(m_PreState));

		GetDeviceState(sizeof(m_CurState), &m_CurState);
	}
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
