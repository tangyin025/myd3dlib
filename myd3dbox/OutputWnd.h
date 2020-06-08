
#pragma once

/////////////////////////////////////////////////////////////////////////////
// COutputEdit window

class COutputEdit : public CRichEditCtrl
{
public:
	COutputEdit();

	virtual ~COutputEdit();

protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();

	DECLARE_MESSAGE_MAP()
};

class COutputWnd : public CDockablePane
{
// Construction
public:
	COutputWnd();

	void UpdateFonts();

// Attributes
protected:
	CMFCTabCtrl	m_wndTabs;

	COutputEdit m_wndOutputDebug;

protected:
	//void AdjustHorzScroll(CListBox& wndListBox);

// Implementation
public:
	virtual ~COutputWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

