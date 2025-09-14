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
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


void CAtlasView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int nBar = HIWORD(nSBCode);

	// Get the minimum and maximum scroll-bar positions.
	int minpos;
	int maxpos;
	GetScrollRange(nBar, &minpos, &maxpos);
	maxpos = GetScrollLimit(nBar);

	// Get the current position of scroll box.
	int curpos = GetScrollPos(nBar);

	// Determine the new position of scroll box.
	switch (LOWORD(nSBCode))
	{
	case SB_LEFT:      // Scroll to far left.
		curpos = minpos;
		break;

	case SB_RIGHT:      // Scroll to far right.
		curpos = maxpos;
		break;

	case SB_ENDSCROLL:   // End scroll.
		break;

	case SB_LINELEFT:      // Scroll left.
		if (curpos > minpos)
			curpos--;
		break;

	case SB_LINERIGHT:   // Scroll right.
		if (curpos < maxpos)
			curpos++;
		break;

	case SB_PAGELEFT:    // Scroll one page left.
	{
		// Get the page size. 
		SCROLLINFO   info;
		GetScrollInfo(nBar, &info, SIF_ALL);

		if (curpos > minpos)
			curpos = max(minpos, curpos - (int)info.nPage);
	}
	break;

	case SB_PAGERIGHT:      // Scroll one page right.
	{
		// Get the page size. 
		SCROLLINFO   info;
		GetScrollInfo(nBar, &info, SIF_ALL);

		if (curpos < maxpos)
			curpos = min(maxpos, curpos + (int)info.nPage);
	}
	break;

	case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
		curpos = nPos;      // of the scroll box at the end of the drag operation.
		break;

	case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
		curpos = nPos;     // position that the scroll box has been dragged to.
		break;
	}

	// Set the new position of the thumb (scroll box).
	switch (nBar)
	{
	case SB_HORZ:
		ScrollToPos(CPoint(curpos, GetScrollPos(SB_VERT)), TRUE);
		break;

	case SB_VERT:
		ScrollToPos(CPoint(GetScrollPos(SB_HORZ), curpos), TRUE);
		break;
	}
}

void CAtlasView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	OnHScroll(MAKELONG(nSBCode, SB_VERT), nPos, pScrollBar);
}

void CAtlasView::SetScrollSizes(const CSize& sizeTotal, BOOL bRedraw, const CPoint& scrollPos)
{
	CRect rectClient;
	GetClientRect(&rectClient);

	CRect rectCanvas;
	if (rectClient.Width() > sizeTotal.cx)
	{
		rectCanvas.left = sizeTotal.cx / 2 - rectClient.Width() / 2;
		rectCanvas.right = rectCanvas.left + rectClient.Width();
		EnableScrollBar(SB_HORZ, ESB_DISABLE_BOTH);
	}
	else
	{
		rectCanvas.left = 0;
		rectCanvas.right = sizeTotal.cx;
		EnableScrollBar(SB_HORZ, ESB_ENABLE_BOTH);
	}
	if (rectClient.Height() > sizeTotal.cy)
	{
		rectCanvas.top = sizeTotal.cy / 2 - rectClient.Height() / 2;
		rectCanvas.bottom = rectCanvas.top + rectClient.Height();
		EnableScrollBar(SB_VERT, ESB_DISABLE_BOTH);
	}
	else
	{
		rectCanvas.top = 0;
		rectCanvas.bottom = sizeTotal.cy;
		EnableScrollBar(SB_VERT, ESB_ENABLE_BOTH);
	}

	SCROLLINFO info;
	info.cbSize = sizeof(info);
	info.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	info.nMin = rectCanvas.left;
	info.nMax = rectCanvas.right;
	info.nPage = rectClient.Width();
	info.nPos = scrollPos.x;
	SetScrollInfo(SB_HORZ, &info, bRedraw);

	info.nMin = rectCanvas.top;
	info.nMax = rectCanvas.bottom;
	info.nPage = rectClient.Height();
	info.nPos = scrollPos.y;
	SetScrollInfo(SB_VERT, &info, bRedraw);
}

void CAtlasView::ScrollToPos(const CPoint& scrollPos, BOOL bRedraw)
{
	CPoint ptOrg(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	SetScrollPos(SB_HORZ, scrollPos.x, bRedraw);

	SetScrollPos(SB_VERT, scrollPos.y, bRedraw);

	if (bRedraw)
		ScrollWindow(ptOrg.x - GetScrollPos(SB_HORZ), ptOrg.y - GetScrollPos(SB_VERT));
}


void CAtlasView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	CAtlasWnd* pParent = DYNAMIC_DOWNCAST(CAtlasWnd, GetParent());
	ASSERT(pParent);
	if (pParent->m_bgimage)
	{
		CSize ImageSize(pParent->m_bgimage->GetWidth(), pParent->m_bgimage->GetHeight());
		SetScrollSizes(ImageSize, TRUE, CPoint(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT)));
	}
}

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

	CAtlasWnd* pParent = DYNAMIC_DOWNCAST(CAtlasWnd, GetParent());
	ASSERT(pParent);
	if (pParent->m_bgimage)
	{
		CSize ImageSize(pParent->m_bgimage->GetWidth(), pParent->m_bgimage->GetHeight());

		Gdiplus::Matrix world;
		world.Translate(-(float)GetScrollPos(SB_HORZ), -(float)GetScrollPos(SB_VERT));
		grap.SetTransform(&world);

		grap.DrawImage(pParent->m_bgimage.get(), Gdiplus::Rect(0, 0, ImageSize.cx, ImageSize.cy));

		boost::ptr_vector<CImgRegion>::iterator reg_iter = pParent->m_regs.begin();
		for (; reg_iter != pParent->m_regs.end(); reg_iter++)
		{
			Gdiplus::Pen pen(Gdiplus::Color(255, 255, 255, 0));
			grap.DrawRectangle(&pen, reg_iter->m_Rect);
		}
	}
}


void CAtlasView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CAtlasWnd* pParent = DYNAMIC_DOWNCAST(CAtlasWnd, GetParent());
	ASSERT(pParent);

	CPoint localPt(point.x + GetScrollPos(SB_HORZ), point.y + GetScrollPos(SB_VERT));

	boost::ptr_vector<CImgRegion>::reverse_iterator reg_iter = pParent->m_regs.rbegin();
	for (; reg_iter != pParent->m_regs.rend(); reg_iter++)
	{
		if (reg_iter->m_Rect.Contains(localPt.x, localPt.y))
		{
			break;
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
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

	// 读取XML文件内容到内存
	std::string xml_contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	rapidxml::xml_document<char> doc;
	doc.parse<0>(&xml_contents[0]);
	rapidxml::xml_node<char>* boost_serialization = doc.first_node("boost_serialization");
	rapidxml::xml_node<char>* ImageStr = boost_serialization->first_node("ImageStr");
	m_bgimage.reset(new Gdiplus::Image(CA2W(ImageStr->value(), CP_UTF8)));

	Gdiplus::Rect rect(
		0, 0,
		atoi(boost_serialization->first_node("Size.cx")->value()),
		atoi(boost_serialization->first_node("Size.cy")->value())
	);

	m_regs.clear();

	LoadImgRegion(boost_serialization->first_node("ImgRegion"), &rect);

	m_viewAtlas.Invalidate();
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
