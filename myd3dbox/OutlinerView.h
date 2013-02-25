#pragma once

class COutlinerView
	: public CDockablePane
	, public my::SingleInstance<COutlinerView>
{
public:
	COutlinerView(void)
	{
	}

	virtual ~COutlinerView(void)
	{
	}

	DECLARE_MESSAGE_MAP()

	CMFCToolBar m_wndToolBar;

	CTreeCtrl m_wndTreeCtrl;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnSize(UINT nType, int cx, int cy);
};
