#include "StdAfx.h"
#include "PivotController.h"

using namespace my;

const float PivotController::PivotRadius = 0.25f;

const float PivotController::PivotHeight = 1.0f;

const float PivotController::PivotOffset = 3.0f;

const D3DCOLOR PivotController::PivotAxisXColor = D3DCOLOR_ARGB(255,255,0,0);

const D3DCOLOR PivotController::PivotAxisYColor = D3DCOLOR_ARGB(255,0,255,0);

const D3DCOLOR PivotController::PivotAxisZColor = D3DCOLOR_ARGB(255,0,0,255);

const D3DCOLOR PivotController::PivotDragAxisColor = D3DCOLOR_ARGB(255,255,255,0);

const D3DCOLOR PivotController::PivotGrayAxisColor = D3DCOLOR_ARGB(255,97,97,97);

const Matrix4 PivotController::mat_to_y = Matrix4::RotationZ(D3DXToRadian(90));

const Matrix4 PivotController::mat_to_z = Matrix4::RotationY(-D3DXToRadian(90));

void PivotController::BuildConeVertices(VertexList & vertex_list, const float radius, const float height, const float offset, const D3DCOLOR color, const my::Matrix4 & Transform)
{
	for(int theta = 0; theta < 360; theta += 30)
	{
		vertex_list.push_back(Vertex(
			Vector3(offset+height, 0, 0).transformCoord(Transform),
			Vector3(radius, (radius+height)*cos(D3DXToRadian(theta)), (radius+height)*sin(D3DXToRadian(theta))).normalize().transformNormal(Transform),
			color));
		vertex_list.push_back(Vertex(
			Vector3(offset, radius*cos(D3DXToRadian(theta)), radius*sin(D3DXToRadian(theta))).transformCoord(Transform),
			Vector3(radius, (radius+height)*cos(D3DXToRadian(theta)), (radius+height)*sin(D3DXToRadian(theta))).normalize().transformNormal(Transform),
			color));
		vertex_list.push_back(Vertex(
			Vector3(offset, radius*cos(D3DXToRadian(theta+30)), radius*sin(D3DXToRadian(theta+30))).transformCoord(Transform),
			Vector3(radius, (radius+height)*cos(D3DXToRadian(theta+30)), (radius+height)*sin(D3DXToRadian(theta+30))).normalize().transformNormal(Transform),
			color));

		vertex_list.push_back(Vertex(
			Vector3(offset, 0, 0).transformCoord(Transform),
			Vector3(-1,0,0).transformNormal(Transform),
			color));
		vertex_list.push_back(Vertex(
			Vector3(offset, radius*cos(D3DXToRadian(theta+30)), radius*sin(D3DXToRadian(theta+30))).transformCoord(Transform),
			Vector3(-1,0,0).transformNormal(Transform),
			color));
		vertex_list.push_back(Vertex(
			Vector3(offset, radius*cos(D3DXToRadian(theta)), radius*sin(D3DXToRadian(theta))).transformCoord(Transform),
			Vector3(-1,0,0).transformNormal(Transform),
			color));
	}
}

void PivotController::BuildCircleVertices(VertexList & vertex_list, const float radius, const D3DCOLOR color, const my::Matrix4 & Transform, const my::Vector3 & ViewPos)
{
	for(int theta = 0; theta < 360; theta += 10)
	{
		Vector3 p0 = Vector3(0, radius*cos(D3DXToRadian(theta)), radius*sin(D3DXToRadian(theta))).transformCoord(Transform);
		Vector3 n0 = p0.normalize();
		Vector3 d0 = (p0 - ViewPos).normalize();
		Vector3 p1 = Vector3(0, radius*cos(D3DXToRadian(theta+10)), radius*sin(D3DXToRadian(theta+10))).transformCoord(Transform);
		Vector3 n1 = p1.normalize();
		Vector3 d1 = (p1 - ViewPos).normalize();

		vertex_list.push_back(Vertex(p0, n0, (n0.dot(d0) <= 0.233f || n1.dot(d1) <= 0.233f) ? color : PivotGrayAxisColor));
		vertex_list.push_back(Vertex(p1, n1, (n0.dot(d0) <= 0.233f || n1.dot(d1) <= 0.233f) ? color : PivotGrayAxisColor));
	}
}

void PivotController::UpdateWorld(const my::Matrix4 & ViewProj, UINT ViewWidth)
{
	const Vector4 ViewPos = m_Position.transform(ViewProj);
	const float ViewScale = ViewPos.z / 25.0f * 800.0f / ViewWidth;
	m_World = Matrix4::RotationQuaternion(m_Rotation) * Matrix4::Scaling(ViewScale,ViewScale,ViewScale) * Matrix4::Translation(m_Position);
}

void PivotController::DrawMoveController(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera)
{
	VertexList vertex_list;
	BuildConeVertices(vertex_list, PivotRadius, PivotHeight, PivotOffset, m_DragAxis == DragAxisX ? PivotDragAxisColor : PivotAxisXColor, Matrix4::identity);
	BuildConeVertices(vertex_list, PivotRadius, PivotHeight, PivotOffset, m_DragAxis == DragAxisY ? PivotDragAxisColor : PivotAxisYColor, mat_to_y);
	BuildConeVertices(vertex_list, PivotRadius, PivotHeight, PivotOffset, m_DragAxis == DragAxisZ ? PivotDragAxisColor : PivotAxisZColor, mat_to_z);

	D3DLIGHT9 light;
	ZeroMemory(&light, sizeof(light));
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r = 1.0f;
	light.Diffuse.g = 1.0f;
	light.Diffuse.b = 1.0f;
	light.Direction = (D3DVECTOR &)(Vector3(0,0,-1).transform(camera->m_Orientation));
	pd3dDevice->SetLight(0, &light);

	pd3dDevice->LightEnable(0, TRUE);
	pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&m_World);
	pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertex_list.size() / 3, &vertex_list[0], sizeof(vertex_list[0]));

	vertex_list.clear();
	vertex_list.push_back(Vertex(Vector3(0,0,0),Vector3(1,1,1),m_DragAxis == DragAxisX ? PivotDragAxisColor : PivotAxisXColor));
	vertex_list.push_back(Vertex(Vector3(PivotOffset,0,0),Vector3(1,1,1),m_DragAxis == DragAxisX ? PivotDragAxisColor : PivotAxisXColor));
	vertex_list.push_back(Vertex(Vector3(0,0,0),Vector3(1,1,1),m_DragAxis == DragAxisY ? PivotDragAxisColor : PivotAxisYColor));
	vertex_list.push_back(Vertex(Vector3(0,PivotOffset,0),Vector3(1,1,1),m_DragAxis == DragAxisY ? PivotDragAxisColor : PivotAxisYColor));
	vertex_list.push_back(Vertex(Vector3(0,0,0),Vector3(1,1,1),m_DragAxis == DragAxisZ ? PivotDragAxisColor : PivotAxisZColor));
	vertex_list.push_back(Vertex(Vector3(0,0,PivotOffset),Vector3(1,1,1),m_DragAxis == DragAxisZ ? PivotDragAxisColor : PivotAxisZColor));

	pd3dDevice->LightEnable(0, FALSE);
	pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&m_World);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, vertex_list.size() / 2, &vertex_list[0], sizeof(vertex_list[0]));

	pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
}

void PivotController::DrawRotationController(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera)
{
	VertexList vertex_list;
	BuildCircleVertices(vertex_list, 5.0f, m_DragAxis == DragAxisX ? PivotDragAxisColor : PivotAxisXColor, Matrix4::identity, camera->m_Position);
	BuildCircleVertices(vertex_list, 5.0f, m_DragAxis == DragAxisY ? PivotDragAxisColor : PivotAxisYColor, mat_to_y, camera->m_Position);
	BuildCircleVertices(vertex_list, 5.0f, m_DragAxis == DragAxisZ ? PivotDragAxisColor : PivotAxisZColor, mat_to_z, camera->m_Position);

	pd3dDevice->LightEnable(0, FALSE);
	pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&m_World);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, vertex_list.size() / 2, &vertex_list[0], sizeof(vertex_list[0]));

	pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
}

BOOL PivotController::OnMoveControllerLButtonDown(const std::pair<my::Vector3, my::Vector3> & ray)
{
	IntersectionTests::TestResult res[3] =
	{
		IntersectionTests::rayAndCylinder(ray.first, ray.second, PivotRadius * 2, PivotHeight + PivotOffset, m_World.inverse()),
		IntersectionTests::rayAndCylinder(ray.first, ray.second, PivotRadius * 2, PivotHeight + PivotOffset, (mat_to_y * m_World).inverse()),
		IntersectionTests::rayAndCylinder(ray.first, ray.second, PivotRadius * 2, PivotHeight + PivotOffset, (mat_to_z * m_World).inverse()),
	};
	m_DragAxis = DragAxisNone;
	float minT = FLT_MAX;
	for(int i = 0; i < 3; i++)
	{
		if(res[i].first && res[i].second < minT)
		{
			minT = res[i].second;
			m_DragAxis = (DragAxis)(DragAxisX + i);
		}
	}
	switch(m_DragAxis)
	{
	case DragAxisX:
		m_DragPos = m_Position;
		m_DragPt = ray.first + ray.second * minT;
		m_DragNormal = Vector3::unitX.cross(Vector3::unitX.cross(ray.second)).normalize();
		m_DragDist = -m_DragPt.dot(m_DragNormal);
		return TRUE;
	case DragAxisY:
		m_DragPos = m_Position;
		m_DragPt = ray.first + ray.second * minT;
		m_DragNormal = Vector3::unitY.cross(Vector3::unitY.cross(ray.second)).normalize();
		m_DragDist = -m_DragPt.dot(m_DragNormal);
		return TRUE;
	case DragAxisZ:
		m_DragPos = m_Position;
		m_DragPt = ray.first + ray.second * minT;
		m_DragNormal = Vector3::unitZ.cross(Vector3::unitZ.cross(ray.second)).normalize();
		m_DragDist = -m_DragPt.dot(m_DragNormal);
		return TRUE;
	}
	return FALSE;
}

BOOL PivotController::OnMoveControllerMouseMove(const std::pair<my::Vector3, my::Vector3> & ray)
{
	IntersectionTests::TestResult res = IntersectionTests::rayAndHalfSpace(ray.first, ray.second, m_DragNormal, m_DragDist);
	if(res.first)
	{
		Vector3 pt = ray.first + ray.second * res.second;
		switch(m_DragAxis)
		{
		case DragAxisX:
			m_Position = Vector3(m_DragPos.x + pt.x - m_DragPt.x, m_DragPos.y, m_DragPos.z);
			return TRUE;
		case DragAxisY:
			m_Position = Vector3(m_DragPos.x, m_DragPos.y + pt.y - m_DragPt.y, m_DragPos.z);
			return TRUE;
		case DragAxisZ:
			m_Position = Vector3(m_DragPos.x, m_DragPos.y, m_DragPos.z + pt.z - m_DragPt.z);
			return TRUE;
		}
	}
	return FALSE;
}

void PivotController::Draw(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera)
{
	switch(m_PovitMode)
	{
	case PivotModeMove:
		DrawMoveController(pd3dDevice, camera);
		break;

	case PivotModeRotation:
		DrawRotationController(pd3dDevice, camera);
		break;
	}
}

BOOL PivotController::OnLButtonDown(const std::pair<my::Vector3, my::Vector3> & ray)
{
	switch(m_PovitMode)
	{
	case PivotModeMove:
		return OnMoveControllerLButtonDown(ray);

	case PivotModeRotation:
		return FALSE;
	}
	return FALSE;
}

BOOL PivotController::OnMouseMove(const std::pair<my::Vector3, my::Vector3> & ray)
{
	switch(m_PovitMode)
	{
	case PivotModeMove:
		return OnMoveControllerMouseMove(ray);

	case PivotModeRotation:
		return FALSE;
	}
	return FALSE;
}
