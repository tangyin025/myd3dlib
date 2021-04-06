
// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include "PropertiesWnd.h"
#include "EnvironmentWnd.h"
#include "OutputWnd.h"
#include "ScriptWnd.h"
#include "Actor.h"
#include "PhysxContext.h"
#include "LuaExtension.h"
#include "Pivot.h"

class CMainFrame : public CFrameWndEx
	, public PhysxScene
	, public my::OctRoot
	, public LuaContext
{
	
public: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
protected:
	CSplitterWnd m_wndSplitter;
public:
	BOOL m_bEatAltUp;
	CPropertiesWnd    m_wndProperties;
	CEnvironmentWnd		m_wndEnvironment;
	COutputWnd        m_wndOutput;
	CScriptWnd		  m_wndScript;
	CString m_strPathName;
	Pivot m_Pivot;
	typedef std::vector<Actor *> SelActorList;
	SelActorList m_selactors;
	typedef std::set<ActorPtr> ActorPtrSet;
	ActorPtrSet m_ActorList;
	CPoint m_selchunkid;
	my::AABB m_selbox;
	//EmitterComponentPtr m_emitter;
	my::EventSignal m_EventSelectionChanged;
	my::EventSignal m_EventSelectionPlaying;
	my::EventSignal m_EventPivotModeChanged;
	my::EventSignal m_EventAttributeChanged;
	my::EventSignal m_EventCameraPropChanged;

// Operations
public:

// Overrides
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	void UpdateSelBox(void);
	void UpdatePivotTransform(void);
	void OnFrameTick(float fElapsedTime);
	void OnSelChanged();
	void InitFileContext();
	void ClearFileContext();
	BOOL OpenFileContext(LPCTSTR lpszFileName);
	BOOL SaveFileContext(LPCTSTR lpszPathName);
	bool ExecuteCode(const char * code);
	void AddEntity(my::OctEntity * entity, const my::AABB & aabb, float minblock, float threshold);
	bool RemoveEntity(my::OctEntity * entity);
	void OnMeshComponentReady(my::EventArg* arg);

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

public:
	afx_msg void OnDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnCreateActor();
	afx_msg void OnCreateCharacter();
	afx_msg void OnUpdateCreateCharacter(CCmdUI* pCmdUI);
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
	afx_msg void OnViewClearshader();
	afx_msg void OnToolsBuildnavigation();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
};


