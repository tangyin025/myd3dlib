#include "stdafx.h"
#include "ImgRegionView.h"
#include "MainApp.h"
#include "MainFrm.h"

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

static const Gdiplus::Color HANDLE_COLOR(255,0,0,255);

void CImgRegionView::OnDraw(CDC * pDC)
{
	CRect rectClient;
	GetClientRect(&rectClient);

	CImgRegionDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, rectClient.Width(), rectClient.Height());
	CDC dcMemory;
	dcMemory.CreateCompatibleDC(pDC);
	CBitmap * oldBmp = dcMemory.SelectObject(&bmp);
	dcMemory.FillSolidRect(&rectClient, RGB(192,192,192));

	Gdiplus::Graphics grap(dcMemory.GetSafeHdc());
	Gdiplus::SolidBrush bkBrush(Gdiplus::Color(255,192,192,192));
	grap.FillRectangle(&bkBrush, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height());
	grap.TranslateTransform(-(float)GetScrollPos(SB_HORZ), -(float)GetScrollPos(SB_VERT));
	grap.ScaleTransform(
		(float)m_ImageSizeTable[m_nCurrImageSize].cx / pDoc->m_ImageSize.cx,
		(float)m_ImageSizeTable[m_nCurrImageSize].cy / pDoc->m_ImageSize.cy);

	Gdiplus::SolidBrush brush(pDoc->m_BkColor);
	grap.FillRectangle(&brush, 0, 0, pDoc->m_ImageSize.cx, pDoc->m_ImageSize.cy);

	DrawRegionNode(grap, pDoc->m_TreeCtrl.GetRootItem());

	grap.ResetTransform();

	HTREEITEM hSelected = pDoc->m_TreeCtrl.GetSelectedItem();
	CPoint ptTopLeft;
	if(hSelected && pDoc->LocalToRoot(hSelected, CPoint(0,0), ptTopLeft))
	{
		CImgRegion * pReg = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hSelected);
		ASSERT(pReg);

		CRect rect(ptTopLeft, pReg->m_Size);
		CWindowDC dc(this);
		PrepareDC(&dc, CRect(CPoint(0,0), pDoc->m_ImageSize),
			CRect(CPoint(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT)), m_ImageSizeTable[m_nCurrImageSize]));
		dc.LPtoDP(&rect.TopLeft());
		dc.LPtoDP(&rect.BottomRight());

		DrawRectHandle(grap, rect);

		CPoint ptCenter = rect.CenterPoint();
		DrawSmallHandle(grap, CPoint(rect.left, rect.top), m_nSelectedHandle == HandleTypeLeftTop);
		DrawSmallHandle(grap, CPoint(ptCenter.x, rect.top), m_nSelectedHandle == HandleTypeCenterTop);
		DrawSmallHandle(grap, CPoint(rect.right, rect.top), m_nSelectedHandle == HandleTypeRightTop);
		DrawSmallHandle(grap, CPoint(rect.left, ptCenter.y), m_nSelectedHandle == HandleTypeLeftMiddle);
		DrawSmallHandle(grap, CPoint(rect.right, ptCenter.y), m_nSelectedHandle == HandleTypeRightMiddle);
		DrawSmallHandle(grap, CPoint(rect.left, rect.bottom), m_nSelectedHandle == HandleTypeLeftBottom);
		DrawSmallHandle(grap, CPoint(ptCenter.x, rect.bottom), m_nSelectedHandle == HandleTypeCenterBottom);
		DrawSmallHandle(grap, CPoint(rect.right, rect.bottom), m_nSelectedHandle == HandleTypeRightBottom);
	}

	pDC->BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &dcMemory, 0, 0, SRCCOPY);

	dcMemory.SelectObject(oldBmp);
}

void CImgRegionView::DrawRectHandle(Gdiplus::Graphics & grap, const CRect & rectHandle)
{
	Gdiplus::Pen pen(HANDLE_COLOR, 1.0f);
	pen.SetDashStyle(Gdiplus::DashStyleDash);
	float dashValue[] = { 10.0f, 4.0f };
	pen.SetDashPattern(dashValue, _countof(dashValue));
	grap.DrawRectangle(&pen, rectHandle.left, rectHandle.top, rectHandle.Width(), rectHandle.Height());
}

static const int HANDLE_WIDTH = 4;

void CImgRegionView::DrawSmallHandle(Gdiplus::Graphics & grap, const CPoint & ptHandle, BOOL bSelected)
{
	CRect rectHandle(ptHandle.x - HANDLE_WIDTH, ptHandle.y - HANDLE_WIDTH, ptHandle.x + HANDLE_WIDTH, ptHandle.y + HANDLE_WIDTH);
	Gdiplus::Pen pen(HANDLE_COLOR,1.0f);
	Gdiplus::SolidBrush brush(bSelected ? HANDLE_COLOR : Gdiplus::Color(255,255,255,255));
	grap.FillRectangle(&brush, rectHandle.left, rectHandle.top, rectHandle.Width(), rectHandle.Height());
	grap.DrawRectangle(&pen, rectHandle.left, rectHandle.top, rectHandle.Width(), rectHandle.Height());
}

BOOL CImgRegionView::CheckSmallHandle(const CPoint & ptHandle, const CPoint & ptMouse)
{
	CRect rectHandle(ptHandle.x - HANDLE_WIDTH, ptHandle.y - HANDLE_WIDTH, ptHandle.x + HANDLE_WIDTH + 1, ptHandle.y + HANDLE_WIDTH + 1);
	return rectHandle.PtInRect(ptMouse);
}

void CImgRegionView::DrawRegionNode(Gdiplus::Graphics & grap, HTREEITEM hItem, const CPoint & ptOff)
{
	CImgRegionDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if(hItem)
	{
		CImgRegion * pReg = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hItem);
		ASSERT(pReg);

		CPoint nodePos = pReg->m_Local + ptOff;

		if(pReg->m_Image && Gdiplus::ImageTypeUnknown != pReg->m_Image->GetType())
		{
			DrawRegionImage(grap, pReg->m_Image.get(), CRect(nodePos, pReg->m_Size), pReg->m_Border, pReg->m_Color.GetAlpha());
		}
		else
		{
			Gdiplus::SolidBrush brush(pReg->m_Color);
			grap.FillRectangle(&brush, nodePos.x, nodePos.y, pReg->m_Size.cx, pReg->m_Size.cy);
		}

		if(pReg->m_Font)
		{
			CString strInfo;
			strInfo.Format(_T("x:%d y:%d w:%d h:%d"), pReg->m_Local.x, pReg->m_Local.y, pReg->m_Size.cx, pReg->m_Size.cy);

			Gdiplus::RectF rectF((float)nodePos.x, (float)nodePos.y, (float)pReg->m_Size.cx, (float)pReg->m_Size.cy);
			Gdiplus::SolidBrush solidBrush(pReg->m_FontColor);
			Gdiplus::StringFormat strFormat(Gdiplus::StringFormatFlagsNoWrap | Gdiplus::StringFormatFlagsNoClip);
			strFormat.SetTrimming(Gdiplus::StringTrimmingNone);
			grap.DrawString(strInfo, strInfo.GetLength(), pReg->m_Font.get(), rectF, &strFormat, &solidBrush);
		}

		DrawRegionNode(grap, pDoc->m_TreeCtrl.GetChildItem(hItem), CPoint(ptOff.x + pReg->m_Local.x, ptOff.y + pReg->m_Local.y));

		DrawRegionNode(grap, pDoc->m_TreeCtrl.GetNextSiblingItem(hItem), ptOff);
	}
}

void CImgRegionView::DrawRegionImage(Gdiplus::Graphics & grap, Gdiplus::Image * img, const CRect & dstRect, const Vector4i & border, int alpha)
{
		Gdiplus::ColorMatrix colorMatrix = {	1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
												0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
												0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
												0.0f, 0.0f, 0.0f, alpha / 255.0f, 0.0f,
												0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
		Gdiplus::ImageAttributes imageAtt;
		imageAtt.SetColorMatrix(&colorMatrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
		//imageAtt.SetWrapMode(Gdiplus::WrapModeTileFlipXY);
		Gdiplus::InterpolationMode oldInterpolationMode = grap.GetInterpolationMode();
		grap.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
		Gdiplus::PixelOffsetMode oldPixelOffsetMode = grap.GetPixelOffsetMode();
		grap.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);

		grap.DrawImage(img, Gdiplus::Rect(dstRect.left, dstRect.top, border.x, border.y),
			0, 0, border.x, border.y, Gdiplus::UnitPixel, &imageAtt);

		grap.DrawImage(img, Gdiplus::Rect(dstRect.left + border.x, dstRect.top, dstRect.Width() - border.x - border.z, border.y),
			border.x, 0, img->GetWidth() - border.x - border.z, border.y, Gdiplus::UnitPixel, &imageAtt);

		grap.DrawImage(img, Gdiplus::Rect(dstRect.right - border.z, dstRect.top, border.x, border.y),
			img->GetWidth() - border.z, 0, border.z, border.y, Gdiplus::UnitPixel, &imageAtt);

		grap.DrawImage(img, Gdiplus::Rect(dstRect.left, dstRect.top + border.y, border.x, dstRect.Height() - border.y - border.w),
			0, border.y, border.x, img->GetHeight() - border.y - border.w, Gdiplus::UnitPixel, &imageAtt);

		grap.DrawImage(img, Gdiplus::Rect(dstRect.left + border.x, dstRect.top + border.y, dstRect.Width() - border.x - border.z, dstRect.Height() - border.y - border.w),
			border.x, border.y, img->GetWidth() - border.x - border.z, img->GetHeight() - border.y - border.w, Gdiplus::UnitPixel, &imageAtt);

		grap.DrawImage(img, Gdiplus::Rect(dstRect.right - border.z, dstRect.top + border.y, border.x, dstRect.Height() - border.y - border.w),
			img->GetWidth() - border.z, border.y, border.z, img->GetHeight() - border.y - border.w, Gdiplus::UnitPixel, &imageAtt);

		grap.DrawImage(img, Gdiplus::Rect(dstRect.left, dstRect.bottom - border.w, border.x, border.w),
			0, img->GetHeight() - border.w, border.x, border.w, Gdiplus::UnitPixel, &imageAtt);

		grap.DrawImage(img, Gdiplus::Rect(dstRect.left + border.x, dstRect.bottom - border.w, dstRect.Width() - border.x - border.z, border.w),
			border.x, img->GetHeight() - border.w, img->GetWidth() - border.x - border.z, border.w, Gdiplus::UnitPixel, &imageAtt);

		grap.DrawImage(img, Gdiplus::Rect(dstRect.right - border.z, dstRect.bottom - border.w, border.x, border.w),
			img->GetWidth() - border.z, img->GetHeight() - border.w, border.z, border.w, Gdiplus::UnitPixel, &imageAtt);

		grap.SetInterpolationMode(oldInterpolationMode);
		grap.SetPixelOffsetMode(oldPixelOffsetMode);
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

	UpdateImageSizeTable(pDoc->m_ImageSize);

	SetScrollSizes(m_ImageSizeTable[m_nCurrImageSize]);
}

void CImgRegionView::UpdateImageSizeTable(const CSize & sizeRoot)
{
	for(int i = 0; i < _countof(m_ImageSizeTable); i++)
	{
		m_ImageSizeTable[i] = CSize((int)(sizeRoot.cx * ZoomTable[i]), (int)(sizeRoot.cy * ZoomTable[i]));
	}
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


			HTREEITEM hSelected = pDoc->m_TreeCtrl.GetSelectedItem();
			ASSERT(hSelected);
			if(hSelected)
			{
				CImgRegion * pReg = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hSelected);
				ASSERT(pReg);

				switch(m_nSelectedHandle)
				{
				case HandleTypeLeftTop:
					pReg->m_Local.x += dragOff.cx;
					pReg->m_Local.y += dragOff.cy;
					break;
				case HandleTypeCenterTop:
					pReg->m_Local.y += dragOff.cy;
					break;
				case HandleTypeRightTop:
					pReg->m_Size.cx += dragOff.cx;
					pReg->m_Local.y += dragOff.cy;
					break;
				case HandleTypeLeftMiddle:
					pReg->m_Local.x += dragOff.cx;
					break;
				case HandleTypeRightMiddle:
					pReg->m_Size.cx += dragOff.cx;
					break;
				case HandleTypeLeftBottom:
					pReg->m_Local.x += dragOff.cx;
					pReg->m_Size.cy += dragOff.cy;
					break;
				case HandleTypeCenterBottom:
					pReg->m_Size.cy += dragOff.cy;
					break;
				case HandleTypeRightBottom:
					pReg->m_Size.cx += dragOff.cx;
					pReg->m_Size.cy += dragOff.cy;
					break;
				default:
					pReg->m_Local += dragOff;
					break;
				}

				Invalidate(TRUE);

				pDoc->UpdateAllViews(this);

				CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
				ASSERT(pFrame);
				pFrame->m_wndProperties.UpdateProperties();
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
		if(CursorTypeCross == m_nCurrCursor && DragStateScroll != m_DragState)
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
		m_DragState = DragStateScroll;
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

			HTREEITEM hSelected = pDoc->m_TreeCtrl.GetSelectedItem();
			CPoint ptTopLeft;
			if(hSelected && pDoc->LocalToRoot(hSelected, CPoint(0,0), ptTopLeft))
			{
				CImgRegion * pReg = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hSelected);
				ASSERT(pReg);

				CRect rect(ptTopLeft, pReg->m_Size);
				CWindowDC dc(this);
				PrepareDC(&dc, CRect(CPoint(0,0), pDoc->m_ImageSize),
					CRect(CPoint(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT)), m_ImageSizeTable[m_nCurrImageSize]));
				dc.LPtoDP(&rect.TopLeft());
				dc.LPtoDP(&rect.BottomRight());

				CPoint ptCenter = rect.CenterPoint();
				if(CheckSmallHandle(CPoint(rect.left, rect.top), point))
				{
					m_nSelectedHandle = HandleTypeLeftTop;
				}
				else if(CheckSmallHandle(CPoint(ptCenter.x, rect.top), point))
				{
					m_nSelectedHandle = HandleTypeCenterTop;
				}
				else if(CheckSmallHandle(CPoint(rect.right, rect.top), point))
				{
					m_nSelectedHandle = HandleTypeRightTop;
				}
				else if(CheckSmallHandle(CPoint(rect.left, ptCenter.y), point))
				{
					m_nSelectedHandle = HandleTypeLeftMiddle;
				}
				else if(CheckSmallHandle(CPoint(rect.right, ptCenter.y), point))
				{
					m_nSelectedHandle = HandleTypeRightMiddle;
				}
				else if(CheckSmallHandle(CPoint(rect.left, rect.bottom), point))
				{
					m_nSelectedHandle = HandleTypeLeftBottom;
				}
				else if(CheckSmallHandle(CPoint(ptCenter.x, rect.bottom), point))
				{
					m_nSelectedHandle = HandleTypeCenterBottom;
				}
				else if(CheckSmallHandle(CPoint(rect.right, rect.bottom), point))
				{
					m_nSelectedHandle = HandleTypeRightBottom;
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
					CRect(CPoint(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT)), m_ImageSizeTable[m_nCurrImageSize]), CRect(CPoint(0,0), pDoc->m_ImageSize));

				hSelected = pDoc->GetPointedRegionNode(pDoc->m_TreeCtrl.GetRootItem(), CPoint((int)ptLocal.x, (int)ptLocal.y));
			}

			if(hSelected)
			{
				CImgRegion * pReg = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hSelected);
				ASSERT(pReg);

				pDoc->m_TreeCtrl.SelectItem(hSelected);
				m_DragState = DragStateControl;
				m_DragPos = point;
				m_DragRegLocal = pReg->m_Local;
				m_DragRegSize = pReg->m_Size;
				SetCapture();
			}
			else
			{
				pDoc->m_TreeCtrl.SelectItem(NULL);
				m_DragState = DragStateNone;
			}

			Invalidate(TRUE);

			pDoc->UpdateAllViews(this);

			//CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
			//ASSERT(pFrame);
			//pFrame->m_wndProperties.UpdateProperties();
		}
		break;
	}
}

void CImgRegionView::OnLButtonUp(UINT nFlags, CPoint point)
{
	switch(m_DragState)
	{
	case DragStateScroll:
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

			CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
			ASSERT(pFrame);
			pFrame->m_wndProperties.UpdateProperties();
		}
		break;
	}
}

void CImgRegionView::OnMouseMove(UINT nFlags, CPoint point)
{
	switch(m_DragState)
	{
	case DragStateScroll:
		{
			CSize sizeDrag = point - m_DragPos;

			ScrollToPos(m_DragScrollPos - sizeDrag, TRUE);
		}
		break;

	case DragStateControl:
		{
			CImgRegionDoc * pDoc = GetDocument();
			ASSERT_VALID(pDoc);
			if (!pDoc)
				return;

			HTREEITEM hSelected = pDoc->m_TreeCtrl.GetSelectedItem();
			ASSERT(hSelected);
	
			CImgRegion * pReg = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hSelected);
			ASSERT(pReg);

			CSize sizeDrag = point - m_DragPos;

			my::Vector2 dragOff = MapPoint(my::Vector2((float)sizeDrag.cx, (float)sizeDrag.cy),
				CRect(CPoint(0, 0), m_ImageSizeTable[m_nCurrImageSize]), CRect(CPoint(0,0), pDoc->m_ImageSize));

			CSize sizeDragLog((int)dragOff.x, (int)dragOff.y);

			switch(m_nSelectedHandle)
			{
			case HandleTypeLeftTop:
				pReg->m_Local = m_DragRegLocal + sizeDragLog;
				pReg->m_Size = m_DragRegSize - sizeDragLog;
				break;
			case HandleTypeCenterTop:
				pReg->m_Local.y = m_DragRegLocal.y + sizeDragLog.cy;
				pReg->m_Size.cy = m_DragRegSize.cy - sizeDragLog.cy;
				break;
			case HandleTypeRightTop:
				pReg->m_Local.y = m_DragRegLocal.y + sizeDragLog.cy;
				pReg->m_Size.cx = m_DragRegSize.cx + sizeDragLog.cx;
				pReg->m_Size.cy = m_DragRegSize.cy - sizeDragLog.cy;
				break;
			case HandleTypeLeftMiddle:
				pReg->m_Local.x = m_DragRegLocal.x + sizeDragLog.cx;
				pReg->m_Size.cx = m_DragRegSize.cx - sizeDragLog.cx;
				break;
			case HandleTypeRightMiddle:
				pReg->m_Size.cx = m_DragRegSize.cx + sizeDragLog.cx;
				break;
			case HandleTypeLeftBottom:
				pReg->m_Local.x = m_DragRegLocal.x + sizeDragLog.cx;
				pReg->m_Size.cx = m_DragRegSize.cx - sizeDragLog.cx;
				pReg->m_Size.cy = m_DragRegSize.cy + sizeDragLog.cy;
				break;
			case HandleTypeCenterBottom:
				pReg->m_Size.cy = m_DragRegSize.cy + sizeDragLog.cy;
				break;
			case HandleTypeRightBottom:
				pReg->m_Size = m_DragRegSize + sizeDragLog;
				break;
			default:
				pReg->m_Local = m_DragRegLocal + sizeDragLog;
				break;
			}

			Invalidate(TRUE);

			UpdateWindow();
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

void CImgRegionView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CImageView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	if(bActivate && pActivateView)
	{
		CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
		ASSERT(pFrame);
		pFrame->m_wndProperties.UpdateProperties();
	}
}
