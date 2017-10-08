// EnvironmentWnd.cpp : implementation file
//

#include "stdafx.h"
#include "EnvironmentWnd.h"
#include "CtrlProps.h"
#include "MainFrm.h"
#include "MainApp.h"
#include "ChildView.h"

#define SSAO_BIAS_RANGE 1.0f
#define SSAO_INTENSITY_RANGE 10.0f
#define SSAO_RADIUS_RANGE 200.0f
#define SSAO_SCALE_RANGE 20.0f

// CEnvironmentWnd

IMPLEMENT_DYNAMIC(CEnvironmentWnd, CDockablePane)

CEnvironmentWnd::CEnvironmentWnd()
{

}

CEnvironmentWnd::~CEnvironmentWnd()
{
}


BEGIN_MESSAGE_MAP(CEnvironmentWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
END_MESSAGE_MAP()

void CEnvironmentWnd::AdjustLayout(void)
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient,rectCombo;
	GetClientRect(rectClient);

	//m_wndObjectCombo.GetWindowRect(&rectCombo);

	int cyCmb = 0;//rectCombo.Size().cy;
	int cyTlb = 0;//m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	//m_wndObjectCombo.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), 200, SWP_NOACTIVATE | SWP_NOZORDER);
	//m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb + cyTlb, rectClient.Width(), rectClient.Height() -(cyCmb+cyTlb), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CEnvironmentWnd::SetPropListFont(void)
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
}

void CEnvironmentWnd::InitPropList()
{
	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

	CMFCPropertyGridProperty * pCamera = new CSimpleProp(_T("Camera"), PropertyCamera, FALSE);
	m_wndPropList.AddProperty(pCamera, FALSE, FALSE);
	CMFCPropertyGridProperty * pLevelId = new CSimpleProp(_T("LevelId"), CameraPropertyLevelId, TRUE);
	pCamera->AddSubItem(pLevelId);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)0l, NULL, LevelIdPropertyX);
	pLevelId->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)0l, NULL, LevelIdPropertyY);
	pLevelId->AddSubItem(pProp);

	CMFCPropertyGridProperty * pLookAt = new CSimpleProp(_T("LookAt"), CameraPropertyLookAt, TRUE);
	pCamera->AddSubItem(pLookAt);
	pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, Vector3PropertyX);
	pLookAt->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, Vector3PropertyY);
	pLookAt->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, Vector3PropertyZ);
	pLookAt->AddSubItem(pProp);

	CMFCPropertyGridProperty * pEular = new CSimpleProp(_T("Eular"), CameraPropertyEular, TRUE);
	pCamera->AddSubItem(pEular);
	pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, Vector3PropertyX);
	pEular->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, Vector3PropertyY);
	pEular->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, Vector3PropertyZ);
	pEular->AddSubItem(pProp);

	CMFCPropertyGridProperty * pSkyBox = new CSimpleProp(_T("SkyBox"), PropertySkyBox, FALSE);
	m_wndPropList.AddProperty(pSkyBox, FALSE, FALSE);
	CMFCPropertyGridProperty * pSkyBoxEnable = new CCheckBoxProp(_T("Enable"), FALSE, NULL, SkyBoxPropertyEnable);
	pSkyBox->AddSubItem(pSkyBoxEnable);
	CMFCPropertyGridProperty * pSkyBoxTextures = new CSimpleProp(_T("Textures"), SkyBoxPropertyTextures, FALSE);
	pSkyBox->AddSubItem(pSkyBoxTextures);
	const TCHAR * tex_name[6] = {_T("Front"), _T("Back"), _T("Left"), _T("Right"), _T("Up"), _T("Down")};
	for (unsigned int i = 0; i < _countof(tex_name); i++)
	{
		CMFCPropertyGridProperty * pProp = new CFileProp(tex_name[i], TRUE, _T(""), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, i);
		pSkyBoxTextures->AddSubItem(pProp);
	}

	CMFCPropertyGridProperty * pSkyLight = new CSimpleProp(_T("SkyLight"), PropertySkyLight, FALSE);
	m_wndPropList.AddProperty(pSkyLight, FALSE, FALSE);
	CMFCPropertyGridProperty * pSkyLightDir = new CSimpleProp(_T("Dir"), SkyLightPropertyDir, TRUE);
	pSkyLight->AddSubItem(pSkyLightDir);
	pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, Vector3PropertyX);
	pSkyLightDir->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, Vector3PropertyY);
	pSkyLightDir->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, Vector3PropertyZ);
	pSkyLightDir->AddSubItem(pProp);

	COLORREF color = RGB(255,255,255);
	CColorProp * pSkyLightDiffuse = new CColorProp(_T("Diffuse"), color, NULL, NULL, SkyLightPropertyDiffuse);
	pSkyLightDiffuse->EnableOtherButton(_T("Other..."));
	pSkyLight->AddSubItem(pSkyLightDiffuse);

	CColorProp * pSkyLightAmbient = new CColorProp(_T("Ambient"), color, NULL, NULL, SkyLightPropertyAmbient);
	pSkyLightAmbient->EnableOtherButton(_T("Other..."));
	pSkyLight->AddSubItem(pSkyLightAmbient);

	CMFCPropertyGridProperty * pSSAO = new CSimpleProp(_T("SSAO"), PropertySSAO, FALSE);
	m_wndPropList.AddProperty(pSSAO, FALSE, FALSE);
	pProp = new CCheckBoxProp(_T("Enable"), FALSE, NULL, SSAOPropertyEnable);
	pSSAO->AddSubItem(pProp);
	pProp = new CSliderProp(_T("Bias"), (_variant_t)0l, NULL, SSAOPropertyBias);
	pSSAO->AddSubItem(pProp);
	pProp = new CSliderProp(_T("Intensity"), (_variant_t)0l, NULL, SSAOPropertyIntensity);
	pSSAO->AddSubItem(pProp);
	pProp = new CSliderProp(_T("Radius"), (_variant_t)0l, NULL, SSAOPropertyRadius);
	pSSAO->AddSubItem(pProp);
	pProp = new CSliderProp(_T("Scale"), (_variant_t)0l, NULL, SSAOPropertyScale);
	pSSAO->AddSubItem(pProp);

	m_wndPropList.AdjustLayout();
}

void CEnvironmentWnd::OnCameraPropChanged(EventArgs * arg)
{
	CameraPropEventArgs * camera_prop_arg = dynamic_cast<CameraPropEventArgs *>(arg);
	ASSERT(camera_prop_arg);

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);

	CMFCPropertyGridProperty * pCamera = m_wndPropList.GetProperty(PropertyCamera);
	ASSERT_VALID(pCamera);
	pCamera->GetSubItem(CameraPropertyLevelId)->GetSubItem(LevelIdPropertyX)->SetValue((_variant_t)pFrame->m_WorldL.m_LevelId.x);
	pCamera->GetSubItem(CameraPropertyLevelId)->GetSubItem(LevelIdPropertyY)->SetValue((_variant_t)pFrame->m_WorldL.m_LevelId.y);

	my::Vector3 LookAt = (camera_prop_arg->pView->m_CameraType == CChildView::CameraTypePerspective ? boost::dynamic_pointer_cast<my::ModelViewerCamera>(camera_prop_arg->pView->m_Camera)->m_LookAt : camera_prop_arg->pView->m_Camera->m_Eye);
	pCamera->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyX)->SetValue((_variant_t)LookAt.x);
	pCamera->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyY)->SetValue((_variant_t)LookAt.y);
	pCamera->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyZ)->SetValue((_variant_t)LookAt.z);

	pCamera->GetSubItem(CameraPropertyEular)->GetSubItem(Vector3PropertyX)->SetValue((_variant_t)D3DXToDegree(camera_prop_arg->pView->m_Camera->m_Eular.x));
	pCamera->GetSubItem(CameraPropertyEular)->GetSubItem(Vector3PropertyY)->SetValue((_variant_t)D3DXToDegree(camera_prop_arg->pView->m_Camera->m_Eular.y));
	pCamera->GetSubItem(CameraPropertyEular)->GetSubItem(Vector3PropertyZ)->SetValue((_variant_t)D3DXToDegree(camera_prop_arg->pView->m_Camera->m_Eular.z));

	CMFCPropertyGridProperty * pSkyBox = m_wndPropList.GetProperty(PropertySkyBox);
	ASSERT_VALID(pSkyBox);
	pSkyBox->GetSubItem(SkyBoxPropertyEnable)->SetValue((_variant_t)(VARIANT_BOOL)camera_prop_arg->pView->m_SkyBoxEnable);
	for (unsigned int i = 0; i < _countof(camera_prop_arg->pView->m_SkyBoxTextures); i++)
	{
		pSkyBox->GetSubItem(SkyBoxPropertyTextures)->GetSubItem(i)->SetValue(
			(_variant_t)ms2ts(camera_prop_arg->pView->m_SkyBoxTextures[i].m_Path).c_str());
	}

	CMFCPropertyGridProperty * pSkyLight = m_wndPropList.GetProperty(PropertySkyLight);
	pSkyLight->GetSubItem(SkyLightPropertyDir)->GetSubItem(Vector3PropertyX)->SetValue((_variant_t)D3DXToDegree(camera_prop_arg->pView->m_SkyLightCam->m_Eular.x));
	pSkyLight->GetSubItem(SkyLightPropertyDir)->GetSubItem(Vector3PropertyY)->SetValue((_variant_t)D3DXToDegree(camera_prop_arg->pView->m_SkyLightCam->m_Eular.y));
	pSkyLight->GetSubItem(SkyLightPropertyDir)->GetSubItem(Vector3PropertyZ)->SetValue((_variant_t)D3DXToDegree(camera_prop_arg->pView->m_SkyLightCam->m_Eular.z));

	COLORREF color = RGB(
		camera_prop_arg->pView->m_SkyLightDiffuse.x * 255,
		camera_prop_arg->pView->m_SkyLightDiffuse.y * 255,
		camera_prop_arg->pView->m_SkyLightDiffuse.z * 255);
	(DYNAMIC_DOWNCAST(CColorProp, pSkyLight->GetSubItem(SkyLightPropertyDiffuse)))->SetColor((_variant_t)color);

	color = RGB(
		camera_prop_arg->pView->m_SkyLightAmbient.x * 255,
		camera_prop_arg->pView->m_SkyLightAmbient.y * 255,
		camera_prop_arg->pView->m_SkyLightAmbient.z * 255);
	(DYNAMIC_DOWNCAST(CColorProp, pSkyLight->GetSubItem(SkyLightPropertyAmbient)))->SetColor((_variant_t)color);

	CMFCPropertyGridProperty * pSSAO = m_wndPropList.GetProperty(PropertySSAO);
	pSSAO->GetSubItem(SSAOPropertyEnable)->SetValue((_variant_t)(VARIANT_BOOL)camera_prop_arg->pView->m_SsaoEnable);
	pSSAO->GetSubItem(SSAOPropertyBias)->SetValue((_variant_t)(long)(camera_prop_arg->pView->m_SsaoBias/SSAO_BIAS_RANGE*CSliderProp::RANGE));
	pSSAO->GetSubItem(SSAOPropertyIntensity)->SetValue((_variant_t)(long)(camera_prop_arg->pView->m_SsaoIntensity/SSAO_INTENSITY_RANGE*CSliderProp::RANGE));
	pSSAO->GetSubItem(SSAOPropertyRadius)->SetValue((_variant_t)(long)(camera_prop_arg->pView->m_SsaoRadius/SSAO_RADIUS_RANGE*CSliderProp::RANGE));
	pSSAO->GetSubItem(SSAOPropertyScale)->SetValue((_variant_t)(long)(camera_prop_arg->pView->m_SsaoScale/SSAO_SCALE_RANGE*CSliderProp::RANGE));

	m_wndPropList.Invalidate(FALSE);
}

CMFCPropertyGridProperty * CEnvironmentWnd::GetTopProp(CMFCPropertyGridProperty * pProp)
{
	if (pProp->GetParent())
	{
		return GetTopProp(pProp->GetParent());
	}
	return pProp;
}

// CEnvironmentWnd message handlers

int CEnvironmentWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	CRect rectDummy;
	rectDummy.SetRectEmpty();

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("Failed to create Properties Grid \n");
		return -1;      // fail to create
	}

	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventCameraPropChanged.connect(boost::bind(&CEnvironmentWnd::OnCameraPropChanged, this, _1));

	SetPropListFont();
	InitPropList();
	AdjustLayout();

	return 0;
}

void CEnvironmentWnd::OnDestroy()
{
	CDockablePane::OnDestroy();

	// TODO: Add your message handler code here
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventCameraPropChanged.disconnect(boost::bind(&CEnvironmentWnd::OnCameraPropChanged, this, _1));
}

void CEnvironmentWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	AdjustLayout();
}

void CEnvironmentWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	// TODO: Add your message handler code here
	m_wndPropList.SetFocus();
}

void CEnvironmentWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);

	// TODO: Add your message handler code here
	SetPropListFont();
}

LRESULT CEnvironmentWnd::OnPropertyChanged(WPARAM wParam, LPARAM lParam)
{
	CMFCPropertyGridProperty * pProp = (CMFCPropertyGridProperty *)lParam;
	ASSERT(pProp);
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CChildView * pView = DYNAMIC_DOWNCAST(CChildView, pFrame->GetActiveView());
	ASSERT_VALID(pView);
	pProp = GetTopProp(pProp);
	DWORD PropertyId = pProp->GetData();
	switch (PropertyId)
	{
	case PropertyCamera:
		{
			CPoint new_level_id(
				my::Clamp<long>(pProp->GetSubItem(CameraPropertyLevelId)->GetSubItem(LevelIdPropertyX)->GetValue().intVal, 0, pFrame->m_WorldL.m_Dimension - 1),
				my::Clamp<long>(pProp->GetSubItem(CameraPropertyLevelId)->GetSubItem(LevelIdPropertyY)->GetValue().intVal, 0, pFrame->m_WorldL.m_Dimension - 1));
			if (new_level_id != pFrame->m_WorldL.m_LevelId)
			{
				pFrame->m_WorldL.ResetLevelId(new_level_id, pFrame);
			}

			if (pView->m_CameraType == CChildView::CameraTypePerspective)
			{
				my::ModelViewerCamera * model_view_camera = dynamic_cast<my::ModelViewerCamera *>(pView->m_Camera.get());
				model_view_camera->m_LookAt = my::Vector3(
					pProp->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyX)->GetValue().fltVal,
					pProp->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyY)->GetValue().fltVal,
					pProp->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyZ)->GetValue().fltVal);
			}
			else
			{
				pView->m_Camera->m_Eye = my::Vector3(
					pProp->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyX)->GetValue().fltVal,
					pProp->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyY)->GetValue().fltVal,
					pProp->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyZ)->GetValue().fltVal);
			}
			pView->m_Camera->m_Eular = my::Vector3(
				D3DXToRadian(pProp->GetSubItem(CameraPropertyEular)->GetSubItem(Vector3PropertyX)->GetValue().fltVal),
				D3DXToRadian(pProp->GetSubItem(CameraPropertyEular)->GetSubItem(Vector3PropertyY)->GetValue().fltVal),
				D3DXToRadian(pProp->GetSubItem(CameraPropertyEular)->GetSubItem(Vector3PropertyZ)->GetValue().fltVal));
			pView->m_Camera->UpdateViewProj();
		}
		break;
	case PropertySkyBox:
		{
			pView->m_SkyBoxEnable = pProp->GetSubItem(SkyBoxPropertyEnable)->GetValue().boolVal != 0;
			for (unsigned int i = 0; i < _countof(pView->m_SkyBoxTextures); i++)
			{
				pView->m_SkyBoxTextures[i].m_Path = ts2ms(pProp->GetSubItem(SkyBoxPropertyTextures)->GetSubItem(i)->GetValue().bstrVal);
				pView->m_SkyBoxTextures[i].RequestResource();
			}
		}
		break;
	case PropertySkyLight:
		{
			pView->m_SkyLightCam->m_Eular = my::Vector3(
				D3DXToRadian(pProp->GetSubItem(SkyLightPropertyDir)->GetSubItem(Vector3PropertyX)->GetValue().fltVal),
				D3DXToRadian(pProp->GetSubItem(SkyLightPropertyDir)->GetSubItem(Vector3PropertyY)->GetValue().fltVal),
				D3DXToRadian(pProp->GetSubItem(SkyLightPropertyDir)->GetSubItem(Vector3PropertyZ)->GetValue().fltVal));
			pView->m_SkyLightCam->UpdateViewProj();

			COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pProp->GetSubItem(SkyLightPropertyDiffuse)))->GetColor();
			pView->m_SkyLightDiffuse.xyz = my::Vector3(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);

			color = (DYNAMIC_DOWNCAST(CColorProp, pProp->GetSubItem(SkyLightPropertyAmbient)))->GetColor();
			pView->m_SkyLightAmbient.xyz = my::Vector3(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
		}
		break;
	case PropertySSAO:
		{
			pView->m_SsaoEnable = pProp->GetSubItem(SSAOPropertyEnable)->GetValue().boolVal != 0;
			pView->m_SsaoBias = pProp->GetSubItem(SSAOPropertyBias)->GetValue().lVal / (float)CSliderProp::RANGE*SSAO_BIAS_RANGE;
			pView->m_SsaoIntensity = pProp->GetSubItem(SSAOPropertyIntensity)->GetValue().lVal / (float)CSliderProp::RANGE*SSAO_INTENSITY_RANGE;
			pView->m_SsaoRadius = pProp->GetSubItem(SSAOPropertyRadius)->GetValue().lVal / (float)CSliderProp::RANGE*SSAO_RADIUS_RANGE;
			pView->m_SsaoScale = pProp->GetSubItem(SSAOPropertyScale)->GetValue().lVal / (float)CSliderProp::RANGE*SSAO_SCALE_RANGE;
		}
		break;
	}
	pView->Invalidate();
	return 0l;
}
