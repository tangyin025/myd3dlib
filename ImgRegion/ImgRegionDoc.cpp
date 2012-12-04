#include "stdafx.h"
#include "ImgRegionDoc.h"
//
//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

IMPLEMENT_DYNCREATE(CImgRegionDoc, CDocument)

BEGIN_MESSAGE_MAP(CImgRegionDoc, CDocument)
END_MESSAGE_MAP()

CImgRegionDoc::CImgRegionDoc(void)
{
	m_root.reset(new CImgRegionNode(CRect(0,0,500,500),_T("Root"), Gdiplus::Color(255,255,0,0)));

	m_root->m_image.reset(Gdiplus::Image::FromFile(L"Checker.bmp"));

	m_root->m_border = Vector4i(100,50,100,50);

	Gdiplus::FontFamily fontFamily(L"Arial");

	m_root->m_font.reset(new Gdiplus::Font(&fontFamily, 12, Gdiplus::FontStyleBold, Gdiplus::UnitPoint));
}

BOOL CImgRegionDoc::OnNewDocument(void)
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	CImgRegionNodePtr node1(new CImgRegionNode(CRect(100,100,200,200), _T("aaa"), Gdiplus::Color(255,255,0,0)));

	node1->m_font = m_root->m_font;

	CImgRegionNodePtr node2(new CImgRegionNode(CRect(25,25,75,75), _T("aaa"), Gdiplus::Color(255,0,255,0)));

	node2->m_font = m_root->m_font;

	node2->m_image.reset(Gdiplus::Image::FromFile(L"com_btn_normal.png"));

	node2->m_border = Vector4i(7,7,7,7);

	CImgRegionNode::InsertChild(m_root, node1);

	CImgRegionNode::InsertChild(node1, node2);

	m_SelectedNode = node1;

	return TRUE;
}

BOOL CImgRegionDoc::LocalToRoot(const CImgRegionNode * node, const CPoint & ptLocal, CPoint & ptResult)
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
