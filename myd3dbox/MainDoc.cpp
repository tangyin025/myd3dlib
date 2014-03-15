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
	ON_COMMAND(ID_CREATE_MESHFROMFILE, &CMainDoc::OnCreateMeshFromFile)
END_MESSAGE_MAP()

CMainDoc::CMainDoc(void)
{
}

void CMainDoc::Clear(void)
{
	CHistoryMgr::ClearAllHistory();

	m_guid = 0;

	COutlinerView::getSingleton().m_TreeCtrl.DeleteAllItems();
}

void CMainDoc::Serialize(CArchive& ar)
{
	if(ar.IsStoring())
	{
		ar << m_guid;
	}
	else
	{
		ar >> m_guid;
	}

	COutlinerView::getSingleton().Serialize(ar);
}

BOOL CMainDoc::OnNewDocument()
{
	Clear();

	return CDocument::OnNewDocument();
}

BOOL CMainDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	Clear();

	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	return TRUE;
}

BOOL CMainDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	return CDocument::OnSaveDocument(lpszPathName);
}

void CMainDoc::OnCloseDocument()
{
	Clear();

	CDocument::OnCloseDocument();
}

void CMainDoc::OnEditUndo()
{
	CHistoryMgr::Undo();

	SetModifiedFlag();

	UpdateAllViews(NULL);
}

void CMainDoc::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nStep >= 0);
}

void CMainDoc::OnEditRedo()
{
	CHistoryMgr::Do();

	SetModifiedFlag();

	UpdateAllViews(NULL);
}

void CMainDoc::OnUpdateEditRedo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nStep < (int)CHistoryMgr::size() - 1);
}

void CMainDoc::OnCreateMeshFromFile()
{
	CFileDialog dlg(TRUE);
	if(IDOK == dlg.DoModal())
	{
		try
		{
			AddMeshTreeNode(dlg.GetPathName());

			SetModifiedFlag();

			UpdateAllViews(NULL);
		}
		catch (const my::Exception & e)
		{
			AfxMessageBox(str_printf(_T("Cannot open: %s\n%s"), dlg.GetFileName(), e.what().c_str()).c_str());
		}
	}
}
