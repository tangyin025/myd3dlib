#include "stdafx.h"
#include "ImgRegionDoc.h"
#include "MainFrm.h"
#include "resource.h"
//
//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

IMPLEMENT_DYNAMIC(CImgRegionTreeCtrl, CTreeCtrl)

CImgRegionTreeCtrl::CImgRegionTreeCtrl(void)
	: m_bDrag(FALSE)
	, m_hDragTagParent(NULL)
	, m_hDragTagFront(NULL)
{
}

BEGIN_MESSAGE_MAP(CImgRegionTreeCtrl, CTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, &CImgRegionTreeCtrl::OnTvnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

void CImgRegionTreeCtrl::OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	TVHITTESTINFO info;
	info.pt = pNMTreeView->ptDrag;
	HitTest(&info);
	if((info.flags & TVHT_ONITEM) && info.hItem)
	{
		SetCapture();
		m_bDrag = TRUE;
		m_hDragItem = info.hItem;
	}
	*pResult = 0;
}

void CImgRegionTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_bDrag)
	{
		TVHITTESTINFO info;
		info.pt = point;
		HitTest(&info);
		if((info.flags & TVHT_ONITEM) && info.hItem)
		{
			CRect rectItem;
			GetItemRect(info.hItem, &rectItem, FALSE);

			int nBorder = rectItem.Height() / 3;
			if(point.y < rectItem.top + nBorder)
			{
				SetInsertMark(info.hItem, FALSE);
				SelectDropTarget(info.hItem);
				m_hDragTagParent = GetSafeParentItem(info.hItem);
				m_hDragTagFront = GetSafePreSiblingItem(info.hItem);
			}
			else if(point.y >= rectItem.bottom - nBorder)
			{
				SetInsertMark(info.hItem, TRUE);
				SelectDropTarget(info.hItem);
				m_hDragTagParent = GetSafeParentItem(info.hItem);
				m_hDragTagFront = info.hItem;
			}
			else
			{
				SetInsertMark(NULL);
				SelectDropTarget(info.hItem);
				m_hDragTagParent = info.hItem;
				m_hDragTagFront = TVI_LAST;
			}
		}
		else
		{
			SetInsertMark(NULL);
			SelectDropTarget(NULL);
			m_hDragTagParent = TVI_ROOT;
			m_hDragTagFront = TVI_LAST;
		}
	}
	CTreeCtrl::OnMouseMove(nFlags, point);
}

void CImgRegionTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(m_bDrag)
	{
		ReleaseCapture();
		m_bDrag = FALSE;
		SetInsertMark(NULL);
		SelectDropTarget(NULL);

		NMTREEVIEWDRAG dragInfo;
		dragInfo.hdr.hwndFrom = GetSafeHwnd();
		dragInfo.hdr.idFrom = GetDlgCtrlID();
		dragInfo.hdr.code = TVN_DRAGCHANGED;
		dragInfo.hDragItem = m_hDragItem;
		dragInfo.hDragTagParent = m_hDragTagParent;
		dragInfo.hDragTagFront = m_hDragTagFront;
		GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&dragInfo);
	}
	CTreeCtrl::OnLButtonUp(nFlags, point);
}

BOOL CImgRegionTreeCtrl::FindTreeChildItem(HTREEITEM hParent, HTREEITEM hChild)
{
	if(hParent == hChild)
		return TRUE;

	HTREEITEM hItem = GetChildItem(hParent);
	for(; hItem; hItem = GetNextSiblingItem(hItem))
	{
		if(FindTreeChildItem(hItem, hChild))
			return TRUE;
	}

	return FALSE;
}

HTREEITEM CImgRegionTreeCtrl::MoveTreeItem(HTREEITEM hParent, HTREEITEM hInsertAfter, HTREEITEM hOtherItem)
{
	if(hParent == hOtherItem || hInsertAfter == hOtherItem || FindTreeChildItem(hOtherItem, hParent))
	{
		return hOtherItem;
	}

	HTREEITEM hItem = InsertItem(GetItemText(hOtherItem), 0, 0, hParent, hInsertAfter);
	SetItemData(hItem, GetItemData(hOtherItem));

	HTREEITEM hNextOtherChild = NULL;
	HTREEITEM hChild = TVI_LAST;
	for(HTREEITEM hOtherChild = GetChildItem(hOtherItem); hOtherChild; hOtherChild = hNextOtherChild)
	{
		HTREEITEM hNextOtherChild = GetNextSiblingItem(hOtherChild);

		hChild = MoveTreeItem(hItem, hChild, hOtherChild);
	}

	DeleteItem(hOtherItem);
	return hItem;
}

HTREEITEM CImgRegionTreeCtrl::GetSafeParentItem(HTREEITEM hItem)
{
	HTREEITEM hParent = GetParentItem(hItem);
	if(!hParent)
		return TVI_ROOT;

	return hParent;
}

HTREEITEM CImgRegionTreeCtrl::GetSafePreSiblingItem(HTREEITEM hItem)
{
	HTREEITEM hSibling = GetPrevSiblingItem(hItem);
	if(!hSibling)
		return TVI_FIRST;

	return hSibling;
}

IMPLEMENT_DYNCREATE(CImgRegionDoc, CDocument)

BEGIN_MESSAGE_MAP(CImgRegionDoc, CDocument)
	ON_COMMAND(ID_ADD_REGION, &CImgRegionDoc::OnAddRegion)
	ON_UPDATE_COMMAND_UI(ID_ADD_REGION, &CImgRegionDoc::OnUpdateAddRegion)
	ON_COMMAND(ID_DEL_REGION, &CImgRegionDoc::OnDelRegion)
	ON_UPDATE_COMMAND_UI(ID_DEL_REGION, &CImgRegionDoc::OnUpdateDelRegion)
END_MESSAGE_MAP()

CImgRegionDoc::CImgRegionDoc(void)
	: CImgRegion(CPoint(0,0), CSize(500,500), Gdiplus::Color::White)
{
	m_Font = GetFont(L"Arial", 12);
}

CPoint CImgRegionDoc::LocalToRoot(HTREEITEM hItem, const CPoint & ptLocal)
{
	if(NULL == hItem)
		return ptLocal;

	CImgRegion * pReg = (CImgRegion *)m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	return LocalToRoot(m_TreeCtrl.GetParentItem(hItem), ptLocal + pReg->m_Local);
}

CPoint CImgRegionDoc::RootToLocal(HTREEITEM hItem, const CPoint & ptRoot)
{
	if(NULL == hItem || TVI_ROOT == hItem)
		return ptRoot;

	CImgRegion * pReg = (CImgRegion *)m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	return RootToLocal(m_TreeCtrl.GetParentItem(hItem), ptRoot - pReg->m_Local);
}

BOOL CImgRegionDoc::CreateTreeCtrl(void)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);

	if (!m_TreeCtrl.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS, CRect(), &pFrame->m_wndFileView, pFrame->m_wndFileView.m_TreeCtrlSet.size() + 4))
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
	HTREEITEM hNextItem = NULL;
	for(HTREEITEM hItem = m_TreeCtrl.GetRootItem(); NULL != hItem; hItem = hNextItem)
	{
		hNextItem = m_TreeCtrl.GetNextSiblingItem(hItem);

		m_TreeCtrl.DeleteTreeItem<CImgRegion>(hItem, TRUE);
	}

	m_TreeCtrl.DestroyWindow();

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);

	pFrame->m_wndFileView.m_TreeCtrlSet.erase(&m_TreeCtrl);

	pFrame->m_wndFileView.AdjustLayout();
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

		if(hRet = GetPointedRegionNode(m_TreeCtrl.GetChildItem(hItem), ptLocal - pReg->m_Local))
			return hRet;

		if(CRect(pReg->m_Local, pReg->m_Size).PtInRect(ptLocal))
			return hItem;
	}
	return NULL;
}

BOOL CImgRegionDoc::OnNewDocument(void)
{
	GetCurrentDirectory(MAX_PATH, m_CurrentDir.GetBufferSetLength(MAX_PATH));
	m_CurrentDir.ReleaseBuffer();

	if (!CDocument::OnNewDocument())
		return FALSE;

	if (!CreateTreeCtrl())
		return FALSE;

	//HTREEITEM hItem = m_TreeCtrl.InsertItem(_T("aaa"));
	//CImgRegion * pRegRoot = new CImgRegion(CPoint(0,0), CSize(500,500), Gdiplus::Color(255,255,255,255));
	//pRegRoot->m_ImageStr = L"Checker.bmp";
	//pRegRoot->m_Image = GetImage(GetFullPath(pRegRoot->m_ImageStr));
	//pRegRoot->m_Border = Vector4i(100,50,100,50);
	//pRegRoot->m_Font = m_Font;
	//m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pRegRoot);

	//hItem = m_TreeCtrl.InsertItem(_T("bbb"), hItem);
	//CImgRegion * pReg = new CImgRegion(CPoint(100,100), CSize(200,200), Gdiplus::Color(192,255,0,0));
	//pReg->m_Font = m_Font;
	//m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pReg);

	//hItem = m_TreeCtrl.InsertItem(_T("ccc"), hItem);
	//pReg = new CImgRegion(CPoint(25,25), CSize(75,75), Gdiplus::Color(255,255,255,255));
	//pReg->m_Font = m_Font;
	//pReg->m_ImageStr = L"com_btn_normal.png";
	//pReg->m_Image = GetImage(GetFullPath(pReg->m_ImageStr));
	//pReg->m_Border = Vector4i(7,7,7,7);
	//m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pReg);

	//m_TreeCtrl.SelectItem(hItem);

	return TRUE;
}

BOOL CImgRegionDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CImgRegionDoc::CreateTreeCtrl())
		return FALSE;

	m_CurrentDir = lpszPathName;
	PathRemoveFileSpec(m_CurrentDir.GetBuffer());
	m_CurrentDir.ReleaseBuffer();

	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	return TRUE;
}

BOOL CImgRegionDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	if(!CDocument::OnSaveDocument(lpszPathName))
		return FALSE;

	m_CurrentDir = lpszPathName;
	PathRemoveFileSpec(m_CurrentDir.GetBuffer());
	m_CurrentDir.ReleaseBuffer();

	return TRUE;
}

void CImgRegionDoc::OnCloseDocument()
{
	DestroyTreeCtrl();

	CDocument::OnCloseDocument();
}

void CImgRegionDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{	// storing code
		ar << m_Size;
		DWORD argb = m_Color.GetValue(); ar << argb;
		ar << m_Border.x << m_Border.y << m_Border.z << m_Border.w;

		SerializeRegionNode(ar, TVI_ROOT);
	}
	else
	{	// loading code
		ar >> m_Size;
		DWORD argb; ar >> argb; m_Color.SetValue(argb);
		ar >> m_Border.x >> m_Border.y >> m_Border.z >> m_Border.w;

		SerializeRegionNode(ar, TVI_ROOT);
	}
}

int CImgRegionDoc::GetChildCount(HTREEITEM hItem)
{
	int nChilds = 0;
	for(HTREEITEM hChild = m_TreeCtrl.GetChildItem(hItem);
		hChild; hChild = m_TreeCtrl.GetNextSiblingItem(hChild))
		nChilds++;

	return nChilds;
}

void CImgRegionDoc::SerializeRegionNode(CArchive & ar, HTREEITEM hParent)
{
	if (ar.IsStoring())
	{
		int nChilds = GetChildCount(hParent); ar << nChilds;

		HTREEITEM hItem = m_TreeCtrl.GetChildItem(hParent);
		for(int i = 0; i < nChilds; i++, hItem = m_TreeCtrl.GetNextSiblingItem(hItem))
		{
			ASSERT(hItem);
			ar << m_TreeCtrl.GetItemText(hItem);

			CImgRegion * pReg = (CImgRegion *)m_TreeCtrl.GetItemData(hItem);
			ASSERT(pReg);

			ar << pReg->m_Local;
			ar << pReg->m_Size;
			DWORD argb = pReg->m_Color.GetValue(); ar << argb;
			ar << pReg->m_ImageStr;
			ar << pReg->m_Border.x << pReg->m_Border.y << pReg->m_Border.z << pReg->m_Border.w;
			Gdiplus::FontFamily family; pReg->m_Font->GetFamily(&family); CString strFamily; family.GetFamilyName(strFamily.GetBufferSetLength(LF_FACESIZE)); strFamily.ReleaseBuffer(); ar << strFamily << pReg->m_Font->GetSize();
			argb = pReg->m_FontColor.GetValue(); ar << argb;
			ar << pReg->m_Text;

			SerializeRegionNode(ar, hItem);
		}
	}
	else
	{
		int nChilds; ar >> nChilds;

		for(int i = 0; i < nChilds; i++)
		{
			CString strName;
			ar >> strName; HTREEITEM hItem = m_TreeCtrl.InsertItem(strName, hParent, TVI_LAST); ASSERT(hItem);

			CImgRegion * pReg = new CImgRegion(CPoint(10,10), CSize(100,100), Gdiplus::Color::White);
			ASSERT(pReg);

			ar >> pReg->m_Local;
			ar >> pReg->m_Size;
			DWORD argb; ar >> argb; pReg->m_Color.SetValue(argb);
			ar >> pReg->m_ImageStr; pReg->m_Image = GetImage(GetFullPath(pReg->m_ImageStr));
			ar >> pReg->m_Border.x >> pReg->m_Border.y >> pReg->m_Border.z >> pReg->m_Border.w;
			CString strFamily; float fSize; ar >> strFamily >> fSize; pReg->m_Font = GetFont(strFamily, fSize);
			ar >> argb; pReg->m_FontColor.SetValue(argb);
			ar >> pReg->m_Text;

			m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pReg);

			SerializeRegionNode(ar, hItem);
		}
	}
}

ImagePtr CImgRegionDoc::GetImage(CString strImg)
{
	if(!strImg.IsEmpty())
	{
		ASSERT(!PathIsRelative(strImg));

		std::wstring key(strImg);
		ImagePtrMap::iterator img_iter = m_ImageMap.find(key);
		if(img_iter == m_ImageMap.end())
		{
			m_ImageMap[key] = ImagePtr(new Gdiplus::Image(strImg));
		}

		return m_ImageMap[key];
	}

	return ImagePtr();
}

FontPtr2 CImgRegionDoc::GetFont(const CString & strFamily, float fSize)
{
	if(!strFamily.IsEmpty())
	{
		CString strFont;
		strFont.Format(_T("%s, %f"), strFamily, fSize);

		std::wstring key(strFont);
		FontPtr2Map::iterator fnt_iter = m_FontMap.find(key);
		if(fnt_iter == m_FontMap.end())
		{
			m_FontMap[key] = FontPtr2(new Gdiplus::Font(strFamily, fSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint));
		}

		return m_FontMap[key];
	}

	return FontPtr2();
}

const CString & CImgRegionDoc::GetCurrentDir(void) const
{
	ASSERT(!m_CurrentDir.IsEmpty());

	return m_CurrentDir;
}

CString CImgRegionDoc::GetRelativePath(const CString & strPath) const
{
	if(PathIsRelative(strPath))
		return strPath;

	CString res;
	PathRelativePathTo(res.GetBufferSetLength(MAX_PATH), GetCurrentDir(), FILE_ATTRIBUTE_DIRECTORY, strPath, FILE_ATTRIBUTE_NORMAL);
	res.ReleaseBuffer();

	return res;
}

CString CImgRegionDoc::GetFullPath(const CString & strPath) const
{
	if(!PathIsRelative(strPath))
		return strPath;

	CString res;
	if(!strPath.IsEmpty())
		PathCombine(res.GetBufferSetLength(MAX_PATH), GetCurrentDir(), strPath);

	return res;
}

void CImgRegionDoc::OnAddRegion()
{
	HTREEITEM hParent = NULL;
	CPoint ptOrg(10,10);
	HTREEITEM hSelected = m_TreeCtrl.GetSelectedItem();
	if(hSelected)
	{
		hParent = m_TreeCtrl.GetParentItem(hSelected);
		CImgRegion * pReg = (CImgRegion *)m_TreeCtrl.GetItemData(hSelected);
		ptOrg += pReg->m_Local;
	}
	if(!hParent)
	{
		hParent = TVI_ROOT;
	}

	HTREEITEM hItem = m_TreeCtrl.InsertItem(_T("aaa"), hParent, TVI_LAST);
	CImgRegion * pReg = new CImgRegion(ptOrg, CSize(100,100),
		Gdiplus::Color(255,my::Random<int>(0,255),my::Random<int>(0,255),my::Random<int>(0,255)));
	pReg->m_Font = m_Font;
	m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pReg);
	m_TreeCtrl.SelectItem(hItem);
}

void CImgRegionDoc::OnUpdateAddRegion(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CImgRegionDoc::OnDelRegion()
{
	HTREEITEM hSelected = m_TreeCtrl.GetSelectedItem();
	if(hSelected)
	{
		m_TreeCtrl.DeleteTreeItem<CImgRegion>(hSelected, TRUE);
	}

	// ! 如果没有selected item，则最后一次的 DeleteTreeItem，不会触发 TVN_SELCHANGED
	if(!m_TreeCtrl.GetSelectedItem())
		UpdateAllViews(NULL);
}

void CImgRegionDoc::OnUpdateDelRegion(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(NULL != m_TreeCtrl.GetSelectedItem());
}
