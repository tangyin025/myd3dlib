// Copyright (c) 2011-2024 tangyin025
// License: MIT

#include "stdafx.h"
#include "EnvironmentWnd.h"
#include "CtrlProps.h"
#include "MainFrm.h"
#include "MainApp.h"
#include "ChildView.h"
#include <boost/regex.hpp>

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
	m_wndPropList.EnableDescriptionArea(TRUE);
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

	CMFCPropertyGridProperty * pCamera = new CSimpleProp(_T("Camera"), PropertyCamera, FALSE);
	m_wndPropList.AddProperty(pCamera, FALSE, FALSE);
	CMFCPropertyGridProperty * pFov = new CSimpleProp(_T("Fov (Y)"), (_variant_t)0.0f, NULL, CameraPropertyFov);
	pCamera->AddSubItem(pFov);
	CMFCPropertyGridProperty * pAov = new CSimpleProp(_T("Aov"), (_variant_t)0.0f, NULL, CameraPropertyAov);
	pAov->Enable(FALSE);
	pCamera->AddSubItem(pAov);

	CMFCPropertyGridProperty * pLookAt = new CSimpleProp(_T("LookAt"), CameraPropertyLookAt, TRUE);
	pCamera->AddSubItem(pLookAt);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, Vector3PropertyX);
	pLookAt->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, Vector3PropertyY);
	pLookAt->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, Vector3PropertyZ);
	pLookAt->AddSubItem(pProp);

	CMFCPropertyGridProperty * pEuler = new CSimpleProp(_T("Euler"), CameraPropertyEuler, TRUE);
	pCamera->AddSubItem(pEuler);
	pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, Vector3PropertyX);
	pEuler->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, Vector3PropertyY);
	pEuler->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, Vector3PropertyZ);
	pEuler->AddSubItem(pProp);

	CMFCPropertyGridProperty* pDistance = new CSimpleProp(_T("Distance"), (_variant_t)0.0f, NULL, CameraPropertyDistance);
	pCamera->AddSubItem(pDistance);
	CMFCPropertyGridProperty* pNearZ = new CSimpleProp(_T("NearZ"), (_variant_t)0.0f, NULL, CameraPropertyNearZ);
	pCamera->AddSubItem(pNearZ);
	CMFCPropertyGridProperty* pFarZ = new CSimpleProp(_T("FarZ"), (_variant_t)0.0f, NULL, CameraPropertyFarZ);
	pCamera->AddSubItem(pFarZ);

	COLORREF color = RGB(255, 255, 255);
	CColorProp * pBkColor = new CColorProp(_T("BkColor"), color, NULL, NULL, CameraPropertyBkColor);
	pBkColor->EnableOtherButton(_T("Other..."));
	pCamera->AddSubItem(pBkColor);

	CMFCPropertyGridProperty * pSkyLight = new CSimpleProp(_T("SkyLight"), PropertySkyLight, FALSE);
	m_wndPropList.AddProperty(pSkyLight, FALSE, FALSE);
	CMFCPropertyGridProperty * pSkyLightDir = new CSimpleProp(_T("Euler"), SkyLightPropertyEuler, TRUE);
	pSkyLight->AddSubItem(pSkyLightDir);
	pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, Vector3PropertyX);
	pSkyLightDir->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, Vector3PropertyY);
	pSkyLightDir->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, Vector3PropertyZ);
	pSkyLightDir->AddSubItem(pProp);

	CColorProp * pSkyLightColor = new CColorProp(_T("SkyLightDiffuse"), color, NULL, NULL, SkyLightPropertyDiffuse);
	pSkyLightColor->EnableOtherButton(_T("Other..."));
	pSkyLight->AddSubItem(pSkyLightColor);

	pProp = new CSimpleProp(_T("SkyLightSpecular"), (_variant_t)1.0f, NULL, SkyLightPropertySpecular);
	pSkyLight->AddSubItem(pProp);

	CColorProp * pAmbientColor = new CColorProp(_T("AmbientColor"), color, NULL, NULL, SkyLightPropertyAmbientColor);
	pAmbientColor->EnableOtherButton(_T("Other..."));
	pSkyLight->AddSubItem(pAmbientColor);

	pProp = new CSimpleProp(_T("AmbientSpecular"), (_variant_t)1.0f, NULL, SkyLightPropertyAmbientSpecular);
	pSkyLight->AddSubItem(pProp);

	CColorProp * pFogColor = new CColorProp(_T("FogColor"), color, NULL, NULL, SkyLightPropertyFogColor);
	pFogColor->EnableOtherButton(_T("Other..."));
	pSkyLight->AddSubItem(pFogColor);

	pProp = new CSimpleProp(_T("FogDensity"), (_variant_t)1.0f, NULL, SkyLightPropertyFogDensity);
	pSkyLight->AddSubItem(pProp);

	CMFCPropertyGridProperty* pSkyLightShadowBias = new CSimpleProp(_T("ShadowBias"), SkyLightPropertyShadowBias, TRUE);
	pSkyLight->AddSubItem(pSkyLightShadowBias);
	pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, Vector3PropertyX);
	pSkyLightShadowBias->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, Vector3PropertyY);
	pSkyLightShadowBias->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, Vector3PropertyZ);
	pSkyLightShadowBias->AddSubItem(pProp);

	CMFCPropertyGridProperty * pDepthOfField = new CSimpleProp(_T("DepthOfField"), PropertyDepthOfField, FALSE);
	m_wndPropList.AddProperty(pDepthOfField, FALSE, FALSE);
	pProp = new CSimpleProp(_T("Param0"), (_variant_t)1.0f, NULL, DepthOfFieldParam0);
	pDepthOfField->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Param1"), (_variant_t)1.0f, NULL, DepthOfFieldParam1);
	pDepthOfField->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Param2"), (_variant_t)1.0f, NULL, DepthOfFieldParam2);
	pDepthOfField->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Param3"), (_variant_t)1.0f, NULL, DepthOfFieldParam3);
	pDepthOfField->AddSubItem(pProp);

	CMFCPropertyGridProperty * pBloom = new CSimpleProp(_T("Bloom"), PropertyBloom, FALSE);
	m_wndPropList.AddProperty(pBloom, FALSE, FALSE);
	pProp = new CCheckBoxProp(_T("Enable"), FALSE, NULL, BloomPropertyEnable);
	pBloom->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("LuminanceThreshold"), (_variant_t)1.0f, NULL, BloomPropertyLuminanceThreshold);
	pBloom->AddSubItem(pProp);
	CColorProp * pBloomColor = new CColorProp(_T("Color"), color, NULL, NULL, BloomPropertyColor);
	pBloomColor->EnableOtherButton(_T("Other..."));
	pBloom->AddSubItem(pBloomColor);
	pProp = new CSimpleProp(_T("Factor"), (_variant_t)1.0f, NULL, BloomPropertyFactor);
	pBloom->AddSubItem(pProp);

	CMFCPropertyGridProperty * pSSAO = new CSimpleProp(_T("SSAO"), PropertySSAO, FALSE);
	m_wndPropList.AddProperty(pSSAO, FALSE, FALSE);
	pProp = new CCheckBoxProp(_T("Enable"), FALSE, NULL, SSAOPropertyEnable);
	pSSAO->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Bias"), (_variant_t)1.0f, NULL, SSAOPropertyBias);
	pSSAO->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Intensity"), (_variant_t)1.0f, NULL, SSAOPropertyIntensity);
	pSSAO->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Radius"), (_variant_t)1.0f, NULL, SSAOPropertyRadius);
	pSSAO->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Scale"), (_variant_t)1.0f, NULL, SSAOPropertyScale);
	pSSAO->AddSubItem(pProp);

	m_wndPropList.AdjustLayout();
}

void CEnvironmentWnd::OnCameraPropChanged(my::EventArg * arg)
{
	CameraPropEventArgs * camera_prop_arg = dynamic_cast<CameraPropEventArgs *>(arg);
	ASSERT(camera_prop_arg);

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);

	CMFCPropertyGridProperty * pCamera = m_wndPropList.GetProperty(PropertyCamera);
	ASSERT_VALID(pCamera);
	my::ModelViewerCamera * model_view_camera = dynamic_cast<my::ModelViewerCamera *>(camera_prop_arg->pView->m_Camera.get());
	pCamera->GetSubItem(CameraPropertyFov)->SetValue((_variant_t)D3DXToDegree(model_view_camera->m_Fov));
	pCamera->GetSubItem(CameraPropertyAov)->SetValue((_variant_t)D3DXToDegree(atanf(tanf(model_view_camera->m_Fov) * model_view_camera->m_Aspect)));

	pCamera->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyX)->SetValue((_variant_t)model_view_camera->m_LookAt.x);
	pCamera->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyY)->SetValue((_variant_t)model_view_camera->m_LookAt.y);
	pCamera->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyZ)->SetValue((_variant_t)model_view_camera->m_LookAt.z);

	pCamera->GetSubItem(CameraPropertyEuler)->GetSubItem(Vector3PropertyX)->SetValue((_variant_t)D3DXToDegree(camera_prop_arg->pView->m_Camera->m_Euler.x));
	pCamera->GetSubItem(CameraPropertyEuler)->GetSubItem(Vector3PropertyY)->SetValue((_variant_t)D3DXToDegree(camera_prop_arg->pView->m_Camera->m_Euler.y));
	pCamera->GetSubItem(CameraPropertyEuler)->GetSubItem(Vector3PropertyZ)->SetValue((_variant_t)D3DXToDegree(camera_prop_arg->pView->m_Camera->m_Euler.z));

	pCamera->GetSubItem(CameraPropertyDistance)->SetValue((_variant_t)model_view_camera->m_Distance);
	pCamera->GetSubItem(CameraPropertyNearZ)->SetValue((_variant_t)camera_prop_arg->pView->m_Camera->m_Nz);
	pCamera->GetSubItem(CameraPropertyFarZ)->SetValue((_variant_t)camera_prop_arg->pView->m_Camera->m_Fz);

	COLORREF color = RGB(LOBYTE(theApp.m_BkColor >> 16), LOBYTE(theApp.m_BkColor >> 8), LOBYTE(theApp.m_BkColor));
	(DYNAMIC_DOWNCAST(CColorProp, pCamera->GetSubItem(CameraPropertyBkColor)))->SetColor((_variant_t)color);

	CMFCPropertyGridProperty * pSkyLight = m_wndPropList.GetProperty(PropertySkyLight);
	pSkyLight->GetSubItem(SkyLightPropertyEuler)->GetSubItem(Vector3PropertyX)->SetValue((_variant_t)D3DXToDegree(theApp.m_SkyLightCam->m_Euler.x));
	pSkyLight->GetSubItem(SkyLightPropertyEuler)->GetSubItem(Vector3PropertyY)->SetValue((_variant_t)D3DXToDegree(theApp.m_SkyLightCam->m_Euler.y));
	pSkyLight->GetSubItem(SkyLightPropertyEuler)->GetSubItem(Vector3PropertyZ)->SetValue((_variant_t)D3DXToDegree(theApp.m_SkyLightCam->m_Euler.z));

	color = RGB(theApp.m_SkyLightColor.x * 255, theApp.m_SkyLightColor.y * 255, theApp.m_SkyLightColor.z * 255);
	(DYNAMIC_DOWNCAST(CColorProp, pSkyLight->GetSubItem(SkyLightPropertyDiffuse)))->SetColor((_variant_t)color);

	pSkyLight->GetSubItem(SkyLightPropertySpecular)->SetValue((_variant_t)theApp.m_SkyLightColor.w);

	color = RGB(theApp.m_AmbientColor.x * 255, theApp.m_AmbientColor.y * 255, theApp.m_AmbientColor.z * 255);
	(DYNAMIC_DOWNCAST(CColorProp, pSkyLight->GetSubItem(SkyLightPropertyAmbientColor)))->SetColor((_variant_t)color);

	pSkyLight->GetSubItem(SkyLightPropertyAmbientSpecular)->SetValue((_variant_t)theApp.m_AmbientColor.w);

	color = RGB(theApp.m_FogColor.x * 255, theApp.m_FogColor.y * 255, theApp.m_FogColor.z * 255);
	(DYNAMIC_DOWNCAST(CColorProp, pSkyLight->GetSubItem(SkyLightPropertyFogColor)))->SetColor((_variant_t)color);

	pSkyLight->GetSubItem(SkyLightPropertyFogDensity)->SetValue((_variant_t)theApp.m_FogColor.w);

	pSkyLight->GetSubItem(SkyLightPropertyShadowBias)->GetSubItem(Vector3PropertyX)->SetValue((_variant_t)theApp.m_CascadeLayerBias.x);
	pSkyLight->GetSubItem(SkyLightPropertyShadowBias)->GetSubItem(Vector3PropertyY)->SetValue((_variant_t)theApp.m_CascadeLayerBias.y);
	pSkyLight->GetSubItem(SkyLightPropertyShadowBias)->GetSubItem(Vector3PropertyZ)->SetValue((_variant_t)theApp.m_CascadeLayerBias.z);

	CMFCPropertyGridProperty * pDepthOfField = m_wndPropList.GetProperty(PropertyDepthOfField);
	pDepthOfField->GetSubItem(DepthOfFieldParam0)->SetValue((_variant_t)theApp.m_DofParams.x);
	pDepthOfField->GetSubItem(DepthOfFieldParam1)->SetValue((_variant_t)theApp.m_DofParams.y);
	pDepthOfField->GetSubItem(DepthOfFieldParam2)->SetValue((_variant_t)theApp.m_DofParams.z);
	pDepthOfField->GetSubItem(DepthOfFieldParam3)->SetValue((_variant_t)theApp.m_DofParams.w);

	CMFCPropertyGridProperty * pBloom = m_wndPropList.GetProperty(PropertyBloom);
	pBloom->GetSubItem(BloomPropertyEnable)->SetValue((_variant_t)(VARIANT_BOOL)camera_prop_arg->pView->m_BloomEnable);
	pBloom->GetSubItem(BloomPropertyLuminanceThreshold)->SetValue((_variant_t)theApp.m_LuminanceThreshold);
	color = RGB(theApp.m_BloomColor.x * 255, theApp.m_BloomColor.y * 255, theApp.m_BloomColor.z * 255);
	(DYNAMIC_DOWNCAST(CColorProp, pBloom->GetSubItem(BloomPropertyColor)))->SetColor((_variant_t)color);
	pBloom->GetSubItem(BloomPropertyFactor)->SetValue((_variant_t)theApp.m_BloomFactor);

	CMFCPropertyGridProperty * pSSAO = m_wndPropList.GetProperty(PropertySSAO);
	pSSAO->GetSubItem(SSAOPropertyEnable)->SetValue((_variant_t)(VARIANT_BOOL)camera_prop_arg->pView->m_SsaoEnable);
	pSSAO->GetSubItem(SSAOPropertyBias)->SetValue((_variant_t)theApp.m_SsaoBias);
	pSSAO->GetSubItem(SSAOPropertyIntensity)->SetValue((_variant_t)theApp.m_SsaoIntensity);
	pSSAO->GetSubItem(SSAOPropertyRadius)->SetValue((_variant_t)theApp.m_SsaoRadius);
	pSSAO->GetSubItem(SSAOPropertyScale)->SetValue((_variant_t)theApp.m_SsaoScale);

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

	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventCameraPropChanged.connect(boost::bind(&CEnvironmentWnd::OnCameraPropChanged, this, boost::placeholders::_1));

	SetPropListFont();
	InitPropList();
	AdjustLayout();

	return 0;
}

void CEnvironmentWnd::OnDestroy()
{
	CDockablePane::OnDestroy();

	// TODO: Add your message handler code here
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventCameraPropChanged.disconnect(boost::bind(&CEnvironmentWnd::OnCameraPropChanged, this, boost::placeholders::_1));
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
	CMFCPropertyGridProperty * pTopProp = GetTopProp(pProp);
	DWORD PropertyId = pTopProp->GetData();
	switch (PropertyId)
	{
	case PropertyCamera:
	{
		my::ModelViewerCamera* model_view_camera = dynamic_cast<my::ModelViewerCamera*>(pView->m_Camera.get());
		model_view_camera->m_Fov = D3DXToRadian(
			pTopProp->GetSubItem(CameraPropertyFov)->GetValue().fltVal);
		model_view_camera->m_LookAt = my::Vector3(
			pTopProp->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyX)->GetValue().fltVal,
			pTopProp->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyY)->GetValue().fltVal,
			pTopProp->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyZ)->GetValue().fltVal);
		pView->m_Camera->m_Euler = my::Vector3(
			D3DXToRadian(pTopProp->GetSubItem(CameraPropertyEuler)->GetSubItem(Vector3PropertyX)->GetValue().fltVal),
			D3DXToRadian(pTopProp->GetSubItem(CameraPropertyEuler)->GetSubItem(Vector3PropertyY)->GetValue().fltVal),
			D3DXToRadian(pTopProp->GetSubItem(CameraPropertyEuler)->GetSubItem(Vector3PropertyZ)->GetValue().fltVal));
		model_view_camera->m_Distance = pTopProp->GetSubItem(CameraPropertyDistance)->GetValue().fltVal;
		pView->m_Camera->m_Nz = pTopProp->GetSubItem(CameraPropertyNearZ)->GetValue().fltVal;
		pView->m_Camera->m_Fz = pTopProp->GetSubItem(CameraPropertyFarZ)->GetValue().fltVal;
		pView->m_Camera->UpdateViewProj();
		COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pTopProp->GetSubItem(CameraPropertyBkColor)))->GetColor();
		theApp.m_BkColor = D3DCOLOR_ARGB(0, GetRValue(color), GetGValue(color), GetBValue(color));
	}
	break;
	case PropertySkyLight:
	{
		theApp.m_SkyLightCam->m_Euler = my::Vector3(
			D3DXToRadian(pTopProp->GetSubItem(SkyLightPropertyEuler)->GetSubItem(Vector3PropertyX)->GetValue().fltVal),
			D3DXToRadian(pTopProp->GetSubItem(SkyLightPropertyEuler)->GetSubItem(Vector3PropertyY)->GetValue().fltVal),
			D3DXToRadian(pTopProp->GetSubItem(SkyLightPropertyEuler)->GetSubItem(Vector3PropertyZ)->GetValue().fltVal));

		COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pTopProp->GetSubItem(SkyLightPropertyDiffuse)))->GetColor();
		theApp.m_SkyLightColor.xyz = my::Vector3(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);

		theApp.m_SkyLightColor.w = pTopProp->GetSubItem(SkyLightPropertySpecular)->GetValue().fltVal;

		color = (DYNAMIC_DOWNCAST(CColorProp, pTopProp->GetSubItem(SkyLightPropertyAmbientColor)))->GetColor();
		theApp.m_AmbientColor.xyz = my::Vector3(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);

		theApp.m_AmbientColor.w = pTopProp->GetSubItem(SkyLightPropertyAmbientSpecular)->GetValue().fltVal;

		color = (DYNAMIC_DOWNCAST(CColorProp, pTopProp->GetSubItem(SkyLightPropertyFogColor)))->GetColor();
		theApp.m_FogColor.xyz = my::Vector3(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);

		theApp.m_FogColor.w = pTopProp->GetSubItem(SkyLightPropertyFogDensity)->GetValue().fltVal;

		theApp.m_CascadeLayerBias = my::Vector4(
			pTopProp->GetSubItem(SkyLightPropertyShadowBias)->GetSubItem(Vector3PropertyX)->GetValue().fltVal,
			pTopProp->GetSubItem(SkyLightPropertyShadowBias)->GetSubItem(Vector3PropertyY)->GetValue().fltVal,
			pTopProp->GetSubItem(SkyLightPropertyShadowBias)->GetSubItem(Vector3PropertyZ)->GetValue().fltVal, 0.0f);
	}
	break;
	case PropertyDepthOfField:
	{
		theApp.m_DofParams.x = pTopProp->GetSubItem(DepthOfFieldParam0)->GetValue().fltVal;
		theApp.m_DofParams.y = pTopProp->GetSubItem(DepthOfFieldParam1)->GetValue().fltVal;
		theApp.m_DofParams.z = pTopProp->GetSubItem(DepthOfFieldParam2)->GetValue().fltVal;
		theApp.m_DofParams.w = pTopProp->GetSubItem(DepthOfFieldParam3)->GetValue().fltVal;
	}
	break;
	case PropertyBloom:
	{
		pView->m_BloomEnable = pTopProp->GetSubItem(BloomPropertyEnable)->GetValue().boolVal != 0;
		theApp.m_LuminanceThreshold = pTopProp->GetSubItem(BloomPropertyLuminanceThreshold)->GetValue().fltVal;
		COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pTopProp->GetSubItem(BloomPropertyColor)))->GetColor();
		theApp.m_BloomColor = my::Vector3(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
		theApp.m_BloomFactor = pTopProp->GetSubItem(BloomPropertyFactor)->GetValue().fltVal;
		break;
	}
	case PropertySSAO:
	{
		pView->m_SsaoEnable = pTopProp->GetSubItem(SSAOPropertyEnable)->GetValue().boolVal != 0;
		theApp.m_SsaoBias = pTopProp->GetSubItem(SSAOPropertyBias)->GetValue().fltVal;
		theApp.m_SsaoIntensity = pTopProp->GetSubItem(SSAOPropertyIntensity)->GetValue().fltVal;
		theApp.m_SsaoRadius = pTopProp->GetSubItem(SSAOPropertyRadius)->GetValue().fltVal;
		theApp.m_SsaoScale = pTopProp->GetSubItem(SSAOPropertyScale)->GetValue().fltVal;
	}
	break;
	}
	pView->Invalidate();
	return 0l;
}
