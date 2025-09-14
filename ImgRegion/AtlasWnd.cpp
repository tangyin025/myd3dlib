// Copyright (c) 2011-2024 tangyin025
// License: MIT

#include "stdafx.h"
#include "AtlasWnd.h"
#include "MainApp.h"
#include "ImgRegionDoc.h"
#include "../rapidxml/include/rapidxml.hpp"
//
//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

IMPLEMENT_DYNCREATE(CAtlasView, CWnd)

CAtlasView::CAtlasView(void)
{

}

BEGIN_MESSAGE_MAP(CAtlasView, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()


void CAtlasView::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: Add your message handler code here
					   // Do not call CWnd::OnPaint() for painting messages

	CRect rectClient;
	GetClientRect(&rectClient);

	Gdiplus::Graphics grap(dc.GetSafeHdc());

	Gdiplus::SolidBrush bkBrush(Gdiplus::Color(255, 192, 192, 192));
	grap.FillRectangle(&bkBrush, Gdiplus::Rect(rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height()));
}

// CAtlasWnd

IMPLEMENT_DYNAMIC(CAtlasWnd, CDockablePane)

CAtlasWnd::CAtlasWnd()
{

}

CAtlasWnd::~CAtlasWnd()
{
}


BEGIN_MESSAGE_MAP(CAtlasWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_LOAD_ATLAS, &CAtlasWnd::OnLoadAtlas)
END_MESSAGE_MAP()



// CAtlasWnd message handlers




int CAtlasWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create combo:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_TOOLBAR1);
	m_wndToolBar.LoadToolBar(IDR_TOOLBAR1, 0, 0, TRUE /* Is locked */);
	//m_wndToolBar.CleanUpLockedImages();
	//m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	m_viewAtlas.Create(NULL, NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | WS_HSCROLL | WS_VSCROLL, rectDummy, this, 3);
	AdjustLayout();
	return 0;
}


void CAtlasWnd::AdjustLayout()
{
	// TODO: Add your implementation code here.
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient, rectCombo;
	GetClientRect(rectClient);

	int cyCmb = 0;
	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);

	m_viewAtlas.SetWindowPos(&m_wndToolBar, rectClient.left, rectClient.top + cyCmb + cyTlb, rectClient.Width(), rectClient.Height() - cyCmb - cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
}


void CAtlasWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	AdjustLayout();
}


void CAtlasWnd::OnLoadAtlas()
{
	// TODO: Add your command handler code here
	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, 0);
	if (IDOK != dlgFile.DoModal())
	{
		return;
	}

	std::ifstream file(dlgFile.GetPathName());
	if (!file)
	{
		return;
	}

	// ��ȡXML�ļ����ݵ��ڴ�
	std::string xml_contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	rapidxml::xml_document<char> doc;
	doc.parse<0>(&xml_contents[0]);
	rapidxml::xml_node<char>* boost_serialization = doc.first_node("boost_serialization");
	rapidxml::xml_node<char>* ImageStr = boost_serialization->first_node("ImageStr");
	m_bgimage.reset(new Gdiplus::Image(CA2W(ImageStr->value())));

	Gdiplus::Rect rect(
		0, 0,
		atoi(boost_serialization->first_node("Size.cx")->value()),
		atoi(boost_serialization->first_node("Size.cy")->value())
	);

	LoadImgRegion(boost_serialization->first_node("ImgRegion"), &rect);
}


void CAtlasWnd::LoadImgRegion(rapidxml::xml_node<char>* node, const Gdiplus::Rect* rect)
{
	// TODO: Add your implementation code here.
	for (; node; node = node->next_sibling("ImgRegion"))
	{
		CImgRegion* reg = new CImgRegion();
		reg->m_x.scale = (float)atof(node->first_node("x.scale")->value());
		reg->m_x.offset = (float)atof(node->first_node("x.offset")->value());
		reg->m_y.scale = (float)atof(node->first_node("y.scale")->value());
		reg->m_y.offset = (float)atof(node->first_node("y.offset")->value());
		reg->m_Width.scale = (float)atof(node->first_node("Width.scale")->value());
		reg->m_Width.offset = (float)atof(node->first_node("Width.offset")->value());
		reg->m_Height.scale = (float)atof(node->first_node("Height.scale")->value());
		reg->m_Height.offset = (float)atof(node->first_node("Height.offset")->value());

		reg->m_Rect.X = rect->X + reg->m_x.scale * rect->Width + reg->m_x.offset;
		reg->m_Rect.Y = rect->Y + reg->m_y.scale * rect->Height + reg->m_y.offset;
		reg->m_Rect.Width = reg->m_Width.scale * rect->Width + reg->m_Width.offset;
		reg->m_Rect.Height = reg->m_Height.scale * rect->Height + reg->m_Height.offset;

		m_regs.push_back(reg);

		LoadImgRegion(node->first_node("ImgRegion"), &reg->m_Rect);
	}
}
