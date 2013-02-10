#pragma once

class CMainDoc
	: public CDocument
	, public my::SingleInstance<CMainDoc>
{
public:
	CMainDoc(void);

	DECLARE_DYNCREATE(CMainDoc)

	virtual BOOL OnNewDocument();

	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);

	virtual void OnCloseDocument();
};
