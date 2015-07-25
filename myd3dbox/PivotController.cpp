#include "stdafx.h"
#include "PivotController.h"

static const float radius[2] = { 0.15f, 0.05f };

static const float offset = 2.0f;

static const float header = 1.0f;

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
		my::Vector3 pos;
		D3DCOLOR color;
	};

	const unsigned int pices = 12;
	const unsigned int stage = 12;
	boost::array<Vertex, pices * stage * 3> vertices;
	const my::Matrix4 Transform[3] = { my::Matrix4::Identity(), my::Matrix4::RotationZ(D3DXToRadian(90)), my::Matrix4::RotationY(D3DXToRadian(-90)) };
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
			vertices[(j * pices + i) * stage + p].pos = my::Vector3(offset + header, 0, 0).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = my::Vector3(offset, radius[0] * cos(theta[0]), radius[0] * sin(theta[0])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = my::Vector3(offset, radius[0] * cos(theta[1]), radius[0] * sin(theta[1])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;

			vertices[(j * pices + i) * stage + p].pos = my::Vector3(offset, radius[0] * cos(theta[1]), radius[0] * sin(theta[1])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = my::Vector3(offset, radius[0] * cos(theta[0]), radius[0] * sin(theta[0])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = my::Vector3(offset, 0, 0).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;

			vertices[(j * pices + i) * stage + p].pos = my::Vector3(offset, radius[1] * cos(theta[1]), radius[1] * sin(theta[1])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = my::Vector3(offset, radius[1] * cos(theta[0]), radius[1] * sin(theta[0])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = my::Vector3(0, radius[1] * cos(theta[0]), radius[1] * sin(theta[0])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;

			vertices[(j * pices + i) * stage + p].pos = my::Vector3(offset, radius[1] * cos(theta[1]), radius[1] * sin(theta[1])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = my::Vector3(0, radius[1] * cos(theta[0]), radius[1] * sin(theta[0])).transformCoord(Transform[j]);
			vertices[(j * pices + i) * stage + p].color = Color[j];
			p++;
			vertices[(j * pices + i) * stage + p].pos = my::Vector3(0, radius[1] * cos(theta[1]), radius[1] * sin(theta[1])).transformCoord(Transform[j]);
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
	float z = my::Vector4(m_Pos, 1.0f).dot(View.column<2>());
	m_Scale = -z * 1 / 25.0f;
}

void PivotController::UpdateWorld(void)
{
	m_World = my::Matrix4::Compose(my::Vector3(m_Scale), my::Quaternion::identity, m_Pos);
}

bool PivotController::OnLButtonDown(const my::Ray & ray)
{
	//PxCapsuleGeometry geom(radius[0], offset + header);
	//PxTransform pose = PxTransform::createIdentity();
	//PxRaycastHit hits[1];
	//unsigned int numHits = PxGeometryQuery::raycast((PxVec3&)ray.first, (PxVec3&)ray.second, geom, pose, FLT_MAX, PxSceneQueryFlag::eNORMAL|PxSceneQueryFlag::eDISTANCE, _countof(hits), hits, true);
	//if (numHits > 0)
	//{
	//	m_PivotDragMode = PivotDragMoveX;
	//}
	//else
	//{
	//	m_PivotDragMode = PivotDragNone;
	//}
	return true;
}
