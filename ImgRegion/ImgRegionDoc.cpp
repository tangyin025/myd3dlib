#include "stdafx.h"
#include "ImgRegionDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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

	CRegionNode::InsertChild(m_root, node1);

	CRegionNode::InsertChild(node1, node2);

	m_SelectedNode = node1;

	return TRUE;
}

BOOL CImgRegionDoc::LocalToRoot(const CRegionNode * node, const CPoint & ptLocal, CPoint & ptResult)
{
	if(node)
	{
		if(node == m_root.get())
		{
			ptResult = m_root->m_rc.TopLeft() + ptLocal;
			return TRUE;
		}

		return LocalToRoot(node->m_Parent.lock().get(), node->m_rc.TopLeft() + ptLocal, ptResult);
	}

	return FALSE;
}
