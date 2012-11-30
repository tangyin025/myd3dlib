#pragma once

class CImageView : public CView
{
protected:
	const CSize m_ExtentLog;

	CSize m_ExtentDev;

	CSize m_totalLog;

	CSize m_totalDev;

	CSize m_pageDev;

	CSize m_lineDev;

	BOOL m_bInsideUpdate;

	std::tr1::shared_ptr<CWindowDC> m_wndDc;

public:
	DECLARE_DYNAMIC(CImageView)

	CImageView(void);

	DECLARE_MESSAGE_MAP()

	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);

	void DoPrepareDC(CDC* pDC, BOOL bIsPrinting = FALSE);

	void SetScrollSizes(const CSize & sizeTotal);

	void UpdateScrollContext(void);

	void UpdateViewportOrg(void);

	void UpdateBars(const CPoint & ptDesiredMove);

	BOOL GetTrueClientSize(CSize& size, CSize& sizeSb);

	void GetScrollBarSizes(CSize& sizeSb);

	void GetScrollBarState(const CSize & sizeClient, const CPoint & ptDesiredMove,
		CSize & needSb, CSize & sizeRange, CPoint & ptMove, BOOL bInsideClient);

	CPoint GetDeviceScrollPosition() const;

	void ScrollToDevicePosition(POINT ptDev);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);

	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);

	afx_msg BOOL OnMouseWheel(UINT fFlags, short zDelta, CPoint point);

	BOOL DoMouseWheel(UINT fFlags, short zDelta, CPoint point);

	void CheckScrollBars(BOOL& bHasHorzBar, BOOL& bHasVertBar) const;

	void SetZoomFactor(float factor, const CPoint & ptLocalLook);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};
