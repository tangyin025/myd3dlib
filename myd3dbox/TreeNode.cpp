#include "StdAfx.h"
#include "TreeNode.h"
#include "PropertiesWnd.h"
#include <boost/bind.hpp>
#include "OutputWnd.h"

using namespace my;

MeshTreeNode::~MeshTreeNode(void)
{
}

void MeshTreeNode::Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Matrix4 & World)
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

void SetPropertyFloat(CSimpleProp * pProp, const float * pValue)
{
	pProp->SetValue(*pValue);
}

void GetPropertyFloat(const CSimpleProp * pProp, float * pValue)
{
	*pValue = pProp->GetValue().fltVal;
}

void SetPropertyQuatX(CSimpleProp * pProp, const Quaternion * pValue)
{
	pProp->SetValue(D3DXToDegree(atan2((pValue->w * pValue->x + pValue->y * pValue->z) * 2, 1 - (pValue->x * pValue->x + pValue->y * pValue->y) * 2)));
}

void GetPropertyQuatX(const CSimpleProp * pProp, Quaternion * pValue)
{
	CSimpleProp * pParentProp = DYNAMIC_DOWNCAST(CSimpleProp, pProp->GetParent());
	ASSERT_VALID(pParentProp);
	*pValue = Quaternion::RotationEulerAngles(Vector3(D3DXToRadian(pProp->GetValue().fltVal), D3DXToRadian(pParentProp->GetSubItem(1)->GetValue().fltVal), D3DXToRadian(pParentProp->GetSubItem(2)->GetValue().fltVal)));
}

void SetPropertyQuatY(CSimpleProp * pProp, const Quaternion * pValue)
{
	pProp->SetValue(D3DXToDegree(asin((pValue->w * pValue->y - pValue->z * pValue->x) * 2)));
}

void GetPropertyQuatY(const CSimpleProp * pProp, Quaternion * pValue)
{
	CSimpleProp * pParentProp = DYNAMIC_DOWNCAST(CSimpleProp, pProp->GetParent());
	ASSERT_VALID(pParentProp);
	*pValue = Quaternion::RotationEulerAngles(Vector3(D3DXToRadian(pParentProp->GetSubItem(0)->GetValue().fltVal), D3DXToRadian(pProp->GetValue().fltVal), D3DXToRadian(pParentProp->GetSubItem(2)->GetValue().fltVal)));
}

void SetPropertyQuatZ(CSimpleProp * pProp, const Quaternion * pValue)
{
	pProp->SetValue(D3DXToDegree(atan2((pValue->w * pValue->z + pValue->x * pValue->y) * 2, 1 - (pValue->y * pValue->y + pValue->z * pValue->z) * 2)));
}

void GetPropertyQuatZ(const CSimpleProp * pProp, Quaternion * pValue)
{
	CSimpleProp * pParentProp = DYNAMIC_DOWNCAST(CSimpleProp, pProp->GetParent());
	ASSERT_VALID(pParentProp);
	*pValue = Quaternion::RotationEulerAngles(Vector3(D3DXToRadian(pParentProp->GetSubItem(0)->GetValue().fltVal), D3DXToRadian(pParentProp->GetSubItem(1)->GetValue().fltVal), D3DXToRadian(pProp->GetValue().fltVal)));
}

void MeshTreeNode::SetupProperties(CMFCPropertyGridCtrl * pPropertyGridCtrl)
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
	pProp = new CSimpleProp(_T("x"), (_variant_t)m_Rotation.x, _T("x"));
	pProp->m_EventChanged = boost::bind(GetPropertyQuatX, pProp, &m_Rotation);
	pProp->m_EventUpdated = boost::bind(SetPropertyQuatX, pProp, &m_Rotation);
	pRotation->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)m_Rotation.y, _T("y"));
	pProp->m_EventChanged = boost::bind(GetPropertyQuatY, pProp, &m_Rotation);
	pProp->m_EventUpdated = boost::bind(SetPropertyQuatY, pProp, &m_Rotation);
	pRotation->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)m_Rotation.y, _T("z"));
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
