#include "StdAfx.h"
#include "myUtility.h"
#include "libc.h"
#include "myCollision.h"
#include "myDxutApp.h"
#include "rapidxml.hpp"
#include <boost/bind.hpp>

using namespace my;

void DrawHelper::BeginLine(void)
{
	m_vertices.clear();
}

void DrawHelper::EndLine(IDirect3DDevice9 * pd3dDevice, const Matrix4 & Transform)
{
	if (!m_vertices.empty())
	{
		HRESULT hr;
		V(pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE));
		V(pd3dDevice->SetTexture(0, NULL));
		V(pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE));
		V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&Transform));
		V(pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, m_vertices.size() / 2, &m_vertices[0], sizeof(m_vertices[0])));
	}
}

void DrawHelper::PushLine(const Vector3 & v0, const Vector3 & v1, D3DCOLOR Color)
{
	m_vertices.push_back(Vertex(v0, Color));
	m_vertices.push_back(Vertex(v1, Color));
}

void DrawHelper::PushWireAABB(const AABB & aabb, D3DCOLOR Color)
{
	Vector3 v[8] = {
		Vector3(aabb.m_min.x, aabb.m_min.y, aabb.m_min.z),
		Vector3(aabb.m_min.x, aabb.m_min.y, aabb.m_max.z),
		Vector3(aabb.m_min.x, aabb.m_max.y, aabb.m_max.z),
		Vector3(aabb.m_min.x, aabb.m_max.y, aabb.m_min.z),
		Vector3(aabb.m_max.x, aabb.m_min.y, aabb.m_min.z),
		Vector3(aabb.m_max.x, aabb.m_min.y, aabb.m_max.z),
		Vector3(aabb.m_max.x, aabb.m_max.y, aabb.m_max.z),
		Vector3(aabb.m_max.x, aabb.m_max.y, aabb.m_min.z),
	};
	PushLine(v[0], v[1], Color); PushLine(v[1], v[2], Color); PushLine(v[2], v[3], Color); PushLine(v[3], v[0], Color);
	PushLine(v[4], v[5], Color); PushLine(v[5], v[6], Color); PushLine(v[6], v[7], Color); PushLine(v[7], v[4], Color);
	PushLine(v[0], v[4], Color); PushLine(v[1], v[5], Color); PushLine(v[2], v[6], Color); PushLine(v[3], v[7], Color);
}

void DrawHelper::PushGrid(float length, float linesEvery, unsigned subLines, D3DCOLOR GridColor, D3DCOLOR AxisColor, const Matrix4 & Transform)
{
	PushLine(Vector3(-length, 0, 0).transformCoord(Transform), Vector3( length, 0, 0).transformCoord(Transform), AxisColor);
	PushLine(Vector3(0, -length, 0).transformCoord(Transform), Vector3(0,  length, 0).transformCoord(Transform), AxisColor);

	float stage = linesEvery / subLines;
	for (float incre = stage; incre < length; incre += stage)
	{
		PushLine(Vector3(-length,  incre, 0).transformCoord(Transform), Vector3( length,  incre, 0).transformCoord(Transform), GridColor);
		PushLine(Vector3(-length, -incre, 0).transformCoord(Transform), Vector3( length, -incre, 0).transformCoord(Transform), GridColor);
		PushLine(Vector3( incre, -length, 0).transformCoord(Transform), Vector3( incre,  length, 0).transformCoord(Transform), GridColor);
		PushLine(Vector3(-incre, -length, 0).transformCoord(Transform), Vector3(-incre,  length, 0).transformCoord(Transform), GridColor);
	}

	PushLine(Vector3(-length,  length, 0).transformCoord(Transform), Vector3( length,  length, 0).transformCoord(Transform), GridColor);
	PushLine(Vector3(-length, -length, 0).transformCoord(Transform), Vector3( length, -length, 0).transformCoord(Transform), GridColor);
	PushLine(Vector3( length, -length, 0).transformCoord(Transform), Vector3( length,  length, 0).transformCoord(Transform), GridColor);
	PushLine(Vector3(-length, -length, 0).transformCoord(Transform), Vector3(-length,  length, 0).transformCoord(Transform), GridColor);
}

void Timer::Step(float fElapsedTime, int MaxIter)
{
	m_RemainingTime = Min(4 * m_Interval, m_RemainingTime + fElapsedTime);
	for(int i = 0; m_RemainingTime >= m_Interval && m_Managed; i++)
	{
		if(m_EventTimer)
			m_EventTimer(m_Interval);

		m_RemainingTime -= m_Interval;
	}
}

TimerPtr TimerMgr::AddTimer(float Interval, TimerEvent EventTimer)
{
	TimerPtr timer(new Timer(Interval, Interval));
	timer->m_EventTimer = EventTimer;
	InsertTimer(timer);
	return timer;
}

void TimerMgr::InsertTimer(TimerPtr timer)
{
	if(timer)
	{
		_ASSERT(!timer->m_Managed);

		m_timerSet.insert(timer);

		timer->m_Managed = true;
	}
}

void TimerMgr::RemoveTimer(TimerPtr timer)
{
	if(timer)
	{
		_ASSERT(timer->m_Managed);

		m_timerSet.erase(timer);

		timer->m_Managed = false;
	}
}

void TimerMgr::RemoveAllTimer(void)
{
	m_timerSet.clear();
}

void TimerMgr::Update(
	double fTime,
	float fElapsedTime)
{
	TimerPtrSet::const_iterator timer_iter = m_timerSet.begin();
	for(; timer_iter != m_timerSet.end(); )
	{
		// ! must step iterator before Update, because TimerEvent will remove self from set
		(*timer_iter++)->Step(fElapsedTime, m_MaxIterCount);
	}
}

Vector3 BaseCamera::ScreenToWorld(const Matrix4 & InverseViewProj, const Vector2 & pt, const Vector2 & dim, float z)
{
	return Vector3(Lerp(-1.0f, 1.0f, pt.x / dim.x), Lerp(1.0f, -1.0f, pt.y / dim.y), z).transformCoord(InverseViewProj);
}

Ray BaseCamera::PerspectiveRay(const Matrix4 & InverseViewProj, const Vector3 & pos, const Vector2 & pt, const Vector2 & dim)
{
	Vector3 At = ScreenToWorld(InverseViewProj, pt, dim, 0.0f);

	return Ray(pos, (At - pos).normalize());
}

Ray BaseCamera::OrthoRay(const Matrix4 & InverseViewProj, const Vector3 & dir, const Vector2 & pt, const Vector2 & dim)
{
	_ASSERT(IS_NORMALIZED(dir));

	Vector3 At = ScreenToWorld(InverseViewProj, pt, dim, 0.0f);

	return Ray(At, dir);
}

Frustum BaseCamera::RectangleToFrustum(const Matrix4 & InverseViewProj, const my::Rectangle & rc, const Vector2 & dim)
{
	Vector3 nlt = ScreenToWorld(InverseViewProj, rc.LeftTop(), dim, 0.0f);
	Vector3 nrt = ScreenToWorld(InverseViewProj, rc.RightTop(), dim, 0.0f);
	Vector3 nlb = ScreenToWorld(InverseViewProj, rc.LeftBottom(), dim, 0.0f);
	Vector3 nrb = ScreenToWorld(InverseViewProj, rc.RightBottom(), dim, 0.0f);
	Vector3 flt = ScreenToWorld(InverseViewProj, rc.LeftTop(), dim, 1.0f);
	Vector3 frt = ScreenToWorld(InverseViewProj, rc.RightTop(), dim, 1.0f);
	Vector3 flb = ScreenToWorld(InverseViewProj, rc.LeftBottom(), dim, 1.0f);
	Vector3 frb = ScreenToWorld(InverseViewProj, rc.RightBottom(), dim, 1.0f);

	return Frustum(
		Plane::FromTriangle(nlt,flt,frt),
		Plane::FromTriangle(nrb,frb,flb),
		Plane::FromTriangle(nlb,flb,flt),
		Plane::FromTriangle(nrt,frt,frb),
		Plane::FromTriangle(nlt,nrt,nrb),
		Plane::FromTriangle(frt,flt,flb));
}

void OrthoCamera::UpdateViewProj(void)
{
	Matrix4 Rotation = Matrix4::RotationYawPitchRoll(m_Eular.y, m_Eular.x, m_Eular.z);

	m_View = (Rotation * Matrix4::Translation(m_Eye)).inverse();

	m_Proj = Matrix4::OrthoRH(m_Diagonal * cos(atan2(1, m_Aspect)), m_Diagonal * sin(atan2(1, m_Aspect)), m_Nz, m_Fz);

	m_ViewProj = m_View * m_Proj;

	m_InverseViewProj = m_ViewProj.inverse();
}

LRESULT OrthoCamera::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	return 0;
}

Ray OrthoCamera::CalculateRay(const Vector2 & pt, const CSize & dim)
{
	return OrthoRay(m_InverseViewProj, -m_View.column<2>().xyz.normalize(), pt, Vector2((float)dim.cx, (float)dim.cy));
}

Frustum OrthoCamera::CalculateFrustum(const my::Rectangle & rc, const CSize & dim)
{
	return RectangleToFrustum(m_InverseViewProj, rc, Vector2((float)dim.cx, (float)dim.cy));
}

void OrthoCamera::OnViewportChanged(const Vector2 & Viewport)
{
	m_Aspect = Viewport.x / Viewport.y;
}

float OrthoCamera::CalculateViewportScaler(Vector3 WorldPos) const
{
	return m_Diagonal * cos(atan2(1, m_Aspect)) * 0.5f;
}

void ModelViewerCamera::UpdateViewProj(void)
{
	Matrix4 Rotation = Matrix4::RotationYawPitchRoll(m_Eular.y, m_Eular.x, m_Eular.z);

	m_Eye = m_LookAt + Rotation[2].xyz * m_Distance;

	m_View = Matrix4::LookAtRH(m_Eye, m_LookAt, Rotation[1].xyz);

	m_Proj = Matrix4::PerspectiveAovRH(m_Fov, m_Aspect, m_Nz, m_Fz);

	m_ViewProj = m_View * m_Proj;

	m_InverseViewProj = m_ViewProj.inverse();
}

LRESULT ModelViewerCamera::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	switch(uMsg)
	{
	case WM_LBUTTONDOWN:
		if ((GetKeyState(VK_MENU) & 0x8000) && m_DragMode == DragModeNone)
		{
			m_DragMode = DragModeRotate;
			m_DragPt.SetPoint((short)LOWORD(lParam),(short)HIWORD(lParam));
			::SetCapture(hWnd);
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_LBUTTONUP:
		if(m_DragMode == DragModeRotate)
		{
			m_DragMode = DragModeNone;
			::ReleaseCapture();
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_MBUTTONDOWN:
		if ((GetKeyState(VK_MENU) & 0x8000) && m_DragMode == DragModeNone)
		{
			m_DragMode = DragModeTrake;
			m_DragPt.SetPoint((short)LOWORD(lParam),(short)HIWORD(lParam));
			::SetCapture(hWnd);
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_MBUTTONUP:
		if (m_DragMode == DragModeTrake)
		{
			m_DragMode = DragModeNone;
			::ReleaseCapture();
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_RBUTTONDOWN:
		if ((GetKeyState(VK_MENU) & 0x8000) && m_DragMode == DragModeNone)
		{
			m_DragMode = DragModeZoom;
			m_DragPt.SetPoint((short)LOWORD(lParam),(short)HIWORD(lParam));
			::SetCapture(hWnd);
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_RBUTTONUP:
		if (m_DragMode == DragModeZoom)
		{
			m_DragMode = DragModeNone;
			::ReleaseCapture();
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_MOUSEMOVE:
		{
			CPoint pt((short)LOWORD(lParam),(short)HIWORD(lParam));
			switch (m_DragMode)
			{
			case DragModeRotate:
				m_Eular.x -= D3DXToRadian((pt.y - m_DragPt.y) * 0.5f);
				m_Eular.y -= D3DXToRadian((pt.x - m_DragPt.x) * 0.5f);
				m_DragPt = pt;
				*pbNoFurtherProcessing = true;
				return 0;

			case DragModeTrake:
				{
					Vector3 Right = m_View.column<0>().xyz.normalize();
					Vector3 Up = m_View.column<1>().xyz.normalize();
					m_LookAt += Right * (float)(m_DragPt.x - pt.x) * 0.03f + Up * (float)(pt.y - m_DragPt.y) * 0.03f;
					m_DragPt = pt;
					*pbNoFurtherProcessing = true;
				}
				return 0;

			case DragModeZoom:
				m_Distance += (m_DragPt.x - pt.x) * 0.03f;
				m_DragPt = pt;
				*pbNoFurtherProcessing = true;
				return 0;
			}
		}
		break;

	case WM_MOUSEWHEEL:
		return 0;
	}
	return 0;
}

Ray ModelViewerCamera::CalculateRay(const Vector2 & pt, const CSize & dim)
{
	return PerspectiveRay(m_InverseViewProj, m_Eye, pt, Vector2((float)dim.cx, (float)dim.cy));
}

Frustum ModelViewerCamera::CalculateFrustum(const my::Rectangle & rc, const CSize & dim)
{
	return RectangleToFrustum(m_InverseViewProj, rc, Vector2((float)dim.cx, (float)dim.cy));
}

void ModelViewerCamera::OnViewportChanged(const Vector2 & Viewport)
{
	m_Aspect = Viewport.x / Viewport.y;
}

float ModelViewerCamera::CalculateViewportScaler(Vector3 WorldPos) const
{
	float z = Vector4(WorldPos, 1.0f).dot(-m_View.column<2>());
	return z * tan(m_Fov * 0.5f);
}

void FirstPersonCamera::UpdateViewProj(void)
{
	Matrix4 Rotation = Matrix4::RotationYawPitchRoll(m_Eular.y, m_Eular.x, m_Eular.z);

	m_Eye += (m_LocalVel * 5.0f * D3DContext::getSingleton().m_fElapsedTime).transform(Rotation).xyz;

	m_View = (Rotation * Matrix4::Translation(m_Eye)).inverse();

	m_Proj = Matrix4::PerspectiveFovRH(m_Fov, m_Aspect, m_Nz, m_Fz);

	m_ViewProj = m_View * m_Proj;

	m_InverseViewProj = m_ViewProj.inverse();
}

LRESULT FirstPersonCamera::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	switch(uMsg)
	{
	case WM_LBUTTONDOWN:
		m_DragMode = 1;
		m_DragPt.SetPoint((short)LOWORD(lParam),(short)HIWORD(lParam));
		::SetCapture(hWnd);
		*pbNoFurtherProcessing = true;
		return 0;

	case WM_LBUTTONUP:
		if(m_DragMode)
		{
			m_DragMode = 0;
			::ReleaseCapture();
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_MOUSEMOVE:
		if(m_DragMode)
		{
			m_Eular.x -= D3DXToRadian(HIWORD(lParam) - m_DragPt.y);
			m_Eular.y -= D3DXToRadian(LOWORD(lParam) - m_DragPt.x);
			m_DragPt.SetPoint((short)LOWORD(lParam),(short)HIWORD(lParam));
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_KEYDOWN:
		switch(wParam)
		{
		case 'W':
			m_LocalVel.z = -1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'S':
			m_LocalVel.z = 1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'A':
			m_LocalVel.x = -1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'D':
			m_LocalVel.x = 1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'E':
			m_LocalVel.y = 1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'Q':
			m_LocalVel.y = -1;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_KEYUP:
		switch(wParam)
		{
		case 'W':
			if(m_LocalVel.z < 0)
				m_LocalVel.z = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'S':
			if(m_LocalVel.z > 0)
				m_LocalVel.z = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'A':
			if(m_LocalVel.x < 0)
				m_LocalVel.x = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'D':
			if(m_LocalVel.x > 0)
				m_LocalVel.x = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'E':
			if(m_LocalVel.y > 0)
				m_LocalVel.y = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'Q':
			if(m_LocalVel.y < 0)
				m_LocalVel.y = 0;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;
	}
	return 0;
}

Ray FirstPersonCamera::CalculateRay(const Vector2 & pt, const CSize & dim)
{
	return PerspectiveRay(m_InverseViewProj, m_Eye, pt, Vector2((float)dim.cx, (float)dim.cy));
}

Frustum FirstPersonCamera::CalculateFrustum(const my::Rectangle & rc, const CSize & dim)
{
	return RectangleToFrustum(m_InverseViewProj, rc, Vector2((float)dim.cx, (float)dim.cy));
}

void FirstPersonCamera::OnViewportChanged(const Vector2 & Viewport)
{
	m_Aspect = Viewport.x / Viewport.y;
}

float FirstPersonCamera::CalculateViewportScaler(Vector3 WorldPos) const
{
	float z = Vector4(WorldPos, 1.0f).dot(m_View.column<2>());
	return z * tan(m_Fov * 0.5f);
}

void InputMgr::Create(HINSTANCE hinst, HWND hwnd)
{
	m_input.reset(new Input);
	m_input->CreateInput(hinst);

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
	m_joystick = desc.joystick;

	::GetCursorPos(&m_MousePos);
	::ScreenToClient(hwnd, &m_MousePos);
}

void InputMgr::Destroy(void)
{
	m_joystick.reset();

	m_input.reset();
}

void InputMgr::Update(double fTime, float fElapsedTime)
{
	if (m_joystick)
	{
		m_joystick->Capture();
	}
}

bool InputMgr::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_KEYDOWN:
		if (!(0x40000000 & lParam) && m_KeyPressedEvent)
		{
			KeyboardEventArg arg(wParam);
			m_KeyPressedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_SYSKEYDOWN:
		if (!(0x40000000 & lParam) && m_KeyPressedEvent)
		{
			KeyboardEventArg arg(wParam);
			m_KeyPressedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_KEYUP:
		if (m_KeyReleasedEvent)
		{
			KeyboardEventArg arg(wParam);
			m_KeyReleasedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_SYSKEYUP:
		if (m_KeyReleasedEvent)
		{
			KeyboardEventArg arg(wParam);
			m_KeyReleasedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_MOUSEMOVE:
		if (m_MouseMovedEvent)
		{
			CPoint OldMousePos(m_MousePos);
			m_MousePos.SetPoint(LOWORD(lParam), HIWORD(lParam));
			MouseMoveEventArg arg(m_MousePos.x - OldMousePos.x, m_MousePos.y - OldMousePos.y, 0);
			m_MouseMovedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_LBUTTONDOWN:
		if (m_MousePressedEvent)
		{
			MouseBtnEventArg arg(0);
			m_MousePressedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_LBUTTONUP:
		if (m_MouseReleasedEvent)
		{
			MouseBtnEventArg arg(0);
			m_MouseReleasedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_MBUTTONDOWN:
		if (m_MousePressedEvent)
		{
			MouseBtnEventArg arg(2);
			m_MousePressedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_MBUTTONUP:
		if (m_MouseReleasedEvent)
		{
			MouseBtnEventArg arg(2);
			m_MouseReleasedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_RBUTTONDOWN:
		if (m_MousePressedEvent)
		{
			MouseBtnEventArg arg(1);
			m_MousePressedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_RBUTTONUP:
		if (m_MouseReleasedEvent)
		{
			MouseBtnEventArg arg(1);
			m_MouseReleasedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_MOUSEWHEEL:
		if (m_MouseMovedEvent)
		{
			MouseMoveEventArg arg(0, 0, (short)HIWORD(wParam) / WHEEL_DELTA);
			m_MouseMovedEvent(&arg);
			return arg.handled;
		}
		break;
	}
	return false;
}

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
