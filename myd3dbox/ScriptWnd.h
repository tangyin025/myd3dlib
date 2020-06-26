#pragma once
#include "afxdockablepane.h"

/////////////////////////////////////////////////////////////////////////////
// CScriptEdit window

class CScriptEdit : public CRichEditCtrl
{
public:
	CScriptEdit();

	virtual ~CScriptEdit();

protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnScriptExecute();
};

class CScriptWnd :
	public CDockablePane
{
public:
	CScriptWnd();
	virtual ~CScriptWnd();
	CScriptEdit m_editScript;
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
};

