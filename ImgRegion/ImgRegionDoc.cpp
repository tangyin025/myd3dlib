#include "stdafx.h"
#include "ImgRegionDoc.h"
#include "MainFrm.h"
#include "resource.h"
#include "ImgRegionFilePropertyDlg.h"
#include "ImgRegionView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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
	ON_COMMAND(ID_EXPORT_IMG, &CImgRegionDoc::OnExportImg)
	ON_COMMAND(ID_FILE_PROPERTY, &CImgRegionDoc::OnFileProperty)
	ON_COMMAND(ID_EDIT_COPY, &CImgRegionDoc::OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, &CImgRegionDoc::OnEditPaste)
END_MESSAGE_MAP()

CImgRegionDoc::CImgRegionDoc(void)
	: CImgRegion(CPoint(0,0), CSize(500,500), Gdiplus::Color::White)
{
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

	static DWORD dwCtrlID = 4;

	if (!m_TreeCtrl.CreateEx(WS_EX_CLIENTEDGE,
		WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS, CRect(), &pFrame->m_wndFileView, dwCtrlID++))
	{
		TRACE0("CImgRegionDoc::CreateTreeCtrl failed \n");
		return FALSE;
	}

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
	CImgRegionFilePropertyDlg dlg;
	if(dlg.DoModal() == IDOK)
	{
		if (!CreateTreeCtrl())
			return FALSE;

		if (!CDocument::OnNewDocument())
			return FALSE;

		m_Size = dlg.m_Size;
		m_Color.SetFromCOLORREF(dlg.m_Color);
		m_ImageStr = dlg.m_ImageStr;
		m_Image = theApp.GetImage(m_ImageStr);
		m_Font = theApp.GetFont(dlg.m_strFontFamily, (float)dlg.m_FontSize);

		CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
		ASSERT(pFrame);

		pFrame->m_wndFileView.InvalidLayout();

		return TRUE;
	}

	return FALSE;
}

BOOL CImgRegionDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CImgRegionDoc::CreateTreeCtrl())
		return FALSE;

	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	return TRUE;
}

BOOL CImgRegionDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	if(!CDocument::OnSaveDocument(lpszPathName))
		return FALSE;

	return TRUE;
}

void CImgRegionDoc::OnCloseDocument()
{
	if(m_TreeCtrl.m_hWnd)
	{
		DestroyTreeCtrl();
	}

	CDocument::OnCloseDocument();
}

static const int FILE_VERSION = 339;

static const TCHAR FILE_VERSION_DESC[] = _T("ImgRegion File Version: %d");

void CImgRegionDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{	// storing code
		CString strVersion; strVersion.Format(FILE_VERSION_DESC, FILE_VERSION); ar << strVersion;

		ar << m_Size;
		DWORD argb = m_Color.GetValue(); ar << argb;
		ar << m_ImageStr;
		Gdiplus::FontFamily family; m_Font->GetFamily(&family); CString strFamily; family.GetFamilyName(strFamily.GetBufferSetLength(LF_FACESIZE)); strFamily.ReleaseBuffer(); ar << strFamily;
		ar << m_Font->GetSize();

		SerializeRegionNodeTree(ar);
	}
	else
	{	// loading code
		CString strVersion; ar >> strVersion;

		// ! 版本控制用以将来转换低版本文档
		int nVersion;
		if(1 != _stscanf_s(strVersion, FILE_VERSION_DESC, &nVersion))
		{
			AfxThrowArchiveException(CArchiveException::genericException);
		}
		if(nVersion != FILE_VERSION)
		{
			AfxThrowArchiveException(CArchiveException::badIndex);
		}

		ar >> m_Size;
		DWORD argb; ar >> argb; m_Color.SetValue(argb);
		ar >> m_ImageStr; m_Image = theApp.GetImage(m_ImageStr);
		CString strFamily; float fSize; ar >> strFamily;
		ar >> fSize; m_Font = theApp.GetFont(strFamily, fSize);

		SerializeRegionNodeTree(ar);
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

void CImgRegionDoc::SerializeRegionNode(CArchive & ar, CImgRegion * pReg)
{
	if (ar.IsStoring())
	{
		ar << pReg->m_Local;
		ar << pReg->m_Size;
		DWORD argb = pReg->m_Color.GetValue(); ar << argb;
		ar << pReg->m_ImageStr;
		ar << pReg->m_Border.x << pReg->m_Border.y << pReg->m_Border.z << pReg->m_Border.w;
		Gdiplus::FontFamily family; pReg->m_Font->GetFamily(&family); CString strFamily; family.GetFamilyName(strFamily.GetBufferSetLength(LF_FACESIZE)); strFamily.ReleaseBuffer(); ar << strFamily;
		ar << pReg->m_Font->GetSize();
		argb = pReg->m_FontColor.GetValue(); ar << argb;
		ar << pReg->m_Text;
	}
	else
	{
		ar >> pReg->m_Local;
		ar >> pReg->m_Size;
		DWORD argb; ar >> argb; pReg->m_Color.SetValue(argb);
		ar >> pReg->m_ImageStr; pReg->m_Image = theApp.GetImage(pReg->m_ImageStr);
		ar >> pReg->m_Border.x >> pReg->m_Border.y >> pReg->m_Border.z >> pReg->m_Border.w;
		CString strFamily; float fSize; ar >> strFamily;
		ar >> fSize; pReg->m_Font = theApp.GetFont(strFamily, fSize);
		ar >> argb; pReg->m_FontColor.SetValue(argb);
		ar >> pReg->m_Text;
	}
}

void CImgRegionDoc::SerializeRegionNodeTree(CArchive & ar, HTREEITEM hParent)
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

			SerializeRegionNode(ar, pReg);

			SerializeRegionNodeTree(ar, hItem);
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

			SerializeRegionNode(ar, pReg);

			m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pReg);

			SerializeRegionNodeTree(ar, hItem);
		}
	}
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

	SetModifiedFlag();
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

	SetModifiedFlag();
}

void CImgRegionDoc::OnUpdateDelRegion(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(NULL != m_TreeCtrl.GetSelectedItem());
}

static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0; j < num; ++j)
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}    
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

void CImgRegionDoc::OnExportImg()
{
	CFileDialog dlg(FALSE, _T("png"), GetTitle(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("PNG(*.png)|*.png|JPEG(*.jpg; *.jpeg; *.jpe)|*.jpg; *.jpeg; *.jpe|BMP(*.bmp; *.rle; *.dib)|*.bmp; *.rle; *.dib||"), NULL);
	if(dlg.DoModal() == IDOK)
	{
		CStringW format;
		format.Format(L"image/%s", theApp.GetMIME(dlg.GetFileExt()));
		CLSID encoderClsid;
		if(GetEncoderClsid(format, &encoderClsid) < 0)
		{
			AfxMessageBox(_T("the specified format was not supported"));
			return;
		}

		Gdiplus::Bitmap bmp(m_Size.cx, m_Size.cy, PixelFormat24bppRGB);
		Gdiplus::Graphics grap(&bmp);

		CImgRegionView::DrawRegionDoc(grap, this);

		bmp.Save(dlg.GetPathName(), &encoderClsid, NULL);
	}
}

void CImgRegionDoc::OnFileProperty()
{
	ASSERT(m_Font);

	CImgRegionFilePropertyDlg dlg;
	dlg.m_Size = m_Size;
	dlg.m_Color = m_Color.ToCOLORREF();
	dlg.m_ImageStr = m_ImageStr;
	Gdiplus::FontFamily family; m_Font->GetFamily(&family); family.GetFamilyName(dlg.m_strFontFamily.GetBufferSetLength(LF_FACESIZE)); dlg.m_strFontFamily.ReleaseBuffer();
	dlg.m_FontSize = (LONG)m_Font->GetSize();
	if(dlg.DoModal() == IDOK)
	{
		m_Size = dlg.m_Size;
		m_Color.SetFromCOLORREF(dlg.m_Color);
		m_ImageStr = dlg.m_ImageStr;
		m_Image = theApp.GetImage(m_ImageStr);
		m_Font = theApp.GetFont(dlg.m_strFontFamily, (float)dlg.m_FontSize);

		POSITION pos = GetFirstViewPosition();
		while(NULL != pos)
		{
			CImgRegionView * pView = DYNAMIC_DOWNCAST(CImgRegionView, GetNextView(pos));
			ASSERT(pView);

			pView->UpdateImageSizeTable(m_Size);
		}
	}
}

void CImgRegionDoc::OnEditCopy()
{
	HTREEITEM hSelected = m_TreeCtrl.GetSelectedItem();
	if(hSelected)
	{
		CImgRegion * pReg = (CImgRegion *)m_TreeCtrl.GetItemData(hSelected);
		ASSERT(pReg);

		theApp.m_ClipboardFile.SetLength(0);
		CArchive ar(&theApp.m_ClipboardFile, CArchive::store);
		SerializeRegionNode(ar, pReg);
		ar.Close();
	}
}

#define DELETE_EXCEPTION(e) do { if(e) { e->Delete(); } } while (0)

void CImgRegionDoc::OnEditPaste()
{
	if(theApp.m_ClipboardFile.GetLength() > 0)
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

		CImgRegion * pReg = new CImgRegion(ptOrg, CSize(100,100), Gdiplus::Color::White);
		ASSERT(pReg);

		TRY
		{
			theApp.m_ClipboardFile.SeekToBegin();
			CArchive ar(&theApp.m_ClipboardFile, CArchive::load);
			SerializeRegionNode(ar, pReg);

			pReg->m_Local = ptOrg;
		}
		CATCH_ALL(e)
		{
			TCHAR buff[256];
			e->GetErrorMessage(buff, _countof(buff));
			AfxMessageBox(buff);

			DELETE_EXCEPTION(e);
			delete pReg;
			return;
		}
		END_CATCH_ALL

		HTREEITEM hItem = m_TreeCtrl.InsertItem(_T("aaa"), hParent, TVI_LAST);
		m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pReg);
		m_TreeCtrl.SelectItem(hItem);
	}
}
