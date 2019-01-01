
// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include "OutlinerWnd.h"
#include "PropertiesWnd.h"
#include "EnvironmentWnd.h"
#include "Actor.h"
#include "PhysXContext.h"
#include "Terrain.h"
#include "Pivot.h"
#include "EventDefine.h"
#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"

class CMainFrame : public CFrameWndEx
	, public PhysXSceneContext
	, public rcContext
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
	my::OctRoot m_Root;
	typedef boost::unordered_set<Actor *> ActorSet;
	ActorSet m_selactors;
	CPoint m_selchunkid;
	my::AABB m_selbox;
	//EmitterComponentPtr m_emitter;
	Event m_EventSelectionChanged;
	Event m_EventSelectionPlaying;
	Event m_EventPivotModeChanged;
	Event m_EventAttributeChanged;
	Event m_EventCameraPropChanged;

	/// These are just sample areas to use consistent values across the samples.
	/// The use should specify these base on his needs.
	enum SamplePolyAreas
	{
		SAMPLE_POLYAREA_GROUND,
		SAMPLE_POLYAREA_WATER,
		SAMPLE_POLYAREA_ROAD,
		SAMPLE_POLYAREA_DOOR,
		SAMPLE_POLYAREA_GRASS,
		SAMPLE_POLYAREA_JUMP,
	};
	enum SamplePolyFlags
	{
		SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
		SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
		SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
		SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
		SAMPLE_POLYFLAGS_DISABLED = 0x10,		// Disabled polygon
		SAMPLE_POLYFLAGS_ALL = 0xffff	// All abilities.
	};
	rcHeightfield* m_solid;
	//unsigned char* m_triareas;
	rcCompactHeightfield* m_chf;
	rcContourSet* m_cset;
	rcPolyMesh* m_pmesh;
	rcConfig m_cfg;
	rcPolyMeshDetail* m_dmesh;
	dtNavMesh* m_navMesh;
	dtNavMeshQuery* m_navQuery;
	//dtCrowd* m_crowd;

// Operations
public:

// Overrides
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	void UpdateSelBox(void);
	void UpdatePivotTransform(void);
	BOOL OnFrameTick(float fElapsedTime);
	void OnSelChanged();

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
	afx_msg void OnViewClearshader();
	afx_msg void OnToolsBuildnavigation();
	afx_msg void OnToolsBuildgrass();
};


