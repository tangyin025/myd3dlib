#include "stdafx.h"
#include "ScriptWnd.h"
#include "resource.h"
#include "MainFrm.h"

/////////////////////////////////////////////////////////////////////////////
// CScriptEdit

CScriptEdit::CScriptEdit()
{
}

CScriptEdit::~CScriptEdit()
{
}

BEGIN_MESSAGE_MAP(CScriptEdit, CRichEditCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_WINDOWPOSCHANGING()
	ON_COMMAND(ID_SCRIPT_EXECUTE, &CScriptEdit::OnScriptExecute)
	ON_COMMAND(ID_SCRIPT_OPEN, &CScriptEdit::OnScriptOpen)
	ON_COMMAND(ID_SCRIPT_NEW, &CScriptEdit::OnScriptNew)
	ON_COMMAND(ID_SCRIPT_SAVE, &CScriptEdit::OnScriptSave)
	ON_COMMAND(ID_SCRIPT_SAVE_AS, &CScriptEdit::OnScriptSaveAs)
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// COutputList message handlers

void CScriptEdit::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_POPUP_SCRIPT);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}

	SetFocus();
}

BOOL CScriptEdit::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_ESCAPE:
			// ! rich edit VK_ESCAPE will destroy window
			return TRUE;

		case VK_RETURN:
			if (::GetKeyState(VK_CONTROL) < 0)
			{
				OnCmdMsg(ID_SCRIPT_EXECUTE, 0, NULL, NULL);
				return TRUE;
			}
			break;
		}
	}

	return CRichEditCtrl::PreTranslateMessage(pMsg);
}

void CScriptEdit::OnScriptExecute()
{
	// TODO: Add your command handler code here
	CString ret;
	GetWindowText(ret);
	if (!ret.IsEmpty())
	{
		CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
		ASSERT_VALID(pFrame);
		pFrame->ExecuteCode(tstou8((LPCTSTR)ret).c_str());
	}
}

void CScriptEdit::OnScriptNew()
{
	// TODO: Add your command handler code here
	SetSel(0, -1);
	Clear();
	EmptyUndoBuffer();
	m_strPathName.Empty();
}

static DWORD CALLBACK _ScriptStreamInCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	CFile * pFile = (CFile *)dwCookie;
	*pcb = pFile->Read(pbBuff, cb);
	return 0;
}

void CScriptEdit::OnScriptOpen()
{
	// TODO: Add your command handler code here
	CString strPathName;
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, 0);
	dlg.m_ofn.lpstrFile = strPathName.GetBuffer(_MAX_PATH);
	INT_PTR nResult = dlg.DoModal();
	strPathName.ReleaseBuffer();
	if (nResult != IDOK)
	{
		return;
	}

	CFile cFile(strPathName, CFile::modeRead);
	EDITSTREAM es;
	es.dwCookie = (DWORD)&cFile;
	es.pfnCallback = _ScriptStreamInCallback;
	StreamIn(SF_TEXT, es);

	m_strPathName = strPathName;
}

static DWORD CALLBACK _ScriptStreamOutCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	CFile* pFile = (CFile*)dwCookie;
	pFile->Write(pbBuff, cb);
	*pcb = cb;
	return 0;
}

void CScriptEdit::OnScriptSave()
{
	// TODO: Add your command handler code here
	if (m_strPathName.IsEmpty())
	{
		OnScriptSaveAs();
		return;
	}

	CFile cFile(m_strPathName, CFile::modeCreate | CFile::modeWrite);
	EDITSTREAM es;
	es.dwCookie = (DWORD)&cFile;
	es.pfnCallback = _ScriptStreamOutCallback;
	StreamOut(SF_TEXT, es);
}

void CScriptEdit::OnScriptSaveAs()
{
	// TODO: Add your command handler code here
	CString strPathName;
	CFileDialog dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, 0);
	dlg.m_ofn.lpstrFile = strPathName.GetBuffer(_MAX_PATH);
	INT_PTR nResult = dlg.DoModal();
	strPathName.ReleaseBuffer();
	if (nResult != IDOK)
	{
		return;
	}

	CFile cFile(strPathName, CFile::modeCreate | CFile::modeWrite);
	EDITSTREAM es;
	es.dwCookie = (DWORD)&cFile;
	es.pfnCallback = _ScriptStreamOutCallback;
	StreamOut(SF_TEXT, es);

	m_strPathName = strPathName;
}

CScriptWnd::CScriptWnd()
{
}


CScriptWnd::~CScriptWnd()
{
}

BEGIN_MESSAGE_MAP(CScriptWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


int CScriptWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	if (!m_editScript.Create(LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE,
		CRect(0, 0, 100, 100), this, 2))
	{
		TRACE0("Failed to create output windows\n");
		return -1;
	}

	m_editScript.SetFont(&afxGlobalData.fontRegular);

	m_editScript.SetOptions(ECOOP_XOR, ECO_WANTRETURN);

	return 0;
}


void CScriptWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	m_editScript.SetWindowPos(NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}


void CScriptWnd::OnDestroy()
{
	CDockablePane::OnDestroy();

	// TODO: Add your message handler code here
}
