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

CImgRegionView::CImgRegionView(void)
	: m_dwCurrCursor(CursorTypeArrow)
	, m_bPrepareDrag(FALSE)
	, m_bDrag(FALSE)
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

	pDoc->m_root->Draw(pDC, CPoint(0,0));
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

	m_hCursor[CursorTypeArrow] = ::LoadCursor(NULL, IDC_ARROW);
	m_hCursor[CursorTypeHand] = theApp.LoadCursor(IDC_CURSOR1);

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
	m_ScaleIdx = max(0, m_ScaleIdx - 1);
	UpdateScrollSize();
}

void CImgRegionView::OnUpdateZoomIn(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_ScaleIdx > 0);
}

void CImgRegionView::OnZoomOut(void)
{
	m_ScaleIdx = min(_countof(ScaleTable) - 1, m_ScaleIdx + 1);
	UpdateScrollSize();
}

void CImgRegionView::OnUpdateZoomOut(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_ScaleIdx < _countof(ScaleTable) - 1);
}

BOOL CImgRegionView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if(m_bPrepareDrag || m_bDrag)
		SetCursor(m_hCursor[CursorTypeHand]);
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
			SetCursor(m_hCursor[CursorTypeHand]);
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
