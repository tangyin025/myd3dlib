#pragma once

#include "HistoryMgr.h"

class CMainDoc
	: public CDocument
	, public CHistoryMgr
	, public my::SingleInstance<CMainDoc>
{
public:
	CMainDoc(void);

	DECLARE_DYNCREATE(CMainDoc)

	void Clear(void);

	virtual void Serialize(CArchive& ar);

	virtual BOOL OnNewDocument();

	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);

	virtual void OnCloseDocument();

	DECLARE_MESSAGE_MAP()

	afx_msg void OnEditUndo();

	afx_msg void OnUpdateEditUndo(CCmdUI *pCmdUI);

	afx_msg void OnEditRedo();

	afx_msg void OnUpdateEditRedo(CCmdUI *pCmdUI);

	afx_msg void OnCreateMeshFromFile();
};
