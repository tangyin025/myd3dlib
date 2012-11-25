#pragma once

class CImageView : public CView
{
public:
	CImageView(void);

	DECLARE_MESSAGE_MAP()

	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);

	void DoPrepareDC(CDC* pDC, BOOL bIsPrinting = FALSE);

	void SetScrollSizes(const CSize & sizeTotal);

	void UpdateScrollSize(void);

	void UpdateBars(void);

	BOOL GetTrueClientSize(CSize& size, CSize& sizeSb);

	void GetScrollBarSizes(CSize& sizeSb);

	void GetScrollBarState(CSize sizeClient, CSize& needSb,
		CSize& sizeRange, CPoint& ptMove, BOOL bInsideClient);

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

	int m_ScaleIdx;

	CSize m_totalLog;

	CSize m_totalDev;

	CSize m_pageDev;

	CSize m_lineDev;

	BOOL m_bInsideUpdate;
};
