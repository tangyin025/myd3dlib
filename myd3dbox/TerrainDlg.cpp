// TerrainDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MainApp.h"
#include "TerrainDlg.h"
#include "afxdialogex.h"
#include "CtrlProps.h"
#include "Component.h"
#include "PropertiesWnd.h"

// TerrainDlg dialog

IMPLEMENT_DYNAMIC(TerrainDlg, CDialogEx)

TerrainDlg::TerrainDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(TerrainDlg::IDD, pParent)
	, m_RowChunks(1)
	, m_ColChunks(1)
	, m_ChunkSize(32)
{
	m_Material.m_Shader = theApp.default_shader;
	m_Material.ParseShaderParameters();
}

TerrainDlg::~TerrainDlg()
{
}

void TerrainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_RowChunks);
	DDX_Text(pDX, IDC_EDIT2, m_ColChunks);
	DDX_Text(pDX, IDC_EDIT3, m_ChunkSize);
	DDX_Control(pDX, IDC_MFCPROPERTYGRID1, m_PropGridCtrl);
	if (!pDX->m_bSaveAndValidate)
	{
		m_PropGridCtrl.RemoveAll();
		CMFCPropertyGridProperty * pComponent = new CSimpleProp(CPropertiesWnd::GetComponentTypeName(Component::ComponentTypeComponent), CPropertiesWnd::PropertyMesh, FALSE);
		m_PropGridCtrl.AddProperty(pComponent, FALSE, TRUE);
		CPropertiesWnd::CreatePropertiesMaterial(pComponent, _T("Material"), &m_Material);
	}
}


BEGIN_MESSAGE_MAP(TerrainDlg, CDialogEx)
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
END_MESSAGE_MAP()


// TerrainDlg message handlers


BOOL TerrainDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	CRect rc;
	m_PropGridCtrl.GetWindowRect(&rc);
	m_PropGridCtrl.SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.Width(), rc.Height()));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT TerrainDlg::OnPropertyChanged(WPARAM wParam, LPARAM lParam)
{
	CMFCPropertyGridProperty * pProp = (CMFCPropertyGridProperty *)lParam;
	ASSERT(pProp);
	DWORD PropertyId = pProp->GetData();
	switch (PropertyId)
	{
	case CPropertiesWnd::PropertyMaterialShader:
	{
		Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
		material->m_Shader = ts2ms(pProp->GetValue().bstrVal);
		material->ParseShaderParameters();
		CPropertiesWnd::UpdatePropertiesMaterial(pProp->GetParent(), material);
		m_PropGridCtrl.AdjustLayout();
		break;
	}
	case CPropertiesWnd::PropertyMaterialPassMask:
	{
		Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(CPropertiesWnd::g_PassMaskDesc));
		material->m_PassMask = CPropertiesWnd::g_PassMaskDesc[i].mask;
		break;
	}
	case CPropertiesWnd::PropertyMaterialCullMode:
	{
		Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(CPropertiesWnd::g_CullModeDesc));
		material->m_CullMode = i + 1;
		break;
	}
	case CPropertiesWnd::PropertyMaterialZEnable:
	{
		Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
		material->m_ZEnable = pProp->GetValue().boolVal != 0;
		break;
	}
	case CPropertiesWnd::PropertyMaterialZWriteEnable:
	{
		Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
		material->m_ZWriteEnable = pProp->GetValue().boolVal != 0;
		break;
	}
	case CPropertiesWnd::PropertyMaterialBlendMode:
	{
		Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(CPropertiesWnd::g_BlendModeDesc));
		material->m_BlendMode = i;
		break;
	}
	case CPropertiesWnd::PropertyMaterialParameterFloat:
	{
		Material * mtl = (Material *)pProp->GetParent()->GetParent()->GetValue().ulVal;
		INT i = CSimpleProp::GetSubIndexInParent(pProp);
		ASSERT(mtl->m_ParameterList[i]->m_Type == MaterialParameter::ParameterTypeFloat);
		boost::dynamic_pointer_cast<MaterialParameterFloat>(mtl->m_ParameterList[i])->m_Value = pProp->GetValue().fltVal;
		break;
	}
	case CPropertiesWnd::PropertyMaterialParameterFloat2:
	case CPropertiesWnd::PropertyMaterialParameterFloat3:
	case CPropertiesWnd::PropertyMaterialParameterFloat4:
	case CPropertiesWnd::PropertyMaterialParameterFloatValueX:
	case CPropertiesWnd::PropertyMaterialParameterFloatValueY:
	case CPropertiesWnd::PropertyMaterialParameterFloatValueZ:
	case CPropertiesWnd::PropertyMaterialParameterFloatValueW:
	{
		CMFCPropertyGridProperty * pParameter = NULL;
		switch (PropertyId)
		{
		case CPropertiesWnd::PropertyMaterialParameterFloat2:
		case CPropertiesWnd::PropertyMaterialParameterFloat3:
		case CPropertiesWnd::PropertyMaterialParameterFloat4:
			pParameter = pProp;
			break;
		case CPropertiesWnd::PropertyMaterialParameterFloatValueX:
		case CPropertiesWnd::PropertyMaterialParameterFloatValueY:
		case CPropertiesWnd::PropertyMaterialParameterFloatValueZ:
		case CPropertiesWnd::PropertyMaterialParameterFloatValueW:
			pParameter = pProp->GetParent();
			break;
		}
		ASSERT(pParameter);
		Material * mtl = (Material *)pParameter->GetParent()->GetParent()->GetValue().ulVal;
		INT i = CSimpleProp::GetSubIndexInParent(pParameter);
		switch (pParameter->GetData())
		{
		case CPropertiesWnd::PropertyMaterialParameterFloat2:
			ASSERT(mtl->m_ParameterList[i]->m_Type == MaterialParameter::ParameterTypeFloat2);
			boost::dynamic_pointer_cast<MaterialParameterFloat2>(mtl->m_ParameterList[i])->m_Value = my::Vector2(
				pParameter->GetSubItem(0)->GetValue().fltVal, pParameter->GetSubItem(1)->GetValue().fltVal);
			break;
		case CPropertiesWnd::PropertyMaterialParameterFloat3:
			ASSERT(mtl->m_ParameterList[i]->m_Type == MaterialParameter::ParameterTypeFloat3);
			boost::dynamic_pointer_cast<MaterialParameterFloat3>(mtl->m_ParameterList[i])->m_Value = my::Vector3(
				pParameter->GetSubItem(0)->GetValue().fltVal, pParameter->GetSubItem(1)->GetValue().fltVal, pParameter->GetSubItem(2)->GetValue().fltVal);
			break;
		case CPropertiesWnd::PropertyMaterialParameterFloat4:
			ASSERT(mtl->m_ParameterList[i]->m_Type == MaterialParameter::ParameterTypeFloat4);
			boost::dynamic_pointer_cast<MaterialParameterFloat4>(mtl->m_ParameterList[i])->m_Value = my::Vector4(
				pParameter->GetSubItem(0)->GetValue().fltVal, pParameter->GetSubItem(1)->GetValue().fltVal, pParameter->GetSubItem(2)->GetValue().fltVal, pParameter->GetSubItem(3)->GetValue().fltVal);
			break;
		}
		break;
	}
	case CPropertiesWnd::PropertyMaterialParameterTexture:
	{
		Material * mtl = (Material *)pProp->GetParent()->GetParent()->GetValue().ulVal;
		INT i = CSimpleProp::GetSubIndexInParent(pProp);
		ASSERT(mtl->m_ParameterList[i]->m_Type == MaterialParameter::ParameterTypeTexture);
		boost::dynamic_pointer_cast<MaterialParameterTexture>(mtl->m_ParameterList[i])->m_TexturePath = ts2ms(pProp->GetValue().bstrVal);
		break;
	}
	}
	return 0;
}

