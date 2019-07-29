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
	//, m_DiffuseTexture(ms2ts(theApp.default_texture).c_str())
	//, m_NormalTexture(ms2ts(theApp.default_normal_texture).c_str())
	//, m_SpecularTexture(ms2ts(theApp.default_specular_texture).c_str())
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
	//  DDX_Text(pDX, IDC_EDIT4, m_DiffuseTexture);
	//  DDX_Text(pDX, IDC_EDIT5, m_NormalTexture);
	//  DDX_Text(pDX, IDC_EDIT6, m_SpecularTexture);
	DDX_Control(pDX, IDC_MFCPROPERTYGRID1, m_PropGridCtrl);
	if (!pDX->m_bSaveAndValidate)
	{
		m_PropGridCtrl.RemoveAll();
		CMFCPropertyGridProperty * pComponent = new CSimpleProp(CPropertiesWnd::GetComponentTypeName(Component::ComponentTypeComponent), CPropertiesWnd::PropertyMesh, FALSE);
		m_PropGridCtrl.AddProperty(pComponent, FALSE, FALSE);
		CPropertiesWnd::CreatePropertiesMaterial(pComponent, _T("Material"), &m_Material);
		m_PropGridCtrl.AdjustLayout();
	}
}


BEGIN_MESSAGE_MAP(TerrainDlg, CDialogEx)
//	ON_BN_CLICKED(IDC_BUTTON1, &TerrainDlg::OnBnClickedButton1)
//	ON_BN_CLICKED(IDC_BUTTON2, &TerrainDlg::OnBnClickedButton2)
//	ON_BN_CLICKED(IDC_BUTTON3, &TerrainDlg::OnBnClickedButton3)
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
END_MESSAGE_MAP()


// TerrainDlg message handlers


//void TerrainDlg::OnBnClickedButton1()
//{
//	// TODO: Add your control notification handler code here
//	CFileDialog dlg(TRUE, NULL, m_DiffuseTexture, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
//	if (IDOK == dlg.DoModal())
//	{
//		GetDlgItem(IDC_EDIT4)->SetWindowText(dlg.GetPathName());
//	}
//}


//void TerrainDlg::OnBnClickedButton2()
//{
//	// TODO: Add your control notification handler code here
//	CFileDialog dlg(TRUE, NULL, m_NormalTexture, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
//	if (IDOK == dlg.DoModal())
//	{
//		GetDlgItem(IDC_EDIT5)->SetWindowText(dlg.GetPathName());
//	}
//}


//void TerrainDlg::OnBnClickedButton3()
//{
//	// TODO: Add your control notification handler code here
//	CFileDialog dlg(TRUE, NULL, m_SpecularTexture, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
//	if (IDOK == dlg.DoModal())
//	{
//		GetDlgItem(IDC_EDIT6)->SetWindowText(dlg.GetPathName());
//	}
//}

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
		UpdateData(TRUE);
		UpdateData(FALSE);
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
		mtl->m_ParameterList[i]->ReleaseResource();
		boost::dynamic_pointer_cast<MaterialParameterTexture>(mtl->m_ParameterList[i])->m_TexturePath = ts2ms(pProp->GetValue().bstrVal);
		mtl->m_ParameterList[i]->RequestResource();
		break;
	}
	}
	return 0;
}
