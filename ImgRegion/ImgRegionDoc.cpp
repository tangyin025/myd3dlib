#include "stdafx.h"
#include "ImgRegionDoc.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "resource.h"
#include "ImgRegionFilePropertyDlg.h"
#include "ImgRegionView.h"
#include "LuaExporterDlg.h"

//#pragma comment(lib, "UxTheme.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void HistoryChangeItemLocation::Do(void)
{
	ASSERT(m_pDoc->m_TreeCtrl.m_ItemMap.find(m_itemID) != m_pDoc->m_TreeCtrl.m_ItemMap.end());

	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];

	CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	pReg->m_Location = m_newValue;
}

void HistoryChangeItemLocation::Undo(void)
{
	ASSERT(m_pDoc->m_TreeCtrl.m_ItemMap.find(m_itemID) != m_pDoc->m_TreeCtrl.m_ItemMap.end());

	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];

	CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	pReg->m_Location = m_oldValue;
}

void HistoryChangeItemSize::Do(void)
{
	ASSERT(m_pDoc->m_TreeCtrl.m_ItemMap.find(m_itemID) != m_pDoc->m_TreeCtrl.m_ItemMap.end());

	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];

	CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	pReg->m_Size = m_newValue;
}

void HistoryChangeItemSize::Undo(void)
{
	ASSERT(m_pDoc->m_TreeCtrl.m_ItemMap.find(m_itemID) != m_pDoc->m_TreeCtrl.m_ItemMap.end());

	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];

	CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	pReg->m_Size = m_oldValue;
}

void HistoryChangeItemTextOff::Do(void)
{
	ASSERT(m_pDoc->m_TreeCtrl.m_ItemMap.find(m_itemID) != m_pDoc->m_TreeCtrl.m_ItemMap.end());

	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];

	CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	pReg->m_TextOff = m_newValue;
}

void HistoryChangeItemTextOff::Undo(void)
{
	ASSERT(m_pDoc->m_TreeCtrl.m_ItemMap.find(m_itemID) != m_pDoc->m_TreeCtrl.m_ItemMap.end());

	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];

	CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	pReg->m_TextOff = m_oldValue;
}

HistoryAddRegion::HistoryAddRegion(CImgRegionDoc * pDoc, LPCTSTR itemID, LPCTSTR parentID, LPCTSTR beforeID)
	: m_pDoc(pDoc)
	, m_itemID(itemID)
	, m_parentID(parentID)
	, m_beforeID(beforeID)
	, m_OverideRegId(0)
{
}

void HistoryAddRegion::Do(void)
{
	HTREEITEM hParent = m_parentID.empty() ? TVI_ROOT : m_pDoc->m_TreeCtrl.m_ItemMap[m_parentID];
	HTREEITEM hBefore = m_beforeID.empty() ? TVI_LAST : m_pDoc->m_TreeCtrl.m_ItemMap[m_beforeID];
	HTREEITEM hItem = m_pDoc->m_TreeCtrl.InsertItem(m_itemID.c_str(), hParent, hBefore);

	CImgRegion * pReg = new CImgRegion(CPoint(10,10), CSize(100,100));
	ASSERT(pReg);

	m_pDoc->m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pReg);

	// ! 使用 OverideRegId，防止发生重名，又可以再undo之后重名创建
	if(0 == m_OverideRegId)
	{
		m_OverideRegId = m_pDoc->m_NextRegId;
	}

	DWORD oldRegId = m_pDoc->m_NextRegId;
	{
		m_pDoc->m_NextRegId = m_OverideRegId;
		ASSERT(m_NodeCache.GetLength() > 0);
		m_NodeCache.SeekToBegin();
		CArchive ar(&m_NodeCache, CArchive::load);
		m_pDoc->SerializeRegionNode(ar, pReg);
		m_pDoc->SerializeRegionNodeSubTree(ar, hItem, TRUE);
		ar.Close();
	}
	m_pDoc->m_NextRegId = max(oldRegId, m_pDoc->m_NextRegId);

	pReg->m_Locked = FALSE;

	m_pDoc->m_TreeCtrl.Expand(hItem, TVE_EXPAND);

	m_pDoc->m_TreeCtrl.SelectItem(hItem);
}

void HistoryAddRegion::Undo(void)
{
	ASSERT(m_pDoc->m_TreeCtrl.m_ItemMap.find(m_itemID) != m_pDoc->m_TreeCtrl.m_ItemMap.end());

	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];

	m_pDoc->m_TreeCtrl.DeleteTreeItem<CImgRegion>(hItem, TRUE);
}

void HistoryDelRegion::Do(void)
{
	ASSERT(m_pDoc->m_TreeCtrl.m_ItemMap.find(m_itemID) != m_pDoc->m_TreeCtrl.m_ItemMap.end());

	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];

	CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	m_NodeCache.SetLength(0);
	CArchive ar(&m_NodeCache, CArchive::store);
	m_pDoc->SerializeRegionNode(ar, pReg);
	m_pDoc->SerializeRegionNodeSubTree(ar, hItem);
	ar.Close();

	HTREEITEM hParent = m_pDoc->m_TreeCtrl.GetParentItem(hItem);
	m_parentID = hParent ? m_pDoc->m_TreeCtrl.GetItemText(hParent) : _T("");

	HTREEITEM hBefore = m_pDoc->m_TreeCtrl.GetPrevSiblingItem(hItem);
	m_beforeID = hBefore ? m_pDoc->m_TreeCtrl.GetItemText(hBefore) : _T("");

	m_pDoc->m_TreeCtrl.DeleteTreeItem<CImgRegion>(hItem, TRUE);
}

void HistoryDelRegion::Undo(void)
{
	HTREEITEM hParent = m_parentID.empty() ? TVI_ROOT : m_pDoc->m_TreeCtrl.m_ItemMap[m_parentID];
	HTREEITEM hBefore = m_beforeID.empty() ? TVI_LAST : m_pDoc->m_TreeCtrl.m_ItemMap[m_beforeID];
	HTREEITEM hItem = m_pDoc->m_TreeCtrl.InsertItem(m_itemID.c_str(), hParent, hBefore);

	CImgRegion * pReg = new CImgRegion(CPoint(10,10), CSize(100,100));
	ASSERT(pReg);

	m_pDoc->m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pReg);

	ASSERT(m_NodeCache.GetLength() > 0);
	m_NodeCache.SeekToBegin();
	CArchive ar(&m_NodeCache, CArchive::load);
	m_pDoc->SerializeRegionNode(ar, pReg);
	m_pDoc->SerializeRegionNodeSubTree(ar, hItem);
	ar.Close();

	if(pReg->m_Locked)
		m_pDoc->m_TreeCtrl.SetItemImage(hItem, 1, 1);
	m_pDoc->m_TreeCtrl.Expand(hItem, TVE_EXPAND);
}

void HistoryMovRegion::Do(void)
{
	ASSERT(m_pDoc->m_TreeCtrl.m_ItemMap.find(m_itemID) != m_pDoc->m_TreeCtrl.m_ItemMap.end());

	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];
	HTREEITEM hParent = m_pDoc->m_TreeCtrl.GetParentItem(hItem);
	HTREEITEM hBefore = m_pDoc->m_TreeCtrl.GetPrevSiblingItem(hItem);

	m_oldParentID = hParent ? m_pDoc->m_TreeCtrl.GetItemText(hParent) : _T("");
	m_oldBeforeID = hBefore ? m_pDoc->m_TreeCtrl.GetItemText(hBefore) : _T("");

	CPoint ptOrg = m_pDoc->LocalToRoot(hItem, CPoint(0,0));

	HTREEITEM hNewParent = m_newParentID.empty() ? TVI_ROOT : m_pDoc->m_TreeCtrl.m_ItemMap[m_newParentID];
	HTREEITEM hNewBefore = m_newBeforeID.empty() ? TVI_LAST : m_pDoc->m_TreeCtrl.m_ItemMap[m_newBeforeID];

	HTREEITEM hNewItem = m_pDoc->m_TreeCtrl.MoveTreeItem(hNewParent, hNewBefore, hItem);
	if(hNewItem != hItem)
	{
		m_pDoc->m_TreeCtrl.SelectItem(hNewItem);

		CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hNewItem);
		ASSERT(pReg);

		pReg->m_Location = m_pDoc->RootToLocal(hNewParent, ptOrg);
	}
}

void HistoryMovRegion::Undo(void)
{
	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];

	CPoint ptOrg = m_pDoc->LocalToRoot(hItem, CPoint(0,0));

	HTREEITEM hOldParent = m_oldParentID.empty() ? TVI_ROOT : m_pDoc->m_TreeCtrl.m_ItemMap[m_oldParentID];
	HTREEITEM hOldBefore = m_oldBeforeID.empty() ? TVI_FIRST : m_pDoc->m_TreeCtrl.m_ItemMap[m_oldBeforeID];

	HTREEITEM hOldItem = m_pDoc->m_TreeCtrl.MoveTreeItem(hOldParent, hOldBefore, hItem);
	if(hOldItem != hItem)
	{
		m_pDoc->m_TreeCtrl.SelectItem(hOldItem);

		CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hOldItem);
		ASSERT(pReg);

		pReg->m_Location = m_pDoc->RootToLocal(hOldParent, ptOrg);
	}
}

void HistoryModifyRegion::Do(void)
{
	iterator hist_iter = begin();
	for(; hist_iter != end(); hist_iter++)
	{
		(*hist_iter)->Do();
	}
}

void HistoryModifyRegion::Undo(void)
{
	iterator hist_iter = begin();
	for(; hist_iter != end(); hist_iter++)
	{
		(*hist_iter)->Undo();
	}
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
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &CImgRegionDoc::OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, &CImgRegionDoc::OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, &CImgRegionDoc::OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, &CImgRegionDoc::OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, &CImgRegionDoc::OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, &CImgRegionDoc::OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, &CImgRegionDoc::OnUpdateEditRedo)
	ON_COMMAND(ID_EXPORT_LUA, &CImgRegionDoc::OnExportLua)
END_MESSAGE_MAP()

CImgRegionDoc::CImgRegionDoc(void)
	: m_HistoryStep(0)
	, m_NextRegId(1)
{
}

CPoint CImgRegionDoc::LocalToRoot(HTREEITEM hItem, const CPoint & ptLocal)
{
	if(NULL == hItem)
		return ptLocal;

	CImgRegion * pReg = (CImgRegion *)m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	return LocalToRoot(m_TreeCtrl.GetParentItem(hItem), ptLocal + pReg->m_Location);
}

CPoint CImgRegionDoc::RootToLocal(HTREEITEM hItem, const CPoint & ptRoot)
{
	if(NULL == hItem || TVI_ROOT == hItem)
		return ptRoot;

	CImgRegion * pReg = (CImgRegion *)m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	return RootToLocal(m_TreeCtrl.GetParentItem(hItem), ptRoot - pReg->m_Location);
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
	//SetWindowTheme(m_TreeCtrl.m_hWnd, L"Explorer", NULL);

	CBitmap bmp;
	if (!bmp.LoadBitmap(IDB_BITMAP2))
	{
		TRACE0("bmp.LoadBitmap failed \n");
		return FALSE;
	}
	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	m_TreeImageList.Create(16, bmpObj.bmHeight, ILC_MASK | ILC_COLOR24, 0, 0);
	m_TreeImageList.Add(&bmp, RGB(255,0,255));
	m_TreeCtrl.SetImageList(&m_TreeImageList, TVSIL_NORMAL);

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

	ASSERT(m_TreeCtrl.m_ItemMap.empty());
	m_TreeCtrl.DestroyWindow();

	m_TreeImageList.DeleteImageList();
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

		if(hRet = GetPointedRegionNode(m_TreeCtrl.GetChildItem(hItem), ptLocal - pReg->m_Location))
			return hRet;

		if(CRect(pReg->m_Location, pReg->m_Size).PtInRect(ptLocal))
			return hItem;
	}
	return NULL;
}

BOOL CImgRegionDoc::OnNewDocument(void)
{
	CImgRegionFilePropertyDlg dlg;
	dlg.m_Size.cx = theApp.GetProfileInt(_T("Settings"), _T("SizeX"), dlg.m_Size.cx);
	dlg.m_Size.cy = theApp.GetProfileInt(_T("Settings"), _T("SizeY"), dlg.m_Size.cy);
	dlg.m_Color = theApp.GetProfileInt(_T("Settings"), _T("Color"), dlg.m_Color);
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

		theApp.WriteProfileInt(_T("Settings"), _T("SizeX"), dlg.m_Size.cx);
		theApp.WriteProfileInt(_T("Settings"), _T("SizeY"), dlg.m_Size.cy);
		theApp.WriteProfileInt(_T("Settings"), _T("Color"), dlg.m_Color);

		CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
		ASSERT(pFrame);

		pFrame->m_wndFileView.InvalidLayout();

		UpdateImageSizeTable(m_Size);

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

	UpdateImageSizeTable(m_Size);

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

static const int FILE_VERSION = 355;

static const TCHAR FILE_VERSION_DESC[] = _T("ImgRegion File Version: %d");

void CImgRegionDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{	// storing code
		CString strVersion; strVersion.Format(FILE_VERSION_DESC, FILE_VERSION); ar << strVersion;

		ar << m_NextRegId;
		ar << m_Size;
		DWORD argb = m_Color.GetValue(); ar << argb;
		ar << m_ImageStr;

		SerializeRegionNodeSubTree(ar);
	}
	else
	{	// loading code
		CString strVersion; ar >> strVersion;

		// ! 版本控制用以将来转换低版本文档
		int nVersion;
		if(1 != _stscanf_s(strVersion, FILE_VERSION_DESC, &nVersion))
		{
			AfxThrowArchiveException(CArchiveException::badIndex);
		}
		if(nVersion != FILE_VERSION)
		{
			CString msg;
			msg.Format(_T("不支持的版本：%d"), nVersion);
			AfxMessageBox(msg);
			AfxThrowUserException();
		}

		ar >> m_NextRegId;
		ar >> m_Size;
		DWORD argb; ar >> argb; m_Color.SetValue(argb);
		ar >> m_ImageStr; m_Image = theApp.GetImage(m_ImageStr);

		SerializeRegionNodeSubTree(ar);
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
		ar << pReg->m_Locked;
		ar << pReg->m_Location;
		ar << pReg->m_Size;
		DWORD argb = pReg->m_Color.GetValue(); ar << argb;
		ar << pReg->m_ImageStr;
		ar << pReg->m_Border.x << pReg->m_Border.y << pReg->m_Border.z << pReg->m_Border.w;
		Gdiplus::FontFamily family; pReg->m_Font->GetFamily(&family); CString strFamily; family.GetFamilyName(strFamily.GetBufferSetLength(LF_FACESIZE)); strFamily.ReleaseBuffer(); ar << strFamily;
		ar << pReg->m_Font->GetSize();
		argb = pReg->m_FontColor.GetValue(); ar << argb;
		ar << pReg->m_Text;
		ar << pReg->m_TextAlign;
		ar << pReg->m_TextWrap;
		ar << pReg->m_TextOff;
	}
	else
	{
		ar >> pReg->m_Locked;
		ar >> pReg->m_Location;
		ar >> pReg->m_Size;
		DWORD argb; ar >> argb; pReg->m_Color.SetValue(argb);
		ar >> pReg->m_ImageStr; pReg->m_Image = theApp.GetImage(pReg->m_ImageStr);
		ar >> pReg->m_Border.x >> pReg->m_Border.y >> pReg->m_Border.z >> pReg->m_Border.w;
		CString strFamily; float fSize; ar >> strFamily;
		ar >> fSize; pReg->m_Font = theApp.GetFont(strFamily, fSize);
		ar >> argb; pReg->m_FontColor.SetValue(argb);
		ar >> pReg->m_Text;
		ar >> pReg->m_TextAlign;
		ar >> pReg->m_TextWrap;
		ar >> pReg->m_TextOff;
	}
}

void CImgRegionDoc::SerializeRegionNodeSubTree(CArchive & ar, HTREEITEM hParent, BOOL bOverideName)
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

			SerializeRegionNodeSubTree(ar, hItem, bOverideName);
		}
	}
	else
	{
		int nChilds; ar >> nChilds;

		for(int i = 0; i < nChilds; i++)
		{
			CString strName;
			ar >> strName;
			if(bOverideName)
				strName.Format(_T("图层 %03d"), m_NextRegId++);

			HTREEITEM hItem = m_TreeCtrl.InsertItem(strName, hParent, TVI_LAST); ASSERT(hItem);

			CImgRegion * pReg = new CImgRegion(CPoint(10,10), CSize(100,100));
			ASSERT(pReg);

			m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pReg);

			SerializeRegionNode(ar, pReg);

			SerializeRegionNodeSubTree(ar, hItem, bOverideName);

			if(pReg->m_Locked)
				m_TreeCtrl.SetItemImage(hItem, 1, 1);
			m_TreeCtrl.Expand(hItem, TVE_EXPAND);
		}
	}
}

void CImgRegionDoc::UpdateImageSizeTable(const CSize & sizeRoot)
{
	POSITION pos = GetFirstViewPosition();
	while(NULL != pos)
	{
		CImgRegionView * pView = DYNAMIC_DOWNCAST(CImgRegionView, GetNextView(pos));
		ASSERT(pView);

		pView->ZoomImage(pView->m_ImageZoomFactor);

		pView->Invalidate();
	}
}

void CImgRegionDoc::AddNewHistory(HistoryPtr hist)
{
	m_HistoryList.resize(m_HistoryStep++);
	m_HistoryList.push_back(hist);
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
		ptOrg += pReg->m_Location;
	}

	CString strName;
	strName.Format(_T("图层 %03d"), m_NextRegId++);
	HistoryAddRegionPtr hist(new HistoryAddRegion(
		this, strName, hParent ? m_TreeCtrl.GetItemText(hParent) : _T(""), hSelected ? m_TreeCtrl.GetItemText(hSelected) : _T("")));

	CImgRegion reg(ptOrg, CSize(100,100));
	reg.m_Color = Gdiplus::Color(255,my::Random<int>(0,255),my::Random<int>(0,255),my::Random<int>(0,255));
	reg.m_Font = theApp.GetFont(_T("微软雅黑"), 16);
	reg.m_FontColor = Gdiplus::Color(255,my::Random<int>(0,255),my::Random<int>(0,255),my::Random<int>(0,255));
	CArchive ar(&hist->m_NodeCache, CArchive::store);
	SerializeRegionNode(ar, &reg);
	ar << 0;
	ar.Close();

	AddNewHistory(hist);
	hist->Do();

	UpdateAllViews(NULL);

	SetModifiedFlag();

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);
	pFrame->m_wndProperties.InvalidProperties();

	// ! 去掉ActiveView的m_nSelectedHandle属性
	POSITION pos = GetFirstViewPosition();
	while(NULL != pos)
	{
		CImgRegionView * pView = DYNAMIC_DOWNCAST(CImgRegionView, GetNextView(pos));
		ASSERT(pView);

		pView->m_nSelectedHandle = CImgRegionView::HandleTypeNone;
	}
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
		CImgRegion * pReg = (CImgRegion *)m_TreeCtrl.GetItemData(hSelected);
		ASSERT(pReg);

		if(!pReg->m_Locked)
		{
			HistoryPtr hist(new HistoryDelRegion(
				this, m_TreeCtrl.GetItemText(hSelected)));

			AddNewHistory(hist);
			hist->Do();

			UpdateAllViews(NULL);

			SetModifiedFlag();

			CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
			ASSERT(pFrame);
			pFrame->m_wndProperties.InvalidProperties();
		}
		else
			::MessageBeep(MB_OK);
	}
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
	CImgRegionFilePropertyDlg dlg;
	dlg.m_Size = m_Size;
	dlg.m_Color = m_Color.ToCOLORREF();
	dlg.m_ImageStr = m_ImageStr;
	if(dlg.DoModal() == IDOK)
	{
		m_Size = dlg.m_Size;
		m_Color.SetFromCOLORREF(dlg.m_Color);
		m_ImageStr = dlg.m_ImageStr;
		m_Image = theApp.GetImage(m_ImageStr);

		UpdateImageSizeTable(m_Size);

		SetModifiedFlag();
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
		SerializeRegionNodeSubTree(ar, hSelected);
		ar.Close();
	}
}

void CImgRegionDoc::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(NULL != m_TreeCtrl.GetSelectedItem());
}

#define DELETE_EXCEPTION(e) do { if(e) { e->Delete(); } } while (0)

void CImgRegionDoc::OnEditPaste()
{
	if(theApp.m_ClipboardFile.GetLength() > 0)
	{
		HTREEITEM hParent = NULL;
		HTREEITEM hSelected = m_TreeCtrl.GetSelectedItem();
		if(hSelected)
		{
			hParent = m_TreeCtrl.GetParentItem(hSelected);
		}

		CString strName;
		strName.Format(_T("图层 %03d"), m_NextRegId++);
		HistoryAddRegionPtr hist(new HistoryAddRegion(
			this, strName, hParent ? m_TreeCtrl.GetItemText(hParent) : _T(""), m_TreeCtrl.GetItemText(hSelected)));

		void * pBuffer;
		void * pBufferMax;
		theApp.m_ClipboardFile.SeekToBegin();
		UINT len = theApp.m_ClipboardFile.GetBufferPtr(CFile::bufferRead, -1, &pBuffer, &pBufferMax);
		hist->m_NodeCache.Write(pBuffer, len);
		hist->m_NodeCache.Flush();

		AddNewHistory(hist);
		hist->Do();

		UpdateAllViews(NULL);

		SetModifiedFlag();

		CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
		ASSERT(pFrame);
		pFrame->m_wndProperties.InvalidProperties();

		// ! 去掉ActiveView的m_nSelectedHandle属性
		POSITION pos = GetFirstViewPosition();
		while(NULL != pos)
		{
			CImgRegionView * pView = DYNAMIC_DOWNCAST(CImgRegionView, GetNextView(pos));
			ASSERT(pView);

			pView->m_nSelectedHandle = CImgRegionView::HandleTypeNone;
		}
	}
}

void CImgRegionDoc::OnUpdateEditPaste(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(theApp.m_ClipboardFile.GetLength() > 0);
}

void CImgRegionDoc::OnEditUndo()
{
	if(m_HistoryStep > 0)
	{
		m_HistoryList[--m_HistoryStep]->Undo();

		UpdateAllViews(NULL);

		SetModifiedFlag();

		CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
		ASSERT(pFrame);
		pFrame->m_wndProperties.InvalidProperties();
	}
}

void CImgRegionDoc::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_HistoryStep > 0);
}

void CImgRegionDoc::OnEditRedo()
{
	if(m_HistoryStep < m_HistoryList.size())
	{
		m_HistoryList[m_HistoryStep++]->Do();

		UpdateAllViews(NULL);

		SetModifiedFlag();

		CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
		ASSERT(pFrame);
		pFrame->m_wndProperties.InvalidProperties();
	}
}

void CImgRegionDoc::OnUpdateEditRedo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_HistoryStep < m_HistoryList.size());
}

void CImgRegionDoc::OnExportLua()
{
	CLuaExporterDlg dlg;
	dlg.DoModal();
}
