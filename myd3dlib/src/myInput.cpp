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

CComPtr<IDirectInputDevice8> Input::CreateDevice(REFGUID rguid)
{
	CComPtr<IDirectInputDevice8> ret;
	if(FAILED(hr = m_ptr->CreateDevice(rguid, &ret, NULL)))
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
	SAFE_RELEASE(m_ptr);
}

void InputDevice::Create(LPDIRECTINPUTDEVICE8 device)
{
	_ASSERT(!m_ptr);

	m_ptr = device;
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
