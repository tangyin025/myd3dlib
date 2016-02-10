
// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include "OutlinerWnd.h"
#include "PropertiesWnd.h"
#include "Component/Component.h"
#include "Component/PhysXContext.h"
#include "Pivot.h"
#include "EventDefine.h"

class CMainFrame : public CFrameWndEx
	, public PhysXSceneContext
{
	
public: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

	static CMainFrame & getSingleton(void)
	{
		return *getSingletonPtr();
	}

	static CMainFrame * getSingletonPtr(void)
	{
		return static_cast<CMainFrame *>(PhysXSceneContext::getSingletonPtr());
	}

// Attributes
protected:
	CSplitterWnd m_wndSplitter;
public:
	BOOL m_bEatAltUp;
	//COutlinerWnd		m_wndOutliner;
	CPropertiesWnd    m_wndProperties;
	CString m_strPathName;
	CRectTracker m_Tracker;
	Pivot m_Pivot;
	my::OctRoot m_Root;
	typedef std::vector<ComponentPtr> ComponentPtrList;
	ComponentPtrList m_cmps;
	typedef std::set<Component *> ComponentSet;
	ComponentSet m_selcmps;
	my::AABB m_selbox;
	EmitterComponentPtr m_emitter;
	Event m_EventSelectionChanged;
	Event m_EventSelectionPlaying;
	Event m_EventPivotModeChanged;
	Event m_EventCmpAttriChanged;

// Operations
public:

// Overrides
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	void UpdateSelBox(void);
	void UpdatePivotTransform(void);

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
	afx_msg void OnComponentEmitter();
	afx_msg void OnComponentSphericalemitter();
	afx_msg void OnRigidSphere();
	afx_msg void OnRigidPlane();
	afx_msg void OnRigidCapsule();
	afx_msg void OnRigidBox();
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateEditDelete(CCmdUI *pCmdUI);
	afx_msg void OnPivotMove();
	afx_msg void OnUpdatePivotMove(CCmdUI *pCmdUI);
	afx_msg void OnPivotRotate();
	afx_msg void OnUpdatePivotRotate(CCmdUI *pCmdUI);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};


