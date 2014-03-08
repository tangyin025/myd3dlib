#pragma once

#include "DocHistoryMgr.h"

class CMainDoc
	: public CDocument
	, public CDocHistoryMgr
	, public my::SingleInstance<CMainDoc>
{
public:
	CMainDoc(void);

	DECLARE_DYNCREATE(CMainDoc)

	void Clear(void);

	virtual BOOL OnNewDocument();

	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);

	virtual void OnCloseDocument();

	DECLARE_MESSAGE_MAP()

	afx_msg void OnEditUndo();

	afx_msg void OnUpdateEditUndo(CCmdUI *pCmdUI);

	afx_msg void OnEditRedo();

	afx_msg void OnUpdateEditRedo(CCmdUI *pCmdUI);

	afx_msg void OnCreateMesh();
};
