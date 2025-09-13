#pragma once

class CPropertiesToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*)GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

// CAtlasView

class CAtlasView : public CDockablePane
{
	DECLARE_DYNAMIC(CAtlasView)

public:
	CAtlasView();
	virtual ~CAtlasView();

protected:
	DECLARE_MESSAGE_MAP()
	CPropertiesToolBar m_wndToolBar;
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void AdjustLayout();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLoadAtlas();
};


