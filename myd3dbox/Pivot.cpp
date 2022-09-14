#include "stdafx.h"
#include "Pivot.h"
#include "MainApp.h"

using namespace my;

static const float radius[2] = { 0.20f, 0.05f };

static const float offset = 2.0f;

static const float header = 1.0f;

static const float pivot_round = 0.1f;

static const Matrix4 Transform[3] = {
	Matrix4::Identity(), Matrix4::RotationX(D3DXToRadian(90)) * Matrix4::RotationZ(D3DXToRadian(90)), Matrix4::RotationX(D3DXToRadian(-90)) * Matrix4::RotationY(D3DXToRadian(-90)) };

Pivot::Pivot(void)
	: m_Mode(PivotModeMove)
	, m_Pos(0, 0, 0)
	, m_Rot(Quaternion::Identity())
	, m_DragAxis(PivotDragNone)
	, m_Captured(false)
	, m_DragPt(0, 0, 0)
	, m_DragRot(Quaternion::Identity())
	, m_DragPlane(Plane::NormalDistance(Vector3(1, 0, 0), 0))
	, m_DragDeltaPos(0, 0, 0)
	, m_DragDeltaRot(Quaternion::Identity())
{
}

Pivot::~Pivot(void)
{
}

void Pivot::Draw(IDirect3DDevice9 * pd3dDevice, const my::BaseCamera * camera, const D3DSURFACE_DESC * desc, float Scale)
{
	switch (m_Mode)
	{
	case PivotModeMove:
		DrawMoveController(pd3dDevice, Scale, camera->m_View.getColumn<2>().xyz);
		break;
	case PivotModeRot:
		DrawRotController(pd3dDevice, Scale);
		break;
	}

	if (m_DragAxis != Pivot::PivotDragNone)
	{
		switch (m_Mode)
		{
		case Pivot::PivotModeMove:
		{
			my::Vector3 pt = camera->WorldToScreen(m_Pos, my::Vector2((float)desc->Width, (float)desc->Height));
			if (pt.z > 0.0f && pt.z < 1.0f)
			{
				wchar_t buff[256];
				swprintf_s(buff, _countof(buff), L"%f, %f, %f", m_Pos.x, m_Pos.y, m_Pos.z);
				theApp.m_UIRender->PushString(my::Rectangle::LeftTop(floorf(pt.x), floorf(pt.y), 1, 1), buff, D3DCOLOR_ARGB(255, 255, 0, 255), my::Font::AlignLeftTop, theApp.m_Font.get());
			}
			break;
		}
		case Pivot::PivotModeRot:
		{
			my::Vector3 pt = camera->WorldToScreen(m_Pos, my::Vector2((float)desc->Width, (float)desc->Height));
			if (pt.z > 0.0f && pt.z < 1.0f)
			{
				my::Vector3 angle = m_Rot.toEulerAngles();
				wchar_t buff[256];
				swprintf_s(buff, _countof(buff), L"%f, %f, %f", D3DXToDegree(angle.x), D3DXToDegree(angle.y), D3DXToDegree(angle.z));
				theApp.m_UIRender->PushString(my::Rectangle::LeftTop(floorf(pt.x), floorf(pt.y), 1, 1), buff, D3DCOLOR_ARGB(255, 255, 0, 255), my::Font::AlignLeftTop, theApp.m_Font.get());
			}
			break;
		}
		}
	}
}

void Pivot::DrawMoveController(IDirect3DDevice9 * pd3dDevice, float Scale, const my::Vector3 & View)
{
	struct Vertex
	{
		Vector3 pos;
		D3DCOLOR color;
	};

	const D3DCOLOR box_color[3] =
	{
		m_DragAxis == PivotDragPlanX ? D3DCOLOR_ARGB(255,255,255,0) : D3DCOLOR_ARGB(255,255,0,0),
		m_DragAxis == PivotDragPlanY ? D3DCOLOR_ARGB(255,255,255,0) : D3DCOLOR_ARGB(255,0,255,0),
		m_DragAxis == PivotDragPlanZ ? D3DCOLOR_ARGB(255,255,255,0) : D3DCOLOR_ARGB(255,0,0,255)
	};

	const AABB box(0, offset * 0.5f);
	const unsigned int box_stage = 2 * 3;
	const unsigned int pices = 12;
	const unsigned int stage = 4 * 3;
	boost::array<Vertex, box_stage * 3 + pices * stage * 3> vertices;
	for (unsigned int j = 0; j < 3; j++)
	{
		vertices[j * box_stage + 0].pos = Vector3(0, box.m_max[(j + 1) % 3], box.m_min[(j + 2) % 3]).transformCoord(Transform[j]);
		vertices[j * box_stage + 0].color = box_color[j];
		vertices[j * box_stage + 1].pos = Vector3(0, box.m_max[(j + 1) % 3], box.m_max[(j + 2) % 3]).transformCoord(Transform[j]);
		vertices[j * box_stage + 1].color = box_color[j];
		vertices[j * box_stage + 2].pos = Vector3(0, box.m_min[(j + 1) % 3], box.m_max[(j + 2) % 3]).transformCoord(Transform[j]);
		vertices[j * box_stage + 2].color = box_color[j];

		vertices[j * box_stage + 3].pos = Vector3(0, box.m_max[(j + 1) % 3], box.m_min[(j + 2) % 3]).transformCoord(Transform[j]);
		vertices[j * box_stage + 3].color = box_color[j];
		vertices[j * box_stage + 4].pos = Vector3(0, box.m_min[(j + 1) % 3], box.m_max[(j + 2) % 3]).transformCoord(Transform[j]);
		vertices[j * box_stage + 4].color = box_color[j];
		vertices[j * box_stage + 5].pos = Vector3(0, box.m_min[(j + 1) % 3], box.m_min[(j + 2) % 3]).transformCoord(Transform[j]);
		vertices[j * box_stage + 5].color = box_color[j];
	}

	const D3DCOLOR color[3] =
	{
		m_DragAxis == PivotDragAxisX ? D3DCOLOR_ARGB(255,255,255,0) : D3DCOLOR_ARGB(255,255,0,0),
		m_DragAxis == PivotDragAxisY ? D3DCOLOR_ARGB(255,255,255,0) : D3DCOLOR_ARGB(255,0,255,0),
		m_DragAxis == PivotDragAxisZ ? D3DCOLOR_ARGB(255,255,255,0) : D3DCOLOR_ARGB(255,0,0,255)
	};

	for (unsigned int j = 0; j < 3; j++)
	{
		for (unsigned int i = 0; i < pices; i++)
		{
			const float alpha = D3DXToRadian(360.0f / pices);
			const float theta[2] = { i * alpha, (i + 1) * alpha };
			vertices[box_stage * 3 + (j * pices + i) * stage + 0].pos = Vector3(offset + header, 0, 0).transformCoord(Transform[j]);
			vertices[box_stage * 3 + (j * pices + i) * stage + 0].color = color[j];
			vertices[box_stage * 3 + (j * pices + i) * stage + 1].pos = Vector3(offset, radius[0] * cos(theta[0]), radius[0] * sin(theta[0])).transformCoord(Transform[j]);
			vertices[box_stage * 3 + (j * pices + i) * stage + 1].color = color[j];
			vertices[box_stage * 3 + (j * pices + i) * stage + 2].pos = Vector3(offset, radius[0] * cos(theta[1]), radius[0] * sin(theta[1])).transformCoord(Transform[j]);
			vertices[box_stage * 3 + (j * pices + i) * stage + 2].color = color[j];

			vertices[box_stage * 3 + (j * pices + i) * stage + 3].pos = Vector3(offset, radius[0] * cos(theta[1]), radius[0] * sin(theta[1])).transformCoord(Transform[j]);
			vertices[box_stage * 3 + (j * pices + i) * stage + 3].color = color[j];
			vertices[box_stage * 3 + (j * pices + i) * stage + 4].pos = Vector3(offset, radius[0] * cos(theta[0]), radius[0] * sin(theta[0])).transformCoord(Transform[j]);
			vertices[box_stage * 3 + (j * pices + i) * stage + 4].color = color[j];
			vertices[box_stage * 3 + (j * pices + i) * stage + 5].pos = Vector3(offset, 0, 0).transformCoord(Transform[j]);
			vertices[box_stage * 3 + (j * pices + i) * stage + 5].color = color[j];

			vertices[box_stage * 3 + (j * pices + i) * stage + 6].pos = Vector3(offset, radius[1] * cos(theta[1]), radius[1] * sin(theta[1])).transformCoord(Transform[j]);
			vertices[box_stage * 3 + (j * pices + i) * stage + 6].color = color[j];
			vertices[box_stage * 3 + (j * pices + i) * stage + 7].pos = Vector3(offset, radius[1] * cos(theta[0]), radius[1] * sin(theta[0])).transformCoord(Transform[j]);
			vertices[box_stage * 3 + (j * pices + i) * stage + 7].color = color[j];
			vertices[box_stage * 3 + (j * pices + i) * stage + 8].pos = Vector3(0, radius[1] * cos(theta[0]), radius[1] * sin(theta[0])).transformCoord(Transform[j]);
			vertices[box_stage * 3 + (j * pices + i) * stage + 8].color = color[j];

			vertices[box_stage * 3 + (j * pices + i) * stage + 9].pos = Vector3(offset, radius[1] * cos(theta[1]), radius[1] * sin(theta[1])).transformCoord(Transform[j]);
			vertices[box_stage * 3 + (j * pices + i) * stage + 9].color = color[j];
			vertices[box_stage * 3 + (j * pices + i) * stage + 10].pos = Vector3(0, radius[1] * cos(theta[0]), radius[1] * sin(theta[0])).transformCoord(Transform[j]);
			vertices[box_stage * 3 + (j * pices + i) * stage + 10].color = color[j];
			vertices[box_stage * 3 + (j * pices + i) * stage + 11].pos = Vector3(0, radius[1] * cos(theta[1]), radius[1] * sin(theta[1])).transformCoord(Transform[j]);
			vertices[box_stage * 3 + (j * pices + i) * stage + 11].color = color[j];
		}
	}

	HRESULT hr;
	V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&CalculateWorld(Scale)));
	V(pd3dDevice->SetTexture(0, NULL));
	V(pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE));
	V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
	V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertices.size() / 3, &vertices[0], sizeof(Vertex)));
}

void Pivot::DrawRotController(IDirect3DDevice9 * pd3dDevice, float Scale)
{
	struct Vertex
	{
		Vector3 pos;
		D3DCOLOR color;
	};

	const D3DCOLOR color[3] =
	{
		m_DragAxis == PivotDragAxisX ? D3DCOLOR_ARGB(255,255,255,0) : D3DCOLOR_ARGB(255,255,0,0),
		m_DragAxis == PivotDragAxisY ? D3DCOLOR_ARGB(255,255,255,0) : D3DCOLOR_ARGB(255,0,255,0),
		m_DragAxis == PivotDragAxisZ ? D3DCOLOR_ARGB(255,255,255,0) : D3DCOLOR_ARGB(255,0,0,255)
	};

	const unsigned int pices = 36;
	const unsigned int stage = 2 * 3;
	boost::array<Vertex, pices * stage * 3> vertices;
	for (unsigned int j = 0; j < 3; j++)
	{
		for (unsigned int i = 0; i < pices; i++)
		{
			const float alpha = D3DXToRadian(360.0f / pices);
			const float theta[2] = { i * alpha, (i + 1) * alpha };
			vertices[(j * pices + i) * stage + 0].pos = Vector3( radius[0], (offset + header) * cos(theta[0]), (offset + header) * sin(theta[0])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + 0].color = color[j];
			vertices[(j * pices + i) * stage + 1].pos = Vector3(-radius[0], (offset + header) * cos(theta[0]), (offset + header) * sin(theta[0])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + 1].color = color[j];
			vertices[(j * pices + i) * stage + 2].pos = Vector3(-radius[0], (offset + header) * cos(theta[1]), (offset + header) * sin(theta[1])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + 2].color = color[j];

			vertices[(j * pices + i) * stage + 3].pos = Vector3( radius[0], (offset + header) * cos(theta[0]), (offset + header) * sin(theta[0])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + 3].color = color[j];
			vertices[(j * pices + i) * stage + 4].pos = Vector3(-radius[0], (offset + header) * cos(theta[1]), (offset + header) * sin(theta[1])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + 4].color = color[j];
			vertices[(j * pices + i) * stage + 5].pos = Vector3( radius[0], (offset + header) * cos(theta[1]), (offset + header) * sin(theta[1])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + 5].color = color[j];
		}
	}

	HRESULT hr;
	V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&CalculateWorld(Scale)));
	V(pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE));
	V(pd3dDevice->SetTexture(0, NULL));
	V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
	V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertices.size() / 3, &vertices[0], sizeof(Vertex)));
}

Matrix4 Pivot::CalculateWorld(float Scale)
{
	return Matrix4::Compose(Vector3(Scale), m_Rot, m_Pos);
}

bool Pivot::OnLButtonDown(const my::Ray & ray, float Scale)
{
	switch (m_Mode)
	{
	case PivotModeMove:
		return OnMoveControllerLButtonDown(ray, Scale);
	case PivotModeRot:
		return OnRotControllerLButtonDown(ray, Scale);
	}
	return false;
}

bool Pivot::OnMoveControllerLButtonDown(const my::Ray & ray, float Scale)
{
	Matrix4 World = CalculateWorld(Scale);
	Matrix4 trans[3] = {
		(Transform[0] * World).inverse(), (Transform[1] * World).inverse(), (Transform[2] * World).inverse() };
	float minT = FLT_MAX;
	m_DragAxis = PivotDragNone;
	m_DragRot.xyz = m_Pos;
	for (unsigned int i = 0; i < _countof(trans); i++)
	{
		const Ray local_ray = ray.transform(trans[i]);
		RayResult res = IntersectionTests::rayAndCylinder(local_ray.p, local_ray.d, radius[0] + pivot_round, offset + header);
		if (res.first && res.second < minT)
		{
			minT = res.second;
			m_DragAxis = (PivotDragAxis)(PivotDragAxisX + i);
			m_DragPt = (local_ray.p + local_ray.d * res.second).transformCoord(Transform[i] * World);
		}

		res = IntersectionTests::rayAndParallelPlane(local_ray.p, local_ray.d, 0, 0);
		if (res.first && res.second < minT)
		{
			Vector3 local = local_ray.p + local_ray.d * res.second;
			if (local.y > 0 && local.y < offset * 0.5f && local.z > 0 && local.z < offset * 0.5f)
			{
				minT = res.second;
				m_DragAxis = (PivotDragAxis)(PivotDragPlanX + i);
				m_DragPt = local.transformCoord(Transform[i] * World);
			}
		}
	}
	switch (m_DragAxis)
	{
	case PivotDragAxisX:
		m_DragPlane.normal = Vector3::unitX.cross(Vector3::unitX.cross(ray.d)).normalize();
		m_DragPlane.d = -m_DragPt.dot(m_DragPlane.normal);
		m_Captured = true;
		return true;
	case PivotDragAxisY:
		m_DragPlane.normal = Vector3::unitY.cross(Vector3::unitY.cross(ray.d)).normalize();
		m_DragPlane.d = -m_DragPt.dot(m_DragPlane.normal);
		m_Captured = true;
		return true;
	case PivotDragAxisZ:
		m_DragPlane.normal = Vector3::unitZ.cross(Vector3::unitZ.cross(ray.d)).normalize();
		m_DragPlane.d = -m_DragPt.dot(m_DragPlane.normal);
		m_Captured = true;
		return true;
	case PivotDragPlanX:
		m_DragPlane.normal = Vector3::unitX;
		m_DragPlane.d = -m_DragPt.x;
		m_Captured = true;
		return true;
	case PivotDragPlanY:
		m_DragPlane.normal = Vector3::unitY;
		m_DragPlane.d = -m_DragPt.y;
		m_Captured = true;
		return true;
	case PivotDragPlanZ:
		m_DragPlane.normal = Vector3::unitZ;
		m_DragPlane.d = -m_DragPt.z;
		m_Captured = true;
		return true;
	}
	return false;
}

bool Pivot::OnRotControllerLButtonDown(const my::Ray & ray, float Scale)
{
	Matrix4 trans = CalculateWorld(Scale).inverse();
	Ray local_ray = ray.transform(trans);
	RayResult res = IntersectionTests::rayAndSphere(local_ray.p, local_ray.d, Vector3::zero, offset + header);
	m_DragAxis = PivotDragNone;
	if(res.first)
	{
		Vector3 k = local_ray.p + local_ray.d * res.second;
		if(fabsf(k.x) <= radius[0] + pivot_round)
		{
			m_DragAxis = PivotDragAxisX;
			m_DragPt = k;
			m_DragRot = m_Rot;
			m_Captured = true;
			return true;
		}
		else if(fabsf(k.y) <= radius[0] + pivot_round)
		{
			m_DragAxis = PivotDragAxisY;
			m_DragPt = k;
			m_DragRot = m_Rot;
			m_Captured = true;
			return true;
		}
		else if(fabsf(k.z) <= radius[0] + pivot_round)
		{
			m_DragAxis = PivotDragAxisZ;
			m_DragPt = k;
			m_DragRot = m_Rot;
			m_Captured = true;
			return true;
		}
	}
	return false;
}

bool Pivot::OnMouseMove(const my::Ray & ray, float Scale)
{
	if (m_Captured)
	{
		switch (m_Mode)
		{
		case PivotModeMove:
			return OnMoveControllerMouseMove(ray, Scale);
		case PivotModeRot:
			return OnRotControllerMouseMove(ray, Scale);
		}
	}
	return false;
}

bool Pivot::OnMoveControllerMouseMove(const my::Ray & ray, float Scale)
{
	RayResult res = IntersectionTests::rayAndHalfSpace(ray.p, ray.d, m_DragPlane);
	if(res.first)
	{
		Vector3 pt = ray.p + ray.d * res.second;
		switch(m_DragAxis)
		{
		case PivotDragAxisX:
			m_DragDeltaPos = Vector3(pt.x - m_DragPt.x, 0, 0);
			m_Pos = m_DragRot.xyz + m_DragDeltaPos;
			if (GetKeyState('X') & 0x8000)
			{
				m_Pos.x = Align(m_Pos.x, theApp.default_grid_lines_every / theApp.default_grid_subdivisions);
			}
			return true;
		case PivotDragAxisY:
			m_DragDeltaPos = Vector3(0, pt.y - m_DragPt.y, 0);
			m_Pos = m_DragRot.xyz + m_DragDeltaPos;
			if (GetKeyState('X') & 0x8000)
			{
				m_Pos.y = Align(m_Pos.y, theApp.default_grid_lines_every / theApp.default_grid_subdivisions);
			}
			return true;
		case PivotDragAxisZ:
			m_DragDeltaPos = Vector3(0, 0, pt.z - m_DragPt.z);
			m_Pos = m_DragRot.xyz + m_DragDeltaPos;
			if (GetKeyState('X') & 0x8000)
			{
				m_Pos.z = Align(m_Pos.z, theApp.default_grid_lines_every / theApp.default_grid_subdivisions);
			}
			return true;
		case PivotDragPlanX:
			m_DragDeltaPos = Vector3(0, pt.y - m_DragPt.y, pt.z - m_DragPt.z);
			m_Pos = m_DragRot.xyz + m_DragDeltaPos;
			if (GetKeyState('X') & 0x8000)
			{
				m_Pos.y = Align(m_Pos.y, theApp.default_grid_lines_every / theApp.default_grid_subdivisions);
				m_Pos.z = Align(m_Pos.z, theApp.default_grid_lines_every / theApp.default_grid_subdivisions);
			}
			return true;
		case PivotDragPlanY:
			m_DragDeltaPos = Vector3(pt.x - m_DragPt.x, 0, pt.z - m_DragPt.z);
			m_Pos = m_DragRot.xyz + m_DragDeltaPos;
			if (GetKeyState('X') & 0x8000)
			{
				m_Pos.x = Align(m_Pos.x, theApp.default_grid_lines_every / theApp.default_grid_subdivisions);
				m_Pos.z = Align(m_Pos.z, theApp.default_grid_lines_every / theApp.default_grid_subdivisions);
			}
			return true;
		case PivotDragPlanZ:
			m_DragDeltaPos = Vector3(pt.x - m_DragPt.x, pt.y - m_DragPt.y, 0);
			m_Pos = m_DragRot.xyz + m_DragDeltaPos;
			if (GetKeyState('X') & 0x8000)
			{
				m_Pos.x = Align(m_Pos.x, theApp.default_grid_lines_every / theApp.default_grid_subdivisions);
				m_Pos.y = Align(m_Pos.y, theApp.default_grid_lines_every / theApp.default_grid_subdivisions);
			}
			return true;
		}
	}
	return false;
}

bool Pivot::OnRotControllerMouseMove(const my::Ray & ray, float Scale)
{
	Matrix4 trans = Matrix4::Compose(Vector3(Scale), m_DragRot, m_Pos).inverse();
	Ray local_ray = ray.transform(trans);
    RayResult res = IntersectionTests::rayAndSphere(local_ray.p, local_ray.d, Vector3::zero, offset + header);
    Vector3 k;
    if(res.first)
    {
        k = local_ray.p + local_ray.d * res.second;
    }
    else
	{
        k = local_ray.p - local_ray.d * local_ray.p.dot(local_ray.d);
	}
	Quaternion rot = Quaternion::Identity();
    switch(m_DragAxis)
    {
    case PivotDragAxisX:
		rot = Quaternion::RotationFromTo(Vector3(0, m_DragPt.y, m_DragPt.z).normalize(), Vector3(0, k.y, k.z).normalize(), Vector3(1, 0, 0)) * m_DragRot;
		m_DragDeltaRot = rot / m_Rot;
		m_Rot = rot;
        return true;
    case PivotDragAxisY:
		rot = Quaternion::RotationFromTo(Vector3(m_DragPt.x, 0, m_DragPt.z).normalize(), Vector3(k.x, 0, k.z).normalize(), Vector3(0, 1, 0)) * m_DragRot;
		m_DragDeltaRot = rot / m_Rot;
		m_Rot = rot;
		return true;
    case PivotDragAxisZ:
		rot = Quaternion::RotationFromTo(Vector3(m_DragPt.x, m_DragPt.y, 0).normalize(), Vector3(k.x, k.y, 0).normalize(), Vector3(0, 0, 1)) * m_DragRot;
		m_DragDeltaRot = rot / m_Rot;
		m_Rot = rot;
        return true;
    }
    return false;
}

bool Pivot::OnLButtonUp(const my::Ray & ray)
{
	switch (m_DragAxis)
	{
	case PivotDragAxisX:
	case PivotDragAxisY:
	case PivotDragAxisZ:
	case PivotDragPlanX:
	case PivotDragPlanY:
	case PivotDragPlanZ:
		m_Captured = false;
		m_DragDeltaPos = Vector3(0,0,0);
		m_DragDeltaRot = Quaternion::Identity();
		return true;
	}
	return false;
}
