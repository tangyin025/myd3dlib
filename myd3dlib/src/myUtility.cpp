#include "StdAfx.h"
#include "myUtility.h"
#include "libc.h"
#include "myCollision.h"
#include "myDxutApp.h"
#include "rapidxml.hpp"
#include <boost/bind.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <fstream>

using namespace my;

void DrawHelper::DrawLine(
	IDirect3DDevice9 * pd3dDevice,
	const Vector3 & v0,
	const Vector3 & v1,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	struct Vertex
	{
		float x, y, z;
		D3DCOLOR color;
	};

	Vertex v[2];
	v[0].x = v0.x; v[0].y = v0.y; v[0].z = v0.z; v[0].color = Color;
	v[1].x = v1.x; v[1].y = v1.y; v[1].z = v1.z; v[1].color = Color;

	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, _countof(v) / 2, v, sizeof(v[0]));
}

void DrawHelper::DrawSphere(
	IDirect3DDevice9 * pd3dDevice,
	float radius,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	DrawSpereStage(pd3dDevice, radius, 0, 20, 0, Color, world);
}

void DrawHelper::DrawBox(
	IDirect3DDevice9 * pd3dDevice,
	const Vector3 & halfSize,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	struct Vertex
	{
		float x, y, z;
		D3DCOLOR color;
	};

	Vertex v[8];
	v[0].x = -halfSize.x; v[0].y = -halfSize.y; v[0].z = -halfSize.z; v[0].color = Color;
	v[1].x =  halfSize.x; v[1].y = -halfSize.y; v[1].z = -halfSize.z; v[1].color = Color;
	v[2].x = -halfSize.x; v[2].y =  halfSize.y; v[2].z = -halfSize.z; v[2].color = Color;
	v[3].x =  halfSize.x; v[3].y =  halfSize.y; v[3].z = -halfSize.z; v[3].color = Color;
	v[4].x = -halfSize.x; v[4].y =  halfSize.y; v[4].z =  halfSize.z; v[4].color = Color;
	v[5].x =  halfSize.x; v[5].y =  halfSize.y; v[5].z =  halfSize.z; v[5].color = Color;
	v[6].x = -halfSize.x; v[6].y = -halfSize.y; v[6].z =  halfSize.z; v[6].color = Color;
	v[7].x =  halfSize.x; v[7].y = -halfSize.y; v[7].z =  halfSize.z; v[7].color = Color;

	unsigned short idx[12 * 2];
	int i = 0;
	idx[i++] = 0; idx[i++] = 1; idx[i++] = 1; idx[i++] = 3; idx[i++] = 3; idx[i++] = 2; idx[i++] = 2; idx[i++] = 0;
	idx[i++] = 0; idx[i++] = 6; idx[i++] = 1; idx[i++] = 7; idx[i++] = 3; idx[i++] = 5; idx[i++] = 2; idx[i++] = 4;
	idx[i++] = 6; idx[i++] = 7; idx[i++] = 7; idx[i++] = 5; idx[i++] = 5; idx[i++] = 4; idx[i++] = 4; idx[i++] = 6;

	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
	pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, _countof(v), _countof(idx) / 2, idx, D3DFMT_INDEX16, v, sizeof(v[0]));
}

void DrawHelper::DrawTriangle(
	IDirect3DDevice9 * pd3dDevice,
	const Vector3 & v0,
	const Vector3 & v1,
	const Vector3 & v2,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	struct Vertex
	{
		float x, y, z;
		D3DCOLOR color;
	};

	Vertex v[4];
	v[0].x = v0.x; v[0].y = v0.y; v[0].z = v0.z; v[0].color = Color;
	v[1].x = v1.x; v[1].y = v1.y; v[1].z = v1.z; v[1].color = Color;
	v[2].x = v2.x; v[2].y = v2.y; v[2].z = v2.z; v[2].color = Color;
	v[3] = v[0];

	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, _countof(v) - 1, v, sizeof(v[0]));
}

void DrawHelper::DrawSpereStage(
	IDirect3DDevice9 * pd3dDevice,
	float radius,
	int VSTAGE_BEGIN,
	int VSTAGE_END,
	float offsetY,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	struct Vertex
	{
		float x, y, z;
		D3DCOLOR color;
	};

	const int VSTAGE = 20;
	const int HSTAGE = 20;
	Vertex v[VSTAGE * HSTAGE * 4];
	for(int j = VSTAGE_BEGIN; j < VSTAGE_END; j++)
	{
		for(int i = 0; i < HSTAGE; i++)
		{
			float Theta[2] = {2 * D3DX_PI / HSTAGE * i, 2 * D3DX_PI / HSTAGE * (i + 1)};
			float Fi[2] = {D3DX_PI / VSTAGE * j, D3DX_PI / VSTAGE * (j + 1)};
			Vertex * pv = &v[(j * HSTAGE + i) * 4];
			pv[0].x = radius * sin(Fi[0]) * cos(Theta[0]);
			pv[0].y = radius * cos(Fi[0]) + offsetY;
			pv[0].z = radius * sin(Fi[0]) * sin(Theta[0]);
			pv[0].color = Color;

			pv[1].x = radius * sin(Fi[0]) * cos(Theta[1]);
			pv[1].y = radius * cos(Fi[0]) + offsetY;
			pv[1].z = radius * sin(Fi[0]) * sin(Theta[1]);
			pv[1].color = Color;

			pv[2] = pv[0];

			pv[3].x = radius * sin(Fi[1]) * cos(Theta[0]);
			pv[3].y = radius * cos(Fi[1]) + offsetY;
			pv[3].z = radius * sin(Fi[1]) * sin(Theta[0]);
			pv[3].color = Color;
		}
	}

	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, _countof(v) / 2, v, sizeof(v[0]));
}

void DrawHelper::DrawCylinderStage(
	IDirect3DDevice9 * pd3dDevice,
	float radius,
	float y0,
	float y1,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	struct Vertex
	{
		float x, y, z;
		D3DCOLOR color;
	};

	const int HSTAGE = 20;
	Vertex v[HSTAGE * 4];
	for(int i = 0; i < HSTAGE; i++)
	{
		float Theta[2] = {2 * D3DX_PI / HSTAGE * i, 2 * D3DX_PI / HSTAGE * (i + 1)};
		Vertex * pv = &v[i * 4];
		pv[0].x = radius * cos(Theta[0]);
		pv[0].y = y0;
		pv[0].z = radius * sin(Theta[0]);
		pv[0].color = Color;

		pv[1].x = radius * cos(Theta[1]);
		pv[1].y = y0;
		pv[1].z = radius * sin(Theta[1]);
		pv[1].color = Color;

		pv[2] = pv[0];

		pv[3].x = radius * cos(Theta[0]);
		pv[3].y = y1;
		pv[3].z = radius * sin(Theta[0]);
		pv[3].color = Color;
	}

	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, _countof(v) / 2, v, sizeof(v[0]));
}

void DrawHelper::DrawCapsule(
	IDirect3DDevice9 * pd3dDevice,
	float radius,
	float height,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	float y0 = height * 0.5f;
	float y1 = -y0;
	DrawSpereStage(pd3dDevice, radius, 0, 10, y0, Color, world);
	DrawSpereStage(pd3dDevice, radius, 10, 20, y1, Color, world);
	DrawCylinderStage(pd3dDevice, radius, y0, y1, Color, world);
}

void DrawHelper::DrawGrid(
	IDirect3DDevice9 * pd3dDevice,
	float length,
	float linesEvery,
	unsigned subLines,
	D3DCOLOR Color)
{
	struct Vertex
	{
		float x, y, z;
		D3DCOLOR color;
		Vertex(float _x, float _y, float _z, D3DCOLOR _color)
			: x(_x), y(_y), z(_z), color(_color)
		{
		}
	};

	std::vector<Vertex> v;
	v.push_back(Vertex(-length, 0, 0, D3DCOLOR_ARGB(255,0,0,0)));
	v.push_back(Vertex( length, 0, 0, D3DCOLOR_ARGB(255,0,0,0)));
	v.push_back(Vertex(0, 0, -length, D3DCOLOR_ARGB(255,0,0,0)));
	v.push_back(Vertex(0, 0,  length, D3DCOLOR_ARGB(255,0,0,0)));

	float stage = linesEvery / subLines;
	for(float incre = stage; incre < length; incre += stage)
	{
		v.push_back(Vertex(-length, 0,  incre, Color));
		v.push_back(Vertex( length, 0,  incre, Color));
		v.push_back(Vertex(-length, 0, -incre, Color));
		v.push_back(Vertex( length, 0, -incre, Color));
		v.push_back(Vertex( incre, 0, -length, Color));
		v.push_back(Vertex( incre, 0,  length, Color));
		v.push_back(Vertex(-incre, 0, -length, Color));
		v.push_back(Vertex(-incre, 0,  length, Color));
	}

	v.push_back(Vertex(-length, 0,  length, Color));
	v.push_back(Vertex( length, 0,  length, Color));
	v.push_back(Vertex(-length, 0, -length, Color));
	v.push_back(Vertex( length, 0, -length, Color));
	v.push_back(Vertex( length, 0, -length, Color));
	v.push_back(Vertex( length, 0,  length, Color));
	v.push_back(Vertex(-length, 0, -length, Color));
	v.push_back(Vertex(-length, 0,  length, Color));

	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&Matrix4::identity);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, v.size() / 2, &v[0], sizeof(v[0]));
}

TimerPtr TimerMgr::AddTimer(float Interval, ControlEvent EventTimer)
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
		_ASSERT(timer->m_Removed);

		m_timerSet.insert(timer);

		timer->m_Removed = false;
	}
}

void TimerMgr::RemoveTimer(TimerPtr timer)
{
	if(timer)
	{
		_ASSERT(!timer->m_Removed);

		m_timerSet.erase(timer);

		timer->m_Removed = true;
	}
}

void TimerMgr::RemoveAllTimer(void)
{
	m_timerSet.clear();
}

void TimerMgr::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	TimerPtrSet::const_iterator timer_iter = m_timerSet.begin();
	for(; timer_iter != m_timerSet.end(); )
	{
		TimerPtr timer = (*timer_iter++);
		timer->m_RemainingTime = Min(m_MaxIterCount * timer->m_Interval, timer->m_RemainingTime + fElapsedTime);
		for(int i = 0; timer->m_RemainingTime >= timer->m_Interval && !timer->m_Removed; i++)
		{
			if(timer->m_EventTimer)
				timer->m_EventTimer(m_DefaultArgs);

			timer->m_RemainingTime -= timer->m_Interval;
		}
	}
}

BaseCamera::~BaseCamera(void)
{
}

void Camera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_View = Matrix4::Translation(-m_Position) * Matrix4::RotationQuaternion(m_Orientation.inverse());

	m_Proj = Matrix4::PerspectiveFovRH(m_Fov, m_Aspect, m_Nz, m_Fz);

	m_ViewProj = m_View * m_Proj;

	m_InverseViewProj = m_ViewProj.inverse();
}

LRESULT Camera::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	return 0;
}

std::pair<Vector3, Vector3> Camera::CalculateRay(const Vector2 & pt, const CSize & dim)
{
	Vector3 ptProj(Lerp(-1.0f, 1.0f, pt.x / dim.cx), Lerp(1.0f, -1.0f, pt.y / dim.cy), 1.0f);

	return std::make_pair(m_Position, (ptProj.transformCoord(m_InverseViewProj) - m_Position).normalize());
}

void ModelViewerCamera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_Orientation = Quaternion::RotationYawPitchRoll(m_Rotation.y, m_Rotation.x, 0);

	m_Position = Vector3(0,0,m_Distance).transform(m_Orientation) + m_LookAt;

	m_View = Matrix4::Translation(-m_LookAt)
		* Matrix4::RotationY(-m_Rotation.y)
		* Matrix4::RotationX(-m_Rotation.x)
		* Matrix4::RotationZ(-m_Rotation.z)
		* Matrix4::Translation(Vector3(0,0,-m_Distance));

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
		m_bDrag = true;
		m_DragPos.SetPoint(LOWORD(lParam),HIWORD(lParam));
		SetCapture(hWnd);
		*pbNoFurtherProcessing = true;
		return 0;

	case WM_LBUTTONUP:
		if(m_bDrag)
		{
			m_bDrag = false;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_MOUSEMOVE:
		if(m_bDrag)
		{
			m_Rotation.x -= D3DXToRadian(HIWORD(lParam) - m_DragPos.y);
			m_Rotation.y -= D3DXToRadian(LOWORD(lParam) - m_DragPos.x);
			m_DragPos.SetPoint(LOWORD(lParam),HIWORD(lParam));
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_MOUSEWHEEL:
		m_Distance -= (short)HIWORD(wParam) / WHEEL_DELTA;
		*pbNoFurtherProcessing = true;
		return 0;
	}
	return 0;
}

void FirstPersonCamera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_Orientation = Quaternion::RotationYawPitchRoll(m_Rotation.y, m_Rotation.x, 0);

	m_Position += (m_Velocity * 5.0f * fElapsedTime).transform(m_Orientation);

	m_View = Matrix4::Translation(-m_Position)
		* Matrix4::RotationY(-m_Rotation.y)
		* Matrix4::RotationX(-m_Rotation.x)
		* Matrix4::RotationZ(-m_Rotation.z);

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
		m_bDrag = true;
		m_DragPos.SetPoint(LOWORD(lParam),HIWORD(lParam));
		SetCapture(hWnd);
		*pbNoFurtherProcessing = true;
		return 0;

	case WM_LBUTTONUP:
		if(m_bDrag)
		{
			m_bDrag = false;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_MOUSEMOVE:
		if(m_bDrag)
		{
			m_Rotation.x -= D3DXToRadian(HIWORD(lParam) - m_DragPos.y);
			m_Rotation.y -= D3DXToRadian(LOWORD(lParam) - m_DragPos.x);
			m_DragPos.SetPoint(LOWORD(lParam),HIWORD(lParam));
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_KEYDOWN:
		switch(wParam)
		{
		case 'W':
			m_Velocity.z = -1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'S':
			m_Velocity.z = 1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'A':
			m_Velocity.x = -1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'D':
			m_Velocity.x = 1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'E':
			m_Velocity.y = 1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'Q':
			m_Velocity.y = -1;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_KEYUP:
		switch(wParam)
		{
		case 'W':
			if(m_Velocity.z < 0)
				m_Velocity.z = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'S':
			if(m_Velocity.z > 0)
				m_Velocity.z = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'A':
			if(m_Velocity.x < 0)
				m_Velocity.x = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'D':
			if(m_Velocity.x > 0)
				m_Velocity.x = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'E':
			if(m_Velocity.y > 0)
				m_Velocity.y = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'Q':
			if(m_Velocity.y < 0)
				m_Velocity.y = 0;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;
	}
	return 0;
}

void DialogMgr::SetDlgViewport(const Vector2 & vp)
{
	m_Camera.m_Position = Vector3(vp.x * 0.5f, vp.y * 0.5f, -vp.y * 0.5f * cot(m_Camera.m_Fov / 2));

	m_Camera.m_Rotation.x = D3DXToRadian(180);

	m_Camera.m_Aspect = vp.x / vp.y;

	m_Camera.OnFrameMove(0,0);

	DialogPtrSetMap::iterator dlg_layer_iter = m_dlgSetMap.begin();
	for(; dlg_layer_iter != m_dlgSetMap.end(); dlg_layer_iter++)
	{
		DialogPtrList::iterator dlg_iter = dlg_layer_iter->second.begin();
		for(; dlg_iter != dlg_layer_iter->second.end(); dlg_iter++)
		{
			if((*dlg_iter)->EventAlign)
				(*dlg_iter)->EventAlign(EventArgsPtr(new EventArgs()));
		}
	}
}

Vector2 DialogMgr::GetDlgViewport(void) const
{
	return Vector2(-m_Camera.m_View._41*2, m_Camera.m_View._42*2);
}

void DialogMgr::Draw(
	UIRender * ui_render,
	double fTime,
	float fElapsedTime)
{
	ui_render->SetViewProj(m_Camera.m_ViewProj);

	DialogPtrSetMap::iterator dlg_layer_iter = m_dlgSetMap.begin();
	for(; dlg_layer_iter != m_dlgSetMap.end(); dlg_layer_iter++)
	{
		DialogPtrList::iterator dlg_iter = dlg_layer_iter->second.begin();
		for(; dlg_iter != dlg_layer_iter->second.end(); dlg_iter++)
		{
			ui_render->SetWorld((*dlg_iter)->m_World);

			(*dlg_iter)->Draw(ui_render, fElapsedTime);
		}
	}
}

bool DialogMgr::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	ControlPtr ControlFocus = Dialog::s_ControlFocus.lock();
	if(ControlFocus)
	{
		_ASSERT(!boost::dynamic_pointer_cast<Dialog>(ControlFocus));
		if(ControlFocus->MsgProc(hWnd, uMsg, wParam, lParam))
			return true;
	}

	switch(uMsg)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if(ControlFocus)
		{
			if(ControlFocus->HandleKeyboard(uMsg, wParam, lParam))
				return true;
		}

		if(uMsg == WM_KEYDOWN)
		{
			DialogPtrSetMap::iterator dlg_layer_iter = m_dlgSetMap.begin();
			for(; dlg_layer_iter != m_dlgSetMap.end(); dlg_layer_iter++)
			{
				DialogPtrList::iterator dlg_iter = dlg_layer_iter->second.begin();
				for(; dlg_iter != dlg_layer_iter->second.end(); dlg_iter++)
				{
					if((*dlg_iter)->HandleKeyboard(uMsg, wParam, lParam))
						return true;
				}
			}
		}
		break;

	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
	case WM_MOUSEWHEEL:
		{
			CRect ClientRect;
			GetClientRect(hWnd, &ClientRect);
			std::pair<Vector3, Vector3> ray = m_Camera.CalculateRay(
				Vector2((short)LOWORD(lParam) + 0.5f, (short)HIWORD(lParam) + 0.5f), ClientRect.Size());

			DialogPtrSetMap::reverse_iterator dlg_layer_iter = m_dlgSetMap.rbegin();
			for(; dlg_layer_iter != m_dlgSetMap.rend(); dlg_layer_iter++)
			{
				DialogPtrList::reverse_iterator dlg_iter = dlg_layer_iter->second.rbegin();
				for(; dlg_iter != dlg_layer_iter->second.rend(); dlg_iter++)
				{
					// ! 只处理看得见的 Dialog
					if((*dlg_iter)->GetEnabled() && (*dlg_iter)->GetVisible())
					{
						Vector3 dialogNormal = Vector3(0, 0, 1).transformNormal((*dlg_iter)->m_World);
						float dialogDistance = ((Vector3 &)(*dlg_iter)->m_World[3]).dot(dialogNormal);
						IntersectionTests::TestResult result = IntersectionTests::rayAndHalfSpace(ray.first, ray.second, dialogNormal, dialogDistance);

						if(result.first)
						{
							Vector3 ptInt(ray.first + ray.second * result.second);
							Vector3 pt = ptInt.transformCoord((*dlg_iter)->m_World.inverse());
							Vector2 ptLocal = pt.xy - (*dlg_iter)->m_Location;

							// ! 只处理自己的 ControlFocus
							if(ControlFocus && (*dlg_iter)->ContainsControl(ControlFocus))
							{
								if(ControlFocus->HandleMouse(uMsg, ptLocal, wParam, lParam))
									return true;
							}

							// ! 只处理自己的 m_ControlMouseOver
							ControlPtr ControlPtd = (*dlg_iter)->GetControlAtPoint(ptLocal);
							ControlPtr m_ControlMouseOver = (*dlg_iter)->m_ControlMouseOver.lock();
							if(ControlPtd != m_ControlMouseOver)
							{
								if(m_ControlMouseOver)
									m_ControlMouseOver->OnMouseLeave();

								if(ControlPtd && ControlPtd->GetEnabled())
								{
									(*dlg_iter)->m_ControlMouseOver = ControlPtd;
									ControlPtd->OnMouseEnter();
								}
								else
									(*dlg_iter)->m_ControlMouseOver.reset();
							}

							if(ControlPtd && ControlPtd->GetEnabled())
							{
								if(ControlPtd->HandleMouse(uMsg, ptLocal, wParam, lParam))
								{
									Dialog::RequestFocus(ControlPtd);
									return true;
								}
							}

							if(uMsg == WM_LBUTTONDOWN
								&& (*dlg_iter)->ContainsControl(ControlFocus) && !(*dlg_iter)->ContainsPoint(pt.xy))
							{
								// ! 用以解决对话框控件丢失焦点
								ControlFocus->OnFocusOut();
								Dialog::s_ControlFocus.reset();
							}

							if((*dlg_iter)->HandleMouse(uMsg, pt.xy, wParam, lParam))
							{
								// ! 强制让自己具有 FocusControl
								(*dlg_iter)->ForceFocusControl();
								return true;
							}
						}
					}
				}
			}
		}
		break;
	}

	return false;
}

void DialogMgr::InsertDlg(DialogPtr dlg)
{
	m_dlgSetMap[0].push_back(dlg);

	if(dlg->EventAlign)
		dlg->EventAlign(EventArgsPtr(new EventArgs()));
}

void DialogMgr::RemoveDlg(DialogPtr dlg)
{
	DialogPtrList::iterator dlg_iter = std::find(m_dlgSetMap[0].begin(), m_dlgSetMap[0].end(), dlg);
	if(dlg_iter != m_dlgSetMap[0].end())
	{
		m_dlgSetMap[0].erase(dlg_iter);
	}
}

void DialogMgr::RemoveAllDlg()
{
	m_dlgSetMap[0].clear();
}

void EmitterMgr::Update(
	double fTime,
	float fElapsedTime)
{
	EmitterPtrSet::iterator emitter_iter = m_EmitterSet.begin();
	for(; emitter_iter != m_EmitterSet.end(); emitter_iter++)
	{
		(*emitter_iter)->Update(fTime, fElapsedTime);
	}
}

void EmitterMgr::Draw(
	EmitterInstance * pInstance,
	Camera * pCamera,
	double fTime,
	float fElapsedTime)
{
	pInstance->SetViewProj(pCamera->m_ViewProj);

	EmitterPtrSet::iterator emitter_iter = m_EmitterSet.begin();
	for(; emitter_iter != m_EmitterSet.end(); emitter_iter++)
	{
		switch((*emitter_iter)->m_WorldType)
		{
		case Emitter::WorldTypeWorld:
			pInstance->SetWorld(Matrix4::Identity());
			break;

		default:
			pInstance->SetWorld(Matrix4::RotationQuaternion((*emitter_iter)->m_Orientation) * Matrix4::Translation((*emitter_iter)->m_Position));
			break;
		}

		switch((*emitter_iter)->m_DirectionType)
		{
		case Emitter::DirectionTypeCamera:
			pInstance->SetDirection(
				Vector3(0,0,1).transform(pCamera->m_Orientation),
				Vector3(0,1,0).transform(pCamera->m_Orientation),
				Vector3(1,0,0).transform(pCamera->m_Orientation));
			break;

		case Emitter::DirectionTypeVertical:
			{
				Vector3 Up(0,1,0);
				Vector3 Right = Vector3(0,0,-1).transform(pCamera->m_Orientation).cross(Up);
				Vector3 Dir = Right.cross(Up);
				pInstance->SetDirection(Dir, Up, Right);
			}
			break;

		case Emitter::DirectionTypeHorizontal:
			pInstance->SetDirection(Vector3(0,1,0), Vector3(0,0,1), Vector3(-1,0,0));
			break;
		}

		(*emitter_iter)->Draw(pInstance, fTime, fElapsedTime);
	}
}

void EmitterMgr::InsertEmitter(EmitterPtr emitter)
{
	_ASSERT(m_EmitterSet.end() == m_EmitterSet.find(emitter));

	m_EmitterSet.insert(emitter);
}

void EmitterMgr::RemoveEmitter(EmitterPtr emitter)
{
	m_EmitterSet.erase(emitter);
}

void EmitterMgr::RemoveAllEmitter(void)
{
	m_EmitterSet.clear();
}

EffectParameterBase::~EffectParameterBase(void)
{
}

template <>
void EffectParameter<bool>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetBool(Name.c_str(), m_Value);
}

template <>
void EffectParameter<float>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetFloat(Name.c_str(), m_Value);
}

template <>
void EffectParameter<int>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetInt(Name.c_str(), m_Value);
}

template <>
void EffectParameter<Vector4>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetVector(Name.c_str(), m_Value);
}

template <>
void EffectParameter<Matrix4>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetMatrix(Name.c_str(), m_Value);
}

template <>
void EffectParameter<std::string>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetString(Name.c_str(), m_Value.c_str());
}

template <>
void EffectParameter<BaseTexturePtr>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetTexture(Name.c_str(), m_Value);
}

void EffectParameterMap::SetBool(const std::string & Name, bool Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<bool>(EffectParameterBase::EffectParameterTypeBool, Value));
}

void EffectParameterMap::SetFloat(const std::string & Name, float Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<float>(EffectParameterBase::EffectParameterTypeFloat, Value));
}

void EffectParameterMap::SetInt(const std::string & Name, int Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<int>(EffectParameterBase::EffectParameterTypeInt, Value));
}

void EffectParameterMap::SetVector(const std::string & Name, const Vector4 & Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<Vector4>(EffectParameterBase::EffectParameterTypeVector, Value));
}

void EffectParameterMap::SetMatrix(const std::string & Name, const Matrix4 & Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<Matrix4>(EffectParameterBase::EffectParameterTypeMatrix, Value));
}

void EffectParameterMap::SetString(const std::string & Name, const std::string & Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<std::string>(EffectParameterBase::EffectParameterTypeString, Value));
}

void EffectParameterMap::SetTexture(const std::string & Name, BaseTexturePtr Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<BaseTexturePtr>(EffectParameterBase::EffectParameterTypeTexture, Value));
}
//
//void Material::OnResetDevice(void)
//{
//}
//
//void Material::OnLostDevice(void)
//{
//}
//
//void Material::OnDestroyDevice(void)
//{
//}
//
//void Material::ApplyParameterBlock(void)
//{
//	if(m_Effect)
//	{
//		EffectParameterMap::const_iterator param_iter = EffectParameterMap::begin();
//		for(; param_iter != EffectParameterMap::end(); param_iter++)
//		{
//			param_iter->second->SetParameter(m_Effect.get(), param_iter->first);
//		}
//	}
//}
//
//UINT Material::Begin(DWORD Flags)
//{
//	if(m_Effect)
//	{
//		return m_Effect->Begin(Flags);
//	}
//	return 0;
//}
//
//void Material::BeginPass(UINT Pass)
//{
//	if(m_Effect)
//	{
//		m_Effect->BeginPass(Pass);
//	}
//}
//
//void Material::EndPass(void)
//{
//	if(m_Effect)
//	{
//		m_Effect->EndPass();
//	}
//}
//
//void Material::End(void)
//{
//	if(m_Effect)
//	{
//		m_Effect->End();
//	}
//}
//
//void Material::DrawMeshSubset(Mesh * pMesh, DWORD AttribId)
//{
//	ApplyParameterBlock();
//
//	UINT Pass = Begin(0);
//	for(UINT p = 0; p < Pass; p++)
//	{
//		BeginPass(p);
//		pMesh->DrawSubset(AttribId);
//		EndPass();
//	}
//	End();
//}
//
//namespace boost
//{
//	namespace serialization
//	{
//		template <class Archive>
//		void serialize(Archive & ar, Vector3 & obj, const unsigned int version)
//		{
//			ar & obj.x;
//			ar & obj.y;
//			ar & obj.z;
//		}
//
//		template <class Archive>
//		void serialize(Archive & ar, Vector4 & obj, const unsigned int version)
//		{
//			ar & obj.x;
//			ar & obj.y;
//			ar & obj.z;
//			ar & obj.w;
//		}
//
//		template <class Archive>
//		void serialize(Archive & ar, Quaternion & obj, const unsigned int version)
//		{
//			ar & obj.x;
//			ar & obj.y;
//			ar & obj.z;
//			ar & obj.w;
//		}
//
//		template <class Archive>
//		void serialize(Archive & ar, Matrix4 & obj, const unsigned int version)
//		{
//			ar & obj[0];
//			ar & obj[1];
//			ar & obj[2];
//			ar & obj[3];
//		}
//
//		template <class Archive>
//		void serialize(Archive & ar, SplineNode & obj, const unsigned int version)
//		{
//			ar & obj.x;
//			ar & obj.y;
//			ar & obj.k0;
//			ar & obj.k;
//		}
//
//		template <class Archive>
//		void serialize(Archive & ar, Spline & obj, const unsigned int version)
//		{
//			ar & boost::serialization::base_object<std::vector<SplineNodePtr> >(obj);
//		}
//
//		template <class Archive, typename T>
//		void serialize(Archive & ar, EmitterParameter<T> & obj, const unsigned int version)
//		{
//			ar & boost::serialization::base_object<Spline>(obj);
//			ar & obj.m_Value;
//		}
//	}
//}
//
//class ResourceMgr::MaterialIORequest : public IORequest
//{
//protected:
//	std::string m_path;
//
//	ResourceMgr * m_arc;
//
//	CachePtr m_cache;
//
//public:
//	MaterialIORequest(const ResourceCallback & callback, const std::string & path, ResourceMgr * arc)
//		: m_path(path)
//		, m_arc(arc)
//	{
//		if(callback)
//		{
//			m_callbacks.push_back(callback);
//		}
//	}
//
//	virtual void DoLoad(void)
//	{
//		if(m_arc->CheckPath(m_path))
//		{
//			m_cache = m_arc->OpenStream(m_path)->GetWholeCache();
//		}
//	}
//
//	static void MaterialSetEffect(ResourceCallbackBoundlePtr boundle, DeviceRelatedObjectBasePtr effect_res)
//	{
//		boost::dynamic_pointer_cast<Material>(boundle->m_res)->m_Effect = boost::dynamic_pointer_cast<Effect>(effect_res);
//	}
//
//	static void MaterialSetEffectTextureParameter(ResourceCallbackBoundlePtr boundle, std::string value, DeviceRelatedObjectBasePtr texture_res)
//	{
//		boost::dynamic_pointer_cast<Material>(boundle->m_res)->SetTexture(value, boost::dynamic_pointer_cast<BaseTexture>(texture_res));
//	}
//
//	virtual void OnLoadEffect(ResourceCallbackBoundlePtr boundle, const std::string & path, const EffectMacroPairList & macros)
//	{
//		m_arc->LoadEffectAsync(path, macros, boost::bind(&MaterialIORequest::MaterialSetEffect, boundle, _1));
//	}
//
//	virtual void OnLoadTexture(ResourceCallbackBoundlePtr boundle, const std::string & path, const std::string & name)
//	{
//		m_arc->LoadTextureAsync(path, boost::bind(&MaterialIORequest::MaterialSetEffectTextureParameter, boundle, name, _1));
//	}
//
//	virtual void OnPostBuildResource(ResourceCallbackBoundlePtr boundle)
//	{
//		boundle->m_callbacks = m_callbacks;
//		m_callbacks.clear();
//	}
//
//	template <class Archive>
//	void Serialize(Archive & ar, EffectParameterMap & effect_param_map, ResourceCallbackBoundlePtr boundle)
//	{
//		if(Archive::is_saving::value)
//		{
//			size_t count = effect_param_map.size(); ar & count;
//			EffectParameterMap::iterator param_iter = effect_param_map.begin();
//			for(; param_iter != effect_param_map.end(); param_iter++)
//			{
//				ar & const_cast<std::string &>(param_iter->first);
//				ar & const_cast<EffectParameterBase::EffectParameterType &>(param_iter->second->m_Type);
//				switch(param_iter->second->m_Type)
//				{
//				case EffectParameterBase::EffectParameterTypeBool:
//					ar & boost::dynamic_pointer_cast<EffectParameter<bool> >(param_iter->second)->m_Value;
//					break;
//				case EffectParameterBase::EffectParameterTypeFloat:
//					ar & boost::dynamic_pointer_cast<EffectParameter<float> >(param_iter->second)->m_Value;
//					break;
//				case EffectParameterBase::EffectParameterTypeInt:
//					ar & boost::dynamic_pointer_cast<EffectParameter<int> >(param_iter->second)->m_Value;
//					break;
//				case EffectParameterBase::EffectParameterTypeVector:
//					ar & boost::dynamic_pointer_cast<EffectParameter<Vector4> >(param_iter->second)->m_Value;
//					break;
//				case EffectParameterBase::EffectParameterTypeMatrix:
//					ar & boost::dynamic_pointer_cast<EffectParameter<Matrix4> >(param_iter->second)->m_Value;
//					break;
//				case EffectParameterBase::EffectParameterTypeString:
//					ar & boost::dynamic_pointer_cast<EffectParameter<std::string> >(param_iter->second)->m_Value;
//					break;
//				case EffectParameterBase::EffectParameterTypeTexture:
//					ar & m_arc->GetResourceKey(boost::dynamic_pointer_cast<EffectParameter<BaseTexturePtr> >(param_iter->second)->m_Value);
//					break;
//				}
//			}
//		}
//		else
//		{
//			size_t count; ar & count;
//			while(count--)
//			{
//				std::string name; ar & name;
//				EffectParameterBase::EffectParameterType Type; ar & Type;
//				switch(Type)
//				{
//				case EffectParameterBase::EffectParameterTypeBool:
//					{
//						bool value; ar & value; effect_param_map.SetBool(name, value);
//					}
//					break;
//				case EffectParameterBase::EffectParameterTypeFloat:
//					{
//						float value; ar & value; effect_param_map.SetFloat(name, value);
//					}
//					break;
//				case EffectParameterBase::EffectParameterTypeInt:
//					{
//						int value; ar & value; effect_param_map.SetInt(name, value);
//					}
//					break;
//				case EffectParameterBase::EffectParameterTypeVector:
//					{
//						Vector4 value; ar & value; effect_param_map.SetVector(name, value);
//					}
//					break;
//				case EffectParameterBase::EffectParameterTypeMatrix:
//					{
//						Matrix4 value; ar & value; effect_param_map.SetMatrix(name, value);
//					}
//					break;
//				case EffectParameterBase::EffectParameterTypeString:
//					{
//						std::string value; ar & value; effect_param_map.SetString(name, value);
//					}
//					break;
//				case EffectParameterBase::EffectParameterTypeTexture:
//					{
//						std::string path; ar & path; OnLoadTexture(boundle, path, name);
//					}
//					break;
//				}
//			}
//		}
//	}
//
//	template <class Archive>
//	void Serialize(Archive & ar, Material & material, ResourceCallbackBoundlePtr boundle)
//	{
//		if(Archive::is_saving::value)
//		{
//			ar & m_arc->GetResourceKey(material.m_Effect);
//		}
//		else
//		{
//			std::string path; ar & path; OnLoadEffect(boundle, path, EffectMacroPairList());
//		}
//		Serialize(ar, static_cast<EffectParameterMap &>(material), boundle);
//	}
//
//	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
//	{
//		if(m_cache)
//		{
//			class membuf : public std::streambuf
//			{
//			public:
//				membuf(const char * buff, size_t size)
//				{
//					char * p = const_cast<char *>(buff);
//					setg(p, p, p + size);
//				}
//			};
//
//			MaterialPtr ret(new Material());
//			ResourceCallbackBoundlePtr boundle(new ResourceCallbackBoundle(ret));
//
//			membuf mb((char *)&(*m_cache)[0], m_cache->size());
//			std::istream ims(&mb);
//			boost::archive::text_iarchive ia(ims);
//
//			Serialize(ia, *ret, boundle);
//
//			m_res = ret;
//			OnPostBuildResource(boundle);
//		}
//	}
//};
//
//void ResourceMgr::LoadMaterialAsync(const std::string & path, const ResourceCallback & callback)
//{
//	LoadResourceAsync(path, IORequestPtr(new MaterialIORequest(callback, path, this)));
//}
//
//MaterialPtr ResourceMgr::LoadMaterial(const std::string & path)
//{
//	class SyncMaterialIORequest : public MaterialIORequest
//	{
//	public:
//		SyncMaterialIORequest(const ResourceCallback & callback, const std::string & path, ResourceMgr * arc)
//			: MaterialIORequest(callback, path, arc)
//		{
//		}
//
//		virtual void OnLoadEffect(ResourceCallbackBoundlePtr boundle, const std::string & path, const EffectMacroPairList & macros)
//		{
//			boost::dynamic_pointer_cast<Material>(boundle->m_res)->m_Effect = m_arc->LoadEffect(path, macros);
//		}
//
//		virtual void OnLoadTexture(ResourceCallbackBoundlePtr boundle, const std::string & path, const std::string & name)
//		{
//			boost::dynamic_pointer_cast<Material>(boundle->m_res)->SetTexture(name, m_arc->LoadTexture(path));
//		}
//
//		virtual void OnPostBuildResource(ResourceCallbackBoundlePtr boundle)
//		{
//		}
//	};
//
//	IORequestPtr request = LoadResourceAsync(path, IORequestPtr(new SyncMaterialIORequest(ResourceCallback(), path, this)));
//	CheckRequest(path, request, INFINITE);
//	return boost::dynamic_pointer_cast<Material>(request->m_res);
//}
//
//void ResourceMgr::SaveMaterial(const std::string & path, MaterialPtr material)
//{
//	std::ofstream ofs(GetFullPath(path).c_str());
//	boost::archive::text_oarchive oa(ofs);
//	MaterialIORequest request(ResourceCallback(), path, this);
//	request.Serialize(oa, *material, ResourceCallbackBoundlePtr());
//}
//
//class ResourceMgr::EmitterIORequest : public IORequest
//{
//public:
//	std::string m_path;
//
//	ResourceMgr * m_arc;
//
//	CachePtr m_cache;
//
//public:
//	EmitterIORequest(const ResourceCallback & callback, const std::string & path, ResourceMgr * arc)
//		: m_path(path)
//		, m_arc(arc)
//	{
//		if(callback)
//		{
//			m_callbacks.push_back(callback);
//		}
//	}
//
//	void EmitterSetTexture(ResourceCallbackBoundlePtr boundle, DeviceRelatedObjectBasePtr texture_res)
//	{
//		boost::dynamic_pointer_cast<Emitter>(boundle->m_res)->m_Texture = boost::dynamic_pointer_cast<BaseTexture>(texture_res);
//	}
//
//	virtual void OnLoadTexture(ResourceCallbackBoundlePtr boundle, const std::string & path)
//	{
//		m_arc->LoadTextureAsync(path, boost::bind(&EmitterIORequest::EmitterSetTexture, this, boundle, _1));
//	}
//
//	virtual void OnPostBuildResource(ResourceCallbackBoundlePtr boundle)
//	{
//		boundle->m_callbacks = m_callbacks;
//		m_callbacks.clear();
//	}
//
//	template <class Archive>
//	void Serialize(Archive & ar, Emitter & emitter, ResourceCallbackBoundlePtr boundle)
//	{
//		ar & emitter.m_WorldType;
//		ar & emitter.m_DirectionType;
//		ar & emitter.m_Position;
//		ar & emitter.m_Orientation;
//		ar & emitter.m_ParticleLifeTime;
//		ar & emitter.m_ParticleColorA;
//		ar & emitter.m_ParticleColorR;
//		ar & emitter.m_ParticleColorG;
//		ar & emitter.m_ParticleColorB;
//		ar & emitter.m_ParticleSizeX;
//		ar & emitter.m_ParticleSizeY;
//		ar & emitter.m_ParticleAngle;
//		ar & emitter.m_ParticleAnimFPS;
//		ar & emitter.m_ParticleAnimColumn;
//		ar & emitter.m_ParticleAnimRow;
//		if(Archive::is_saving::value)
//		{
//			ar & m_arc->GetResourceKey(emitter.m_Texture);
//		}
//		else
//		{
//			std::string path;
//			ar & path;
//			OnLoadTexture(boundle, path);
//		}
//	}
//
//	template <class Archive>
//	void Serialize(Archive & ar, SphericalEmitter & emitter, ResourceCallbackBoundlePtr boundle)
//	{
//		Serialize(ar, static_cast<Emitter &>(emitter), boundle);
//		ar & emitter.m_Time;
//		ar & emitter.m_SpawnInterval;
//		ar & emitter.m_RemainingSpawnTime;
//		ar & emitter.m_HalfSpawnArea;
//		ar & emitter.m_SpawnSpeed;
//		ar & emitter.m_SpawnInclination;
//		ar & emitter.m_SpawnAzimuth;
//		ar & emitter.m_SpawnLoopTime;
//	}
//
//	virtual void DoLoad(void)
//	{
//		if(m_arc->CheckPath(m_path))
//		{
//			m_cache = m_arc->OpenStream(m_path)->GetWholeCache();
//		}
//	}
//
//	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
//	{
//		if(m_cache)
//		{
//			class membuf : public std::streambuf
//			{
//			public:
//				membuf(const char * buff, size_t size)
//				{
//					char * p = const_cast<char *>(buff);
//					setg(p, p, p + size);
//				}
//			};
//
//			EmitterPtr ret;
//			ResourceCallbackBoundlePtr boundle;
//
//			membuf mb((char *)&(*m_cache)[0], m_cache->size());
//			std::istream ims(&mb);
//			boost::archive::text_iarchive ia(ims);
//			Emitter::EmitterType type;
//			ia >> type;
//			switch(type)
//			{
//			case Emitter::EmitterTypeDefault:
//				ret.reset(new Emitter());
//				boundle.reset(new ResourceCallbackBoundle(ret));
//				Serialize(ia, *ret, boundle);
//				break;
//
//			case Emitter::EmitterTypeSpherical:
//				ret.reset(new SphericalEmitter());
//				boundle.reset(new ResourceCallbackBoundle(ret));
//				Serialize(ia, static_cast<SphericalEmitter &>(*ret), boundle);
//				break;
//			}
//
//			m_res = ret;
//			OnPostBuildResource(boundle);
//		}
//	}
//};
//
//void ResourceMgr::LoadEmitterAsync(const std::string & path, const ResourceCallback & callback)
//{
//	LoadResourceAsync(path, IORequestPtr(new EmitterIORequest(callback, path, this)));
//}
//
//EmitterPtr ResourceMgr::LoadEmitter(const std::string & path)
//{
//	class SyncEmitterIORequest : public EmitterIORequest
//	{
//	public:
//		SyncEmitterIORequest(const ResourceCallback & callback, const std::string & path, ResourceMgr * arc)
//			: EmitterIORequest(callback, path, arc)
//		{
//		}
//
//		virtual void OnLoadTexture(ResourceCallbackBoundlePtr boundle, const std::string & path)
//		{
//			boost::dynamic_pointer_cast<Emitter>(boundle->m_res)->m_Texture = m_arc->LoadTexture(path);
//		}
//
//		virtual void OnPostBuildResource(ResourceCallbackBoundlePtr boundle)
//		{
//		}
//	};
//
//	IORequestPtr request = LoadResourceAsync(path, IORequestPtr(new SyncEmitterIORequest(ResourceCallback(), path, this)));
//	CheckRequest(path, request, INFINITE);
//	return boost::dynamic_pointer_cast<Emitter>(request->m_res);
//}
//
//void ResourceMgr::SaveEmitter(const std::string & path, EmitterPtr emitter)
//{
//	std::ofstream ofs(GetFullPath(path).c_str());
//	boost::archive::text_oarchive oa(ofs);
//	EmitterIORequest request(ResourceCallback(), path, this);
//	oa << emitter->m_Type;
//	switch(emitter->m_Type)
//	{
//	case Emitter::EmitterTypeDefault:
//		request.Serialize(oa, *emitter, ResourceCallbackBoundlePtr());
//		break;
//	case Emitter::EmitterTypeSpherical:
//		request.Serialize(oa, dynamic_cast<SphericalEmitter &>(*emitter), ResourceCallbackBoundlePtr());
//		break;
//	}
//}
