
#pragma once

class CPropertiesWnd : public CDockablePane
{
public:
	CPropertiesWnd()
	{
	}

	DECLARE_MESSAGE_MAP()

	CFont m_fntPropList;

	CMFCToolBar m_wndToolBar;

	CMFCPropertyGridCtrl m_wndPropList;

	void AdjustLayout();

	void SetPropListFont();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnSetFocus(CWnd* pOldWnd);

	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);

	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);
};
