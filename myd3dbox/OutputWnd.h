#pragma once

class COutputWnd
	: public CDockablePane
	, public my::SingleInstance<COutputWnd>
{
public:
	COutputWnd(void)
	{
	}

	virtual ~COutputWnd(void)
	{
	}

	DECLARE_MESSAGE_MAP()
public:
	CRichEditCtrl m_EditCtrl;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	virtual void AdjustLayout(void);

	afx_msg void OnSize(UINT nType, int cx, int cy);
};
