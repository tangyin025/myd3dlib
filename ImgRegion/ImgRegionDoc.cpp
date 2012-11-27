#include "stdafx.h"
#include "ImgRegionDoc.h"

IMPLEMENT_DYNCREATE(CImgRegionDoc, CDocument)

BEGIN_MESSAGE_MAP(CImgRegionDoc, CDocument)
END_MESSAGE_MAP()

CImgRegionDoc::CImgRegionDoc(void)
{
	m_root.reset(new CRegionNode(CRect(0,0,500,500),_T("Root"), RGB(255,255,255)));

	HRESULT hres = m_root->m_image.Load(_T("Checker.bmp"));
}

BOOL CImgRegionDoc::OnNewDocument(void)
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	CRegionNodePtr node1(new CRegionNode(CRect(100,100,200,200), _T("aaa"), RGB(255,0,0)));

	CRegionNodePtr node2(new CRegionNode(CRect(25,25,75,75), _T("aaa"), RGB(0,255,0)));

	node1->m_childs.push_back(node2);

	m_root->m_childs.push_back(node1);

	return TRUE;
}
