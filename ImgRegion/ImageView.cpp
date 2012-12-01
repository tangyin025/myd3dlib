#include "StdAfx.h"
#include "ImageView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CImageView, CView)

BEGIN_MESSAGE_MAP(CImageView, CView)
	ON_WM_CREATE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()

CImageView::CImageView(void)
{
}

BOOL CImageView::PreCreateWindow(CREATESTRUCT& cs)
{
	if(!CView::PreCreateWindow(cs))
		return FALSE;

	cs.style = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL;

	return TRUE;
}

int CImageView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CImageView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// Get the minimum and maximum scroll-bar positions.
	int minpos;
	int maxpos;
	GetScrollRange(SB_HORZ, &minpos, &maxpos); 
	maxpos = GetScrollLimit(SB_HORZ);

	// Get the current position of scroll box.
	int curpos = GetScrollPos(SB_HORZ);
	int oldpos = curpos;

	// Determine the new position of scroll box.
	switch (nSBCode)
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
			GetScrollInfo(SB_HORZ, &info, SIF_ALL);

			if (curpos > minpos)
				curpos = max(minpos, curpos - (int) info.nPage);
		}
		break;

	case SB_PAGERIGHT:      // Scroll one page right.
		{
			// Get the page size. 
			SCROLLINFO   info;
			GetScrollInfo(SB_HORZ, &info, SIF_ALL);

			if (curpos < maxpos)
				curpos = min(maxpos, curpos + (int) info.nPage);
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
	SetScrollPos(SB_HORZ, curpos);

	ScrollWindow(oldpos - curpos, 0);
}

void CImageView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// Get the minimum and maximum scroll-bar positions.
	int minpos;
	int maxpos;
	GetScrollRange(SB_VERT, &minpos, &maxpos); 
	maxpos = GetScrollLimit(SB_VERT);

	// Get the current position of scroll box.
	int curpos = GetScrollPos(SB_VERT);
	int oldpos = curpos;

	// Determine the new position of scroll box.
	switch (nSBCode)
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
			GetScrollInfo(SB_VERT, &info, SIF_ALL);

			if (curpos > minpos)
				curpos = max(minpos, curpos - (int) info.nPage);
		}
		break;

	case SB_PAGERIGHT:      // Scroll one page right.
		{
			// Get the page size. 
			SCROLLINFO   info;
			GetScrollInfo(SB_VERT, &info, SIF_ALL);

			if (curpos < maxpos)
				curpos = min(maxpos, curpos + (int) info.nPage);
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
	SetScrollPos(SB_VERT, curpos);

	ScrollWindow(0, oldpos - curpos);
}

void CImageView::SetScrollSizes(const CSize & sizeTotal, BOOL bRedraw, const CPoint & scrollPos)
{
	CRect rectClient;
	GetClientRect(&rectClient);

	CRect rectCanvas;
	if(rectClient.Width() > sizeTotal.cx)
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
	if(rectClient.Height() > sizeTotal.cy)
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

void CImageView::PrepareDC(CDC * pDC, const CRect & rectImageLog, const CRect & rectImageDev)
{
	m_oldMapMode = pDC->SetMapMode(MM_ISOTROPIC);
	m_oldWindowExt = pDC->SetWindowExt(rectImageLog.Size());
	m_oldWindowOrg = pDC->SetWindowOrg(rectImageLog.TopLeft());
	m_oldViewportExt = pDC->SetViewportExt(rectImageDev.Size());
	m_oldViewportOrg = pDC->SetViewportOrg(rectImageDev.TopLeft());
}

void CImageView::RestoreDC(CDC * pDC)
{
	pDC->SetMapMode(m_oldMapMode);
	pDC->SetWindowExt(m_oldWindowExt);
	pDC->SetWindowOrg(m_oldWindowOrg);
	pDC->SetViewportExt(m_oldViewportExt);
	pDC->SetViewportOrg(m_oldViewportOrg);
}

my::Vector2 CImageView::MapPoint(const my::Vector2 & point, const CRect & rectImageSrc, const CRect & rectImageDst)
{
	return my::Vector2(
		my::Lerp((float)rectImageDst.left, (float)rectImageDst.right, (point.x - rectImageSrc.left) / rectImageSrc.Width()),
		my::Lerp((float)rectImageDst.top, (float)rectImageDst.bottom, (point.y - rectImageSrc.top) / rectImageSrc.Height()));
}
