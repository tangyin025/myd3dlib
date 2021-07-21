
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

class dtNavMesh;

class dtNavMeshQuery;

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
	typedef std::vector<ActorPtr> ActorPtrSet;
	ActorPtrSet m_ActorList;
	Pivot m_Pivot;
	typedef std::vector<Actor *> SelActorList;
	SelActorList m_selactors;
	Component * m_selcmp;
	CPoint m_selchunkid;
	int m_selinstid;
	my::AABB m_selbox;
	enum PaintType
	{
		PaintTypeNone,
		PaintTypeTerrainHeightField,
		PaintTypeTerrainColor,
		PaintTypeEmitterInstance,
	};
	PaintType m_PaintType;
	enum PaintShape
	{
		PaintShapeCircle,
	};
	PaintShape m_PaintShape;
	enum PaintMode
	{
		PaintModeGreater,
	};
	PaintMode m_PaintMode;
	float m_PaintRadius;
	float m_PaintHeight;
	my::Spline m_PaintSpline;
	D3DXCOLOR m_PaintColor;
	float m_PaintParticleMinDist;
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
	void OnMeshComponentReady(my::DeviceResourceBasePtr res, boost::weak_ptr<MeshComponent> mesh_cmp_weak_ptr);
	Component* GetSelComponent(Component::ComponentType Type);

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
	afx_msg void OnCreateController();
	afx_msg void OnUpdateCreateController(CCmdUI* pCmdUI);
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
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnPaintTerrainHeightField();
	afx_msg void OnUpdatePaintTerrainHeightField(CCmdUI* pCmdUI);
	afx_msg void OnPaintTerrainColor();
	afx_msg void OnUpdatePaintTerrainColor(CCmdUI* pCmdUI);
	afx_msg void OnPaintEmitterinstance();
	afx_msg void OnUpdatePaintEmitterinstance(CCmdUI* pCmdUI);
	afx_msg void OnComponentAnimator();
	afx_msg void OnUpdateComponentAnimator(CCmdUI* pCmdUI);
	afx_msg void OnCreateNavigation();
	afx_msg void OnUpdateCreateNavigation(CCmdUI* pCmdUI);
};


