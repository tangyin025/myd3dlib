
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

// Attributes
//protected:
	COutputEdit m_wndOutputDebug;

protected:
	//void AdjustHorzScroll(CListBox& wndListBox);

	void OnEventLog(const char * str);

// Implementation
public:
	virtual ~COutputWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
};

