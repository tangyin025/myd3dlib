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

	CString strInfo;
	strInfo.Format(_T("x:%d y:%d w:%d h:%d"), m_rc.left, m_rc.top, m_rc.Width(), m_rc.Height());
	//DWORD bkMode = pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(strInfo, rectNode, DT_SINGLELINE | DT_LEFT | DT_TOP | DT_NOCLIP);
	//pDC->SetBkMode(bkMode);

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
