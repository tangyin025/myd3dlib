
#pragma once

class CImgRegionDoc;

class CFileView : public CDockablePane
{
public:
	CImgRegionDoc * m_pDoc;

	std::set<CTreeCtrl *> m_TreeCtrlSet;

public:
	CFileView(void);

	void AdjustLayout(void);

	void OnChangeVisualStyle(void);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnPaint(void);

	afx_msg void OnSetFocus(CWnd* pOldWnd);

	DECLARE_MESSAGE_MAP()

public:
	void OnIdleUpdate();

	afx_msg void OnTvnSelchangedTree(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnDragchangedTree(UINT id, NMHDR *pNMHDR, LRESULT *pResult);
};

