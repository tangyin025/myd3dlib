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

	CString m_strPathName;

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnScriptExecute();
	afx_msg void OnScriptOpen();
	afx_msg void OnScriptNew();
	afx_msg void OnScriptSave();
	afx_msg void OnScriptSaveAs();
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

