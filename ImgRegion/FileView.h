
#pragma once

class CFileView : public CDockablePane
{
public:
	CFileView(void);

	void AdjustLayout(void);

	void OnChangeVisualStyle(void);

	CTreeCtrl m_wndFileView;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnPaint(void);

	afx_msg void OnSetFocus(CWnd* pOldWnd);

	DECLARE_MESSAGE_MAP()
};

