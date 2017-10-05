#pragma once
#include "afxpropertygridctrl.h"
#include "afxwin.h"
#include "EventDefine.h"

class CChildView;

// CEnvironmentWnd

class CEnvironmentWnd : public CDockablePane
{
	DECLARE_DYNAMIC(CEnvironmentWnd)

	enum Property
	{
		PropertyCamera,
		PropertySSAO,
	};

	enum CameraProperty
	{
		CameraPropertyLevelId,
		CameraPropertyLookAt,
		CameraPropertyEular,
	};

	enum SSAOProperty
	{
		SSAOPropertyBias,
		SSAOPropertyIntensity,
		SSAOPropertyRadius,
		SSAOPropertyScale,
	};

	enum LevelIdProperty
	{
		LevelIdPropertyX,
		LevelIdPropertyY,
	};

	enum Vector3Property
	{
		Vector3PropertyX,
		Vector3PropertyY,
		Vector3PropertyZ,
	};

	struct CameraPropEventArgs : public EventArgs
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
	void OnCameraPropChanged(EventArgs * arg);
	static CMFCPropertyGridProperty * GetTopProp(CMFCPropertyGridProperty * pProp);
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);
};


