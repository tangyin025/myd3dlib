#pragma once
#include "afxpropertygridctrl.h"
#include "afxwin.h"

class CChildView;

// CEnvironmentWnd

class CEnvironmentWnd : public CDockablePane
{
	DECLARE_DYNAMIC(CEnvironmentWnd)

	enum Property
	{
		PropertyCamera,
		PropertySkyLight,
		PropertyDepthOfField,
		PropertyBloom,
		PropertySSAO,
		PropertyFog,
	};

	enum CameraProperty
	{
		CameraPropertyFov,
		CameraPropertyLookAt,
		CameraPropertyEuler,
		CameraPropertyNearZ,
		CameraPropertyFarZ,
	};

	enum SkyBoxProperty
	{
		SkyBoxPropertyTextureFront,
		SkyBoxPropertyTextureBack,
		SkyBoxPropertyTextureLeft,
		SkyBoxPropertyTextureRight,
		SkyBoxPropertyTextureUp,
		SkyBoxPropertyTextureDown,
	};

	enum SkyLightProperty
	{
		SkyLightPropertyEuler,
		SkyLightPropertyDiffuse,
		SkyLightPropertySpecular,
		SkyLightPropertyAmbientColor,
		SkyLightPropertyAmbientSpecular,
	};

	enum DepthOfFieldProperty
	{
		DepthOfFieldEnable,
		DepthOfFieldParam0,
		DepthOfFieldParam1,
		DepthOfFieldParam2,
		DepthOfFieldParam3,
	};

	enum BloomProperty
	{
		BloomPropertyEnable,
		BloomPropertyLuminanceThreshold,
		BloomPropertyColor,
		BloomPropertyFactor,
	};

	enum SSAOProperty
	{
		SSAOPropertyEnable,
		SSAOPropertyBias,
		SSAOPropertyIntensity,
		SSAOPropertyRadius,
		SSAOPropertyScale,
	};

	enum FogProperty
	{
		FogPropertyEnable,
		FogPropertyColor,
		FogPropertyStartDistance,
		FogPropertyHeight,
		FogPropertyFalloff,
	};

	enum Vector3Property
	{
		Vector3PropertyX,
		Vector3PropertyY,
		Vector3PropertyZ,
	};

	struct CameraPropEventArgs : public my::EventArg
	{
	public:
		CChildView * pView;

		CameraPropEventArgs(CChildView * _pView)
			: pView(_pView)
		{
		}
	};

public:
	CEnvironmentWnd();
	virtual ~CEnvironmentWnd();

protected:
	DECLARE_MESSAGE_MAP()
	CMFCPropertyGridCtrl m_wndPropList;
	CFont m_fntPropList;
	void AdjustLayout(void);
	void SetPropListFont(void);
	void InitPropList(void);
	void OnCameraPropChanged(my::EventArg * arg);
	static CMFCPropertyGridProperty * GetTopProp(CMFCPropertyGridProperty * pProp);
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);
};


