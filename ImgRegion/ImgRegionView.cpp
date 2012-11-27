#include "stdafx.h"
#include "ImgRegionView.h"
#include "MainApp.h"

IMPLEMENT_DYNCREATE(CImgRegionView, CImageView)

BEGIN_MESSAGE_MAP(CImgRegionView, CImageView)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_COMMAND(ID_ZOOM_IN, &CImgRegionView::OnZoomIn)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_IN, &CImgRegionView::OnUpdateZoomIn)
	ON_COMMAND(ID_ZOOM_OUT, &CImgRegionView::OnZoomOut)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_OUT, &CImgRegionView::OnUpdateZoomOut)
	ON_WM_SETCURSOR()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

static const float ZoomTable[] = {
	32, 16, 12, 8, 7, 6, 5, 4, 3, 2, 1, 0.667f, 0.500f, 0.333f, 0.250f, 0.167f, 0.125f, 0.083f, 0.062f, 0.050f, 0.040f, 0.030f, 0.020f, 0.015f, 0.010f, 0.007f };

CImgRegionView::CImgRegionView(void)
	: m_dwCurrCursor(CursorTypeArrow)
	, m_bPrepareDrag(FALSE)
	, m_bDrag(FALSE)
	, m_dwZoomIdx(10)
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

	DrawRegionNode(pDC, pDoc->m_root.get());
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
	pDC->FillSolidRect(&rectClient, RGB(192,192,192));
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

	SetScrollSizes(CSize(pDoc->m_root->m_rc.Width(), pDoc->m_root->m_rc.Height()));
}

void CImgRegionView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
}

void CImgRegionView::OnZoomIn(void)
{
	m_dwZoomIdx = max(0, m_dwZoomIdx - 1);

	SetZoomFactor(ZoomTable[m_dwZoomIdx]);
}

void CImgRegionView::OnUpdateZoomIn(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_dwZoomIdx > 0);
}

void CImgRegionView::OnZoomOut(void)
{
	m_dwZoomIdx = min(_countof(ZoomTable) - 1, m_dwZoomIdx + 1);

	SetZoomFactor(ZoomTable[m_dwZoomIdx]);
}

void CImgRegionView::OnUpdateZoomOut(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_dwZoomIdx < _countof(ZoomTable) - 1);
}

BOOL CImgRegionView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if(m_bPrepareDrag || m_bDrag)
		SetCursor(m_hCursor[CursorTypeCross]);
	else
		SetCursor(m_hCursor[m_dwCurrCursor]);

	return TRUE;
}

void CImgRegionView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_SPACE:
		if(!((1 << 14) & nFlags) && !m_bDrag)
		{
			m_bPrepareDrag = TRUE;
			SetCursor(m_hCursor[CursorTypeCross]);
		}
		break;
	}
}

void CImgRegionView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_SPACE:
		if(m_bPrepareDrag)
		{
			m_bPrepareDrag = FALSE;
			if(!m_bDrag)
			{
				SetCursor(m_hCursor[m_dwCurrCursor]);
			}
		}
		break;
	}
}

void CImgRegionView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(m_bPrepareDrag)
	{
		m_bDrag = TRUE;
		m_bDragStartPos = point;
		m_bDragStartScrollPos = GetDeviceScrollPosition();
		SetCapture();
	}
}

void CImgRegionView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(m_bDrag)
	{
		m_bDrag = FALSE;
		ReleaseCapture();
	}
}

void CImgRegionView::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_bDrag)
	{
		CSize sizeClient;
		CSize sizeSb;
		if (!GetTrueClientSize(sizeClient, sizeSb))
		{
			return;
		}
		CSize sizeRange;
		CPoint ptMove;
		CSize needSb;
		GetScrollBarState(sizeClient, needSb, sizeRange, ptMove, TRUE);
		CSize offset = point - m_bDragStartPos;
		ScrollToDevicePosition(CPoint(
			max(0, min(sizeRange.cx, m_bDragStartScrollPos.x - offset.cx)),
			max(0, min(sizeRange.cy, m_bDragStartScrollPos.y - offset.cy))));
	}
}
