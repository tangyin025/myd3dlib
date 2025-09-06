#pragma once

#include "ImgRegionDoc.h"
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>

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

	static void SerializeLoad(CImgRegionDoc* pDoc, boost::archive::polymorphic_iarchive& ar);

	static void Serialize(CImgRegionDoc* pDoc, boost::archive::polymorphic_oarchive& ar);

	static void SerializeLoadSubTreeNode(CImgRegionDoc* pDoc, boost::archive::polymorphic_iarchive& ar, HTREEITEM hParent, BOOL bOverideName);

	static void SerializeSubTreeNode(CImgRegionDoc* pDoc, boost::archive::polymorphic_oarchive& ar, HTREEITEM hParent);

	static void SerializeLoadImgRegion(CImgRegion* pReg, boost::archive::polymorphic_iarchive& ar, CImgRegionDoc* pDoc, HTREEITEM hItem, BOOL bOverideName);

	static void SerializeImgRegion(CImgRegion* pReg, boost::archive::polymorphic_oarchive& ar, CImgRegionDoc* pDoc, HTREEITEM hItem);
};
