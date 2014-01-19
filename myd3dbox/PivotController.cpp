#include "StdAfx.h"
#include "PivotController.h"

using namespace my;

void PivotController::BuildConeVertices(VertexList & vertex_list, const float radius, const float height, const float offset, const D3DCOLOR color)
{
	for(int theta = 0; theta < 360; theta += 30)
	{
		vertex_list.push_back(Vertex(
			Vector3(offset+height, 0, 0),
			Vector3(radius, (radius+height)*cos(D3DXToRadian(theta)), (radius+height)*sin(D3DXToRadian(theta))).normalize(),
			color));
		vertex_list.push_back(Vertex(
			Vector3(offset, radius*cos(D3DXToRadian(theta)), radius*sin(D3DXToRadian(theta))),
			Vector3(radius, (radius+height)*cos(D3DXToRadian(theta)), (radius+height)*sin(D3DXToRadian(theta))).normalize(),
			color));
		vertex_list.push_back(Vertex(
			Vector3(offset, radius*cos(D3DXToRadian(theta+30)), radius*sin(D3DXToRadian(theta+30))),
			Vector3(radius, (radius+height)*cos(D3DXToRadian(theta+30)), (radius+height)*sin(D3DXToRadian(theta+30))).normalize(),
			color));

		vertex_list.push_back(Vertex(
			Vector3(offset, 0, 0),
			Vector3(-1,0,0),
			color));
		vertex_list.push_back(Vertex(
			Vector3(offset, radius*cos(D3DXToRadian(theta+30)), radius*sin(D3DXToRadian(theta+30))),
			Vector3(-1,0,0),
			color));
		vertex_list.push_back(Vertex(
			Vector3(offset, radius*cos(D3DXToRadian(theta)), radius*sin(D3DXToRadian(theta))),
			Vector3(-1,0,0),
			color));
	}
}

void PivotController::Draw(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera)
{
	VertexList vertex_list;
	BuildConeVertices(vertex_list, 0.25f, 1.0f, 4.0f, D3DCOLOR_ARGB(255,255,0,0));

	size_t vertex_list_size = vertex_list.size();
	vertex_list.resize(vertex_list_size * 3);
	Quaternion quat_to_y = Quaternion::RotationFromTo(Vector3(1,0,0), Vector3(0,1,0));
	Quaternion quat_to_z = Quaternion::RotationFromTo(Vector3(1,0,0), Vector3(0,0,1));
	for(size_t i = 0; i < vertex_list_size; i++)
	{
		vertex_list[i + vertex_list_size * 1].pos = vertex_list[i].pos.transform(quat_to_y);
		vertex_list[i + vertex_list_size * 2].pos = vertex_list[i].pos.transform(quat_to_z);
		vertex_list[i + vertex_list_size * 1].normal = vertex_list[i].normal.transform(quat_to_y);
		vertex_list[i + vertex_list_size * 2].normal = vertex_list[i].normal.transform(quat_to_z);
		vertex_list[i + vertex_list_size * 1].color = D3DCOLOR_ARGB(255,0,255,0);
		vertex_list[i + vertex_list_size * 2].color = D3DCOLOR_ARGB(255,0,0,255);
	}

	D3DVIEWPORT9 vp;
	pd3dDevice->GetViewport(&vp);
	const Vector4 ViewPos = m_Pos.transform(camera->m_ViewProj);
	const float ViewScale = ViewPos.z / 25.0f * 800.0f / vp.Width;

	D3DLIGHT9 light;
	ZeroMemory(&light, sizeof(light));
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r = 1.0f;
	light.Diffuse.g = 1.0f;
	light.Diffuse.b = 1.0f;
	light.Direction = (D3DVECTOR &)(Vector3(0,0,-1).transform(camera->m_Orientation));
	pd3dDevice->SetLight(0, &light);

	pd3dDevice->LightEnable(0, TRUE);
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&(Matrix4::Scaling(ViewScale,ViewScale,ViewScale) * Matrix4::Translation(m_Pos)));
	pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertex_list.size() / 3, &vertex_list[0], sizeof(vertex_list[0]));

	vertex_list.clear();
	vertex_list.push_back(Vertex(Vector3(0,0,0),Vector3(1,1,1),D3DCOLOR_ARGB(255,255,0,0)));
	vertex_list.push_back(Vertex(Vector3(4.0f,0,0),Vector3(1,1,1),D3DCOLOR_ARGB(255,255,0,0)));
	vertex_list.push_back(Vertex(Vector3(0,0,0),Vector3(1,1,1),D3DCOLOR_ARGB(255,0,255,0)));
	vertex_list.push_back(Vertex(Vector3(0,4.0f,0),Vector3(1,1,1),D3DCOLOR_ARGB(255,0,255,0)));
	vertex_list.push_back(Vertex(Vector3(0,0,0),Vector3(1,1,1),D3DCOLOR_ARGB(255,0,0,255)));
	vertex_list.push_back(Vertex(Vector3(0,0,4.0f),Vector3(1,1,1),D3DCOLOR_ARGB(255,0,0,255)));

	pd3dDevice->LightEnable(0, FALSE);
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&(Matrix4::Scaling(ViewScale,ViewScale,ViewScale) * Matrix4::Translation(m_Pos)));
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, vertex_list.size() / 3, &vertex_list[0], sizeof(vertex_list[0]));
}
