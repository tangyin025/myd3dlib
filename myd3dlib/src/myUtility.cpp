#include "myUtility.h"
#include "libc.h"
#include "myCollision.h"
#include "myDxutApp.h"
#include "rapidxml.hpp"
#include <boost/bind/bind.hpp>

using namespace my;

void DrawHelper::FlushLine(IDirect3DDevice9 * pd3dDevice)
{
	HRESULT hr;
	if (!m_lineVerts.empty())
	{
		V(pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE));
		V(pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, m_lineVerts.size() / 2, &m_lineVerts[0], sizeof(m_lineVerts[0])));
		m_lineVerts.clear();
	}

	if (!m_triVerts.empty())
	{
		V(pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE));
		V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, m_triVerts.size() / 3, &m_triVerts[0], sizeof(m_triVerts[0])));
		m_triVerts.clear();
	}
}

void DrawHelper::PushLineVertex(float x, float y, float z, D3DCOLOR color)
{
	m_lineVerts.push_back(Vertex(x, y, z, color));
}

void DrawHelper::PushLine(const Vector3 & v0, const Vector3 & v1, D3DCOLOR color)
{
	PushLineVertex(v0.x, v0.y, v0.z, color);
	PushLineVertex(v1.x, v1.y, v1.z, color);
}

void DrawHelper::PushLineAABB(const AABB & aabb, D3DCOLOR color)
{
	const Vector3 v[8] = {
		Vector3(aabb.m_min.x, aabb.m_min.y, aabb.m_min.z),
		Vector3(aabb.m_min.x, aabb.m_min.y, aabb.m_max.z),
		Vector3(aabb.m_min.x, aabb.m_max.y, aabb.m_max.z),
		Vector3(aabb.m_min.x, aabb.m_max.y, aabb.m_min.z),
		Vector3(aabb.m_max.x, aabb.m_min.y, aabb.m_min.z),
		Vector3(aabb.m_max.x, aabb.m_min.y, aabb.m_max.z),
		Vector3(aabb.m_max.x, aabb.m_max.y, aabb.m_max.z),
		Vector3(aabb.m_max.x, aabb.m_max.y, aabb.m_min.z) };

	PushLine(v[0], v[1], color); PushLine(v[1], v[2], color); PushLine(v[2], v[3], color); PushLine(v[3], v[0], color);
	PushLine(v[4], v[5], color); PushLine(v[5], v[6], color); PushLine(v[6], v[7], color); PushLine(v[7], v[4], color);
	PushLine(v[0], v[4], color); PushLine(v[1], v[5], color); PushLine(v[2], v[6], color); PushLine(v[3], v[7], color);
}

void DrawHelper::PushLineBox(float hx, float hy, float hz, const Matrix4 & world, D3DCOLOR color)
{
	const Vector3 v[8] = {
		Vector3(-hx, -hy, -hz).transformCoord(world),
		Vector3(-hx, -hy,  hz).transformCoord(world),
		Vector3(-hx,  hy,  hz).transformCoord(world),
		Vector3(-hx,  hy, -hz).transformCoord(world),
		Vector3( hx, -hy, -hz).transformCoord(world),
		Vector3( hx, -hy,  hz).transformCoord(world),
		Vector3( hx,  hy,  hz).transformCoord(world),
		Vector3( hx,  hy, -hz).transformCoord(world) };

	PushLine(v[0], v[1], color); PushLine(v[1], v[2], color); PushLine(v[2], v[3], color); PushLine(v[3], v[0], color);
	PushLine(v[4], v[5], color); PushLine(v[5], v[6], color); PushLine(v[6], v[7], color); PushLine(v[7], v[4], color);
	PushLine(v[0], v[4], color); PushLine(v[1], v[5], color); PushLine(v[2], v[6], color); PushLine(v[3], v[7], color);
}

void DrawHelper::PushLineGrid(float length, float linesEvery, unsigned subLines, D3DCOLOR GridColor, D3DCOLOR AxisColor, const Matrix4 & Transform)
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

void DrawHelper::PushTriangleVertex(float x, float y, float z, D3DCOLOR color)
{
	m_triVerts.push_back(Vertex(x, y, z, color));
}

void DrawHelper::PushTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, D3DCOLOR color)
{
	PushTriangleVertex(v0.x, v0.y, v0.z, color);
	PushTriangleVertex(v1.x, v1.y, v1.z, color);
	PushTriangleVertex(v2.x, v2.y, v2.z, color);
}

bool Timer::Step(void)
{
	if (m_RemainingTime >= m_Interval)
	{
		m_RemainingTime -= m_Interval;

		return true;
	}

	return false;
}

Vector3 BaseCamera::ScreenToWorld(const Vector2 & pt, const Vector2 & dim, float z)
{
	return Vector3(Lerp(-1.0f, 1.0f, pt.x / dim.x), Lerp(1.0f, -1.0f, pt.y / dim.y), z).transformCoord(m_InverseViewProj);
}

Vector3 BaseCamera::WorldToScreen(const Vector3 & pos, const Vector2 & dim)
{
	const Vector3 ptProj = pos.transformCoordSafe(m_ViewProj);

	return Vector3(Lerp(0.0f, dim.x, (ptProj.x + 1) / 2), Lerp(0.0f, dim.y, (1 - ptProj.y) / 2), ptProj.z);
}

Frustum BaseCamera::RectangleToFrustum(const my::Rectangle & rc, const Vector2 & dim)
{
	Vector3 nlt = ScreenToWorld(rc.LeftTop(), dim, 0.0f);
	Vector3 nrt = ScreenToWorld(rc.RightTop(), dim, 0.0f);
	Vector3 nlb = ScreenToWorld(rc.LeftBottom(), dim, 0.0f);
	Vector3 nrb = ScreenToWorld(rc.RightBottom(), dim, 0.0f);
	Vector3 flt = ScreenToWorld(rc.LeftTop(), dim, 1.0f);
	Vector3 frt = ScreenToWorld(rc.RightTop(), dim, 1.0f);
	Vector3 flb = ScreenToWorld(rc.LeftBottom(), dim, 1.0f);
	Vector3 frb = ScreenToWorld(rc.RightBottom(), dim, 1.0f);

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
	Matrix4 Rotation = Matrix4::RotationYawPitchRoll(m_Euler.y, m_Euler.x, m_Euler.z);

	m_View = (Rotation * Matrix4::Translation(m_Eye)).inverse();

	m_Proj = Matrix4::OrthoRH(m_Diagonal * cosf(atan2f(1, m_Aspect)), m_Diagonal * sinf(atan2f(1, m_Aspect)), m_Nz, m_Fz);

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
	Vector3 dir = -m_View.getColumn<2>().xyz.normalize();

	Vector3 At = ScreenToWorld(pt, Vector2((float)dim.cx, (float)dim.cy), 0.0f);

	return Ray(At, dir);
}

Frustum OrthoCamera::CalculateFrustum(const my::Rectangle & rc, const CSize & dim)
{
	return RectangleToFrustum(rc, Vector2((float)dim.cx, (float)dim.cy));
}

void OrthoCamera::OnViewportChanged(const Vector2 & Viewport)
{
	m_Aspect = Viewport.x / Viewport.y;
}

float OrthoCamera::CalculateViewportScaler(Vector3 WorldPos) const
{
	return m_Diagonal * cosf(atan2f(1, m_Aspect)) * 0.5f;
}

void PerspectiveCamera::UpdateViewProj(void)
{
	m_View = Matrix4::Compose(Vector3::one, m_Euler, m_Eye).inverse();

	m_Proj = Matrix4::PerspectiveFovRH(m_Fov, m_Aspect, m_Nz, m_Fz);

	m_ViewProj = m_View * m_Proj;

	m_InverseViewProj = m_ViewProj.inverse();
}

LRESULT PerspectiveCamera::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	return 0;
}

Ray PerspectiveCamera::CalculateRay(const Vector2 & pt, const CSize & dim)
{
	Vector3 At = ScreenToWorld(pt, Vector2((float)dim.cx, (float)dim.cy), 0.0f);

	return Ray(At, (At - m_Eye).normalize());
}

Frustum PerspectiveCamera::CalculateFrustum(const my::Rectangle & rc, const CSize & dim)
{
	return RectangleToFrustum(rc, Vector2((float)dim.cx, (float)dim.cy));
}

void PerspectiveCamera::OnViewportChanged(const Vector2 & Viewport)
{
	m_Aspect = Viewport.x / Viewport.y;
}

float PerspectiveCamera::CalculateViewportScaler(Vector3 WorldPos) const
{
	float z = Vector4(WorldPos, 1.0f).dot(-m_View.getColumn<2>());
	return z * tan(m_Fov * 0.5f);
}

void ModelViewerCamera::UpdateViewProj(void)
{
	Matrix4 Rotation = Matrix4::RotationYawPitchRoll(m_Euler.y, m_Euler.x, m_Euler.z);

	m_Eye = m_LookAt + Rotation[2].xyz * m_Distance;

	m_View = Matrix4::LookAtRH(m_Eye, m_LookAt, Rotation[1].xyz);

	m_Proj = Matrix4::PerspectiveFovRH(m_Fov, m_Aspect, m_Nz, Max(m_Fz, m_Distance * 2.f));

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
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
		if ((GetKeyState(VK_MENU) & 0x8000) && m_DragMode == DragModeNone)
		{
			m_DragMode = DragModeRotate;
			m_DragPt.SetPoint((short)LOWORD(lParam), (short)HIWORD(lParam));
			::SetCapture(hWnd);
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_LBUTTONUP:
		if (m_DragMode == DragModeRotate)
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
			m_DragPt.SetPoint((short)LOWORD(lParam), (short)HIWORD(lParam));
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
			m_DragPt.SetPoint((short)LOWORD(lParam), (short)HIWORD(lParam));
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
		CRect rc;
		::GetClientRect(hWnd, &rc);
		CPoint pt((short)LOWORD(lParam), (short)HIWORD(lParam));
		my::Vector2 delta((float)(m_DragPt.x - pt.x) / rc.Width() * m_Distance, (float)(pt.y - m_DragPt.y) / rc.Height() * m_Distance);
		switch (m_DragMode)
		{
		case DragModeRotate:
		{
			m_Euler.x -= D3DXToRadian((pt.y - m_DragPt.y) * 0.5f);
			m_Euler.y -= D3DXToRadian((pt.x - m_DragPt.x) * 0.5f);
			m_DragPt = pt;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		case DragModeTrake:
		{
			Vector3 Right = m_View.getColumn<0>().xyz.normalize();
			Vector3 Up = m_View.getColumn<1>().xyz.normalize();
			m_LookAt += Right * delta.x + Up * delta.y;
			m_DragPt = pt;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		case DragModeZoom:
		{
			m_Distance += delta.x;
			m_DragPt = pt;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		}
		break;
	}

	case WM_MOUSEWHEEL:
		return 0;
	}
	return 0;
}

void FirstPersonCamera::UpdateViewProj(void)
{
	Matrix4 Rotation = Matrix4::RotationYawPitchRoll(m_Euler.y, m_Euler.x, m_Euler.z);

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
			m_Euler.x -= D3DXToRadian(HIWORD(lParam) - m_DragPt.y);
			m_Euler.y -= D3DXToRadian(LOWORD(lParam) - m_DragPt.x);
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
