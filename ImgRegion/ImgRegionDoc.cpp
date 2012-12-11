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
	: CImgRegion(CPoint(0,0), CSize(500,500), Gdiplus::Color::White)
{
	m_Font = GetFont(L"Arial", 12);
}

BOOL CImgRegionDoc::LocalToRoot(HTREEITEM hItem, const CPoint & ptLocal, CPoint & ptResult)
{
	if(hItem)
	{
		CImgRegion * pReg = (CImgRegion *)m_TreeCtrl.GetItemData(hItem);
		ASSERT(pReg);

		if(hItem == m_TreeCtrl.GetRootItem())
		{
			ptResult = pReg->m_Local + ptLocal;
			return TRUE;
		}

		return LocalToRoot(m_TreeCtrl.GetParentItem(hItem), pReg->m_Local + ptLocal, ptResult);
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

	HTREEITEM hItem = m_TreeCtrl.InsertItem(_T("aaa"));
	CImgRegion * pRegRoot = new CImgRegion(CPoint(0,0), CSize(500,500), Gdiplus::Color(255,255,255,255));
	pRegRoot->m_ImageStr = L"Checker.bmp";
	pRegRoot->m_Image = GetImage(GetFullPath(pRegRoot->m_ImageStr));
	pRegRoot->m_Border = Vector4i(100,50,100,50);
	Gdiplus::FontFamily fontFamily(L"Arial");
	pRegRoot->m_Font = m_Font;
	m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pRegRoot);

	hItem = m_TreeCtrl.InsertItem(_T("bbb"), hItem);
	CImgRegion * pReg = new CImgRegion(CPoint(100,100), CSize(200,200), Gdiplus::Color(192,255,0,0));
	pReg->m_Font = m_Font;
	m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pReg);

	hItem = m_TreeCtrl.InsertItem(_T("ccc"), hItem);
	pReg = new CImgRegion(CPoint(25,25), CSize(75,75), Gdiplus::Color(255,255,255,255));
	pReg->m_Font = m_Font;
	pReg->m_ImageStr = L"com_btn_normal.png";
	pReg->m_Image = GetImage(GetFullPath(pReg->m_ImageStr));
	pReg->m_Border = Vector4i(7,7,7,7);
	m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pReg);

	m_TreeCtrl.SelectItem(hItem);

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
	PathCombine(res.GetBufferSetLength(MAX_PATH), GetCurrentDir(), strPath);
	return res;
}
