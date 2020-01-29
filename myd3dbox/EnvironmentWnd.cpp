// EnvironmentWnd.cpp : implementation file
//

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
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

	CMFCPropertyGridProperty * pCamera = new CSimpleProp(_T("Camera"), PropertyCamera, FALSE);
	m_wndPropList.AddProperty(pCamera, FALSE, FALSE);
	CMFCPropertyGridProperty * pLookAt = new CSimpleProp(_T("LookAt"), CameraPropertyLookAt, TRUE);
	pCamera->AddSubItem(pLookAt);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, Vector3PropertyX);
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

	CColorProp * pBgColor = new CColorProp(_T("BgColor"), 0, NULL, NULL, CameraPropertyBgColor);
	pBgColor->EnableOtherButton(_T("Other..."));
	pCamera->AddSubItem(pBgColor);

	CMFCPropertyGridProperty * pSkyBox = new CSimpleProp(_T("SkyBox"), PropertySkyBox, FALSE);
	m_wndPropList.AddProperty(pSkyBox, FALSE, FALSE);
	const TCHAR * tex_name[6] = {_T("Front"), _T("Back"), _T("Left"), _T("Right"), _T("Up"), _T("Down")};
	for (unsigned int i = 0; i < _countof(tex_name); i++)
	{
		CMFCPropertyGridProperty * pProp = new CFileProp(tex_name[i], TRUE, _T(""), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, SkyBoxPropertyTextureFront + i);
		pSkyBox->AddSubItem(pProp);
	}

	CMFCPropertyGridProperty * pSkyLight = new CSimpleProp(_T("SkyLight"), PropertySkyLight, FALSE);
	m_wndPropList.AddProperty(pSkyLight, FALSE, FALSE);
	CMFCPropertyGridProperty * pSkyLightDir = new CSimpleProp(_T("Eular"), SkyLightPropertyEular, TRUE);
	pSkyLight->AddSubItem(pSkyLightDir);
	pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, Vector3PropertyX);
	pSkyLightDir->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, Vector3PropertyY);
	pSkyLightDir->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, Vector3PropertyZ);
	pSkyLightDir->AddSubItem(pProp);

	COLORREF color = RGB(255,255,255);
	CColorProp * pSkyLightColor = new CColorProp(_T("SkyLightDiffuse"), color, NULL, NULL, SkyLightPropertyDiffuse);
	pSkyLightColor->EnableOtherButton(_T("Other..."));
	pSkyLight->AddSubItem(pSkyLightColor);

	pProp = new CSimpleProp(_T("SkyLightSpecular"), (_variant_t)1.0f, NULL, SkyLightPropertySpecular);
	pSkyLight->AddSubItem(pProp);

	CColorProp * pAmbientColor = new CColorProp(_T("AmbientColor"), color, NULL, NULL, SkyLightPropertyAmbientColor);
	pAmbientColor->EnableOtherButton(_T("Other..."));
	pSkyLight->AddSubItem(pAmbientColor);

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

	CMFCPropertyGridProperty * pFog = new CSimpleProp(_T("HeightFog"), PropertyFog, FALSE);
	m_wndPropList.AddProperty(pFog, FALSE, FALSE);
	pProp = new CCheckBoxProp(_T("Enable"), FALSE, NULL, FogPropertyEnable);
	pFog->AddSubItem(pProp);
	pBgColor = new CColorProp(_T("Color"), 0, NULL, NULL, FogPropertyColor);
	pBgColor->EnableOtherButton(_T("Other..."));
	pFog->AddSubItem(pBgColor);
	pProp = new CSimpleProp(_T("StartDistance"), (_variant_t)0.0f, NULL, FogPropertyStartDistance);
	pFog->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Height"), (_variant_t)0.0f, NULL, FogPropertyHeight);
	pFog->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Falloff"), (_variant_t)0.0f, NULL, FogPropertyFalloff);
	pFog->AddSubItem(pProp);

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
	my::ModelViewerCamera * model_view_camera = dynamic_cast<my::ModelViewerCamera *>(camera_prop_arg->pView->m_Camera.get());
	pCamera->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyX)->SetValue((_variant_t)model_view_camera->m_LookAt.x);
	pCamera->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyY)->SetValue((_variant_t)model_view_camera->m_LookAt.y);
	pCamera->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyZ)->SetValue((_variant_t)model_view_camera->m_LookAt.z);

	pCamera->GetSubItem(CameraPropertyEular)->GetSubItem(Vector3PropertyX)->SetValue((_variant_t)D3DXToDegree(camera_prop_arg->pView->m_Camera->m_Eular.x));
	pCamera->GetSubItem(CameraPropertyEular)->GetSubItem(Vector3PropertyY)->SetValue((_variant_t)D3DXToDegree(camera_prop_arg->pView->m_Camera->m_Eular.y));
	pCamera->GetSubItem(CameraPropertyEular)->GetSubItem(Vector3PropertyZ)->SetValue((_variant_t)D3DXToDegree(camera_prop_arg->pView->m_Camera->m_Eular.z));

	COLORREF color = RGB(theApp.m_BgColor.x * 255, theApp.m_BgColor.y * 255, theApp.m_BgColor.z * 255);
	(DYNAMIC_DOWNCAST(CColorProp, pCamera->GetSubItem(CameraPropertyBgColor)))->SetColor((_variant_t)color);

	CMFCPropertyGridProperty * pSkyBox = m_wndPropList.GetProperty(PropertySkyBox);
	ASSERT_VALID(pSkyBox);
	for (unsigned int i = 0; i < _countof(theApp.m_SkyBoxTextures); i++)
	{
		pSkyBox->GetSubItem(SkyBoxPropertyTextureFront + i)->SetValue(
			(_variant_t)ms2ts(theApp.m_SkyBoxTextures[i].m_TexturePath).c_str());
	}

	CMFCPropertyGridProperty * pSkyLight = m_wndPropList.GetProperty(PropertySkyLight);
	pSkyLight->GetSubItem(SkyLightPropertyEular)->GetSubItem(Vector3PropertyX)->SetValue((_variant_t)D3DXToDegree(camera_prop_arg->pView->m_SkyLightCam->m_Eular.x));
	pSkyLight->GetSubItem(SkyLightPropertyEular)->GetSubItem(Vector3PropertyY)->SetValue((_variant_t)D3DXToDegree(camera_prop_arg->pView->m_SkyLightCam->m_Eular.y));
	pSkyLight->GetSubItem(SkyLightPropertyEular)->GetSubItem(Vector3PropertyZ)->SetValue((_variant_t)D3DXToDegree(camera_prop_arg->pView->m_SkyLightCam->m_Eular.z));

	color = RGB(theApp.m_SkyLightColor.x * 255, theApp.m_SkyLightColor.y * 255, theApp.m_SkyLightColor.z * 255);
	(DYNAMIC_DOWNCAST(CColorProp, pSkyLight->GetSubItem(SkyLightPropertyDiffuse)))->SetColor((_variant_t)color);

	pSkyLight->GetSubItem(SkyLightPropertySpecular)->SetValue((_variant_t)theApp.m_SkyLightColor.w);

	color = RGB(theApp.m_AmbientColor.x * 255, theApp.m_AmbientColor.y * 255, theApp.m_AmbientColor.z * 255);
	(DYNAMIC_DOWNCAST(CColorProp, pSkyLight->GetSubItem(SkyLightPropertyAmbientColor)))->SetColor((_variant_t)color);

	CMFCPropertyGridProperty * pSSAO = m_wndPropList.GetProperty(PropertySSAO);
	pSSAO->GetSubItem(SSAOPropertyEnable)->SetValue((_variant_t)(VARIANT_BOOL)camera_prop_arg->pView->m_SsaoEnable);
	pSSAO->GetSubItem(SSAOPropertyBias)->SetValue((_variant_t)theApp.m_SsaoBias);
	pSSAO->GetSubItem(SSAOPropertyIntensity)->SetValue((_variant_t)theApp.m_SsaoIntensity);
	pSSAO->GetSubItem(SSAOPropertyRadius)->SetValue((_variant_t)theApp.m_SsaoRadius);
	pSSAO->GetSubItem(SSAOPropertyScale)->SetValue((_variant_t)theApp.m_SsaoScale);

	CMFCPropertyGridProperty * pFog = m_wndPropList.GetProperty(PropertyFog);
	pFog->GetSubItem(FogPropertyEnable)->SetValue((_variant_t)(VARIANT_BOOL)camera_prop_arg->pView->m_FogEnable);
	color = RGB(theApp.m_FogColor.x * 255, theApp.m_FogColor.y * 255, theApp.m_FogColor.z * 255);
	(DYNAMIC_DOWNCAST(CColorProp, pFog->GetSubItem(FogPropertyColor)))->SetColor((_variant_t)color);
	pFog->GetSubItem(FogPropertyStartDistance)->SetValue((_variant_t)theApp.m_FogStartDistance);
	pFog->GetSubItem(FogPropertyHeight)->SetValue((_variant_t)theApp.m_FogHeight);
	pFog->GetSubItem(FogPropertyFalloff)->SetValue((_variant_t)theApp.m_FogFalloff);

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
	CMFCPropertyGridProperty * pTopProp = GetTopProp(pProp);
	DWORD PropertyId = pTopProp->GetData();
	switch (PropertyId)
	{
	case PropertyCamera:
		{
			my::ModelViewerCamera * model_view_camera = dynamic_cast<my::ModelViewerCamera *>(pView->m_Camera.get());
			model_view_camera->m_LookAt = my::Vector3(
				pTopProp->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyX)->GetValue().fltVal,
				pTopProp->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyY)->GetValue().fltVal,
				pTopProp->GetSubItem(CameraPropertyLookAt)->GetSubItem(Vector3PropertyZ)->GetValue().fltVal);
			pView->m_Camera->m_Eular = my::Vector3(
				D3DXToRadian(pTopProp->GetSubItem(CameraPropertyEular)->GetSubItem(Vector3PropertyX)->GetValue().fltVal),
				D3DXToRadian(pTopProp->GetSubItem(CameraPropertyEular)->GetSubItem(Vector3PropertyY)->GetValue().fltVal),
				D3DXToRadian(pTopProp->GetSubItem(CameraPropertyEular)->GetSubItem(Vector3PropertyZ)->GetValue().fltVal));
			COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pTopProp->GetSubItem(CameraPropertyBgColor)))->GetColor();
			theApp.m_BgColor.xyz = my::Vector3(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
			pView->m_Camera->UpdateViewProj();
		}
		break;
	case PropertySkyBox:
		{
			std::wstring path = pProp->GetValue().bstrVal;
			boost::basic_regex<TCHAR> reg(_T("_(FR|BK|LF|RT|UP|DN)"));
			boost::match_results<std::basic_string<TCHAR>::const_iterator> what;
			if (boost::regex_search(path, what, reg, boost::match_default) && what[1].matched)
			{
				const TCHAR * tex_name[6] = { _T("FR"), _T("BK"), _T("LF"), _T("RT"), _T("UP"), _T("DN") };
				for (unsigned int i = 0; i < _countof(theApp.m_SkyBoxTextures); i++)
				{
					std::basic_string<TCHAR> new_path;
					new_path.insert(new_path.end(), path.begin(), what[1].first);
					new_path.append(tex_name[i]);
					new_path.insert(new_path.end(), what[1].second, path.end());
					theApp.m_SkyBoxTextures[i].ReleaseResource();
					theApp.m_SkyBoxTextures[i].m_TexturePath = ts2ms(new_path);
					theApp.m_SkyBoxTextures[i].RequestResource();
				}
				CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
				ASSERT_VALID(pFrame);
				CEnvironmentWnd::CameraPropEventArgs arg(pView);
				pFrame->m_EventCameraPropChanged(&arg);
			}
			else
			{
				int i = pProp->GetData() - SkyBoxPropertyTextureFront;
				_ASSERT(i >= 0 && i < _countof(theApp.m_SkyBoxTextures));
				theApp.m_SkyBoxTextures[i].ReleaseResource();
				theApp.m_SkyBoxTextures[i].m_TexturePath = ts2ms(path);
				theApp.m_SkyBoxTextures[i].RequestResource();
			}
		}
		break;
	case PropertySkyLight:
		{
			pView->m_SkyLightCam->m_Eular = my::Vector3(
				D3DXToRadian(pTopProp->GetSubItem(SkyLightPropertyEular)->GetSubItem(Vector3PropertyX)->GetValue().fltVal),
				D3DXToRadian(pTopProp->GetSubItem(SkyLightPropertyEular)->GetSubItem(Vector3PropertyY)->GetValue().fltVal),
				D3DXToRadian(pTopProp->GetSubItem(SkyLightPropertyEular)->GetSubItem(Vector3PropertyZ)->GetValue().fltVal));

			COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pTopProp->GetSubItem(SkyLightPropertyDiffuse)))->GetColor();
			theApp.m_SkyLightColor.xyz = my::Vector3(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);

			theApp.m_SkyLightColor.w = pTopProp->GetSubItem(SkyLightPropertySpecular)->GetValue().fltVal;

			color = (DYNAMIC_DOWNCAST(CColorProp, pTopProp->GetSubItem(SkyLightPropertyAmbientColor)))->GetColor();
			theApp.m_AmbientColor = my::Vector3(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
		}
		break;
	case PropertySSAO:
		{
			pView->m_SsaoEnable = pTopProp->GetSubItem(SSAOPropertyEnable)->GetValue().boolVal != 0;
			theApp.m_SsaoBias = pTopProp->GetSubItem(SSAOPropertyBias)->GetValue().fltVal;
			theApp.m_SsaoIntensity = pTopProp->GetSubItem(SSAOPropertyIntensity)->GetValue().fltVal;
			theApp.m_SsaoRadius = pTopProp->GetSubItem(SSAOPropertyRadius)->GetValue().fltVal;
			theApp.m_SsaoScale = pTopProp->GetSubItem(SSAOPropertyScale)->GetValue().fltVal;
		}
		break;
	case PropertyFog:
		{
			pView->m_FogEnable = pTopProp->GetSubItem(FogPropertyEnable)->GetValue().boolVal != 0;
			COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pTopProp->GetSubItem(FogPropertyColor)))->GetColor();
			theApp.m_FogColor.xyz = my::Vector3(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
			theApp.m_FogStartDistance = pTopProp->GetSubItem(FogPropertyStartDistance)->GetValue().fltVal;
			theApp.m_FogHeight = pTopProp->GetSubItem(FogPropertyHeight)->GetValue().fltVal;
			theApp.m_FogFalloff = pTopProp->GetSubItem(FogPropertyFalloff)->GetValue().fltVal;
		}
		break;
	}
	pView->Invalidate();
	return 0l;
}
