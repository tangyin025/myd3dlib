
#pragma once

class CImgRegionDoc;

class CFileView : public CDockablePane
{
public:
	BOOL m_bIsLayoutInvalid;

public:
	CFileView(void);

	void AdjustLayout(void);

	void OnChangeVisualStyle(void);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	afx_msg void OnSetFocus(CWnd* pOldWnd);

	DECLARE_MESSAGE_MAP()

public:
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);

	afx_msg void OnTvnSelchangedTree(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnDragchangedTree(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnBeginlabeledit(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnEndlabeledit(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

	void InvalidLayout(void)
	{
		m_bIsLayoutInvalid = TRUE;
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

