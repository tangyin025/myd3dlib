
// MainDoc.cpp : implementation of the CMainDoc class
//

#include "stdafx.h"
#include "MainApp.h"
#include "MainFrm.h"
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

CMainFrame * CMainDoc::GetMainFrame(void)
{
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		ASSERT(::IsWindow(pView->m_hWnd));
		if (pView->IsWindowVisible())   // Do not count invisible windows.
		{
			CFrameWnd* pFrame = pView->GetParentFrame();
			if (pFrame)
			{
				return DYNAMIC_DOWNCAST(CMainFrame, pFrame);
			}
		}
	}
	return NULL;
}

BOOL CMainDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	m_Actors.clear();

	ActorPtr actor(new Actor(my::AABB(-50,50), 1.0f));
	m_Actors.push_back(actor);
	MeshComponentPtr cmp = actor->CreateComponent<MeshComponent>(my::AABB(-FLT_MAX,FLT_MAX));
	theApp.MeshComponentLoadMeshFromFile(cmp, "mesh/casual19_m_highpoly.mesh.xml");
	cmp->m_World = my::Matrix4::Scaling(0.05f,0.05f,0.05f);
	my::OgreMeshSetPtr mesh_set = theApp.LoadMeshSet("mesh/scene.mesh.xml");
	if (mesh_set)
	{
		my::OgreMeshSet::iterator mesh_iter = mesh_set->begin();
		for (; mesh_iter != mesh_set->end(); mesh_iter++)
		{
			cmp = actor->CreateComponent<MeshComponent>((*mesh_iter)->m_aabb);
			theApp.MeshComponentLoadMesh(cmp, *mesh_iter, false);
		}
	}

	CMainFrame * pFrame = GetMainFrame();
	ASSERT_VALID(pFrame);
	pFrame->m_wndOutliner.InsertActor(actor.get());

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
