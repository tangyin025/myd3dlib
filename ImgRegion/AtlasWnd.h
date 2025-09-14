// Copyright (c) 2011-2024 tangyin025
// License: MIT

#pragma once

#include <boost/ptr_container/ptr_vector.hpp>

class CPropertiesToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*)GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CAtlasView : public CWnd
{
public:
	DECLARE_DYNCREATE(CAtlasView)

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	void SetScrollSizes(const CSize& sizeTotal, BOOL bRedraw = TRUE, const CPoint& scrollPos = CPoint(0, 0));

	void ScrollToPos(const CPoint& scrollPos, BOOL bRedraw = TRUE);

	CAtlasView(void);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};

// CAtlasWnd

namespace rapidxml {
	template <class Ch>
	class xml_node;
}

class CImgRegion;

class CAtlasWnd : public CDockablePane
{
	friend CAtlasView;

	DECLARE_DYNAMIC(CAtlasWnd)

public:
	CAtlasWnd();
	virtual ~CAtlasWnd();

protected:
	DECLARE_MESSAGE_MAP()
	CPropertiesToolBar m_wndToolBar;
	CAtlasView m_viewAtlas;
	boost::shared_ptr<Gdiplus::Image> m_bgimage;
	boost::ptr_vector<CImgRegion> m_regs;
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void AdjustLayout();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLoadAtlas();
	void LoadImgRegion(rapidxml::xml_node<char>* node, const Gdiplus::Rect* rect);
};


