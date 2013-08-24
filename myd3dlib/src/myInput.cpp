#include "stdafx.h"
#include "myInput.h"

using namespace my;

Input::Input(void)
: m_ptr(NULL)
{
}

Input::~Input(void)
{
	SAFE_RELEASE(m_ptr);
}

void Input::Create(IDirectInput8 * dinput)
{
	_ASSERT(!m_ptr);

	m_ptr = dinput;
}

void Input::CreateInput(HINSTANCE hinst)
{
	LPDIRECTINPUT8 input;
	HRESULT hres;
	if(FAILED(hres = DirectInput8Create(hinst, DIRECTINPUT_HEADER_VERSION, IID_IDirectInput8, (LPVOID *)&input, NULL)))
	{
		THROW_DINPUTEXCEPTION(hres);
	}

	Create(input);
}

void Keyboard::CreateKeyboard(LPDIRECTINPUT8 input)
{
	HRESULT hres;
	LPDIRECTINPUTDEVICE8 device;
	if(FAILED(hres = input->CreateDevice(GUID_SysKeyboard, &device, NULL)))
	{
		THROW_DINPUTEXCEPTION(hres);
	}

	Create(device);

	SetDataFormat(&c_dfDIKeyboard);

	ZeroMemory(&m_CurState, sizeof(m_CurState));
}

void Keyboard::Capture(void)
{
	memcpy(m_PreState, m_CurState, sizeof(m_PreState));

	hr = m_ptr->GetDeviceState(sizeof(m_CurState), m_CurState);
	if(FAILED(hr))
	{
		hr = m_ptr->Acquire();
		if(SUCCEEDED(hr))
		{
			GetDeviceState(sizeof(m_CurState), m_CurState);
		}
	}
}

void Mouse::CreateMouse(LPDIRECTINPUT8 input)
{
	HRESULT hres;
	LPDIRECTINPUTDEVICE8 device;
	if(FAILED(hres = input->CreateDevice(GUID_SysMouse, &device, NULL)))
	{
		THROW_DINPUTEXCEPTION(hres);
	}

	Create(device);

	SetDataFormat(&c_dfDIMouse);

	ZeroMemory(&m_CurState, sizeof(m_CurState));
}

void Mouse::Capture(void)
{
	memcpy(&m_PreState, &m_CurState, sizeof(m_PreState));

	hr = m_ptr->GetDeviceState(sizeof(m_CurState), &m_CurState);
	if(FAILED(hr))
	{
		hr = m_ptr->Acquire();
		if(SUCCEEDED(hr))
		{
			GetDeviceState(sizeof(m_CurState), &m_CurState);
		}
	}
}
