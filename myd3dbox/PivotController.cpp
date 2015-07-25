#include "stdafx.h"
#include "PivotController.h"

using namespace my;

static const float radius[2] = { 0.15f, 0.05f };

static const float offset = 2.0f;

static const float header = 1.0f;

static const Matrix4 Transform[3] = {
	Matrix4::Identity(), Matrix4::RotationZ(D3DXToRadian(90)), Matrix4::RotationY(D3DXToRadian(-90)) };

PivotController::PivotController(void)
	: m_Pos(0,0,0)
	, m_PivotDragMode(PivotDragNone)
	, m_Scale(1.0f)
{
}

PivotController::~PivotController(void)
{
}

void PivotController::Draw(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera)
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
		m_PivotDragMode == PivotDragMoveX ? D3DCOLOR_ARGB(255,255,255,0) : D3DCOLOR_ARGB(255,255,0,0),
		m_PivotDragMode == PivotDragMoveY ? D3DCOLOR_ARGB(255,255,255,0) : D3DCOLOR_ARGB(255,0,255,0),
		m_PivotDragMode == PivotDragMoveZ ? D3DCOLOR_ARGB(255,255,255,0) : D3DCOLOR_ARGB(255,0,0,255)
	};
	for (unsigned int j = 0; j < 3; j++)
	{
		for (unsigned int i = 0; i < pices; i++)
		{
			const float theta[2] = { D3DXToRadian(i * 30.0f), D3DXToRadian((i + 1) * 30.0f) };
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

	UpdateScale(camera->m_View);

	UpdateWorld();

	HRESULT hr;
	V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&m_World));
	V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertices.size() / 3, &vertices[0], sizeof(Vertex)));
}

void PivotController::UpdateScale(const my::Matrix4 & View)
{
	float z = Vector4(m_Pos, 1.0f).dot(View.column<2>());
	m_Scale = -z * 1 / 25.0f;
}

void PivotController::UpdateWorld(void)
{
	m_World = Matrix4::Compose(Vector3(m_Scale), Quaternion::identity, m_Pos);
}

bool PivotController::OnLButtonDown(const my::Ray & ray)
{
	Matrix4 trans[3] = {
		(Transform[0] * m_World).inverse(), (Transform[1] * m_World).inverse(), (Transform[2] * m_World).inverse() };
	float minT = FLT_MAX;
	m_PivotDragMode = PivotDragNone;
	for (unsigned int i = 0; i < _countof(trans); i++)
	{
		Ray local_ray = ray.transform(trans[i]);
		IntersectionTests::TestResult res = IntersectionTests::rayAndCylinder(local_ray.p, local_ray.d, radius[0], offset + header);
		if (res.first && res.second < minT)
		{
			minT = res.second;
			m_PivotDragMode = (PivotDragMode)(PivotDragMoveX + i);
		}
	}
	m_DragPos = m_Pos;
	m_DragPt = ray.p + ray.d * minT;
	switch (m_PivotDragMode)
	{
	case PivotDragMoveX:
		m_DragPlane.normal = Vector3::unitX.cross(Vector3::unitX.cross(ray.d)).normalize();
		break;
	case PivotDragMoveY:
		m_DragPlane.normal = Vector3::unitY.cross(Vector3::unitY.cross(ray.d)).normalize();
		break;
	case PivotDragMoveZ:
		m_DragPlane.normal = Vector3::unitZ.cross(Vector3::unitZ.cross(ray.d)).normalize();
		break;
	}
	m_DragPlane.d = -m_DragPt.dot(m_DragPlane.normal);
	return true;
}

bool PivotController::OnMouseMove(const my::Ray & ray)
{
	IntersectionTests::TestResult res = IntersectionTests::rayAndHalfSpace(ray.p, ray.d, m_DragPlane);
	if(res.first)
	{
		Vector3 pt = ray.p + ray.d * res.second;
		switch(m_PivotDragMode)
		{
		case PivotDragMoveX:
			m_Pos = Vector3(m_DragPos.x + pt.x - m_DragPt.x, m_DragPos.y, m_DragPos.z);
			return true;
		case PivotDragMoveY:
			m_Pos = Vector3(m_DragPos.x, m_DragPos.y + pt.y - m_DragPt.y, m_DragPos.z);
			return true;
		case PivotDragMoveZ:
			m_Pos = Vector3(m_DragPos.x, m_DragPos.y, m_DragPos.z + pt.z - m_DragPt.z);
			return true;
		}
	}
	return false;
}

bool PivotController::OnLButtonUp(const my::Ray & ray)
{
	switch (m_PivotDragMode)
	{
	case PivotDragMoveX:
	case PivotDragMoveY:
	case PivotDragMoveZ:
		m_PivotDragMode = PivotDragNone;
		return true;
	}
	return false;
}
