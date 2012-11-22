#include "ImgRegionView.h"

IMPLEMENT_DYNCREATE(CImgRegionView, CView)

BEGIN_MESSAGE_MAP(CImgRegionView, CView)
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
}
