#include "stdafx.h"
#include "ImgRegionView.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(CImgRegionView, CImageView)

BEGIN_MESSAGE_MAP(CImgRegionView, CImageView)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_COMMAND(ID_ZOOM_IN, &CImgRegionView::OnZoomIn)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_IN, &CImgRegionView::OnUpdateZoomIn)
	ON_COMMAND(ID_ZOOM_OUT, &CImgRegionView::OnZoomOut)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_OUT, &CImgRegionView::OnUpdateZoomOut)
END_MESSAGE_MAP()

CImgRegionView::CImgRegionView(void)
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

static DWORD ScaleTable[] = { 32000, 16000, 12000, 8000, 7000, 6000, 5000, 4000, 3000, 2000, 1000, 667, 500, 333, 250, 167, 125, 83, 62, 50, 40, 30, 20, 15, 10, 7 };

void CImgRegionView::OnZoomOut(void)
{
	m_ScaleIdx = min(_countof(ScaleTable) - 1, m_ScaleIdx + 1);
	UpdateScrollSize();
}

void CImgRegionView::OnUpdateZoomOut(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_ScaleIdx < _countof(ScaleTable) - 1);
}
