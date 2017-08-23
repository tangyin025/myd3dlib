
// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include "OutlinerWnd.h"
#include "PropertiesWnd.h"
#include "EnvironmentWnd.h"
#include "Component/Actor.h"
#include "Component/PhysXContext.h"
#include "Component/Terrain.h"
#include "Pivot.h"
#include "EventDefine.h"

class CMainFrame : public CFrameWndEx
	, public PhysXSceneContext
{
	
public: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
protected:
	CSplitterWnd m_wndSplitter;
public:
	BOOL m_bEatAltUp;
	//COutlinerWnd		m_wndOutliner;
	CPropertiesWnd    m_wndProperties;
	CEnvironmentWnd		m_wndEnvironment;
	CString m_strPathName;
	CRectTracker m_Tracker;
	Pivot m_Pivot;
	WorldL m_WorldL;
	typedef boost::unordered_set<Actor *> ActorSet;
	ActorSet m_selactors;
	my::AABB m_selbox;
	//EmitterComponentPtr m_emitter;
	Event m_EventSelectionChanged;
	Event m_EventSelectionPlaying;
	Event m_EventPivotModeChanged;
	Event m_EventAttributeChanged;
	Event m_EventCameraPropChanged;

// Operations
public:

// Overrides
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	void SafeAdjustLevelIdAndPosition(CPoint & level_id, my::Vector3 & pos);
	void SafeChangeActorPose(Actor * actor, my::Vector3 Position, const my::Quaternion & Rotation, const my::Vector3 & Scale);
	void UpdateSelBox(void);
	void UpdatePivotTransform(void);
	BOOL OnFrameTick(float fElapsedTime);

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
	void ClearFileContext();

public:
	afx_msg void OnDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnCreateActor();
	afx_msg void OnCreateCharacter();
	afx_msg void OnComponentMesh();
	afx_msg void OnUpdateComponentMesh(CCmdUI *pCmdUI);
	afx_msg void OnComponentCloth();
	afx_msg void OnUpdateComponentCloth(CCmdUI *pCmdUI);
	afx_msg void OnComponentStaticEmitter();
	afx_msg void OnUpdateComponentStaticEmitter(CCmdUI *pCmdUI);
	afx_msg void OnComponentSphericalemitter();
	afx_msg void OnUpdateComponentSphericalemitter(CCmdUI *pCmdUI);
	afx_msg void OnComponentTerrain();
	afx_msg void OnUpdateComponentTerrain(CCmdUI *pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateEditDelete(CCmdUI *pCmdUI);
	afx_msg void OnPivotMove();
	afx_msg void OnUpdatePivotMove(CCmdUI *pCmdUI);
	afx_msg void OnPivotRotate();
	afx_msg void OnUpdatePivotRotate(CCmdUI *pCmdUI);
};


