// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "StdAfx.h"
#include "ImgRegionDocFileVersions.h"
#include "MainApp.h"
#include <boost/serialization/string.hpp>

const int CImgRegionDocFileVersions::FILE_VERSION = 498;

const CString CImgRegionDocFileVersions::DEFAULT_CONTROL_NAME(_T("editor_control_%d"));

void CImgRegionDocFileVersions::SerializeLoad(CImgRegionDoc * pDoc, CArchive & ar, int version)
{
	if(version < 498)
	{
		AfxMessageBox(str_printf(_T("不再支持的版本：%d"), version).c_str());
		AfxThrowUserException();
	}

	ar >> pDoc->m_NextRegId;
	ar >> pDoc->m_Size;
	DWORD argb; ar >> argb; pDoc->m_Color.SetValue(argb);
	ar >> pDoc->m_ImageStr; pDoc->m_Image = theApp.GetImage(pDoc->m_ImageStr);
	ar >> pDoc->m_strProjectDir;
	ar >> pDoc->m_strLuaPath;

	SerializeSubTreeNode(pDoc, ar, version, TVI_ROOT, FALSE);
}

void CImgRegionDocFileVersions::Serialize(CImgRegionDoc * pDoc, CArchive & ar, int version)
{
	if (ar.IsLoading())
	{
		SerializeLoad(pDoc, ar, version); return;
	}

	ar << pDoc->m_NextRegId;
	ar << pDoc->m_Size;
	DWORD argb = pDoc->m_Color.GetValue(); ar << argb;
	ar << pDoc->m_ImageStr;
	ar << pDoc->m_strProjectDir;
	ar << pDoc->m_strLuaPath;

	SerializeSubTreeNode(pDoc, ar, CImgRegionDocFileVersions::FILE_VERSION, TVI_ROOT, FALSE);
}

void CImgRegionDocFileVersions::SerializeLoadSubTreeNode(CImgRegionDoc * pDoc, CArchive & ar, int version, HTREEITEM hParent, BOOL bOverideName)
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

		SerializeImgRegion(pReg.get(), ar, version);
		SerializeLoadSubTreeNode(pDoc, ar, version, hItem, bOverideName);

		if(pReg->m_Locked)
			pDoc->m_TreeCtrl.SetItemImage(hItem, 1, 1);
		pDoc->m_TreeCtrl.Expand(hItem, TVE_EXPAND);
	}
}

void CImgRegionDocFileVersions::SerializeSubTreeNode(CImgRegionDoc * pDoc, CArchive & ar, int version, HTREEITEM hParent, BOOL bOverideName)
{
	if (ar.IsLoading())
	{
		SerializeLoadSubTreeNode(pDoc, ar, version, hParent, bOverideName); return;
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
		SerializeImgRegion(pReg.get(), ar, version);
		SerializeSubTreeNode(pDoc, ar, version, hItem, bOverideName);
	}
}

void CImgRegionDocFileVersions::SerializeLoadImgRegion(CImgRegion * pReg, CArchive & ar, int version)
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
		SerializeLoadImgRegion(pReg, ar, version); return;
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

namespace boost {
	namespace serialization {
		template<class Archive>
		void save(Archive& ar, const CString& str, const unsigned int version) {
			// 将 CString 转换为 std::string (UTF-8)
			CStringA utf8 = CW2A(str, CP_UTF8);
			std::string std_str(utf8.GetString(), utf8.GetLength());
			ar& std_str;  // 序列化标准字符串
		}

		template<class Archive>
		void load(Archive& ar, CString& str, const unsigned int version) {
			std::string std_str;
			ar& std_str;  // 反序列化到 std::string

			// 将 UTF-8 字符串转换为 CString
			str = CA2W(std_str.c_str(), CP_UTF8);
		}

		// 拆分 save/load 函数
		template<class Archive>
		inline void serialize(Archive& ar, CString& str, const unsigned int version) {
			split_free(ar, str, version);
		}
	}
}

void CImgRegionDocFileVersions::SerializeLoad(CImgRegionDoc* pDoc, boost::archive::polymorphic_iarchive& ar)
{
	ar >> boost::serialization::make_nvp("NextRegId", pDoc->m_NextRegId);
	ar >> boost::serialization::make_nvp("Size.cx", pDoc->m_Size.cx);
	ar >> boost::serialization::make_nvp("Size.cy", pDoc->m_Size.cy);
	DWORD argb; ar >> BOOST_SERIALIZATION_NVP(argb); pDoc->m_Color.SetValue(argb);
	ar >> boost::serialization::make_nvp("ImageStr", pDoc->m_ImageStr); pDoc->m_Image = theApp.GetImage(pDoc->m_ImageStr);
	ar >> boost::serialization::make_nvp("strProjectDir", pDoc->m_strProjectDir);
	ar >> boost::serialization::make_nvp("strLuaPath", pDoc->m_strLuaPath);
	ar >> boost::serialization::make_nvp("GridSize.cx", pDoc->m_GridSize.cx);
	ar >> boost::serialization::make_nvp("GridSize.cy", pDoc->m_GridSize.cy);

	SerializeLoadSubTreeNode(pDoc, ar, TVI_ROOT, FALSE);
}

void CImgRegionDocFileVersions::Serialize(CImgRegionDoc* pDoc, boost::archive::polymorphic_oarchive& ar)
{
	ar << boost::serialization::make_nvp("NextRegId", pDoc->m_NextRegId);
	ar << boost::serialization::make_nvp("Size.cx", pDoc->m_Size.cx);
	ar << boost::serialization::make_nvp("Size.cy", pDoc->m_Size.cy);
	DWORD argb = pDoc->m_Color.GetValue(); ar << BOOST_SERIALIZATION_NVP(argb);
	ar << boost::serialization::make_nvp("ImageStr", pDoc->m_ImageStr);
	ar << boost::serialization::make_nvp("strProjectDir", pDoc->m_strProjectDir);
	ar << boost::serialization::make_nvp("strLuaPath", pDoc->m_strLuaPath);
	ar << boost::serialization::make_nvp("GridSize.cx", pDoc->m_GridSize.cx);
	ar << boost::serialization::make_nvp("GridSize.cy", pDoc->m_GridSize.cy);

	SerializeSubTreeNode(pDoc, ar, TVI_ROOT);
}

void CImgRegionDocFileVersions::SerializeLoadSubTreeNode(CImgRegionDoc* pDoc, boost::archive::polymorphic_iarchive& ar, HTREEITEM hParent, BOOL bOverideName)
{
	int nChilds; ar >> BOOST_SERIALIZATION_NVP(nChilds);

	for (int i = 0; i < nChilds; i++)
	{
		UINT id;
		ar >> BOOST_SERIALIZATION_NVP(id);
		CString szName;
		ar >> BOOST_SERIALIZATION_NVP(szName);

		if (bOverideName)
		{
			id = ++pDoc->m_NextRegId;
			szName.Format(CImgRegionDocFileVersions::DEFAULT_CONTROL_NAME, id);
		}

		CImgRegionPtr pReg(new CImgRegion);
		HTREEITEM hItem = pDoc->InsertItem(id, (LPCTSTR)szName, pReg, hParent, TVI_LAST);

		SerializeLoadImgRegion(pReg.get(), ar, pDoc, hItem, bOverideName);

		if (pReg->m_Locked)
			pDoc->m_TreeCtrl.SetItemImage(hItem, 1, 1);
		pDoc->m_TreeCtrl.Expand(hItem, TVE_EXPAND);
	}
}

void CImgRegionDocFileVersions::SerializeSubTreeNode(CImgRegionDoc* pDoc, boost::archive::polymorphic_oarchive& ar, HTREEITEM hParent)
{
	if (!hParent)
	{
		int nChilds = 0;
		ar << BOOST_SERIALIZATION_NVP(nChilds);
		return;
	}

	int nChilds = pDoc->m_TreeCtrl.CalcChildCount(hParent); ar << BOOST_SERIALIZATION_NVP(nChilds);

	HTREEITEM hItem = pDoc->m_TreeCtrl.GetChildItem(hParent);
	for (int i = 0; i < nChilds; i++, hItem = pDoc->m_TreeCtrl.GetNextSiblingItem(hItem))
	{
		ASSERT(hItem);
		UINT id = pDoc->GetItemId(hItem);
		ar << BOOST_SERIALIZATION_NVP(id);
		CString szName = pDoc->m_TreeCtrl.GetItemText(hItem);
		ar << BOOST_SERIALIZATION_NVP(szName);

		CImgRegionPtr pReg = pDoc->GetItemNode(hItem);
		ASSERT(pReg);
		SerializeImgRegion(pReg.get(), ar, pDoc, hItem);
	}
}

struct CImgRegionWrapper
{
	CImgRegion* pReg;
	CImgRegionDoc* pDoc;
	HTREEITEM hItem;
	BOOL bOverideName;

	template<class Archive>
	void save(Archive& ar, const unsigned int version) const
	{
		ar << boost::serialization::make_nvp("Class", pReg->m_Class);
		ar << boost::serialization::make_nvp("Locked", pReg->m_Locked);
		ar << boost::serialization::make_nvp("Visible", pReg->m_Visible);
		ar << boost::serialization::make_nvp("x.scale", pReg->m_x.scale);
		ar << boost::serialization::make_nvp("x.offset", pReg->m_x.offset);
		ar << boost::serialization::make_nvp("y.scale", pReg->m_y.scale);
		ar << boost::serialization::make_nvp("y.offset", pReg->m_y.offset);
		ar << boost::serialization::make_nvp("Width.scale", pReg->m_Width.scale);
		ar << boost::serialization::make_nvp("Width.offset", pReg->m_Width.offset);
		ar << boost::serialization::make_nvp("Height.scale", pReg->m_Height.scale);
		ar << boost::serialization::make_nvp("Height.offset", pReg->m_Height.offset);
		DWORD argb = pReg->m_Color.GetValue(); ar << BOOST_SERIALIZATION_NVP(argb);
		ar << boost::serialization::make_nvp("ImageStr", pReg->m_ImageStr);
		ar << boost::serialization::make_nvp("ImageRect.X", pReg->m_ImageRect.X);
		ar << boost::serialization::make_nvp("ImageRect.Y", pReg->m_ImageRect.Y);
		ar << boost::serialization::make_nvp("ImageRect.Width", pReg->m_ImageRect.Width);
		ar << boost::serialization::make_nvp("ImageRect.Height", pReg->m_ImageRect.Height);
		ar << boost::serialization::make_nvp("ImageBorder.x", pReg->m_ImageBorder.x);
		ar << boost::serialization::make_nvp("ImageBorder.y", pReg->m_ImageBorder.y);
		ar << boost::serialization::make_nvp("ImageBorder.z", pReg->m_ImageBorder.z);
		ar << boost::serialization::make_nvp("ImageBorder.w", pReg->m_ImageBorder.w);
		Gdiplus::FontFamily family; pReg->m_Font->GetFamily(&family); CString strFamily; family.GetFamilyName(strFamily.GetBufferSetLength(LF_FACESIZE)); strFamily.ReleaseBuffer(); ar << BOOST_SERIALIZATION_NVP(strFamily);
		float fSize = pReg->m_Font->GetSize(); ar << BOOST_SERIALIZATION_NVP(fSize);
		argb = pReg->m_FontColor.GetValue(); ar << boost::serialization::make_nvp("TextARGB", argb);
		ar << boost::serialization::make_nvp("Text", pReg->m_Text);
		ar << boost::serialization::make_nvp("TextAlign", pReg->m_TextAlign);
		ar << boost::serialization::make_nvp("TextWrap", pReg->m_TextWrap);
		ar << boost::serialization::make_nvp("TextOff.x", pReg->m_TextOff.x);
		ar << boost::serialization::make_nvp("TextOff.y", pReg->m_TextOff.y);

		CImgRegionDocFileVersions::SerializeSubTreeNode(pDoc, ar, hItem);
	}

	template<class Archive>
	void load(Archive& ar, const unsigned int version)
	{
		ar >> boost::serialization::make_nvp("Class", pReg->m_Class);
		ar >> boost::serialization::make_nvp("Locked", pReg->m_Locked);
		ar >> boost::serialization::make_nvp("Visible", pReg->m_Visible);
		ar >> boost::serialization::make_nvp("x.scale", pReg->m_x.scale);
		ar >> boost::serialization::make_nvp("x.offset", pReg->m_x.offset);
		ar >> boost::serialization::make_nvp("y.scale", pReg->m_y.scale);
		ar >> boost::serialization::make_nvp("y.offset", pReg->m_y.offset);
		ar >> boost::serialization::make_nvp("Width.scale", pReg->m_Width.scale);
		ar >> boost::serialization::make_nvp("Width.offset", pReg->m_Width.offset);
		ar >> boost::serialization::make_nvp("Height.scale", pReg->m_Height.scale);
		ar >> boost::serialization::make_nvp("Height.offset", pReg->m_Height.offset);
		DWORD argb; ar >> BOOST_SERIALIZATION_NVP(argb); pReg->m_Color.SetValue(argb);
		ar >> boost::serialization::make_nvp("ImageStr", pReg->m_ImageStr); pReg->m_Image = theApp.GetImage(pReg->m_ImageStr);
		ar >> boost::serialization::make_nvp("ImageRect.X", pReg->m_ImageRect.X);
		ar >> boost::serialization::make_nvp("ImageRect.Y", pReg->m_ImageRect.Y);
		ar >> boost::serialization::make_nvp("ImageRect.Width", pReg->m_ImageRect.Width);
		ar >> boost::serialization::make_nvp("ImageRect.Height", pReg->m_ImageRect.Height);
		ar >> boost::serialization::make_nvp("ImageBorder.x", pReg->m_ImageBorder.x);
		ar >> boost::serialization::make_nvp("ImageBorder.y", pReg->m_ImageBorder.y);
		ar >> boost::serialization::make_nvp("ImageBorder.z", pReg->m_ImageBorder.z);
		ar >> boost::serialization::make_nvp("ImageBorder.w", pReg->m_ImageBorder.w);
		CString strFamily; float fSize; ar >> BOOST_SERIALIZATION_NVP(strFamily);
		ar >> BOOST_SERIALIZATION_NVP(fSize); pReg->m_Font = theApp.GetFont(strFamily, fSize);
		ar >> boost::serialization::make_nvp("TextARGB", argb); pReg->m_FontColor.SetValue(argb);
		ar >> boost::serialization::make_nvp("Text", pReg->m_Text);
		ar >> boost::serialization::make_nvp("TextAlign", pReg->m_TextAlign);
		ar >> boost::serialization::make_nvp("TextWrap", pReg->m_TextWrap);
		ar >> boost::serialization::make_nvp("TextOff.x", pReg->m_TextOff.x);
		ar >> boost::serialization::make_nvp("TextOff.y", pReg->m_TextOff.y);

		CImgRegionDocFileVersions::SerializeLoadSubTreeNode(pDoc, ar, hItem, bOverideName);
	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}
};

void CImgRegionDocFileVersions::SerializeLoadImgRegion(CImgRegion* pReg, boost::archive::polymorphic_iarchive& ar, CImgRegionDoc* pDoc, HTREEITEM hItem, BOOL bOverideName)
{
	CImgRegionWrapper ImgRegion = { pReg, pDoc, hItem, bOverideName };
	ar >> BOOST_SERIALIZATION_NVP(ImgRegion);
}

void CImgRegionDocFileVersions::SerializeImgRegion(CImgRegion* pReg, boost::archive::polymorphic_oarchive& ar, CImgRegionDoc* pDoc, HTREEITEM hItem)
{
	CImgRegionWrapper ImgRegion = { pReg, pDoc, hItem, FALSE };
	ar << BOOST_SERIALIZATION_NVP(ImgRegion);
}
