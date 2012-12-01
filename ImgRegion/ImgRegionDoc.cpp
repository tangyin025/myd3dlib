#include "stdafx.h"
#include "ImgRegionDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void CRegionNode::SetParent(CRegionNodePtr child, CRegionNodePtr parent)
{
	parent->m_childs.push_back(child);

	child->m_Parent = parent;
}

CPoint CRegionNode::LocalToRoot(CRegionNodePtr node, const CPoint & ptLocal)
{
	CPoint ptRet(node->m_rc.left + ptLocal.x, node->m_rc.top + ptLocal.y);

	CRegionNodePtr parent = node->m_Parent.lock();

	if(parent)
		return LocalToRoot(parent, ptRet);

	return ptRet;
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

	CRegionNode::SetParent(node1, m_root);

	CRegionNode::SetParent(node2, node1);

	return TRUE;
}
