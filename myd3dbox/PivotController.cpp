#include "stdafx.h"
#include "PivotController.h"

PivotController::PivotController(void)
	: m_World(my::Matrix4::Identity())
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
	const D3DCOLOR Color[3] = { D3DCOLOR_ARGB(255,255,0,0), D3DCOLOR_ARGB(255,0,255,0), D3DCOLOR_ARGB(255,0,0,255) };
	for (unsigned int j = 0; j < 3; j++)
	{
		for (unsigned int i = 0; i < pices; i++)
		{
			const float theta[2] = { D3DXToRadian(i * 30.0f), D3DXToRadian((i + 1) * 30.0f) };
			const float radius[2] = { 0.15f, 0.05f };
			const float offset = 2.0f;
			const float header = 1.0f;
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

	HRESULT hr;
	V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&m_World));
	V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertices.size() / 3, &vertices[0], sizeof(Vertex)));
}
