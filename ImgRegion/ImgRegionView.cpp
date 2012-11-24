#include "stdafx.h"
#include "ImgRegionView.h"

IMPLEMENT_DYNCREATE(CImgRegionView, CView)

BEGIN_MESSAGE_MAP(CImgRegionView, CView)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
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

	CRect rectClient;
	GetClientRect(rectClient);
	pDC->FillSolidRect(rectClient, RGB(192,192,192));

	pDoc->m_root->Draw(pDC, CPoint(0,0));
}

BOOL CImgRegionView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

int CImgRegionView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CImgRegionDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return -1;

	SetScrollSizes(MM_TEXT, CSize(pDoc->m_root->m_rc.Width(), pDoc->m_root->m_rc.Height()));

	return 0;
}
