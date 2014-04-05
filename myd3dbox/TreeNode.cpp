#include "StdAfx.h"
#include "TreeNode.h"
#include "PropertiesWnd.h"
#include <boost/bind.hpp>
#include "OutputWnd.h"
#include "MainApp.h"
#include "IceHelpers.h"

using namespace my;

void TreeNodeBase::SetPropertyFloat(CSimpleProp * pProp, const float * pValue)
{
	pProp->SetValue(*pValue);
}

void TreeNodeBase::GetPropertyFloat(const CSimpleProp * pProp, float * pValue)
{
	*pValue = pProp->GetValue().fltVal;
}

void TreeNodeBase::SetPropertyString(CSimpleProp * pProp, const CString * pValue)
{
	pProp->SetValue((LPCTSTR)*pValue);
}

void TreeNodeBase::GetPropertyString(const CSimpleProp * pProp, CString * pValue)
{
	*pValue = pProp->GetValue().bstrVal;
}

void TreeNodeBase::SetPropertyQuatX(CSimpleProp * pProp, const Quaternion * pValue)
{
	pProp->SetValue(D3DXToDegree(pValue->ToEulerAngleX()));
}

void TreeNodeBase::GetPropertyQuatX(const CSimpleProp * pProp, Quaternion * pValue)
{
	CSimpleProp * pParentProp = DYNAMIC_DOWNCAST(CSimpleProp, pProp->GetParent());
	ASSERT_VALID(pParentProp);
	*pValue = Quaternion::RotationEulerAngles(Vector3(D3DXToRadian(pProp->GetValue().fltVal), D3DXToRadian(pParentProp->GetSubItem(1)->GetValue().fltVal), D3DXToRadian(pParentProp->GetSubItem(2)->GetValue().fltVal)));
}

void TreeNodeBase::SetPropertyQuatY(CSimpleProp * pProp, const Quaternion * pValue)
{
	pProp->SetValue(D3DXToDegree(pValue->ToEulerAngleY()));
}

void TreeNodeBase::GetPropertyQuatY(const CSimpleProp * pProp, Quaternion * pValue)
{
	CSimpleProp * pParentProp = DYNAMIC_DOWNCAST(CSimpleProp, pProp->GetParent());
	ASSERT_VALID(pParentProp);
	*pValue = Quaternion::RotationEulerAngles(Vector3(D3DXToRadian(pParentProp->GetSubItem(0)->GetValue().fltVal), D3DXToRadian(pProp->GetValue().fltVal), D3DXToRadian(pParentProp->GetSubItem(2)->GetValue().fltVal)));
}

void TreeNodeBase::SetPropertyQuatZ(CSimpleProp * pProp, const Quaternion * pValue)
{
	pProp->SetValue(D3DXToDegree(pValue->ToEulerAngleZ()));
}

void TreeNodeBase::GetPropertyQuatZ(const CSimpleProp * pProp, Quaternion * pValue)
{
	CSimpleProp * pParentProp = DYNAMIC_DOWNCAST(CSimpleProp, pProp->GetParent());
	ASSERT_VALID(pParentProp);
	*pValue = Quaternion::RotationEulerAngles(Vector3(D3DXToRadian(pParentProp->GetSubItem(0)->GetValue().fltVal), D3DXToRadian(pParentProp->GetSubItem(1)->GetValue().fltVal), D3DXToRadian(pProp->GetValue().fltVal)));
}

void TreeNodeBase::Serialize(CArchive & ar)
{
	if(ar.IsStoring())
	{
		ar << m_Position.x;
		ar << m_Position.y;
		ar << m_Position.z;
		ar << m_Rotation.x;
		ar << m_Rotation.y;
		ar << m_Rotation.z;
		ar << m_Rotation.w;
		ar << m_Scale.x;
		ar << m_Scale.y;
		ar << m_Scale.z;
	}
	else
	{
		ar >> m_Position.x;
		ar >> m_Position.y;
		ar >> m_Position.z;
		ar >> m_Rotation.x;
		ar >> m_Rotation.y;
		ar >> m_Rotation.z;
		ar >> m_Rotation.w;
		ar >> m_Scale.x;
		ar >> m_Scale.y;
		ar >> m_Scale.z;
	}
}

IMPLEMENT_SERIAL(TreeNodeMesh, TreeNodeBase, 1)

void TreeNodeMesh::Serialize(CArchive & ar)
{
	TreeNodeBase::Serialize(ar);

	if(ar.IsStoring())
	{
		std::basic_string<TCHAR> tmp(ms2ts(theApp.GetResourceKey(m_Mesh)));
		CString path(tmp.c_str());
		ar << path;
	}
	else
	{
		CString path;
		ar >> path;
		LoadFromMesh(path);
	}
}

void TreeNodeBase::SetupProperties(CMFCPropertyGridCtrl * pPropertyGridCtrl)
{
	CSimpleProp * pWorld = new CSimpleProp(_T("World"));
	CSimpleProp * pPosition = new CSimpleProp(_T("Position"), 0, TRUE);
	pWorld->AddSubItem(pPosition);
	CSimpleProp * pProp = new CSimpleProp(_T("x"), (_variant_t)m_Position.x, _T("x"));
	pProp->m_EventChanged = boost::bind(GetPropertyFloat, pProp, &m_Position.x);
	pProp->m_EventUpdated = boost::bind(SetPropertyFloat, pProp, &m_Position.x);
	pPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)m_Position.y, _T("y"));
	pProp->m_EventChanged = boost::bind(GetPropertyFloat, pProp, &m_Position.y);
	pProp->m_EventUpdated = boost::bind(SetPropertyFloat, pProp, &m_Position.y);
	pPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)m_Position.y, _T("z"));
	pProp->m_EventChanged = boost::bind(GetPropertyFloat, pProp, &m_Position.z);
	pProp->m_EventUpdated = boost::bind(SetPropertyFloat, pProp, &m_Position.z);
	pPosition->AddSubItem(pProp);

	CSimpleProp * pRotation = new CSimpleProp(_T("Rotation"), 0, TRUE);
	pWorld->AddSubItem(pRotation);
	pProp = new CSimpleProp(_T("x"), (_variant_t)D3DXToDegree(m_Rotation.ToEulerAngleX()), _T("x"));
	pProp->m_EventChanged = boost::bind(GetPropertyQuatX, pProp, &m_Rotation);
	pProp->m_EventUpdated = boost::bind(SetPropertyQuatX, pProp, &m_Rotation);
	pRotation->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)D3DXToDegree(m_Rotation.ToEulerAngleY()), _T("y"));
	pProp->m_EventChanged = boost::bind(GetPropertyQuatY, pProp, &m_Rotation);
	pProp->m_EventUpdated = boost::bind(SetPropertyQuatY, pProp, &m_Rotation);
	pRotation->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)D3DXToDegree(m_Rotation.ToEulerAngleZ()), _T("z"));
	pProp->m_EventChanged = boost::bind(GetPropertyQuatZ, pProp, &m_Rotation);
	pProp->m_EventUpdated = boost::bind(SetPropertyQuatZ, pProp, &m_Rotation);
	pRotation->AddSubItem(pProp);

	CSimpleProp * pScale = new CSimpleProp(_T("Scale"), 0, TRUE);
	pWorld->AddSubItem(pScale);
	pProp = new CSimpleProp(_T("x"), (_variant_t)m_Scale.x, _T("x"));
	pProp->m_EventChanged = boost::bind(GetPropertyFloat, pProp, &m_Scale.x);
	pProp->m_EventUpdated = boost::bind(SetPropertyFloat, pProp, &m_Scale.x);
	pScale->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)m_Scale.y, _T("y"));
	pProp->m_EventChanged = boost::bind(GetPropertyFloat, pProp, &m_Scale.y);
	pProp->m_EventUpdated = boost::bind(SetPropertyFloat, pProp, &m_Scale.y);
	pScale->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)m_Scale.y, _T("z"));
	pProp->m_EventChanged = boost::bind(GetPropertyFloat, pProp, &m_Scale.z);
	pProp->m_EventUpdated = boost::bind(SetPropertyFloat, pProp, &m_Scale.z);
	pScale->AddSubItem(pProp);

	pPropertyGridCtrl->AddProperty(pWorld);
}

bool TreeNodeMesh::LoadFromMesh(LPCTSTR lpszMesh)
{
	m_Mesh = theApp.LoadMesh(ts2ms(lpszMesh));
	if(!m_Mesh)
	{
		return false;
	}

	m_OpcMeshInterface.SetNbTriangles(m_Mesh->GetNumFaces());
	m_OpcMeshInterface.SetNbVertices(m_Mesh->GetNumVertices());
	m_OpcMeshInterfaceCB.m_pMesh = m_Mesh.get();
	m_OpcMeshInterfaceCB.m_pIndices = m_Mesh->LockIndexBuffer();
	m_OpcMeshInterfaceCB.m_pVertices = m_Mesh->LockVertexBuffer();
	if(m_Mesh->GetOptions() & D3DXMESH_32BIT)
	{
		m_OpcMeshInterface.SetCallback(Callback::Func<DWORD>, &m_OpcMeshInterfaceCB);
	}
	else
	{
		m_OpcMeshInterface.SetCallback(Callback::Func<WORD>, &m_OpcMeshInterfaceCB);
	}
	Opcode::OPCODECREATE Create;
	Create.mIMesh			= &m_OpcMeshInterface;
	Create.mSettings.mLimit	= 1;
	Create.mSettings.mRules	= Opcode::SPLIT_SPLATTER_POINTS | Opcode::SPLIT_GEOM_CENTER;
	Create.mNoLeaf			= true;
	Create.mQuantized		= true;
	Create.mKeepOriginal	= false;
	Create.mCanRemap		= false;
	m_OpcMode.Build(Create);
	// ! �����d3dxmesh������D3DPOOL_MANAGED����֤buffer�ڴ��ַʼ����Ч
	m_Mesh->UnlockIndexBuffer();
	m_Mesh->UnlockVertexBuffer();

	std::vector<std::string>::const_iterator mat_name_iter = m_Mesh->m_MaterialNameList.begin();
	for(; mat_name_iter != m_Mesh->m_MaterialNameList.end(); mat_name_iter++)
	{
		MaterialPtr mat = theApp.LoadMaterial(str_printf("material/%s.txt", mat_name_iter->c_str()));
		mat = mat ? mat : theApp.m_DefaultMat;
		m_Materials.push_back(TreeNodeMesh::MaterialPair(mat, theApp.LoadEffect("shader/SimpleSample.fx", EffectMacroPairList())));
	}
	return true;
}

void TreeNodeMesh::SetupProperties(CMFCPropertyGridCtrl * pPropertyGridCtrl)
{
	TreeNodeBase::SetupProperties(pPropertyGridCtrl);
}

void TreeNodeMesh::Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Matrix4 & World)
{
	for(DWORD i = 0; i < m_Materials.size(); i++)
	{
		MaterialPairList::reference mat_pair = m_Materials[i];
		_ASSERT(mat_pair.second);
		mat_pair.second->SetMatrix("g_World", Matrix4::Compose(m_Scale, m_Rotation, m_Position) * World);
		mat_pair.second->SetTexture("g_MeshTexture", mat_pair.first->m_DiffuseTexture);
		mat_pair.second->SetTechnique("RenderScene");
		UINT passes = mat_pair.second->Begin();
		for(UINT p = 0; p < passes; p++)
		{
			mat_pair.second->BeginPass(p);
			m_Mesh->DrawSubset(i);
			mat_pair.second->EndPass();
		}
		mat_pair.second->End();
	}
}

bool TreeNodeMesh::RayTest(const std::pair<Vector3, Vector3> & ray, const Matrix4 & World)
{
	// ! Opcode��֧�����ž���������Ҫ�ȱ任Ϊģ�ͱ�������
	Matrix4 w2l = (Matrix4::Compose(m_Scale, m_Rotation, m_Position) * World).inverse();
	IceMaths::Ray ir((IceMaths::Point&)ray.first.transform(w2l).xyz, (IceMaths::Point&)ray.second.transformNormal(w2l));
	Opcode::RayCollider collider;
	collider.SetFirstContact(false);
	collider.SetPrimitiveTests(true);
	collider.SetTemporalCoherence(false);
	return collider.Collide(ir, m_OpcMode, NULL, NULL) && collider.GetContactStatus();
}

void TreeNodeCollision::Serialize(CArchive & ar)
{
	TreeNodeBase::Serialize(ar);

	if(ar.IsStoring())
	{
		ar << m_BindBone;
	}
	else
	{
		ar >> m_BindBone;
	}
}

void TreeNodeCollision::SetupProperties(CMFCPropertyGridCtrl * pPropertyGridCtrl)
{
	TreeNodeBase::SetupProperties(pPropertyGridCtrl);

	CSimpleProp * pCollision = new CSimpleProp(_T("pCollision"));
	CSimpleProp * pProp = new CSimpleProp(_T("BindBone"), (_variant_t)m_BindBone, _T("BindBone"));
	pProp->m_EventChanged = boost::bind(GetPropertyString, pProp, &m_BindBone);
	pProp->m_EventUpdated = boost::bind(GetPropertyString, pProp, &m_BindBone);
	pCollision->AddSubItem(pProp);

	pPropertyGridCtrl->AddProperty(pCollision);
}

IMPLEMENT_SERIAL(TreeNodeCollisionCapsule, TreeNodeCollision, 1)

void TreeNodeCollisionCapsule::Serialize(CArchive & ar)
{
	TreeNodeCollision::Serialize(ar);

	if(ar.IsStoring())
	{
		ar << m_Radius;
		ar << m_Height;
		ar << m_BindBone;
	}
	else
	{
		ar >> m_Radius;
		ar >> m_Height;
		ar >> m_BindBone;
	}
}

void TreeNodeCollisionCapsule::SetupProperties(CMFCPropertyGridCtrl * pPropertyGridCtrl)
{
	TreeNodeCollision::SetupProperties(pPropertyGridCtrl);

	CSimpleProp * pCapsule = new CSimpleProp(_T("Capsule"));
	CSimpleProp * pProp = new CSimpleProp(_T("Radius"), (_variant_t)m_Radius, _T("Radius"));
	pProp->m_EventChanged = boost::bind(GetPropertyFloat, pProp, &m_Radius);
	pProp->m_EventUpdated = boost::bind(SetPropertyFloat, pProp, &m_Radius);
	pCapsule->AddSubItem(pProp);

	pProp = new CSimpleProp(_T("Height"), (_variant_t)m_Radius, _T("Height"));
	pProp->m_EventChanged = boost::bind(GetPropertyFloat, pProp, &m_Height);
	pProp->m_EventUpdated = boost::bind(SetPropertyFloat, pProp, &m_Height);
	pCapsule->AddSubItem(pProp);

	pPropertyGridCtrl->AddProperty(pCapsule);
}

void TreeNodeCollisionCapsule::Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Matrix4 & World)
{
	DrawHelper::DrawCapsule(pd3dDevice, m_Radius, m_Height, D3DCOLOR_ARGB(255,255,0,255), Matrix4::Compose(m_Scale, m_Rotation, m_Position) * World);
}

bool TreeNodeCollisionCapsule::RayTest(const std::pair<Vector3, Vector3> & ray, const Matrix4 & World)
{
	// ! �����CapsuleĬ����Y��������ܺ�Nvidia Physx��һ��
	Matrix4 w2l = (Matrix4::Compose(m_Scale, m_Rotation, m_Position) * World).inverse();
	IceMaths::Ray ir((IceMaths::Point&)ray.first.transform(w2l).xyz, (IceMaths::Point&)ray.second.transformNormal(w2l));
	m_Capsule.mP0 = IceMaths::Point(0, -m_Height * 0.5f, 0);
	m_Capsule.mP1 = IceMaths::Point(0,  m_Height * 0.5f, 0);
	m_Capsule.mRadius = m_Radius;
	float s[2];
	return 0 != RayCapsuleOverlap(ir.mOrig, ir.mDir, m_Capsule, s);
}

IMPLEMENT_SERIAL(TreeNodeCollisionBox, TreeNodeCollision, 1)

void TreeNodeCollisionBox::Serialize(CArchive & ar)
{
	TreeNodeCollision::Serialize(ar);

	if(ar.IsStoring())
	{
		ar << m_Box.mExtents.x;
		ar << m_Box.mExtents.y;
		ar << m_Box.mExtents.z;
	}
	else
	{
		ar >> m_Box.mExtents.x;
		ar >> m_Box.mExtents.y;
		ar >> m_Box.mExtents.z;
	}
}

void TreeNodeCollisionBox::SetupProperties(CMFCPropertyGridCtrl * pPropertyGridCtrl)
{
	TreeNodeCollision::SetupProperties(pPropertyGridCtrl);

	CSimpleProp * pBox = new CSimpleProp(_T("Box"));
	CSimpleProp * pExtent = new CSimpleProp(_T("Extent"), 0, TRUE);
	pBox->AddSubItem(pExtent);
	CSimpleProp * pProp = new CSimpleProp(_T("x"), (_variant_t)m_Box.mExtents.x, _T("x"));
	pProp->m_EventChanged = boost::bind(GetPropertyFloat, pProp, &m_Box.mExtents.x);
	pProp->m_EventUpdated = boost::bind(SetPropertyFloat, pProp, &m_Box.mExtents.x);
	pExtent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)m_Box.mExtents.y, _T("y"));
	pProp->m_EventChanged = boost::bind(GetPropertyFloat, pProp, &m_Box.mExtents.y);
	pProp->m_EventUpdated = boost::bind(SetPropertyFloat, pProp, &m_Box.mExtents.y);
	pExtent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)m_Box.mExtents.y, _T("z"));
	pProp->m_EventChanged = boost::bind(GetPropertyFloat, pProp, &m_Box.mExtents.z);
	pProp->m_EventUpdated = boost::bind(SetPropertyFloat, pProp, &m_Box.mExtents.z);
	pExtent->AddSubItem(pProp);

	pPropertyGridCtrl->AddProperty(pBox);
}

void TreeNodeCollisionBox::Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Matrix4 & World)
{
	DrawHelper::DrawBox(pd3dDevice, (Vector3&)m_Box.mExtents, D3DCOLOR_ARGB(255,255,0,255), Matrix4::Compose(m_Scale, m_Rotation, m_Position) * World);
}

bool TreeNodeCollisionBox::RayTest(const std::pair<Vector3, Vector3> & ray, const Matrix4 & World)
{
	Matrix4 w2l = (Matrix4::Compose(m_Scale, m_Rotation, m_Position) * World).inverse();
	IceMaths::Ray ir((IceMaths::Point&)ray.first.transform(w2l).xyz, (IceMaths::Point&)ray.second.transformNormal(w2l));
	float dist;
	IceMaths::Point hit;
	return RayOBB(ir.mOrig, ir.mDir, m_Box, dist, hit);
}
