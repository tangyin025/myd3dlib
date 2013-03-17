#include "StdAfx.h"
#include "MainDoc.h"
#include "MainFrm.h"
#include "MainView.h"
#include "resource.h"

CMainDoc::SingleInstance * my::SingleInstance<CMainDoc>::s_ptr(NULL);

IMPLEMENT_DYNCREATE(CMainDoc, CDocument)

BEGIN_MESSAGE_MAP(CMainDoc, CDocument)
	ON_COMMAND(ID_CREATE_STATICMESH, &CMainDoc::OnCreateStaticmesh)
END_MESSAGE_MAP()

CMainDoc::CMainDoc(void)
{
}

BOOL CMainDoc::OnNewDocument()
{
	// TODO: Add your specialized code here and/or call the base class
	const my::Vector3 boxHalfExtents(10.0f, 10.0f, 10.0f);
	m_groundShape.reset(new btBoxShape(btVector3(boxHalfExtents.x, boxHalfExtents.y, boxHalfExtents.z)));

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(0, -boxHalfExtents.y, 0));
	m_groundMotionState.reset(new btDefaultMotionState(transform));

	btVector3 localInertia(0, 0, 0);
	m_groundBody.reset(new btRigidBody(
		btRigidBody::btRigidBodyConstructionInfo(0.0f, m_groundMotionState.get(), m_groundShape.get(), localInertia)));
	m_groundBody->setRestitution(1.0f);

	CMainView::getSingleton().m_dynamicsWorld->addRigidBody(m_groundBody.get());

	return CDocument::OnNewDocument();
}

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

void CMainDoc::OnCreateStaticmesh()
{
	CFileDialog dlg(TRUE);
	if(IDOK == dlg.DoModal())
	{
		try
		{
			my::OgreMeshPtr mesh = CMainFrame::getSingleton().LoadMesh(ws2ms(dlg.GetPathName()));
		}
		catch (const my::Exception & e)
		{
			AfxMessageBox(str_printf(_T("Cannot open: %s\n%s"),
				dlg.GetFileName(), ms2ws(e.GetFullDescription().c_str()).c_str()).c_str());
		}
	}
}
