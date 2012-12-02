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
	, m_nSelectedHandle(HandleTypeNone)
{
}

CImgRegionDoc * CImgRegionView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CImgRegionDoc)));
	return (CImgRegionDoc *)m_pDocument;
}

static const COLORREF HANDLE_COLOR = RGB(0,0,255);

void CImgRegionView::OnDraw(CDC * pDC)
{
	CRect rectClient;
	GetClientRect(&rectClient);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, rectClient.Width(), rectClient.Height());
	CDC dcMemory;
	dcMemory.CreateCompatibleDC(pDC);
	CBitmap * oldBmp = dcMemory.SelectObject(&bmp);
	dcMemory.FillSolidRect(&rectClient, RGB(192,192,192));

	CImgRegionDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	PrepareDC(&dcMemory, pDoc->m_root->m_rc,
		CRect(CPoint(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT)), m_ImageSizeTable[m_nCurrImageSize]));

	DrawRegionNode(&dcMemory, pDoc->m_root.get());

	CImgRegionNodePtr SelectedNode = pDoc->m_SelectedNode.lock();
	ASSERT(SelectedNode != pDoc->m_root);
	CPoint ptTopLeft;
	if(SelectedNode && pDoc->LocalToRoot(SelectedNode.get(), CPoint(0,0), ptTopLeft))
	{
		CRect rect(ptTopLeft, SelectedNode->m_rc.Size());
		dcMemory.LPtoDP(&rect.TopLeft());
		dcMemory.LPtoDP(&rect.BottomRight());
		RestoreDC(&dcMemory);

		CPen penDash(PS_DASH, 1, HANDLE_COLOR);
		CPen * oldPen = dcMemory.SelectObject(&penDash);
		int oldBk = dcMemory.SetBkMode(TRANSPARENT);
		DrawRectHandle(&dcMemory, rect);

		CPen penSolid(PS_SOLID, 1, HANDLE_COLOR);
		dcMemory.SelectObject(&penSolid);
		CPoint ptCenter = rect.CenterPoint();
		DrawSmallHandle(&dcMemory, CPoint(rect.left, rect.top), m_nSelectedHandle == HandleTypeLeftTop);
		DrawSmallHandle(&dcMemory, CPoint(ptCenter.x, rect.top), m_nSelectedHandle == HandleTypeCenterTop);
		DrawSmallHandle(&dcMemory, CPoint(rect.right, rect.top), m_nSelectedHandle == HandleTypeRightTop);
		DrawSmallHandle(&dcMemory, CPoint(rect.left, ptCenter.y), m_nSelectedHandle == HandleTypeLeftMiddle);
		DrawSmallHandle(&dcMemory, CPoint(rect.right, ptCenter.y), m_nSelectedHandle == HandleTypeRightMiddle);
		DrawSmallHandle(&dcMemory, CPoint(rect.left, rect.bottom), m_nSelectedHandle == HandleTypeLeftBottom);
		DrawSmallHandle(&dcMemory, CPoint(ptCenter.x, rect.bottom), m_nSelectedHandle == HandleTypeCenterBottom);
		DrawSmallHandle(&dcMemory, CPoint(rect.right, rect.bottom), m_nSelectedHandle == HandleTypeRightBottom);

		dcMemory.SelectObject(oldPen);
	}
	else
		RestoreDC(&dcMemory);

	pDC->BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &dcMemory, 0, 0, SRCCOPY);

	dcMemory.SelectObject(oldBmp);
}

void CImgRegionView::DrawRectHandle(CDC * pDC, const CRect & rectHandle)
{
	pDC->MoveTo(rectHandle.left, rectHandle.top);
	pDC->LineTo(rectHandle.right, rectHandle.top);
	pDC->LineTo(rectHandle.right, rectHandle.bottom);
	pDC->LineTo(rectHandle.left, rectHandle.bottom);
	pDC->LineTo(rectHandle.left, rectHandle.top);
}

static const int HANDLE_WIDTH = 4;

void CImgRegionView::DrawSmallHandle(CDC * pDC, const CPoint & ptHandle, BOOL bSelected)
{
	CRect rectHandle(ptHandle.x - HANDLE_WIDTH, ptHandle.y - HANDLE_WIDTH, ptHandle.x + HANDLE_WIDTH, ptHandle.y + HANDLE_WIDTH);
	bSelected ? pDC->FillSolidRect(&rectHandle, HANDLE_COLOR) : pDC->Rectangle(&rectHandle);
}

BOOL CImgRegionView::CheckSmallHandle(const CPoint & ptHandle, const CPoint & ptMouse)
{
	CRect rectHandle(ptHandle.x - HANDLE_WIDTH, ptHandle.y - HANDLE_WIDTH, ptHandle.x + HANDLE_WIDTH, ptHandle.y + HANDLE_WIDTH);
	return rectHandle.PtInRect(ptMouse);
}

void CImgRegionView::DrawRegionNode(CDC * pDC, const CImgRegionNode * node, const CPoint & ptOff)
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

	CImgRegionNodePtrList::const_iterator child_iter = node->m_childs.begin();
	for(; child_iter != node->m_childs.end(); child_iter++)
	{
		DrawRegionNode(pDC, child_iter->get(), CPoint(ptOff.x + node->m_rc.left, ptOff.y + node->m_rc.top));
	}
}

BOOL CImgRegionView::OnEraseBkgnd(CDC* pDC)
{
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

	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
		if(DragStateNone == m_DragState)
		{
			CImgRegionDoc * pDoc = GetDocument();
			ASSERT_VALID(pDoc);
			if (!pDoc)
				return;

			CSize dragOff(
				nChar == VK_LEFT ? -1 : (nChar == VK_RIGHT ? 1 : 0),
				nChar == VK_UP ? -1 : (nChar == VK_DOWN ? 1 : 0));

			CImgRegionNodePtr SelectedNode = pDoc->m_SelectedNode.lock();
			if(SelectedNode)
			{
				switch(m_nSelectedHandle)
				{
				case HandleTypeLeftTop:
					m_DragHandlePos.x = (SelectedNode->m_rc.left += dragOff.cx);
					m_DragHandlePos.y = (SelectedNode->m_rc.top += dragOff.cy);
					break;
				case HandleTypeCenterTop:
					m_DragHandlePos.y = (SelectedNode->m_rc.top += dragOff.cy);
					break;
				case HandleTypeRightTop:
					m_DragHandlePos.x = (SelectedNode->m_rc.right += dragOff.cx);
					m_DragHandlePos.y = (SelectedNode->m_rc.top += dragOff.cy);
					break;
				case HandleTypeLeftMiddle:
					m_DragHandlePos.x = (SelectedNode->m_rc.left += dragOff.cx);
					break;
				case HandleTypeRightMiddle:
					m_DragHandlePos.x = (SelectedNode->m_rc.right += dragOff.cx);
					break;
				case HandleTypeLeftBottom:
					m_DragHandlePos.x = (SelectedNode->m_rc.left += dragOff.cx);
					m_DragHandlePos.y = (SelectedNode->m_rc.bottom += dragOff.cy);
					break;
				case HandleTypeCenterBottom:
					m_DragHandlePos.y = (SelectedNode->m_rc.bottom += dragOff.cy);
					break;
				case HandleTypeRightBottom:
					m_DragHandlePos.x = (SelectedNode->m_rc.right += dragOff.cx);
					m_DragHandlePos.y = (SelectedNode->m_rc.bottom += dragOff.cy);
					break;
				default:
					SelectedNode->m_rc.OffsetRect(dragOff);
					m_DragControlPos = SelectedNode->m_rc.TopLeft();
					break;
				}

				Invalidate(TRUE);

				pDoc->UpdateAllViews(this);
			}
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
		m_DragPos = point;
		m_DragScrollPos = CPoint(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
		SetCapture();
		break;

	default:
		{
			CImgRegionDoc * pDoc = GetDocument();
			ASSERT_VALID(pDoc);
			if (!pDoc)
				return;

			CImgRegionNodePtr SelectedNode = pDoc->m_SelectedNode.lock();
			CPoint ptTopLeft;
			if(SelectedNode && pDoc->LocalToRoot(SelectedNode.get(), CPoint(0,0), ptTopLeft))
			{
				CRect rect(ptTopLeft, SelectedNode->m_rc.Size());
				CWindowDC dc(this);
				PrepareDC(&dc, pDoc->m_root->m_rc,
					CRect(CPoint(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT)), m_ImageSizeTable[m_nCurrImageSize]));
				dc.LPtoDP(&rect.TopLeft());
				dc.LPtoDP(&rect.BottomRight());

				CPoint ptCenter = rect.CenterPoint();
				if(CheckSmallHandle(CPoint(rect.left, rect.top), point))
				{
					m_nSelectedHandle = HandleTypeLeftTop;
					m_DragHandlePos.SetPoint(SelectedNode->m_rc.left, SelectedNode->m_rc.top);
				}
				else if(CheckSmallHandle(CPoint(ptCenter.x, rect.top), point))
				{
					m_nSelectedHandle = HandleTypeCenterTop;
					m_DragHandlePos.SetPoint(SelectedNode->m_rc.left + SelectedNode->m_rc.Width() / 2, SelectedNode->m_rc.top);
				}
				else if(CheckSmallHandle(CPoint(rect.right, rect.top), point))
				{
					m_nSelectedHandle = HandleTypeRightTop;
					m_DragHandlePos.SetPoint(SelectedNode->m_rc.right, SelectedNode->m_rc.top);
				}
				else if(CheckSmallHandle(CPoint(rect.left, ptCenter.y), point))
				{
					m_nSelectedHandle = HandleTypeLeftMiddle;
					m_DragHandlePos.SetPoint(SelectedNode->m_rc.left, SelectedNode->m_rc.top + SelectedNode->m_rc.Height() / 2);
				}
				else if(CheckSmallHandle(CPoint(rect.right, ptCenter.y), point))
				{
					m_nSelectedHandle = HandleTypeRightMiddle;
					m_DragHandlePos.SetPoint(SelectedNode->m_rc.right, SelectedNode->m_rc.top + SelectedNode->m_rc.Height() / 2);
				}
				else if(CheckSmallHandle(CPoint(rect.left, rect.bottom), point))
				{
					m_nSelectedHandle = HandleTypeLeftBottom;
					m_DragHandlePos.SetPoint(SelectedNode->m_rc.left, SelectedNode->m_rc.bottom);
				}
				else if(CheckSmallHandle(CPoint(ptCenter.x, rect.bottom), point))
				{
					m_nSelectedHandle = HandleTypeCenterBottom;
					m_DragHandlePos.SetPoint(SelectedNode->m_rc.left + SelectedNode->m_rc.Width() / 2, SelectedNode->m_rc.bottom);
				}
				else if(CheckSmallHandle(CPoint(rect.right, rect.bottom), point))
				{
					m_nSelectedHandle = HandleTypeRightBottom;
					m_DragHandlePos.SetPoint(SelectedNode->m_rc.right, SelectedNode->m_rc.bottom);
				}
				else
				{
					m_nSelectedHandle = HandleTypeNone;
				}
			}

			if(HandleTypeNone == m_nSelectedHandle)
			{
				// 由于dc.DPtoLP所得的结果被四啥五入，所以使用MapPoint获得更精确的结果
				my::Vector2 ptLocal = MapPoint(my::Vector2((float)point.x, (float)point.y),
					CRect(CPoint(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT)), m_ImageSizeTable[m_nCurrImageSize]), pDoc->m_root->m_rc);

				SelectedNode = CImgRegionNode::GetPointedRegion(pDoc->m_root, CPoint((int)ptLocal.x, (int)ptLocal.y));
			}

			if(SelectedNode && SelectedNode != pDoc->m_root)
			{
				pDoc->m_SelectedNode = SelectedNode;
				m_DragState = DragStateControl;
				m_DragPos = point;
				m_DragControlPos = SelectedNode->m_rc.TopLeft();
				SetCapture();
			}
			else
			{
				pDoc->m_SelectedNode.reset();
				m_DragState = DragStateNone;
			}

			Invalidate(TRUE);

			pDoc->UpdateAllViews(this);
		}
		break;
	}
}

void CImgRegionView::OnLButtonUp(UINT nFlags, CPoint point)
{
	switch(m_DragState)
	{
	case DragStateImage:
		m_DragState = DragStateNone;
		if(0 == HIBYTE(GetKeyState(VK_SPACE)))
		{
			m_nCurrCursor = CursorTypeArrow;
		}
		ReleaseCapture();
		break;

	case DragStateControl:
		{
			m_DragState = DragStateNone;
			ReleaseCapture();

			CImgRegionDoc * pDoc = GetDocument();
			ASSERT_VALID(pDoc);
			if (!pDoc)
				return;

			pDoc->UpdateAllViews(this);
		}
		break;
	}
}

void CImgRegionView::OnMouseMove(UINT nFlags, CPoint point)
{
	switch(m_DragState)
	{
	case DragStateImage:
		{
			CSize sizeDrag = point - m_DragPos;

			ScrollToPos(CPoint(m_DragScrollPos.x - sizeDrag.cx, m_DragScrollPos.y - sizeDrag.cy), TRUE);
		}
		break;

	case DragStateControl:
		{
			CImgRegionDoc * pDoc = GetDocument();
			ASSERT_VALID(pDoc);
			if (!pDoc)
				return;

			CSize sizeDrag = point - m_DragPos;

			my::Vector2 dragOff = MapPoint(my::Vector2((float)sizeDrag.cx, (float)sizeDrag.cy),
				CRect(CPoint(0, 0), m_ImageSizeTable[m_nCurrImageSize]), pDoc->m_root->m_rc);

			CImgRegionNodePtr SelectedNode = pDoc->m_SelectedNode.lock();
			ASSERT(SelectedNode);
			switch(m_nSelectedHandle)
			{
			case HandleTypeLeftTop:
				SelectedNode->m_rc.left = m_DragHandlePos.x + (int)dragOff.x;
				SelectedNode->m_rc.top = m_DragHandlePos.y + (int)dragOff.y;
				break;
			case HandleTypeCenterTop:
				SelectedNode->m_rc.top = m_DragHandlePos.y + (int)dragOff.y;
				break;
			case HandleTypeRightTop:
				SelectedNode->m_rc.right = m_DragHandlePos.x + (int)dragOff.x;
				SelectedNode->m_rc.top = m_DragHandlePos.y + (int)dragOff.y;
				break;
			case HandleTypeLeftMiddle:
				SelectedNode->m_rc.left = m_DragHandlePos.x + (int)dragOff.x;
				break;
			case HandleTypeRightMiddle:
				SelectedNode->m_rc.right = m_DragHandlePos.x + (int)dragOff.x;
				break;
			case HandleTypeLeftBottom:
				SelectedNode->m_rc.left = m_DragHandlePos.x + (int)dragOff.x;
				SelectedNode->m_rc.bottom = m_DragHandlePos.y + (int)dragOff.y;
				break;
			case HandleTypeCenterBottom:
				SelectedNode->m_rc.bottom = m_DragHandlePos.y + (int)dragOff.y;
				break;
			case HandleTypeRightBottom:
				SelectedNode->m_rc.right = m_DragHandlePos.x + (int)dragOff.x;
				SelectedNode->m_rc.bottom = m_DragHandlePos.y + (int)dragOff.y;
				break;
			default:
				SelectedNode->m_rc.MoveToXY(m_DragControlPos.x + (int)dragOff.x, m_DragControlPos.y + (int)dragOff.y);
				break;
			}

			Invalidate(TRUE);
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
