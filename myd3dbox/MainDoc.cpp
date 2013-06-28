#include "StdAfx.h"
#include "MainDoc.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "MainView.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(CMainDoc, CDocument)

BEGIN_MESSAGE_MAP(CMainDoc, CDocument)
	ON_COMMAND(ID_EDIT_UNDO, &CMainDoc::OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, &CMainDoc::OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, &CMainDoc::OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, &CMainDoc::OnUpdateEditRedo)
	ON_COMMAND(ID_CREATE_STATICMESH, &CMainDoc::OnCreateStaticmesh)
END_MESSAGE_MAP()

CMainDoc::CMainDoc(void)
{
}

void CMainDoc::Clear(void)
{
	CDocHistoryMgr::ClearAllHistory();

	COutlinerView::getSingleton().m_TreeCtrl.DeleteAllItems();
}

BOOL CMainDoc::OnNewDocument()
{
	// TODO: Add your specialized code here and/or call the base class
	Clear();

	return CDocument::OnNewDocument();
}

BOOL CMainDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO:  Add your specialized creation code here
	Clear();

	return TRUE;
}

BOOL CMainDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	// TODO: Add your specialized code here and/or call the base class
	Clear();

	return CDocument::OnSaveDocument(lpszPathName);
}

void CMainDoc::OnCloseDocument()
{
	// TODO: Add your specialized code here and/or call the base class
	Clear();

	CDocument::OnCloseDocument();
}

void CMainDoc::OnEditUndo()
{
	CDocHistoryMgr::Undo();

	SetModifiedFlag();

	UpdateAllViews(NULL);
}

void CMainDoc::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nStep >= 0);
}

void CMainDoc::OnEditRedo()
{
	CDocHistoryMgr::Do();

	SetModifiedFlag();

	UpdateAllViews(NULL);
}

void CMainDoc::OnUpdateEditRedo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nStep < (int)CDocHistoryMgr::size() - 1);
}

void CMainDoc::OnCreateStaticmesh()
{
	CFileDialog dlg(TRUE);
	if(IDOK == dlg.DoModal())
	{
		try
		{
			my::OgreMeshPtr mesh = theApp.LoadMesh(ws2ms(dlg.GetPathName()));

			static unsigned int i = 0;
			CString strItem;
			strItem.Format(_T("mesh_%03d"), i++);
			AddStaticMeshTreeNode(strItem, mesh);

			SetModifiedFlag();

			UpdateAllViews(NULL);
		}
		catch (const my::Exception & e)
		{
			AfxMessageBox(str_printf(_T("Cannot open: %s\n%s"),
				dlg.GetFileName(), ms2ws(e.GetFullDescription().c_str()).c_str()).c_str());
		}
	}
}
