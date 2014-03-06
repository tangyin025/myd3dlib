#include "StdAfx.h"
#include "PivotController.h"

using namespace my;

const float PivotControllerBase::MovePivotRadius = 0.25f;

const float PivotControllerBase::MovePivotHeight = 1.0f;

const float PivotControllerBase::MovePivotOffset = 3.0f;

const float PivotControllerBase::MovePlaneWidth = 1.0f;

const float PivotControllerBase::RotationPivotRadius = MovePivotOffset + MovePivotHeight + MovePivotRadius;

const D3DCOLOR PivotControllerBase::PivotAxisXColor = D3DCOLOR_ARGB(255,255,0,0);

const D3DCOLOR PivotControllerBase::PivotAxisYColor = D3DCOLOR_ARGB(255,0,255,0);

const D3DCOLOR PivotControllerBase::PivotAxisZColor = D3DCOLOR_ARGB(255,0,0,255);

const D3DCOLOR PivotControllerBase::PivotHighLightAxisColor = D3DCOLOR_ARGB(255,255,255,0);

const D3DCOLOR PivotControllerBase::PivotGrayAxisColor = D3DCOLOR_ARGB(255,127,127,127);

const Matrix4 PivotControllerBase::mat_to_y = Matrix4::RotationZ(D3DXToRadian(90));

const Matrix4 PivotControllerBase::mat_to_z = Matrix4::RotationY(-D3DXToRadian(90));

static void BuildConeVertices(PivotController::VertexList & vertex_list, const float radius, const float height, const float offset, const D3DCOLOR color, const my::Matrix4 & Transform)
{
	for(int theta = 0; theta < 360; theta += 30)
	{
		vertex_list.push_back(PivotController::Vertex(
			Vector3(offset+height, 0, 0).transform(Transform).xyz,
			Vector3(radius, (radius+height)*cos(D3DXToRadian(theta)), (radius+height)*sin(D3DXToRadian(theta))).normalize().transformNormal(Transform),
			color));
		vertex_list.push_back(PivotController::Vertex(
			Vector3(offset, radius*cos(D3DXToRadian(theta)), radius*sin(D3DXToRadian(theta))).transform(Transform).xyz,
			Vector3(radius, (radius+height)*cos(D3DXToRadian(theta)), (radius+height)*sin(D3DXToRadian(theta))).normalize().transformNormal(Transform),
			color));
		vertex_list.push_back(PivotController::Vertex(
			Vector3(offset, radius*cos(D3DXToRadian(theta+30)), radius*sin(D3DXToRadian(theta+30))).transform(Transform).xyz,
			Vector3(radius, (radius+height)*cos(D3DXToRadian(theta+30)), (radius+height)*sin(D3DXToRadian(theta+30))).normalize().transformNormal(Transform),
			color));

		vertex_list.push_back(PivotController::Vertex(
			Vector3(offset, 0, 0).transform(Transform).xyz,
			Vector3(-1,0,0).transformNormal(Transform),
			color));
		vertex_list.push_back(PivotController::Vertex(
			Vector3(offset, radius*cos(D3DXToRadian(theta+30)), radius*sin(D3DXToRadian(theta+30))).transform(Transform).xyz,
			Vector3(-1,0,0).transformNormal(Transform),
			color));
		vertex_list.push_back(PivotController::Vertex(
			Vector3(offset, radius*cos(D3DXToRadian(theta)), radius*sin(D3DXToRadian(theta))).transform(Transform).xyz,
			Vector3(-1,0,0).transformNormal(Transform),
			color));
	}
}

static void BuildPlaneVerties(PivotController::VertexList & vertex_list, float z, float y, const D3DCOLOR color, const my::Matrix4 & Transform)
{
	vertex_list.push_back(PivotController::Vertex(
		Vector3(0, 0, 0).transform(Transform).xyz,
		Vector3(0,0,1).transformNormal(Transform),
		color));
	vertex_list.push_back(PivotController::Vertex(
		Vector3(0, y > 0 ? PivotController::MovePlaneWidth : -PivotController::MovePlaneWidth, 0).transform(Transform).xyz,
		Vector3(0,0,1).transformNormal(Transform),
		color));
	vertex_list.push_back(PivotController::Vertex(
		Vector3(0, y > 0 ? PivotController::MovePlaneWidth : -PivotController::MovePlaneWidth, z > 0 ? PivotController::MovePlaneWidth : -PivotController::MovePlaneWidth).transform(Transform).xyz,
		Vector3(0,0,1).transformNormal(Transform),
		color));

	vertex_list.push_back(PivotController::Vertex(
		Vector3(0, 0, 0).transform(Transform).xyz,
		Vector3(0,0,1).transformNormal(Transform),
		color));
	vertex_list.push_back(PivotController::Vertex(
		Vector3(0, y > 0 ? PivotController::MovePlaneWidth : -PivotController::MovePlaneWidth, z > 0 ? PivotController::MovePlaneWidth : -PivotController::MovePlaneWidth).transform(Transform).xyz,
		Vector3(0,0,1).transformNormal(Transform),
		color));
	vertex_list.push_back(PivotController::Vertex(
		Vector3(0, 0, z > 0 ? PivotController::MovePlaneWidth : -PivotController::MovePlaneWidth).transform(Transform).xyz,
		Vector3(0,0,1).transformNormal(Transform),
		color));
}

static void BuildWireCircleVertices(PivotController::VertexList & vertex_list, const my::Vector3 & center, const float radius, const D3DCOLOR color, const my::Matrix4 & Transform, const my::Vector3 & ViewPos, const float discrm)
{
	for(int theta = 0; theta < 360; theta += 10)
	{
		Vector3 p0 = Vector3(0, radius*cos(D3DXToRadian(theta)), radius*sin(D3DXToRadian(theta))).transform(Transform).xyz;
		Vector3 n0 = (p0 - center).normalize();
		Vector3 d0 = (p0 - ViewPos).normalize();
		Vector3 p1 = Vector3(0, radius*cos(D3DXToRadian(theta+10)), radius*sin(D3DXToRadian(theta+10))).transform(Transform).xyz;
		Vector3 n1 = (p1 - center).normalize();
		Vector3 d1 = (p1 - ViewPos).normalize();

		if(n0.dot(d0) <= discrm || n1.dot(d1) <= discrm)
		{
			vertex_list.push_back(PivotController::Vertex(p0, n0, color));
			vertex_list.push_back(PivotController::Vertex(p1, n1, color));
		}
	}
}

void PivotControllerBase::UpdateViewTransform(const my::Matrix4 & ViewProj, UINT ViewWidth)
{
	const my::Vector4 ViewPos = m_Position.transform(ViewProj);
	const float ViewScale = ViewPos.z / 25.0f * 800.0f / ViewWidth;
	m_ViewTransform = Matrix4::Scaling(ViewScale, ViewScale, ViewScale) * Matrix4::Translation(m_Position);
}

void PivotControllerBase::DrawMoveController(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera, HighLightAxis high_light_axis)
{
	VertexList vertex_list;
	BuildConeVertices(vertex_list, MovePivotRadius, MovePivotHeight, MovePivotOffset, high_light_axis == HighLightAxisX ? PivotHighLightAxisColor : PivotAxisXColor, Matrix4::identity);
	BuildConeVertices(vertex_list, MovePivotRadius, MovePivotHeight, MovePivotOffset, high_light_axis == HighLightAxisY ? PivotHighLightAxisColor : PivotAxisYColor, mat_to_y);
	BuildConeVertices(vertex_list, MovePivotRadius, MovePivotHeight, MovePivotOffset, high_light_axis == HighLightAxisZ ? PivotHighLightAxisColor : PivotAxisZColor, mat_to_z);

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
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&m_ViewTransform);
	pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertex_list.size() / 3, &vertex_list[0], sizeof(vertex_list[0]));

	vertex_list.clear();
	vertex_list.push_back(Vertex(Vector3(0,0,0),Vector3(1,1,1),high_light_axis == HighLightAxisX ? PivotHighLightAxisColor : PivotAxisXColor));
	vertex_list.push_back(Vertex(Vector3(MovePivotOffset,0,0),Vector3(1,1,1),high_light_axis == HighLightAxisX ? PivotHighLightAxisColor : PivotAxisXColor));
	vertex_list.push_back(Vertex(Vector3(0,0,0),Vector3(1,1,1),high_light_axis == HighLightAxisY ? PivotHighLightAxisColor : PivotAxisYColor));
	vertex_list.push_back(Vertex(Vector3(0,MovePivotOffset,0),Vector3(1,1,1),high_light_axis == HighLightAxisY ? PivotHighLightAxisColor : PivotAxisYColor));
	vertex_list.push_back(Vertex(Vector3(0,0,0),Vector3(1,1,1),high_light_axis == HighLightAxisZ ? PivotHighLightAxisColor : PivotAxisZColor));
	vertex_list.push_back(Vertex(Vector3(0,0,MovePivotOffset),Vector3(1,1,1),high_light_axis == HighLightAxisZ ? PivotHighLightAxisColor : PivotAxisZColor));

	pd3dDevice->LightEnable(0, FALSE);
	pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&m_ViewTransform);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, vertex_list.size() / 2, &vertex_list[0], sizeof(vertex_list[0]));

	Matrix4 InvToXWorld = m_ViewTransform.inverse();
	Vector3 local_camera = camera->m_Position.transform(InvToXWorld).xyz;
	vertex_list.clear();
	BuildPlaneVerties(vertex_list, local_camera.z, local_camera.y, high_light_axis == HighLightPlaneX ? D3DCOLOR_ARGB(100,255,255,0) : D3DCOLOR_ARGB(100,255,0,0), Matrix4::identity);
	BuildPlaneVerties(vertex_list, local_camera.z, -local_camera.x, high_light_axis == HighLightPlaneY ? D3DCOLOR_ARGB(100,255,255,0) : D3DCOLOR_ARGB(100,0,255,0), mat_to_y);
	BuildPlaneVerties(vertex_list, -local_camera.x, local_camera.y, high_light_axis == HighLightPlaneZ ? D3DCOLOR_ARGB(100,255,255,0) : D3DCOLOR_ARGB(100,0,0,255), mat_to_z);
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertex_list.size() / 3, &vertex_list[0], sizeof(vertex_list[0]));

	pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
}

void PivotControllerBase::DrawRotationController(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera, HighLightAxis high_light_axis)
{
	Matrix4 World = Matrix4::RotationQuaternion(m_Rotation) * m_ViewTransform;

	VertexList vertex_list;
	Quaternion quat_to_camera = Quaternion::RotationFromTo(Vector3::unitX, camera->m_Position - m_Position);
	BuildWireCircleVertices(vertex_list, m_Position, RotationPivotRadius, PivotGrayAxisColor, Matrix4::RotationQuaternion(quat_to_camera) * m_ViewTransform, camera->m_Position, 1.0f);
	BuildWireCircleVertices(vertex_list, m_Position, RotationPivotRadius, high_light_axis == HighLightAxisX ? PivotHighLightAxisColor : PivotAxisXColor, World, camera->m_Position, 0.0f);
	BuildWireCircleVertices(vertex_list, m_Position, RotationPivotRadius, high_light_axis == HighLightAxisY ? PivotHighLightAxisColor : PivotAxisYColor, mat_to_y * World, camera->m_Position, 0.0f);
	BuildWireCircleVertices(vertex_list, m_Position, RotationPivotRadius, high_light_axis == HighLightAxisZ ? PivotHighLightAxisColor : PivotAxisZColor, mat_to_z * World, camera->m_Position, 0.0f);

	pd3dDevice->LightEnable(0, FALSE);
	pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&Matrix4::identity);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, vertex_list.size() / 2, &vertex_list[0], sizeof(vertex_list[0]));

	pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
}

BOOL PivotController::OnMoveControllerLButtonDown(const std::pair<my::Vector3, my::Vector3> & ray)
{
	Matrix4 InvToXWorld = m_ViewTransform.inverse();
	std::pair<Vector3, Vector3> local_ray(ray.first.transform(InvToXWorld).xyz, ray.second.transformNormal(InvToXWorld));
	m_HighLightAxis = HighLightAxisNone;
	IntersectionTests::TestResult res[3];
	res[0] = IntersectionTests::rayAndXPlane(local_ray.first, local_ray.second, 0);
	res[1] = IntersectionTests::rayAndYPlane(local_ray.first, local_ray.second, 0);
	res[2] = IntersectionTests::rayAndZPlane(local_ray.first, local_ray.second, 0);
	float minT = FLT_MAX;
	for(int i = 0; i < 3; i++)
	{
		if(res[i].first && res[i].second < minT)
		{
			minT = res[i].second;
			m_HighLightAxis = (HighLightAxis)(HighLightPlaneX + i);
		}
	}
	switch(m_HighLightAxis)
	{
	case HighLightPlaneX:
		if(abs(local_ray.first.y + local_ray.second.y * minT) < MovePlaneWidth && abs(local_ray.first.z + local_ray.second.z * minT) < MovePlaneWidth)
		{
			m_DragPos = m_Position;
			m_DragPt = ray.first + ray.second * minT;
			m_DragNormal = Vector3::unitX;
			m_DragDist = -m_DragPt.dot(m_DragNormal);
			return TRUE;
		}
		break;
	case HighLightPlaneY:
		if(abs(local_ray.first.x + local_ray.second.x * minT) < MovePlaneWidth && abs(local_ray.first.z + local_ray.second.z * minT) < MovePlaneWidth)
		{
			m_DragPos = m_Position;
			m_DragPt = ray.first + ray.second * minT;
			m_DragNormal = Vector3::unitY;
			m_DragDist = -m_DragPt.dot(m_DragNormal);
			return TRUE;
		}
		break;
	case HighLightPlaneZ:
		if(abs(local_ray.first.x + local_ray.second.x * minT) < MovePlaneWidth && abs(local_ray.first.y + local_ray.second.y * minT) < MovePlaneWidth)
		{
			m_DragPos = m_Position;
			m_DragPt = ray.first + ray.second * minT;
			m_DragNormal = Vector3::unitZ;
			m_DragDist = -m_DragPt.dot(m_DragNormal);
			return TRUE;
		}
		break;
	}

	Matrix4 InvToYWorld = (mat_to_y * m_ViewTransform).inverse();
	Matrix4 InvToZWorld = (mat_to_z * m_ViewTransform).inverse();
	m_HighLightAxis = HighLightAxisNone;
	res[0] = IntersectionTests::rayAndCylinder(ray.first.transform(InvToXWorld).xyz, ray.second.transformNormal(InvToXWorld), MovePivotRadius * 2, MovePivotHeight + MovePivotOffset);
	res[1] = IntersectionTests::rayAndCylinder(ray.first.transform(InvToYWorld).xyz, ray.second.transformNormal(InvToYWorld), MovePivotRadius * 2, MovePivotHeight + MovePivotOffset);
	res[2] = IntersectionTests::rayAndCylinder(ray.first.transform(InvToZWorld).xyz, ray.second.transformNormal(InvToZWorld), MovePivotRadius * 2, MovePivotHeight + MovePivotOffset);
	minT = FLT_MAX;
	for(int i = 0; i < 3; i++)
	{
		if(res[i].first && res[i].second < minT)
		{
			minT = res[i].second;
			m_HighLightAxis = (HighLightAxis)(HighLightAxisX + i);
		}
	}
	switch(m_HighLightAxis)
	{
	case HighLightAxisX:
		m_DragPos = m_Position;
		m_DragPt = ray.first + ray.second * minT;
		m_DragNormal = Vector3::unitX.cross(Vector3::unitX.cross(ray.second)).normalize();
		m_DragDist = -m_DragPt.dot(m_DragNormal);
		return TRUE;
	case HighLightAxisY:
		m_DragPos = m_Position;
		m_DragPt = ray.first + ray.second * minT;
		m_DragNormal = Vector3::unitY.cross(Vector3::unitY.cross(ray.second)).normalize();
		m_DragDist = -m_DragPt.dot(m_DragNormal);
		return TRUE;
	case HighLightAxisZ:
		m_DragPos = m_Position;
		m_DragPt = ray.first + ray.second * minT;
		m_DragNormal = Vector3::unitZ.cross(Vector3::unitZ.cross(ray.second)).normalize();
		m_DragDist = -m_DragPt.dot(m_DragNormal);
		return TRUE;
	}

	return FALSE;
}

BOOL PivotController::OnRotationControllerButtonDown(const std::pair<my::Vector3, my::Vector3> & ray)
{
	Matrix4 World = Matrix4::RotationQuaternion(m_Rotation) * m_ViewTransform;
	Matrix4 InvWorld = World.inverse();
	std::pair<Vector3, Vector3> locRay(ray.first.transform(InvWorld).xyz, ray.second.transformNormal(InvWorld));
	IntersectionTests::TestResult res = IntersectionTests::rayAndSphere(locRay.first, locRay.second, Vector3::zero, RotationPivotRadius);
	m_HighLightAxis = HighLightAxisNone;
	if(res.first)
	{
		Vector3 k = locRay.first + locRay.second * res.second;
		if(k.x <= MovePivotRadius && k.x >= -MovePivotRadius)
		{
			m_HighLightAxis = HighLightAxisX;
			m_DragPt = k;
			m_DragRot = m_Rotation;
			return TRUE;
		}
		else if(k.y <= MovePivotRadius && k.y >= -MovePivotRadius)
		{
			m_HighLightAxis = HighLightAxisY;
			m_DragPt = k;
			m_DragRot = m_Rotation;
			return TRUE;
		}
		else if(k.z <= MovePivotRadius && k.z >= -MovePivotRadius)
		{
			m_HighLightAxis = HighLightAxisZ;
			m_DragPt = k;
			m_DragRot = m_Rotation;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL PivotController::OnMoveControllerMouseMove(const std::pair<my::Vector3, my::Vector3> & ray)
{
	IntersectionTests::TestResult res = IntersectionTests::rayAndHalfSpace(ray.first, ray.second, m_DragNormal, m_DragDist);
	if(res.first)
	{
		Vector3 pt = ray.first + ray.second * res.second;
		switch(m_HighLightAxis)
		{
		case HighLightAxisX:
			m_Position = Vector3(m_DragPos.x + pt.x - m_DragPt.x, m_DragPos.y, m_DragPos.z);
			return TRUE;
		case HighLightAxisY:
			m_Position = Vector3(m_DragPos.x, m_DragPos.y + pt.y - m_DragPt.y, m_DragPos.z);
			return TRUE;
		case HighLightAxisZ:
			m_Position = Vector3(m_DragPos.x, m_DragPos.y, m_DragPos.z + pt.z - m_DragPt.z);
			return TRUE;
		case HighLightPlaneX:
			m_Position = Vector3(m_DragPos.x, m_DragPos.y + pt.y - m_DragPt.y, m_DragPos.z + pt.z - m_DragPt.z);
			return TRUE;
		case HighLightPlaneY:
			m_Position = Vector3(m_DragPos.x + pt.x - m_DragPt.x, m_DragPos.y, m_DragPos.z + pt.z - m_DragPt.z);
			return TRUE;
		case HighLightPlaneZ:
			m_Position = Vector3(m_DragPos.x + pt.x - m_DragPt.x, m_DragPos.y + pt.y - m_DragPt.y, m_DragPos.z);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL PivotController::OnRotationControllerMouseMove(const std::pair<my::Vector3, my::Vector3> & ray)
{
	Matrix4 World = Matrix4::RotationQuaternion(m_DragRot) * m_ViewTransform;
	Matrix4 InvWorld = World.inverse();
	std::pair<Vector3, Vector3> locRay(ray.first.transform(InvWorld).xyz, ray.second.transformNormal(InvWorld));
	IntersectionTests::TestResult res = IntersectionTests::rayAndSphere(locRay.first, locRay.second, Vector3::zero, RotationPivotRadius);
	Vector3 k;
	if(res.first)
	{
		k = locRay.first + locRay.second * res.second;
	}
	else
		k = locRay.first - locRay.second.normalize() * locRay.first.dot(locRay.second.normalize());
	switch(m_HighLightAxis)
	{
	case HighLightAxisX:
		m_Rotation = Quaternion::RotationFromTo(Vector3(0, m_DragPt.y, m_DragPt.z), Vector3(0, k.y, k.z)) * m_DragRot;
		return TRUE;
	case HighLightAxisY:
		m_Rotation = Quaternion::RotationFromTo(Vector3(m_DragPt.x, 0, m_DragPt.z), Vector3(k.x, 0, k.z)) * m_DragRot;
		return TRUE;
	case HighLightAxisZ:
		m_Rotation = Quaternion::RotationFromTo(Vector3(m_DragPt.x, m_DragPt.y, 0), Vector3(k.x, k.y, 0)) * m_DragRot;
		return TRUE;
	}
	return FALSE;
}

void PivotController::Draw(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera)
{
	switch(m_PovitMode)
	{
	case PivotModeMove:
		DrawMoveController(pd3dDevice, camera, m_HighLightAxis);
		break;

	case PivotModeRotation:
		DrawRotationController(pd3dDevice, camera, m_HighLightAxis);
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
		return OnRotationControllerButtonDown(ray);
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
		return OnRotationControllerMouseMove(ray);
	}
	return FALSE;
}
