#pragma once
#include "afxpropertygridctrl.h"
#include "afxwin.h"


// CEnvironmentWnd

class CEnvironmentWnd : public CDockablePane
{
	DECLARE_DYNAMIC(CEnvironmentWnd)

	enum Property
	{
		PropertyCamera,
	};

	enum CameraProperty
	{
		CameraPropertyLevelId,
		CameraPropertyLookAt,
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
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
};


