
// MainDoc.cpp : implementation of the CMainDoc class
//

#include "stdafx.h"
#include "MainApp.h"

#include "MainDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainDoc

IMPLEMENT_DYNCREATE(CMainDoc, CDocument)

BEGIN_MESSAGE_MAP(CMainDoc, CDocument)
END_MESSAGE_MAP()


// CMainDoc construction/destruction

CMainDoc::CMainDoc()
{
	// TODO: add one-time construction code here

}

CMainDoc::~CMainDoc()
{
}

BOOL CMainDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	m_Actors.clear();

	ActorPtr actor(new Actor(my::AABB(-FLT_MAX,FLT_MAX)));
	m_Actors.push_back(actor);
	MeshComponentPtr cmp = actor->CreateComponent<MeshComponent>();
	theApp.MeshComponentLoadMeshFromFile(cmp, "mesh/casual19_m_highpoly.mesh.xml");
	cmp->m_World = my::Matrix4::Scaling(0.1f,0.1f,0.1f);
	cmp = actor->CreateComponent<MeshComponent>();
	theApp.MeshComponentLoadMeshFromFile(cmp, "mesh/plane.mesh.xml");

	return TRUE;
}




// CMainDoc serialization

void CMainDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CMainDoc diagnostics

#ifdef _DEBUG
void CMainDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMainDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CMainDoc commands

BOOL CMainDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO:  Add your specialized creation code here

	return TRUE;
}

BOOL CMainDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	// TODO: Add your specialized code here and/or call the base class

	return CDocument::OnSaveDocument(lpszPathName);
}

void CMainDoc::OnCloseDocument()
{
	// TODO: Add your specialized code here and/or call the base class

	CDocument::OnCloseDocument();
}
