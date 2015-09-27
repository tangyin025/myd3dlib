#pragma once

class Actor;

class CClassToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class COutlinerWnd : public CDockablePane
{
public:
	COutlinerWnd();
	virtual ~COutlinerWnd();

	void AdjustLayout();
	void OnChangeVisualStyle();

public:
	CTreeCtrl m_wndClassView;
	CImageList m_ClassViewImages;
	CMenu m_ContextMenu;
	CMenu m_ContextMenuAdd;

// Overrides
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnChangeActiveTab(WPARAM, LPARAM);
	afx_msg void OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMenuAddMeshComponent();
};
