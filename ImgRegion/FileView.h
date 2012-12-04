
#pragma once

#include "ImgRegionDoc.h"

class CFileView : public CDockablePane
{
public:
	CFileView(void);

	void AdjustLayout(void);

	void OnChangeVisualStyle(void);

	CTreeCtrl m_wndFileView;

	CImgRegionDoc * m_pDoc;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnPaint(void);

	afx_msg void OnSetFocus(CWnd* pOldWnd);

	DECLARE_MESSAGE_MAP()

public:
	void OnIdleUpdate();

	void InsertRegionNode(const CImgRegionNode * node, HTREEITEM hParent = TVI_ROOT);
};

