#pragma once

class CMainDoc
	: public CDocument
	, public my::SingleInstance<CMainDoc>
{
public:
	CMainDoc(void);

	DECLARE_DYNCREATE(CMainDoc)

	typedef std::vector<my::OgreMeshPtr> MeshPtrList;

	MeshPtrList m_StaticMeshes;

	virtual BOOL OnNewDocument();

	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);

	virtual void OnCloseDocument();
};
