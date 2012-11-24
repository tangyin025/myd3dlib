#include "stdafx.h"
#include "ImgRegionDoc.h"

void CRegionNode::Draw(CDC * pDC, const CPoint & ptOff)
{
	CRect rectNode(m_rc);
	rectNode.OffsetRect(ptOff);

	if(m_image.IsNull())
		pDC->FillSolidRect(rectNode, m_color);
	else
		m_image.Draw(pDC->m_hDC, rectNode);

	CRegionNodePtrList::const_iterator child_iter = m_childs.begin();
	for(; child_iter != m_childs.end(); child_iter++)
	{
		(*child_iter)->Draw(pDC, CPoint(ptOff.x + m_rc.left, ptOff.y + m_rc.top));
	}
}

IMPLEMENT_DYNCREATE(CImgRegionDoc, CDocument)

BEGIN_MESSAGE_MAP(CImgRegionDoc, CDocument)
END_MESSAGE_MAP()

CImgRegionDoc::CImgRegionDoc(void)
{
	m_root.reset(new CRegionNode(CRect(0,0,500,500),"Root", RGB(255,255,255)));

	HRESULT hres = m_root->m_image.Load(_T("Checker.bmp"));
}

BOOL CImgRegionDoc::OnNewDocument(void)
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	CRegionNodePtr node1(new CRegionNode(CRect(100,100,200,200), "aaa", RGB(255,0,0)));

	CRegionNodePtr node2(new CRegionNode(CRect(25,25,75,75), "aaa", RGB(0,255,0)));

	node1->m_childs.push_back(node2);

	m_root->m_childs.push_back(node1);

	return TRUE;
}
