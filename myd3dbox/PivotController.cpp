#include "stdafx.h"
#include "PivotController.h"

using namespace my;

static const float radius[2] = { 0.15f, 0.05f };

static const float offset = 2.0f;

static const float header = 1.0f;

static const Matrix4 Transform[3] = {
	Matrix4::Identity(), Matrix4::RotationZ(D3DXToRadian(90)), Matrix4::RotationY(D3DXToRadian(-90)) };

PivotController::PivotController(void)
	: m_Pos(Vector3::zero)
	, m_Rot(Quaternion::Identity())
	, m_PivotDragAxis(PivotDragNone)
	, m_PivotMode(PivotModeMove)
	, m_Scale(1.0f)
	, m_Captured(false)
{
}

PivotController::~PivotController(void)
{
}

void PivotController::Draw(IDirect3DDevice9 * pd3dDevice, const my::BaseCamera * camera, const D3DSURFACE_DESC * desc)
{
	UpdateScale(camera, desc);

	UpdateWorld();

	switch (m_PivotMode)
	{
	case PivotModeMove:
		DrawMoveController(pd3dDevice);
		break;
	case PivotModeRot:
		DrawRotController(pd3dDevice);
		break;
	}
}

void PivotController::DrawMoveController(IDirect3DDevice9 * pd3dDevice)
{
	struct Vertex
	{
		Vector3 pos;
		D3DCOLOR color;
	};

	const unsigned int pices = 12;
	const unsigned int stage = 12;
	boost::array<Vertex, pices * stage * 3> vertices;
	const D3DCOLOR Color[3] =
	{
		m_PivotDragAxis == PivotDragAxisX ? D3DCOLOR_ARGB(255,255,255,0) : D3DCOLOR_ARGB(255,255,0,0),
		m_PivotDragAxis == PivotDragAxisY ? D3DCOLOR_ARGB(255,255,255,0) : D3DCOLOR_ARGB(255,0,255,0),
		m_PivotDragAxis == PivotDragAxisZ ? D3DCOLOR_ARGB(255,255,255,0) : D3DCOLOR_ARGB(255,0,0,255)
	};
	for (unsigned int j = 0; j < 3; j++)
	{
		for (unsigned int i = 0; i < pices; i++)
		{
			const float alpha = D3DXToRadian(360.0f / pices);
			const float theta[2] = { i * alpha, (i + 1) * alpha };
			unsigned int p = 0;
			vertices[(j * pices + i) * stage + p].pos = Vector3(offset + header, 0, 0).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = Vector3(offset, radius[0] * cos(theta[0]), radius[0] * sin(theta[0])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = Vector3(offset, radius[0] * cos(theta[1]), radius[0] * sin(theta[1])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;

			vertices[(j * pices + i) * stage + p].pos = Vector3(offset, radius[0] * cos(theta[1]), radius[0] * sin(theta[1])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = Vector3(offset, radius[0] * cos(theta[0]), radius[0] * sin(theta[0])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = Vector3(offset, 0, 0).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;

			vertices[(j * pices + i) * stage + p].pos = Vector3(offset, radius[1] * cos(theta[1]), radius[1] * sin(theta[1])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = Vector3(offset, radius[1] * cos(theta[0]), radius[1] * sin(theta[0])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = Vector3(0, radius[1] * cos(theta[0]), radius[1] * sin(theta[0])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;

			vertices[(j * pices + i) * stage + p].pos = Vector3(offset, radius[1] * cos(theta[1]), radius[1] * sin(theta[1])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = Vector3(0, radius[1] * cos(theta[0]), radius[1] * sin(theta[0])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = Vector3(0, radius[1] * cos(theta[1]), radius[1] * sin(theta[1])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
		}
	}

	HRESULT hr;
	V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&m_World));
	V(pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE));
	V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertices.size() / 3, &vertices[0], sizeof(Vertex)));
}

void PivotController::DrawRotController(IDirect3DDevice9 * pd3dDevice)
{
	struct Vertex
	{
		Vector3 pos;
		D3DCOLOR color;
	};

	const unsigned int pices = 36;
	const unsigned int stage = 6;
	boost::array<Vertex, pices * stage * 3> vertices;
	const D3DCOLOR Color[3] =
	{
		m_PivotDragAxis == PivotDragAxisX ? D3DCOLOR_ARGB(255,255,255,0) : D3DCOLOR_ARGB(255,255,0,0),
		m_PivotDragAxis == PivotDragAxisY ? D3DCOLOR_ARGB(255,255,255,0) : D3DCOLOR_ARGB(255,0,255,0),
		m_PivotDragAxis == PivotDragAxisZ ? D3DCOLOR_ARGB(255,255,255,0) : D3DCOLOR_ARGB(255,0,0,255)
	};
	for (unsigned int j = 0; j < 3; j++)
	{
		for (unsigned int i = 0; i < pices; i++)
		{
			const float alpha = D3DXToRadian(360.0f / pices);
			const float theta[2] = { i * alpha, (i + 1) * alpha };
			unsigned int p = 0;
			vertices[(j * pices + i) * stage + p].pos = Vector3( radius[0], (offset + header) * cos(theta[0]), (offset + header) * sin(theta[0])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = Vector3(-radius[0], (offset + header) * cos(theta[0]), (offset + header) * sin(theta[0])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = Vector3(-radius[0], (offset + header) * cos(theta[1]), (offset + header) * sin(theta[1])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;

			vertices[(j * pices + i) * stage + p].pos = Vector3( radius[0], (offset + header) * cos(theta[0]), (offset + header) * sin(theta[0])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = Vector3(-radius[0], (offset + header) * cos(theta[1]), (offset + header) * sin(theta[1])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = Vector3( radius[0], (offset + header) * cos(theta[1]), (offset + header) * sin(theta[1])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
		}
	}

	HRESULT hr;
	V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&m_World));
	V(pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE));
	V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertices.size() / 3, &vertices[0], sizeof(Vertex)));
}

void PivotController::UpdateScale(const my::BaseCamera * camera, const D3DSURFACE_DESC * desc)
{
	float z = Vector4(m_Pos, 1.0f).dot(camera->m_View.column<2>());
	m_Scale = -z * 1 / 25.0f * 1024.0f / desc->Width;
}

void PivotController::UpdateWorld(void)
{
	m_World = Matrix4::Compose(Vector3(m_Scale), m_Rot, m_Pos);
}

bool PivotController::OnLButtonDown(const my::Ray & ray)
{
	switch (m_PivotMode)
	{
	case PivotModeMove:
		return OnMoveControllerLButtonDown(ray);
	case PivotModeRot:
		return OnRotControllerLButtonDown(ray);
	}
	return false;
}

bool PivotController::OnMoveControllerLButtonDown(const my::Ray & ray)
{
	Matrix4 trans[3] = {
		(Transform[0] * m_World).inverse(), (Transform[1] * m_World).inverse(), (Transform[2] * m_World).inverse() };
	float minT = FLT_MAX;
	m_PivotDragAxis = PivotDragNone;
	for (unsigned int i = 0; i < _countof(trans); i++)
	{
		Ray local_ray = ray.transform(trans[i]);
		IntersectionTests::TestResult res = IntersectionTests::rayAndCylinder(local_ray.p, local_ray.d, radius[0] + 0.1f, offset + header);
		if (res.first && res.second < minT)
		{
			minT = res.second;
			m_PivotDragAxis = (PivotDragAxis)(PivotDragAxisX + i);
		}
	}
	switch (m_PivotDragAxis)
	{
	case PivotDragAxisX:
		m_DragPos = m_Pos;
		m_DragPt = ray.p + ray.d * minT;
		m_DragPlane.normal = Vector3::unitX.cross(Vector3::unitX.cross(ray.d)).normalize();
		m_DragPlane.d = -m_DragPt.dot(m_DragPlane.normal);
		m_Captured = true;
		break;
	case PivotDragAxisY:
		m_DragPos = m_Pos;
		m_DragPt = ray.p + ray.d * minT;
		m_DragPlane.normal = Vector3::unitY.cross(Vector3::unitY.cross(ray.d)).normalize();
		m_DragPlane.d = -m_DragPt.dot(m_DragPlane.normal);
		m_Captured = true;
		break;
	case PivotDragAxisZ:
		m_DragPos = m_Pos;
		m_DragPt = ray.p + ray.d * minT;
		m_DragPlane.normal = Vector3::unitZ.cross(Vector3::unitZ.cross(ray.d)).normalize();
		m_DragPlane.d = -m_DragPt.dot(m_DragPlane.normal);
		m_Captured = true;
		break;
	}
	return true;
}

bool PivotController::OnRotControllerLButtonDown(const my::Ray & ray)
{
	Matrix4 trans = m_World.inverse();
	Ray local_ray = ray.transform(trans);
	IntersectionTests::TestResult res = IntersectionTests::rayAndSphere(local_ray.p, local_ray.d, Vector3::zero, offset + header);
	m_PivotDragAxis = PivotDragNone;
	if(res.first)
	{
		Vector3 k = local_ray.p + local_ray.d * res.second;
		if(k.x <= radius[0] && k.x >= -radius[0])
		{
			m_PivotDragAxis = PivotDragAxisX;
			m_DragPt = k;
			m_DragRot = m_Rot;
			m_Captured = true;
			return true;
		}
		else if(k.y <= radius[0] && k.y >= -radius[0])
		{
			m_PivotDragAxis = PivotDragAxisY;
			m_DragPt = k;
			m_DragRot = m_Rot;
			m_Captured = true;
			return true;
		}
		else if(k.z <= radius[0] && k.z >= -radius[0])
		{
			m_PivotDragAxis = PivotDragAxisZ;
			m_DragPt = k;
			m_DragRot = m_Rot;
			m_Captured = true;
			return true;
		}
	}
	return true;
}

bool PivotController::OnMouseMove(const my::Ray & ray)
{
	if (m_Captured)
	{
		switch (m_PivotMode)
		{
		case PivotModeMove:
			return OnMoveControllerMouseMove(ray);
		case PivotModeRot:
			return OnRotControllerMouseMove(ray);
		}
	}
	return false;
}

bool PivotController::OnMoveControllerMouseMove(const my::Ray & ray)
{
	IntersectionTests::TestResult res = IntersectionTests::rayAndHalfSpace(ray.p, ray.d, m_DragPlane);
	if(res.first)
	{
		Vector3 pt = ray.p + ray.d * res.second;
		switch(m_PivotDragAxis)
		{
		case PivotDragAxisX:
			m_Pos = Vector3(m_DragPos.x + pt.x - m_DragPt.x, m_DragPos.y, m_DragPos.z);
			return true;
		case PivotDragAxisY:
			m_Pos = Vector3(m_DragPos.x, m_DragPos.y + pt.y - m_DragPt.y, m_DragPos.z);
			return true;
		case PivotDragAxisZ:
			m_Pos = Vector3(m_DragPos.x, m_DragPos.y, m_DragPos.z + pt.z - m_DragPt.z);
			return true;
		}
	}
	return false;
}

bool PivotController::OnRotControllerMouseMove(const my::Ray & ray)
{
	Matrix4 trans = Matrix4::Compose(m_Scale, m_DragRot, m_Pos).inverse();
	Ray local_ray = ray.transform(trans);
    IntersectionTests::TestResult res = IntersectionTests::rayAndSphere(local_ray.p, local_ray.d, Vector3::zero, offset + header);
    Vector3 k;
    if(res.first)
    {
        k = local_ray.p + local_ray.d * res.second;
    }
    else
	{
        k = local_ray.p - local_ray.d * local_ray.p.dot(local_ray.d);
	}
    switch(m_PivotDragAxis)
    {
    case PivotDragAxisX:
        m_Rot = Quaternion::RotationFromTo(Vector3(0, m_DragPt.y, m_DragPt.z), Vector3(0, k.y, k.z)) * m_DragRot;
        return true;
    case PivotDragAxisY:
        m_Rot = Quaternion::RotationFromTo(Vector3(m_DragPt.x, 0, m_DragPt.z), Vector3(k.x, 0, k.z)) * m_DragRot;
        return true;
    case PivotDragAxisZ:
        m_Rot = Quaternion::RotationFromTo(Vector3(m_DragPt.x, m_DragPt.y, 0), Vector3(k.x, k.y, 0)) * m_DragRot;
        return true;
    }
    return false;
}

bool PivotController::OnLButtonUp(const my::Ray & ray)
{
	switch (m_PivotDragAxis)
	{
	case PivotDragAxisX:
	case PivotDragAxisY:
	case PivotDragAxisZ:
		m_Captured = false;
		return true;
	}
	return false;
}
