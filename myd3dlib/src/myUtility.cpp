// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "myUtility.h"
#include "libc.h"
#include "myCollision.h"
#include "myDxutApp.h"
#include "myMesh.h"
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/multi_array.hpp>
#include <boost/tuple/tuple_comparison.hpp>

using namespace my;

const int BoxPrimitive::i[36] = {
	0, 1, 2, 2, 3, 0,
	1, 5, 6, 6, 2, 1,
	5, 4, 7, 7, 6, 5,
	4, 0, 3, 3, 7, 4,
	2, 6, 7, 7, 3, 2,
	0, 4, 5, 5, 1, 0};

BoxPrimitive::BoxPrimitive(const AABB& aabb)
	: _1(aabb.m_min.x, aabb.m_min.y, aabb.m_min.z)
	, _2(aabb.m_min.x, aabb.m_min.y, aabb.m_max.z)
	, _3(aabb.m_min.x, aabb.m_max.y, aabb.m_max.z)
	, _4(aabb.m_min.x, aabb.m_max.y, aabb.m_min.z)
	, _5(aabb.m_max.x, aabb.m_min.y, aabb.m_min.z)
	, _6(aabb.m_max.x, aabb.m_min.y, aabb.m_max.z)
	, _7(aabb.m_max.x, aabb.m_max.y, aabb.m_max.z)
	, _8(aabb.m_max.x, aabb.m_max.y, aabb.m_min.z)
{

}

BoxPrimitive::BoxPrimitive(float hx, float hy, float hz)
	: _1(-hx, -hy, -hz)
	, _2(-hx, -hy, hz)
	, _3(-hx, hy, hz)
	, _4(-hx, hy, -hz)
	, _5(hx, -hy, -hz)
	, _6(hx, -hy, hz)
	, _7(hx, hy, hz)
	, _8(hx, hy, -hz)
{

}

BoxPrimitive::BoxPrimitive(float hx, float hy, float hz, const Matrix4& world)
	: _1(Vector3(-hx, -hy, -hz).transformCoord(world))
	, _2(Vector3(-hx, -hy, hz).transformCoord(world))
	, _3(Vector3(-hx, hy, hz).transformCoord(world))
	, _4(Vector3(-hx, hy, -hz).transformCoord(world))
	, _5(Vector3(hx, -hy, -hz).transformCoord(world))
	, _6(Vector3(hx, -hy, hz).transformCoord(world))
	, _7(Vector3(hx, hy, hz).transformCoord(world))
	, _8(Vector3(hx, hy, -hz).transformCoord(world))
{

}

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
	const BoxPrimitive box(aabb);
	for (int i = 0; i < _countof(BoxPrimitive::i); i += 6)
	{
		PushLine(box.v[box.i[i + 0]], box.v[box.i[i + 1]], color);
		PushLine(box.v[box.i[i + 1]], box.v[box.i[i + 2]], color);
		PushLine(box.v[box.i[i + 3]], box.v[box.i[i + 4]], color);
		PushLine(box.v[box.i[i + 4]], box.v[box.i[i + 5]], color);
	}
}

void DrawHelper::PushLineBox(float hx, float hy, float hz, const Matrix4 & world, D3DCOLOR color)
{
	const BoxPrimitive box(hx, hy, hz, world);
	for (int i = 0; i < _countof(BoxPrimitive::i); i += 6)
	{
		PushLine(box.v[box.i[i + 0]], box.v[box.i[i + 1]], color);
		PushLine(box.v[box.i[i + 1]], box.v[box.i[i + 2]], color);
		PushLine(box.v[box.i[i + 3]], box.v[box.i[i + 4]], color);
		PushLine(box.v[box.i[i + 4]], box.v[box.i[i + 5]], color);
	}
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

bool Timer::Step(const float interval)
{
	if (interval > 0 && interval <= m_RemainingTime)
	{
		m_RemainingTime -= interval;

		return true;
	}
	return false;
}

Vector3 BaseCamera::ScreenToWorld(const Vector2 & pt, const Vector2 & dim, float z) const
{
	return Vector3(Lerp(-1.0f, 1.0f, pt.x / dim.x), Lerp(1.0f, -1.0f, pt.y / dim.y), z).transformCoord(m_InverseViewProj);
}

Vector3 BaseCamera::WorldToScreen(const Vector3 & pos, const Vector2 & dim) const
{
	const Vector3 ptProj = pos.transformCoordSafe(m_ViewProj);

	return Vector3(Lerp(0.0f, dim.x, (ptProj.x + 1) / 2), Lerp(0.0f, dim.y, (1 - ptProj.y) / 2), ptProj.z);
}

Frustum BaseCamera::RectangleToFrustum(const my::Rectangle & rc, const Vector2 & dim) const
{
	Vector3 nlt = ScreenToWorld(rc.LeftTop(), dim, 1.0f);
	Vector3 nrt = ScreenToWorld(rc.RightTop(), dim, 1.0f);
	Vector3 nlb = ScreenToWorld(rc.LeftBottom(), dim, 1.0f);
	Vector3 nrb = ScreenToWorld(rc.RightBottom(), dim, 1.0f);
	Vector3 flt = ScreenToWorld(rc.LeftTop(), dim, 0.0f);
	Vector3 frt = ScreenToWorld(rc.RightTop(), dim, 0.0f);
	Vector3 flb = ScreenToWorld(rc.LeftBottom(), dim, 0.0f);
	Vector3 frb = ScreenToWorld(rc.RightBottom(), dim, 0.0f);

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

	m_Proj = Matrix4::OrthoRH(m_Width, m_Height, m_Nz, m_Fz);

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

Ray OrthoCamera::CalculateRay(const Vector2 & pt, const CSize & dim) const
{
	Vector3 dir = -m_View.getColumn<2>().xyz.normalize();

	Vector3 At = ScreenToWorld(pt, Vector2((float)dim.cx, (float)dim.cy), 1.0f);

	return Ray(At, dir);
}

void OrthoCamera::OnDimensionChanged(const CSize & dim)
{
	if (dim.cx > dim.cy)
	{
		m_Width = (float)dim.cx / dim.cy * m_Height;
	}
	else
	{
		m_Height = (float)dim.cy / dim.cx * m_Width;
	}
}

float OrthoCamera::CalculateDimensionScaler(const Vector3 & WorldPos) const
{
	return m_Width > m_Height ? m_Height * 0.5f : m_Width * 0.5f;
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

Ray PerspectiveCamera::CalculateRay(const Vector2 & pt, const CSize & dim) const
{
	Vector3 At = ScreenToWorld(pt, Vector2((float)dim.cx, (float)dim.cy), 1.0f);

	return Ray(At, (At - m_Eye).normalize());
}

void PerspectiveCamera::OnDimensionChanged(const CSize & dim)
{
	m_Aspect = (float)dim.cx / dim.cy;
}

float PerspectiveCamera::CalculateDimensionScaler(const Vector3 & WorldPos) const
{
	float z = Vector4(WorldPos, 1.0f).dot(-m_View.getColumn<2>());
	return z * tan(m_Fov * 0.5f);
}

void ModelViewerCamera::UpdateViewProj(void)
{
	Matrix4 Rotation = Matrix4::RotationYawPitchRoll(m_Euler.y, m_Euler.x, m_Euler.z);

	m_Eye = m_LookAt + Rotation[2].xyz * m_Distance;

	m_View = Matrix4::LookAtRH(m_Eye, m_LookAt, Rotation[1].xyz);

	m_Proj = Matrix4::PerspectiveFovRH(m_Fov, m_Aspect, m_Nz, m_Fz + m_Distance);

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

ProgressiveMesh::ProgressiveMesh(OgreMesh* Mesh)
	: m_Mesh(Mesh)
	, m_Verts(Mesh->GetNumVertices())
{
	VOID* pIndices = m_Mesh->LockIndexBuffer(D3DLOCK_READONLY);
	for (int i = 0; i < m_Mesh->GetNumFaces(); i++)
	{
		PMTriangle tri = { {
			m_Mesh->GetOptions() & D3DXMESH_32BIT ? *((DWORD*)pIndices + i * 3 + 0) : *((WORD*)pIndices + i * 3 + 0),
			m_Mesh->GetOptions() & D3DXMESH_32BIT ? *((DWORD*)pIndices + i * 3 + 1) : *((WORD*)pIndices + i * 3 + 1),
			m_Mesh->GetOptions() & D3DXMESH_32BIT ? *((DWORD*)pIndices + i * 3 + 2) : *((WORD*)pIndices + i * 3 + 2) }, -1 };
		m_Tris.push_back(tri);
	}
	m_Mesh->UnlockIndexBuffer();

	for (int i = 0; i < m_Mesh->m_AttribTable.size(); i++)
	{
		const D3DXATTRIBUTERANGE& rang = m_Mesh->m_AttribTable[i];
		for (int j = rang.FaceStart; j < rang.FaceStart + rang.FaceCount; j++)
		{
			_ASSERT(m_Tris[j].AttribId < 0);
			m_Tris[j].AttribId = i;

			for (int k = 0; k < _countof(m_Tris[j].vi); k++)
			{
				PMVertex& pmv = m_Verts[m_Tris[j].vi[k]];
				_ASSERT(pmv.tris.end() == std::find(pmv.tris.begin(), pmv.tris.end(), j));
				pmv.tris.push_back(j);
			}
		}
	}

	typedef std::map<boost::tuple<float, float, float>, boost::shared_ptr<std::vector<Plane> > > pos_plane_map;
	pos_plane_map ppmap;

	VOID* pVertices = m_Mesh->LockVertexBuffer();
	std::vector<PMVertex>::iterator vert_iter = m_Verts.begin();
	for (; vert_iter != m_Verts.end(); vert_iter++)
	{
		const Vector3& pos = m_Mesh->m_VertexElems.GetPosition((unsigned char*)pVertices + std::distance(m_Verts.begin(), vert_iter) * m_Mesh->GetNumBytesPerVertex());
		std::pair<pos_plane_map::iterator, bool> res = ppmap.insert(std::make_pair(
			boost::make_tuple(pos.x, pos.y, pos.z), boost::shared_ptr<std::vector<Plane> >(new std::vector<Plane>)));
		vert_iter->planes = res.first->second;

		std::vector<int>::iterator tri_iter = vert_iter->tris.begin();
		for (; tri_iter != vert_iter->tris.end(); tri_iter++)
		{
			const Vector3 v[3] = {
				m_Mesh->m_VertexElems.GetPosition((unsigned char*)pVertices + m_Tris[*tri_iter].vi[0] * m_Mesh->GetNumBytesPerVertex()),
				m_Mesh->m_VertexElems.GetPosition((unsigned char*)pVertices + m_Tris[*tri_iter].vi[1] * m_Mesh->GetNumBytesPerVertex()),
				m_Mesh->m_VertexElems.GetPosition((unsigned char*)pVertices + m_Tris[*tri_iter].vi[2] * m_Mesh->GetNumBytesPerVertex()) };

			vert_iter->planes->push_back(Plane::FromTriangle(v[0], v[1], v[2]));
		}
	}
	m_Mesh->UnlockVertexBuffer();

	for (vert_iter = m_Verts.begin(); vert_iter != m_Verts.end(); vert_iter++)
	{
		UpdateNeighbors(vert_iter);
	}

	for (vert_iter = m_Verts.begin(); vert_iter != m_Verts.end(); vert_iter++)
	{
		UpdateCollapseCost(vert_iter);
	}
}

void ProgressiveMesh::UpdateNeighbors(std::vector<PMVertex>::iterator vert_iter)
{
	vert_iter->neighbors.clear();
	std::vector<int>::iterator tri_iter = vert_iter->tris.begin();
	for (; tri_iter != vert_iter->tris.end(); tri_iter++)
	{
		PMTriangle& tri = m_Tris[*tri_iter];
		for (int i = 0; i < _countof(tri.vi); i++)
		{
			if (tri.vi[i] != std::distance(m_Verts.begin(), vert_iter))
			{
				std::pair<std::map<int, int>::iterator, bool> res = vert_iter->neighbors.insert(std::make_pair(tri.vi[i], 1));
				if (!res.second)
				{
					res.first->second++;
				}
			}
		}
	}
	vert_iter->isBorder = vert_iter->neighbors.end() != std::find_if(vert_iter->neighbors.begin(),
		vert_iter->neighbors.end(), boost::lambda::bind(&std::map<int, int>::value_type::second, boost::lambda::_1) <= 1);
}

void ProgressiveMesh::UpdateCollapseCost(std::vector<PMVertex>::iterator vert_iter)
{
	vert_iter->collapsecost = FLT_MAX;
	vert_iter->collapseto = -1;
	VOID* pVertices = m_Mesh->LockVertexBuffer();
	std::map<int, int>::iterator nei_iter = vert_iter->neighbors.begin();
	for (; nei_iter != vert_iter->neighbors.end(); nei_iter++)
	{
		PMVertex& neivert = m_Verts[nei_iter->first];
		_ASSERT(neivert.collapsecost < FLT_MAX);
		if (!vert_iter->isBorder || neivert.isBorder)
		{
			const Vector3& pos = m_Mesh->m_VertexElems.GetPosition((unsigned char*)pVertices + nei_iter->first * m_Mesh->GetNumBytesPerVertex());
			float cost = 0;
			std::vector<Plane>::iterator plane_iter = vert_iter->planes->begin();
			for (; plane_iter != vert_iter->planes->end(); plane_iter++)
			{
				cost += fabsf(plane_iter->DistanceToPoint(pos));
			}

			if (cost < vert_iter->collapsecost)
			{
				vert_iter->collapsecost = cost;
				vert_iter->collapseto = nei_iter->first;
			}
		}
	}
	m_Mesh->UnlockVertexBuffer();
}

void ProgressiveMesh::Collapse(int numCollapses)
{
	for (int Collapse = 0; Collapse < numCollapses; Collapse++)
	{
		float bestCost = FLT_MAX;
		int collapseverti = -1;
		std::vector<PMVertex>::iterator vert_iter = m_Verts.begin();
		for (; vert_iter != m_Verts.end(); vert_iter++)
		{
			if (vert_iter->collapsecost < bestCost)
			{
				collapseverti = std::distance(m_Verts.begin(), vert_iter);
				bestCost = vert_iter->collapsecost;
			}
		}

		if (collapseverti < 0)
		{
			break;
		}

		PMVertex& collapsevert = m_Verts[collapseverti];
		std::vector<int>::iterator tri_iter = collapsevert.tris.begin();
		for (; tri_iter != collapsevert.tris.end(); tri_iter++)
		{
			m_Tris.reserve(m_Tris.size() + 1);  // ! avoid dangling reference
			PMTriangle& tri = m_Tris[*tri_iter];
			_ASSERT(tri.AttribId >= 0);
			int* viend = tri.vi + _countof(tri.vi);
			if (std::find(&tri.vi[0], viend, collapsevert.collapseto) != viend)
			{
				for (int i = 0; i < _countof(tri.vi); i++)
				{
					if (tri.vi[i] != collapseverti)
					{
						PMVertex& neivert = m_Verts[tri.vi[i]];
						std::vector<int>::iterator rem_tri_iter = std::find(neivert.tris.begin(), neivert.tris.end(), *tri_iter);
						_ASSERT(rem_tri_iter != neivert.tris.end());
						neivert.tris.erase(rem_tri_iter);
					}
				}
			}
			else
			{
				PMTriangle new_tri(tri);
				_ASSERT(new_tri.AttribId >= 0);
				int* new_viend = new_tri.vi + _countof(new_tri.vi);
				int* replace_vi = std::find(&new_tri.vi[0], new_viend, collapseverti);
				_ASSERT(replace_vi != new_viend);
				*replace_vi = collapsevert.collapseto;
				m_Tris.push_back(new_tri);
				int new_trii = m_Tris.size() - 1;

				for (int i = 0; i < _countof(tri.vi); i++)
				{
					if (tri.vi[i] != collapseverti)
					{
						PMVertex& neivert = m_Verts[tri.vi[i]];
						std::vector<int>::iterator rem_tri_iter = std::find(neivert.tris.begin(), neivert.tris.end(), *tri_iter);
						_ASSERT(rem_tri_iter != neivert.tris.end());
						*rem_tri_iter = new_trii;
					}
				}

				PMVertex& collapsetovert = m_Verts[collapsevert.collapseto];
				_ASSERT(collapsetovert.tris.end() == std::find(collapsetovert.tris.begin(), collapsetovert.tris.end(), new_trii));
				collapsetovert.tris.push_back(new_trii);
			}
			tri.AttribId = -1;
		}
		collapsevert.collapsecost = FLT_MAX;

		std::map<int, int>::iterator nei_iter = collapsevert.neighbors.begin();
		for (; nei_iter != collapsevert.neighbors.end(); nei_iter++)
		{
			std::vector<PMVertex>::iterator nei_vert_iter = m_Verts.begin() + nei_iter->first;
			UpdateNeighbors(nei_vert_iter);
		}

		for (nei_iter = collapsevert.neighbors.begin(); nei_iter != collapsevert.neighbors.end(); nei_iter++)
		{
			std::vector<PMVertex>::iterator nei_vert_iter = m_Verts.begin() + nei_iter->first;
			UpdateCollapseCost(nei_vert_iter);
		}
	}
}

boost::shared_ptr<OgreMesh> ProgressiveMesh::BuildMesh(void)
{
	DWORD NumFaces = std::count_if(m_Tris.begin(), m_Tris.end(), boost::lambda::bind(&PMTriangle::AttribId, boost::lambda::_1) >= 0);

	OgreMeshPtr mesh(new OgreMesh());
	mesh->m_VertexElems = m_Mesh->m_VertexElems;

	HRESULT hr;
	std::vector<D3DVERTEXELEMENT9> velist = mesh->m_VertexElems.BuildVertexElementList(0);
	D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
	velist.push_back(ve_end);
	if (FAILED(hr = my::D3DContext::getSingleton().m_d3dDevice->CreateVertexDeclaration(velist.data(), &mesh->m_Decl)))
	{
		THROW_D3DEXCEPTION(hr);
	}

	mesh->CreateMesh(NumFaces, m_Mesh->GetNumVertices(), velist.data(), m_Mesh->GetOptions());
	DWORD VertexStride = mesh->GetNumBytesPerVertex();
	_ASSERT(m_Mesh->GetNumBytesPerVertex() == VertexStride);
	VOID* pVertices = mesh->LockVertexBuffer();
	VOID* pOtherVertices = m_Mesh->LockVertexBuffer();
	memcpy(pVertices, pOtherVertices, m_Mesh->GetNumVertices() * VertexStride);
	mesh->UnlockVertexBuffer();
	m_Mesh->UnlockVertexBuffer();

	VOID* pIndices = mesh->LockIndexBuffer();
	DWORD* pAttrBuffer = mesh->LockAttributeBuffer();
	int face_start = 0, face_i = 0;
	for (int i = 0; i < m_Mesh->m_AttribTable.size(); i++)
	{
		D3DXATTRIBUTERANGE rang = m_Mesh->m_AttribTable[i];
		std::vector<PMTriangle>::iterator tri_iter = m_Tris.begin();
		for (; tri_iter != m_Tris.end(); tri_iter++)
		{
			if (tri_iter->AttribId == i)
			{
				if (mesh->GetOptions() & D3DXMESH_32BIT)
				{
					boost::multi_array_ref<DWORD, 1> idx((DWORD*)pIndices, boost::extents[NumFaces * 3]);
					idx[face_i * 3 + 0] = tri_iter->vi[0];
					idx[face_i * 3 + 1] = tri_iter->vi[1];
					idx[face_i * 3 + 2] = tri_iter->vi[2];
				}
				else
				{
					boost::multi_array_ref<WORD, 1> idx((WORD*)pIndices, boost::extents[NumFaces * 3]);
					idx[face_i * 3 + 0] = tri_iter->vi[0];
					idx[face_i * 3 + 1] = tri_iter->vi[1];
					idx[face_i * 3 + 2] = tri_iter->vi[2];
				}
				pAttrBuffer[face_i++] = i;
			}
		}
		rang.FaceStart = face_start;
		rang.FaceCount = face_i - face_start;
		mesh->m_AttribTable.push_back(rang);
		face_start = face_i;
	}
	_ASSERT(face_i == NumFaces);
	mesh->UnlockIndexBuffer();
	mesh->UnlockAttributeBuffer();

	mesh->SetAttributeTable(&mesh->m_AttribTable[0], mesh->m_AttribTable.size());

	return mesh;
}
