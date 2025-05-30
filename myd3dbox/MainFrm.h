// Copyright (c) 2011-2024 tangyin025
// License: MIT

#pragma once

#include "PropertiesWnd.h"
#include "EnvironmentWnd.h"
#include "OutputWnd.h"
#include "ScriptWnd.h"
#include "OutlinerWnd.h"
#include "Actor.h"
#include "PhysxContext.h"
#include "LuaExtension.h"
#include "Pivot.h"
#include <boost/intrusive/list.hpp>

class dtNavMesh;

class dtNavMeshQuery;

class OffmeshConnectionChunk
	: public my::OctEntity
{
public:
	float m_Verts[3 * 2];
	float m_Rad;
	unsigned char m_Dir;
	unsigned char m_Area;
	unsigned short m_Flag;
	unsigned int m_Id;

	OffmeshConnectionChunk(void)
	{
	}

	virtual ~OffmeshConnectionChunk(void)
	{
	}
};

typedef boost::shared_ptr<OffmeshConnectionChunk> OffmeshConnectionChunkPtr;

class CMainFrame : public CFrameWndEx
	, public my::DialogMgr
	, public my::OctRoot
	, public LuaContext
	, public PhysxScene
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
	COutlinerWnd      m_wndOutliner;
	CString m_strPathName;
	typedef std::vector<ActorPtr> ActorPtrList;
	ActorPtrList m_ActorList;
	typedef std::vector<my::DialogPtr> DialogPtrList;
	DialogPtrList m_DialogList;
	my::Vector3 m_hitPos;
	bool m_hitPosSet;
	my::OctRoot m_offMeshConRoot;
	typedef std::vector<OffmeshConnectionChunkPtr> OffmeshConnectionChunkList;
	OffmeshConnectionChunkList m_offMeshConChunks;
	Pivot m_Pivot;
	typedef std::vector<Actor *> ActorList;
	ActorList m_selactors;
	Component * m_selcmp;
	CPoint m_selchunkid;
	int m_selinstid;
	my::AABB m_selbox;
	typedef std::vector<my::Control *> ControlList;
	ControlList m_selctls;
	typedef boost::intrusive::list<Actor, boost::intrusive::base_hook<boost::intrusive::list_base_hook<boost::intrusive::tag<ViewedActorTag> > > > ViewedActorSet;
	ViewedActorSet m_ViewedActors;
	bool m_ctlcaptured;
	enum ControlHandleType
	{
		ControlHandleNone = 0,
		ControlHandleLeftTop,
		ControlHandleCenterTop,
		ControlHandleRightTop,
		ControlHandleLeftMiddle,
		ControlHandleCenterMiddle,
		ControlHandleRightMiddle,
		ControlHandleLeftBottom,
		ControlHandleCenterBottom,
		ControlHandleRightBottom,
		ControlHandleCount
	};
	ControlHandleType m_ctlhandle;
	my::Vector2 m_ctlhandleoff;
	my::Vector2 m_ctlhandlesz;
	enum PaintType
	{
		PaintTypeNone,
		PaintTypeTerrainHeightField,
		PaintTypeTerrainColor,
		PaintTypeEmitterInstance,
		PaintTypeOffmeshConnections,
	};
	PaintType m_PaintType;
	enum PaintShape
	{
		PaintShapeCircle,
	};
	PaintShape m_PaintShape;
	enum PaintMode
	{
		PaintModeAssign,
	};
	PaintMode m_PaintMode;
	float m_PaintRadius;
	float m_PaintHeight;
	my::Spline m_PaintSpline;
	D3DXCOLOR m_PaintColor;
	int m_PaintEmitterSiblingId;
	float m_PaintParticleMinDist;
	my::Vector2 m_PaintParticleAngle;
	//EmitterComponentPtr m_emitter;
	my::EventSignal m_EventSelectionChanged;
	my::EventSignal m_EventSelectionPlaying;
	my::EventSignal m_EventPivotModeChanged;
	my::EventSignal m_EventAttributeChanged;
	my::EventSignal m_EventCameraPropChanged;
	std::vector<std::string> m_ToolScripts;
	CChildView* m_RenderingView;
	my::Vector3 m_IndicatorCoord;
	ActorPtr m_Player;

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
	void RemoveEntity(my::OctEntity * entity);
	void addOffMeshConnection(const my::Vector3& v0, const my::Vector3& v1, float rad, unsigned char bidir, unsigned char area, unsigned short flags);

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
	afx_msg void OnFileClose();
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
	afx_msg void OnToolsOffmeshconnections();
	afx_msg void OnUpdateToolsOffmeshconnections(CCmdUI* pCmdUI);
	afx_msg void OnComponentAnimator();
	afx_msg void OnUpdateComponentAnimator(CCmdUI* pCmdUI);
	afx_msg void OnCreateNavigation();
	afx_msg void OnUpdateCreateNavigation(CCmdUI* pCmdUI);
	afx_msg void OnCreateDialog();
	afx_msg void OnControlStatic();
	afx_msg void OnUpdateControlStatic(CCmdUI* pCmdUI);
	afx_msg void OnControlProgressbar();
	afx_msg void OnUpdateControlProgressbar(CCmdUI* pCmdUI);
	afx_msg void OnControlButton();
	afx_msg void OnUpdateControlButton(CCmdUI* pCmdUI);
	afx_msg void OnControlImeeditbox();
	afx_msg void OnUpdateControlImeeditbox(CCmdUI* pCmdUI);
	afx_msg void OnControlCheckbox();
	afx_msg void OnUpdateControlCheckbox(CCmdUI* pCmdUI);
	afx_msg void OnControlCombobox();
	afx_msg void OnUpdateControlCombobox(CCmdUI* pCmdUI);
	afx_msg void OnControlListbox();
	afx_msg void OnUpdateControlListbox(CCmdUI* pCmdUI);
	afx_msg void OnControlScrollbar();
	afx_msg void OnUpdateControlScrollbar(CCmdUI* pCmdUI);
	virtual BOOL OnShowPopupMenu(CMFCPopupMenu* pMenuPopup);
	afx_msg void OnToolsScript1(UINT id);
	afx_msg void OnUpdateToolsScript1(CCmdUI* pCmdUI);
	afx_msg void OnToolsSnapshot();
	afx_msg void OnUpdateIndicatorCoord(CCmdUI* pCmdUI);
	afx_msg void OnUpdateIndicatorPostfix(CCmdUI* pCmdUI);
	afx_msg void OnToolsPlaying();
	afx_msg void OnUpdateToolsPlaying(CCmdUI* pCmdUI);
	afx_msg void OnToolsSnapToGrid();
	afx_msg void OnUpdateToolsSnapToGrid(CCmdUI* pCmdUI);
	afx_msg void OnToolsTerraintoobj();
	afx_msg void OnUpdateToolsTerraintoobj(CCmdUI* pCmdUI);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	afx_msg void OnAlignLefts();
	afx_msg void OnUpdateAlignLefts(CCmdUI* pCmdUI);
	afx_msg void OnAlignRights();
	afx_msg void OnUpdateAlignRights(CCmdUI* pCmdUI);
	afx_msg void OnAlignTops();
	afx_msg void OnUpdateAlignTops(CCmdUI* pCmdUI);
	afx_msg void OnAlignBottoms();
	afx_msg void OnUpdateAlignBottoms(CCmdUI* pCmdUI);
	afx_msg void OnAlignVertical();
	afx_msg void OnUpdateAlignVertical(CCmdUI* pCmdUI);
	afx_msg void OnAlignHorizontal();
	afx_msg void OnUpdateAlignHorizontal(CCmdUI* pCmdUI);
	afx_msg void OnAlignAcross();
	afx_msg void OnUpdateAlignAcross(CCmdUI* pCmdUI);
	afx_msg void OnAlignDown();
	afx_msg void OnUpdateAlignDown(CCmdUI* pCmdUI);
	afx_msg void OnAlignWidth();
	afx_msg void OnUpdateAlignWidth(CCmdUI* pCmdUI);
	afx_msg void OnAlignHeight();
	afx_msg void OnUpdateAlignHeight(CCmdUI* pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
};


