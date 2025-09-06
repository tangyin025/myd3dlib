#pragma once

#include "ImgRegionDoc.h"

class CImgRegionDocFileVersions
{
public:
	static const int FILE_VERSION;

	static const CString DEFAULT_CONTROL_NAME;

	static void SerializeLoad(CImgRegionDoc* pDoc, CArchive& ar, int version);

	static void Serialize(CImgRegionDoc * pDoc, CArchive & ar, int version);

	static void SerializeLoadSubTreeNode(CImgRegionDoc* pDoc, CArchive& ar, int version, HTREEITEM hParent, BOOL bOverideName);

	static void SerializeSubTreeNode(CImgRegionDoc * pDoc, CArchive & ar, int version, HTREEITEM hParent, BOOL bOverideName);

	static void SerializeLoadImgRegion(CImgRegion* pReg, CArchive& ar, int version);

	static void SerializeImgRegion(CImgRegion * pReg, CArchive & ar, int version);
};
