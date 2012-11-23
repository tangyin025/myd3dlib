
#pragma once

class CPropertiesWnd : public CDockablePane
{
public:
	CPropertiesWnd();

	void AdjustLayout();

	CFont m_fntPropList;

	CMFCPropertyGridCtrl m_wndPropList;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnSetFocus(CWnd* pOldWnd);

	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);

	DECLARE_MESSAGE_MAP()

	void SetPropListFont();
};

