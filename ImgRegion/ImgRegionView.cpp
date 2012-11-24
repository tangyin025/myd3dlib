#include "stdafx.h"
#include "ImgRegionView.h"

BEGIN_MESSAGE_MAP(CImageView, CView)
END_MESSAGE_MAP()

CImageView::CImageView(void)
{
}

void CImageView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	CView::OnPrepareDC(pDC, pInfo);
}

IMPLEMENT_DYNCREATE(CImgRegionView, CImageView)

BEGIN_MESSAGE_MAP(CImgRegionView, CImageView)
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

	pDoc->m_root->Draw(pDC, CPoint(0,0));
}

BOOL CImgRegionView::OnEraseBkgnd(CDC* pDC)
{
	return CImageView::OnEraseBkgnd(pDC);
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
}

void CImgRegionView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
}
