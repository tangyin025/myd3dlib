#pragma once

#include "MainDoc.h"

class CMainView
	: public CView
	, public my::SingleInstance<CMainView>
	, public my::DrawHelper
	, public btIDebugDraw
{
public:
	CMainView(void)
		: m_bAltDown(FALSE)
		, m_bEatAltUp(FALSE)
		, m_DragCameraMode(DragCameraNone)
		, m_Camera()
		, m_RenderMode(RenderModeDefault)
	{
	}

	DECLARE_DYNCREATE(CMainView)

	virtual void drawLine(const btVector3 & from,const btVector3 & to,const btVector3 & color);

	virtual void drawContactPoint(const btVector3 & PointOnB,const btVector3 & normalOnB, btScalar distance, int lifeTime, const btVector3 & color);

	virtual void reportErrorWarning(const char * warningString);

	virtual void draw3dText(const btVector3 & location,const char * textString);

	virtual void setDebugMode(int debugMode);

	virtual int getDebugMode() const;

	CMainDoc * GetDocument(void) const;

	virtual void OnDraw(CDC* pDC) {}

	int m_DebugDrawModes;

	BOOL m_bAltDown;

	BOOL m_bEatAltUp;

	enum DragCameraMode
	{
		DragCameraNone = 0,
		DragCameraRotate,
		DragCameraTrack,
		DragCameraZoom,
	};

	DragCameraMode m_DragCameraMode;

	CRectTracker m_Tracker;

	CComPtr<IDirect3DSwapChain9> m_d3dSwapChain;

	my::Surface m_DepthStencil;

	my::ModelViewerCamera m_Camera;

	//my::KinematicPtr m_Character;

	//my::Seek m_Seek;

	//my::Arrive m_Arrive;

	enum RenderMode
	{
		RenderModeDefault = 0,
		RenderModeWire,
	};

	RenderMode m_RenderMode;

	DECLARE_MESSAGE_MAP()

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnDestroy();

	afx_msg void OnPaint();

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
public:
	HRESULT OnDeviceReset(void);

	void OnDeviceLost(void);

	void OnFrameMove(
		double fTime,
		float fElapsedTime);

	void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime);

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnKillFocus(CWnd* pNewWnd);

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);

	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
