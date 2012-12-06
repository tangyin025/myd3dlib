#include "stdafx.h"
#include "ImgRegionDoc.h"
#include "MainFrm.h"
//
//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

IMPLEMENT_DYNCREATE(CImgRegionDoc, CDocument)

BEGIN_MESSAGE_MAP(CImgRegionDoc, CDocument)
END_MESSAGE_MAP()

CImgRegionDoc::CImgRegionDoc(void)
	: m_ImageSize(500,500)
	, m_BkColor(255,255,255,255)
{
}

BOOL CImgRegionDoc::LocalToRoot(HTREEITEM hItem, const CPoint & ptLocal, CPoint & ptResult)
{
	if(hItem)
	{
		CImgRegion * pReg = (CImgRegion *)m_TreeCtrl.GetItemData(hItem);
		ASSERT(pReg);

		if(hItem == m_TreeCtrl.GetRootItem())
		{
			ptResult = pReg->m_rc.TopLeft() + ptLocal;
			return TRUE;
		}

		return LocalToRoot(m_TreeCtrl.GetParentItem(hItem), pReg->m_rc.TopLeft() + ptLocal, ptResult);
	}

	return FALSE;
}

BOOL CImgRegionDoc::CreateTreeCtrl(void)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);

	if (!m_TreeCtrl.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS, CRect(), &pFrame->m_wndFileView, pFrame->m_wndFileView.m_TreeCtrlSet.size() + 4))
	{
		TRACE0("CImgRegionDoc::CreateTreeCtrl failed \n");
		return FALSE;
	}

	pFrame->m_wndFileView.m_TreeCtrlSet.insert(&m_TreeCtrl);

	pFrame->m_wndFileView.AdjustLayout();

	return TRUE;
}

void CImgRegionDoc::DestroyTreeCtrl(void)
{
	HTREEITEM hItem = m_TreeCtrl.GetRootItem();
	for(; NULL != hItem; hItem = m_TreeCtrl.GetNextSiblingItem(hItem))
	{
		DeleteItemTreeData(hItem);
	}

	m_TreeCtrl.DeleteAllItems();

	m_TreeCtrl.DestroyWindow();

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);

	pFrame->m_wndFileView.m_TreeCtrlSet.erase(&m_TreeCtrl);

	pFrame->m_wndFileView.AdjustLayout();
}

void CImgRegionDoc::DeleteItemTreeData(HTREEITEM hItem)
{
	CImgRegion * pReg = (CImgRegion *)m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);
	delete pReg;

	HTREEITEM hChild = m_TreeCtrl.GetChildItem(hItem);
	for(; NULL != hChild; hChild = m_TreeCtrl.GetNextSiblingItem(hItem))
	{
		DeleteItemTreeData(hChild);
	}
}

HTREEITEM CImgRegionDoc::GetPointedRegionNode(HTREEITEM hItem, const CPoint & ptLocal)
{
	// 这里的碰撞检测应当反过来检测，因为优先画在前面的
	if(hItem)
	{
		HTREEITEM hRet;
		if(hRet = GetPointedRegionNode(m_TreeCtrl.GetNextSiblingItem(hItem), ptLocal))
			return hRet;

		CImgRegion * pReg = (CImgRegion *)m_TreeCtrl.GetItemData(hItem);
		ASSERT(pReg);

		if(hRet = GetPointedRegionNode(m_TreeCtrl.GetChildItem(hItem), ptLocal - pReg->m_rc.TopLeft()))
			return hRet;

		if(pReg->m_rc.PtInRect(ptLocal))
			return hItem;
	}
	return NULL;
}

BOOL CImgRegionDoc::OnNewDocument(void)
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	if (!CreateTreeCtrl())
		return FALSE;

	HTREEITEM hItem = m_TreeCtrl.InsertItem(_T("aaa"));
	CImgRegion * pRegRoot = new CImgRegion(CRect(0,0,500,500), Gdiplus::Color(255,255,255,255));
	pRegRoot->m_image.reset(Gdiplus::Image::FromFile(L"Checker.bmp"));
	pRegRoot->m_border = Vector4i(100,50,100,50);
	Gdiplus::FontFamily fontFamily(L"Arial");
	pRegRoot->m_font.reset(new Gdiplus::Font(&fontFamily, 12, Gdiplus::FontStyleBold, Gdiplus::UnitPoint));
	m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pRegRoot);

	hItem = m_TreeCtrl.InsertItem(_T("bbb"), hItem);
	CImgRegion * pReg = new CImgRegion(CRect(100,100,200,200), Gdiplus::Color(192,255,0,0));
	pReg->m_font = pRegRoot->m_font;
	m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pReg);

	hItem = m_TreeCtrl.InsertItem(_T("ccc"), hItem);
	pReg = new CImgRegion(CRect(25,25,75,75), Gdiplus::Color(255,0,255,0));
	pReg->m_font = pRegRoot->m_font;
	pReg->m_image.reset(Gdiplus::Image::FromFile(L"com_btn_normal.png"));
	pReg->m_border = Vector4i(7,7,7,7);
	m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pReg);

	m_TreeCtrl.SelectItem(hItem);

	return TRUE;
}

void CImgRegionDoc::OnCloseDocument()
{
	DestroyTreeCtrl();

	CDocument::OnCloseDocument();
}
