#include "StdAfx.h"
#include "ImgRegionDocFileVersions.h"
#include "MainApp.h"

const int CImgRegionDocFileVersions::FILE_VERSION = 498;

const CString CImgRegionDocFileVersions::DEFAULT_CONTROL_NAME(_T("control_%03d"));

static void CImgRegionDocFileVersions_Loading355(CImgRegionDoc * pDoc, CArchive & ar, int version)
{
	if(version < 355)
	{
		AfxMessageBox(str_printf(_T("不支持的版本: %d"), version).c_str());
		AfxThrowUserException();
	}

	ar >> pDoc->m_NextRegId;
	ar >> pDoc->m_Size;
	DWORD argb; ar >> argb; pDoc->m_Color.SetValue(argb);
	ar >> pDoc->m_ImageStr; pDoc->m_Image = theApp.GetImage(pDoc->m_ImageStr);

	pDoc->SerializeSubTreeNode(ar, version);
}

static void CImgRegionDocFileVersions_Loading498(CImgRegionDoc * pDoc, CArchive & ar, int version)
{
	if(version < 498)
	{
		CImgRegionDocFileVersions_Loading355(pDoc, ar, version); return;
	}

	ar >> pDoc->m_NextRegId;
	ar >> pDoc->m_Size;
	DWORD argb; ar >> argb; pDoc->m_Color.SetValue(argb);
	ar >> pDoc->m_ImageStr; pDoc->m_Image = theApp.GetImage(pDoc->m_ImageStr);
	ar >> pDoc->m_strProjectDir;
	ar >> pDoc->m_strLuaPath;

	pDoc->SerializeSubTreeNode(ar, version);
}

void CImgRegionDocFileVersions::Serialize(CImgRegionDoc * pDoc, CArchive & ar, int version)
{
	if (ar.IsLoading())
	{
		CImgRegionDocFileVersions_Loading498(pDoc, ar, version); return;
	}

	ar << pDoc->m_NextRegId;
	ar << pDoc->m_Size;
	DWORD argb = pDoc->m_Color.GetValue(); ar << argb;
	ar << pDoc->m_ImageStr;
	ar << pDoc->m_strProjectDir;
	ar << pDoc->m_strLuaPath;

	pDoc->SerializeSubTreeNode(ar, CImgRegionDocFileVersions::FILE_VERSION);
}

static void CImgRegionDocFileVersions_LoadingSubTreeNode355(CImgRegionDoc * pDoc, CArchive & ar, int version, HTREEITEM hParent, BOOL bOverideName)
{
	if(version < 355)
	{
		AfxMessageBox(str_printf(_T("不支持的版本: %d"), version).c_str());
		AfxThrowUserException();
	}

	int nChilds; ar >> nChilds;

	for(int i = 0; i < nChilds; i++)
	{
		CString strName;
		ar >> strName;
		if(bOverideName)
			strName.Format(CImgRegionDocFileVersions::DEFAULT_CONTROL_NAME, pDoc->m_NextRegId++);

		HTREEITEM hItem = pDoc->m_TreeCtrl.InsertItem(strName, hParent, TVI_LAST); ASSERT(hItem);

		CImgRegion * pReg = new CImgRegion;
		ASSERT(pReg);

		pDoc->m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pReg);
		pReg->Serialize(ar, version);
		CImgRegionDocFileVersions_LoadingSubTreeNode355(pDoc, ar, version, hItem, bOverideName);

		if(pReg->m_Locked)
			pDoc->m_TreeCtrl.SetItemImage(hItem, 1, 1);
		pDoc->m_TreeCtrl.Expand(hItem, TVE_EXPAND);
	}
}

void CImgRegionDocFileVersions::SerializeSubTreeNode(CImgRegionDoc * pDoc, CArchive & ar, int version, HTREEITEM hParent, BOOL bOverideName)
{
	if (ar.IsLoading())
	{
		CImgRegionDocFileVersions_LoadingSubTreeNode355(pDoc, ar, version, hParent, bOverideName); return;
	}

	int nChilds = pDoc->m_TreeCtrl.CalcChildCount(hParent); ar << nChilds;

	HTREEITEM hItem = pDoc->m_TreeCtrl.GetChildItem(hParent);
	for(int i = 0; i < nChilds; i++, hItem = pDoc->m_TreeCtrl.GetNextSiblingItem(hItem))
	{
		ASSERT(hItem);
		ar << pDoc->m_TreeCtrl.GetItemText(hItem);

		CImgRegion * pReg = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hItem);
		ASSERT(pReg);
		pReg->Serialize(ar, version);
		SerializeSubTreeNode(pDoc, ar, version, hItem, bOverideName);
	}
}

static void CImgRegionDocFileVersions_LoadingImgRegion355(CImgRegion * pReg, CArchive & ar, int version)
{
	if(version < 355)
	{
		AfxMessageBox(str_printf(_T("不支持的版本: %d"), version).c_str());
		AfxThrowUserException();
	}

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

void CImgRegionDocFileVersions::SerializeImgRegion(CImgRegion * pReg, CArchive & ar, int version)
{
	if (ar.IsLoading())
	{
		CImgRegionDocFileVersions_LoadingImgRegion355(pReg, ar, version); return;
	}

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
