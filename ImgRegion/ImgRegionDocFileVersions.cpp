#include "StdAfx.h"
#include "ImgRegionDocFileVersions.h"
#include "MainApp.h"

const int CImgRegionDocFileVersions::FILE_VERSION = 498;

const CString CImgRegionDocFileVersions::DEFAULT_CONTROL_NAME(_T("editor_control_%d"));

static void CImgRegionDocFileVersions_Loading355(CImgRegionDoc * pDoc, CArchive & ar, int version)
{
	if(version < 355)
	{
		AfxMessageBox(str_printf(_T("不再支持的版本：%d"), version).c_str());
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
		AfxMessageBox(str_printf(_T("不再支持的版本：%d"), version).c_str());
		AfxThrowUserException();
	}

	int nChilds; ar >> nChilds;

	for(int i = 0; i < nChilds; i++)
	{
		UINT id;
		ar >> id;
		CString szName;
		ar >> szName;

		if(bOverideName)
		{
			id = ++pDoc->m_NextRegId;
			szName.Format(CImgRegionDocFileVersions::DEFAULT_CONTROL_NAME, id);
		}

		CImgRegionPtr pReg(new CImgRegion);
		HTREEITEM hItem = pDoc->InsertItem(id, (LPCTSTR)szName, pReg, hParent, TVI_LAST);

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
		ar << pDoc->GetItemId(hItem);
		ar << pDoc->m_TreeCtrl.GetItemText(hItem);

		CImgRegionPtr pReg = pDoc->GetItemNode(hItem);
		ASSERT(pReg);
		pReg->Serialize(ar, version);
		SerializeSubTreeNode(pDoc, ar, version, hItem, bOverideName);
	}
}

static void CImgRegionDocFileVersions_LoadingImgRegion355(CImgRegion * pReg, CArchive & ar, int version)
{
	if(version < 355)
	{
		AfxMessageBox(str_printf(_T("不再支持的版本：%d"), version).c_str());
		AfxThrowUserException();
	}

	ar >> pReg->m_Class;
	ar >> pReg->m_Locked;
	ar >> pReg->m_x.scale;
	ar >> pReg->m_x.offset;
	ar >> pReg->m_y.scale;
	ar >> pReg->m_y.offset;
	ar >> pReg->m_Width.scale;
	ar >> pReg->m_Width.offset;
	ar >> pReg->m_Height.scale;
	ar >> pReg->m_Height.offset;
	DWORD argb; ar >> argb; pReg->m_Color.SetValue(argb);
	ar >> pReg->m_ImageStr; pReg->m_Image = theApp.GetImage(pReg->m_ImageStr);
	ar >> pReg->m_ImageRect.X >> pReg->m_ImageRect.Y >> pReg->m_ImageRect.Width >> pReg->m_ImageRect.Height;
	ar >> pReg->m_ImageBorder.x >> pReg->m_ImageBorder.y >> pReg->m_ImageBorder.z >> pReg->m_ImageBorder.w;
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

	ar << pReg->m_Class;
	ar << pReg->m_Locked;
	ar << pReg->m_x.scale;
	ar << pReg->m_x.offset;
	ar << pReg->m_y.scale;
	ar << pReg->m_y.offset;
	ar << pReg->m_Width.scale;
	ar << pReg->m_Width.offset;
	ar << pReg->m_Height.scale;
	ar << pReg->m_Height.offset;
	DWORD argb = pReg->m_Color.GetValue(); ar << argb;
	ar << pReg->m_ImageStr;
	ar << pReg->m_ImageRect.X << pReg->m_ImageRect.Y << pReg->m_ImageRect.Width << pReg->m_ImageRect.Height;
	ar << pReg->m_ImageBorder.x << pReg->m_ImageBorder.y << pReg->m_ImageBorder.z << pReg->m_ImageBorder.w;
	Gdiplus::FontFamily family; pReg->m_Font->GetFamily(&family); CString strFamily; family.GetFamilyName(strFamily.GetBufferSetLength(LF_FACESIZE)); strFamily.ReleaseBuffer(); ar << strFamily;
	ar << pReg->m_Font->GetSize();
	argb = pReg->m_FontColor.GetValue(); ar << argb;
	ar << pReg->m_Text;
	ar << pReg->m_TextAlign;
	ar << pReg->m_TextWrap;
	ar << pReg->m_TextOff;
}
