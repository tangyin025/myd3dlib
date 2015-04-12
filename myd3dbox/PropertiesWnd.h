
#pragma once

class CPropertiesToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class TreeNodeBase;

class CPropertiesWnd
	: public CDockablePane
	, public my::SingleInstance<CPropertiesWnd>
{
public:
	CPropertiesWnd()
		: m_bIsPropInvalid(TRUE)
	{
	}

	DECLARE_MESSAGE_MAP()
public:
	CPropertiesToolBar m_wndToolBar;

	CFont m_fntPropList;

	CMFCPropertyGridCtrl m_wndPropList;

	//boost::weak_ptr<TreeNodeBase> m_SelectedNode;

	BOOL m_bIsPropInvalid;

	void AdjustLayout();

	void SetPropListFont();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnSetFocus(CWnd* pOldWnd);

	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);

	afx_msg void OnExpandAllProperties();

	afx_msg void OnUpdateExpandAllProperties(CCmdUI* pCmdUI);

	afx_msg void OnSortProperties();

	afx_msg void OnUpdateSortProperties(CCmdUI* pCmdUI);

	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
};
