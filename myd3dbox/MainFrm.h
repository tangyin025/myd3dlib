
// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include "OutlinerWnd.h"
#include "PropertiesWnd.h"
#include "../demo2_3/Component/Component.h"
#include "PivotController.h"

struct EventArg
{
public:
	EventArg(void)
	{
	}

	virtual ~EventArg(void)
	{
	}
};

class CMainFrame : public CFrameWndEx
	, public my::SingleInstance<CMainFrame>
{
	
public: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
protected:
	CSplitterWnd m_wndSplitter;
public:
	BOOL m_bEatAltUp;
	COutlinerWnd		m_wndOutliner;
	CPropertiesWnd    m_wndProperties;
	CRectTracker m_Tracker;
	PivotController m_Pivot;
	my::OctRoot m_Root;
	typedef std::vector<ComponentPtr> ComponentPtrList;
	ComponentPtrList m_cmps;
	typedef boost::signals2::signal<void (EventArg *)> Event;
	Event m_EventSelectionChanged;
	Event m_EventHistoryChanged;

// Operations
public:

// Overrides
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;
	CMFCToolBarImages m_UserImages;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
	void ClearAllComponents();

public:
	afx_msg void OnDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnComponentMesh();
	afx_msg void OnComponentMeshset();
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateEditDelete(CCmdUI *pCmdUI);
};


