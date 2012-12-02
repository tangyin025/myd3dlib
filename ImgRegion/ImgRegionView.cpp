#include "stdafx.h"
#include "ImgRegionView.h"
#include "MainApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CImgRegionView, CImageView)

BEGIN_MESSAGE_MAP(CImgRegionView, CImageView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_COMMAND(ID_ZOOM_IN, &CImgRegionView::OnZoomIn)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_IN, &CImgRegionView::OnUpdateZoomIn)
	ON_COMMAND(ID_ZOOM_OUT, &CImgRegionView::OnZoomOut)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_OUT, &CImgRegionView::OnUpdateZoomOut)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

CImgRegionView::CImgRegionView(void)
	: m_nCurrCursor(CursorTypeArrow)
	, m_nCurrImageSize(10)
	, m_DragState(DragStateNone)
{
}

CImgRegionDoc * CImgRegionView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CImgRegionDoc)));
	return (CImgRegionDoc *)m_pDocument;
}

void CImgRegionView::OnDraw(CDC * pDC)
{
	CImgRegionDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	PrepareDC(pDC, pDoc->m_root->m_rc,
		CRect(CPoint(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT)), m_ImageSizeTable[m_nCurrImageSize]));

	DrawRegionNode(pDC, pDoc->m_root.get());

	CRegionNodePtr SelectedNode = pDoc->m_SelectedNode.lock();
	CPoint ptTopLeft;
	if(SelectedNode && pDoc->LocalToRoot(SelectedNode.get(), CPoint(0,0), ptTopLeft))
	{
		CRect rect(ptTopLeft, SelectedNode->m_rc.Size());
		pDC->LPtoDP(&rect.TopLeft());
		pDC->LPtoDP(&rect.BottomRight());
		RestoreDC(pDC);

		CPen penDash(PS_DASH, 1, RGB(0,0,255));
		CPen * oldPen = pDC->SelectObject(&penDash);
		int oldBk = pDC->SetBkMode(TRANSPARENT);
		DrawRectHandle(pDC, rect);

		CPen penSolid(PS_SOLID, 1, RGB(0,0,255));
		pDC->SelectObject(&penSolid);
		CPoint ptCenter = rect.CenterPoint();
		DrawSmallHandle(pDC, CPoint(rect.left, rect.top));
		DrawSmallHandle(pDC, CPoint(ptCenter.x, rect.top));
		DrawSmallHandle(pDC, CPoint(rect.right, rect.top));
		DrawSmallHandle(pDC, CPoint(rect.left, ptCenter.y));
		DrawSmallHandle(pDC, CPoint(rect.right, ptCenter.y));
		DrawSmallHandle(pDC, CPoint(rect.left, rect.bottom));
		DrawSmallHandle(pDC, CPoint(ptCenter.x, rect.bottom));
		DrawSmallHandle(pDC, CPoint(rect.right, rect.bottom));

		pDC->SetBkMode(oldBk);
		pDC->SelectObject(oldPen);
	}
}

void CImgRegionView::DrawRectHandle(CDC * pDC, const CRect & rectHandle)
{
	pDC->MoveTo(rectHandle.left, rectHandle.top);
	pDC->LineTo(rectHandle.right, rectHandle.top);
	pDC->LineTo(rectHandle.right, rectHandle.bottom);
	pDC->LineTo(rectHandle.left, rectHandle.bottom);
	pDC->LineTo(rectHandle.left, rectHandle.top);
}

void CImgRegionView::DrawSmallHandle(CDC * pDC, const CPoint & ptHandle)
{
	const int HANDLE_WIDTH = 4;
	CRect rectHandle(ptHandle.x - HANDLE_WIDTH, ptHandle.y - HANDLE_WIDTH, ptHandle.x + HANDLE_WIDTH, ptHandle.y + HANDLE_WIDTH);
	pDC->Rectangle(&rectHandle);
}

void CImgRegionView::DrawRegionNode(CDC * pDC, const CRegionNode * node, const CPoint & ptOff)
{
	CRect rectNode(node->m_rc);
	rectNode.OffsetRect(ptOff);

	if(node->m_image.IsNull())
		pDC->FillSolidRect(rectNode, node->m_color);
	else
		node->m_image.Draw(pDC->m_hDC, rectNode);

	CString strInfo;
	strInfo.Format(_T("x:%d y:%d w:%d h:%d"), node->m_rc.left, node->m_rc.top, node->m_rc.Width(), node->m_rc.Height());
	pDC->DrawText(strInfo, rectNode, DT_SINGLELINE | DT_LEFT | DT_TOP | DT_NOCLIP);

	CRegionNodePtrList::const_iterator child_iter = node->m_childs.begin();
	for(; child_iter != node->m_childs.end(); child_iter++)
	{
		DrawRegionNode(pDC, child_iter->get(), CPoint(ptOff.x + node->m_rc.left, ptOff.y + node->m_rc.top));
	}
}

BOOL CImgRegionView::OnEraseBkgnd(CDC* pDC)
{
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(&rectClient, RGB(197,197,197));
	return TRUE;
}

int CImgRegionView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CImageView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_hCursor[CursorTypeArrow] = theApp.LoadCursor(IDC_CURSOR1);
	m_hCursor[CursorTypeCross] = theApp.LoadCursor(IDC_CURSOR2);

	return 0;
}

void CImgRegionView::OnInitialUpdate()
{
	CImageView::OnInitialUpdate();

	CImgRegionDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	for(int i = 0; i < _countof(m_ImageSizeTable); i++)
	{
		m_ImageSizeTable[i] = CSize(
			(int)((pDoc->m_root->m_rc.right - pDoc->m_root->m_rc.left) * ZoomTable[i]),
			(int)((pDoc->m_root->m_rc.bottom - pDoc->m_root->m_rc.top) * ZoomTable[i]));
	}

	SetScrollSizes(m_ImageSizeTable[m_nCurrImageSize]);
}

void CImgRegionView::OnSize(UINT nType, int cx, int cy)
{
	CImageView::OnSize(nType, cx, cy);

	SetScrollSizes(m_ImageSizeTable[m_nCurrImageSize], TRUE, CPoint(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT)));
}

void CImgRegionView::OnZoomIn()
{
	CRect rectClient;
	GetClientRect(rectClient);

	ZoomImage(m_nCurrImageSize - 1, rectClient.CenterPoint());
}

void CImgRegionView::OnUpdateZoomIn(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nCurrImageSize > 0);
}

void CImgRegionView::OnZoomOut()
{
	CRect rectClient;
	GetClientRect(rectClient);

	ZoomImage(m_nCurrImageSize + 1, rectClient.CenterPoint());
}

void CImgRegionView::OnUpdateZoomOut(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nCurrImageSize < _countof(m_ImageSizeTable) - 1);
}

void CImgRegionView::ZoomImage(int ImageSizeIdx, const CPoint & ptLook, BOOL bRedraw)
{
	CImgRegionDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CRect SrcImageRect(CPoint(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT)), m_ImageSizeTable[m_nCurrImageSize]);

	m_nCurrImageSize = max(0, min((int)_countof(m_ImageSizeTable) - 1, ImageSizeIdx));

	my::Vector2 center = MapPoint(my::Vector2((float)ptLook.x, (float)ptLook.y), SrcImageRect, CRect(CPoint(0, 0), m_ImageSizeTable[m_nCurrImageSize]));

	SetScrollSizes(m_ImageSizeTable[m_nCurrImageSize], bRedraw, CPoint((int)(center.x - ptLook.x), (int)(center.y - ptLook.y)));

	if(bRedraw)
		Invalidate(TRUE);
}

void CImgRegionView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_SPACE:
		if(!((1 << 14) & nFlags) && DragStateNone == m_DragState)
		{
			m_nCurrCursor = CursorTypeCross;
			SetCursor(m_hCursor[m_nCurrCursor]);
		}
		break;
	}
}

void CImgRegionView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_SPACE:
		if(CursorTypeCross == m_nCurrCursor && DragStateImage != m_DragState)
		{
			m_nCurrCursor = CursorTypeArrow;
			SetCursor(m_hCursor[m_nCurrCursor]);
		}
		break;
	}
}

void CImgRegionView::OnLButtonDown(UINT nFlags, CPoint point)
{
	switch(m_nCurrCursor)
	{
	case CursorTypeCross:
		ASSERT(DragStateNone == m_DragState);
		m_DragState = DragStateImage;
		m_DragImagePos = point;
		m_DragImageScrollPos = CPoint(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
		SetCapture();
		break;

	default:
		{
			CImgRegionDoc * pDoc = GetDocument();
			ASSERT_VALID(pDoc);
			if (!pDoc)
				return;

			my::Vector2 ptLocal = MapPoint(my::Vector2((float)point.x, (float)point.y),
				CRect(CPoint(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT)), m_ImageSizeTable[m_nCurrImageSize]), pDoc->m_root->m_rc);

			pDoc->m_SelectedNode = CRegionNode::GetPointedRegion(pDoc->m_root, CPoint((int)ptLocal.x, (int)ptLocal.y));

			Invalidate(TRUE);
		}
		m_DragState = DragStateControl;
		SetCapture();
		break;
	}
}

void CImgRegionView::OnLButtonUp(UINT nFlags, CPoint point)
{
	switch(m_nCurrCursor)
	{
	case CursorTypeCross:
		ASSERT(DragStateImage == m_DragState);
		m_DragState = DragStateNone;
		if(0 == HIBYTE(GetKeyState(VK_SPACE)))
		{
			m_nCurrCursor = CursorTypeArrow;
		}
		ReleaseCapture();
		break;

	default:
		m_DragState = DragStateNone;
		ReleaseCapture();
		break;
	}
}

void CImgRegionView::OnMouseMove(UINT nFlags, CPoint point)
{
	switch(m_DragState)
	{
	case DragStateImage:
		{
			CSize sizeDrag = point - m_DragImagePos;

			ScrollToPos(CPoint(m_DragImageScrollPos.x - sizeDrag.cx, m_DragImageScrollPos.y - sizeDrag.cy), TRUE);
		}
		break;
	}
}

BOOL CImgRegionView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	switch(m_nCurrCursor)
	{
	case CursorTypeCross:
		{
			int nToScroll = ::MulDiv(-zDelta, 1, WHEEL_DELTA);
			ScreenToClient(&pt);
			ZoomImage(m_nCurrImageSize + nToScroll, pt);
		}
		break;
	}
	return TRUE;
}

BOOL CImgRegionView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	SetCursor(m_hCursor[m_nCurrCursor]);

	return TRUE;
}
