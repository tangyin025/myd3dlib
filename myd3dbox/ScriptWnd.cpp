#include "stdafx.h"
#include "ScriptWnd.h"
#include "MainApp.h"

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
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	////ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
	ON_WM_WINDOWPOSCHANGING()
	ON_COMMAND(ID_EDIT_CUT, &CScriptEdit::OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, &CScriptEdit::OnEditPaste)
	ON_COMMAND(ID_SCRIPT_EXECUTE, &CScriptEdit::OnScriptExecute)
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

void CScriptEdit::OnEditCopy()
{
	Copy();
}

void CScriptEdit::OnEditClear()
{
	Clear();
}

void CScriptEdit::OnEditCut()
{
	// TODO: Add your command handler code here
	Cut();
}


void CScriptEdit::OnEditPaste()
{
	// TODO: Add your command handler code here
	Paste();
}

void CScriptEdit::OnScriptExecute()
{
	// TODO: Add your command handler code here
	CString ret;
	GetWindowText(ret);
	if (!ret.IsEmpty())
	{
		theApp.ExecuteCode(tstou8((LPCTSTR)ret).c_str());
	}
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
