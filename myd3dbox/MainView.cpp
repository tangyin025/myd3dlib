#include "StdAfx.h"
#include "MainView.h"
#include "MainFrm.h"
#include "MainApp.h"

using namespace my;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMainView::SingleInstance * SingleInstance<CMainView>::s_ptr(NULL);

IMPLEMENT_DYNCREATE(CMainView, CView)

BEGIN_MESSAGE_MAP(CMainView, CView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

void CMainView::DrawTextAtWorld(const Vector3 & pos, LPCWSTR lpszText, D3DCOLOR Color, my::Font::Align align)
{
	CMainFrame * pFrame = CMainFrame::getSingletonPtr();
	ASSERT(pFrame);

	pFrame->m_UIRender->SetWorld(Matrix4::identity);
	pFrame->m_UIRender->SetViewProj(DialogMgr::m_Camera.m_ViewProj);
	pFrame->m_UIRender->Begin();

	Vector3 ptProj = pos.transformCoord(m_Camera.m_ViewProj);

	Vector2 vp = DialogMgr::GetDlgViewport();

	Vector2 ptVp(Lerp(0.0f, vp.x, (ptProj.x + 1) / 2), Lerp(0.0f, vp.y, (1 - ptProj.y) / 2));

	pFrame->m_Font->DrawString(
		pFrame->m_UIRender.get(), lpszText, my::Rectangle(ptVp, ptVp), Color, align);

	pFrame->m_UIRender->End();
}

CMainDoc * CMainView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMainDoc)));
	return (CMainDoc *)m_pDocument;
}

int CMainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_Camera.m_Rotation = Vector3(D3DXToRadian(-30),D3DXToRadian(0),D3DXToRadian(0));
	m_Camera.m_LookAt = Vector3(0,0,0);
	m_Camera.m_Distance = 20;

	//m_Character.reset(new my::Kinematic(Vector3(0,0,0),D3DXToRadian(0),Vector3(0,0,2),0));

	//m_Seek.character = m_Character.get();
	//m_Seek.target = Vector3(5,0,5);
	//m_Seek.maxAcceleration = 2.0f;

	//m_Arrive.character = m_Character.get();
	//m_Arrive.target = Vector3(5,0,5);
	//m_Arrive.maxAcceleration = 2.0f;
	//m_Arrive.radius = 0.2f;
	//m_Arrive.timeToTarget = 2.0f;

	return 0;
}

void CMainView::OnDestroy()
{
	CView::OnDestroy();

	m_d3dSwapChain.Release();
}

void CMainView::OnPaint()
{
	CPaintDC dc(this);

	if(m_d3dSwapChain)
	{
		OnFrameRender(CMainFrame::getSingleton().m_d3dDevice, theApp.m_fAbsoluteTime, 0);
	}

	m_Tracker.Draw(&dc);
}

void CMainView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if(cx > 0 && cy > 0 && m_d3dSwapChain)
	{
		OnDeviceLost();

		HRESULT hr;
		if(FAILED(hr = OnDeviceReset()))
		{
			TRACE(D3DException(hr, __FILE__, __LINE__).GetFullDescription().c_str());
		}
	}
}

BOOL CMainView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

HRESULT CMainView::OnDeviceReset(void)
{
	D3DPRESENT_PARAMETERS d3dpp = {0};
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.hDeviceWindow = m_hWnd;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	HRESULT hr = CMainFrame::getSingleton().m_d3dDevice->CreateAdditionalSwapChain(&d3dpp, &m_d3dSwapChain);
	if(FAILED(hr))
	{
		TRACE(D3DException(hr, __FILE__, __LINE__).GetFullDescription().c_str());
		return hr;
	}

	Surface BackBuffer;
	V(m_d3dSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer.m_ptr));
	D3DSURFACE_DESC desc = BackBuffer.GetDesc();

	DialogMgr::SetDlgViewport(Vector2((float)desc.Width, (float)desc.Height));

	m_DepthStencil.CreateDepthStencilSurface(
		CMainFrame::getSingleton().m_d3dDevice, desc.Width, desc.Height, D3DFMT_D24X8, d3dpp.MultiSampleType, d3dpp.MultiSampleQuality);

	m_Camera.m_Aspect = (float)desc.Width / desc.Height;

	return S_OK;
}

void CMainView::OnDeviceLost(void)
{
	m_DepthStencil.OnDestroyDevice();
	m_d3dSwapChain.Release();
}

void CMainView::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	//SteeringOutput steer;
	////m_Seek.getSteering(&steer);
	//m_Arrive.getSteering(&steer);
	//m_Character->integrate(steer, 0.35f, 0.01f);
	//m_Character->setOrientationFromVelocity(m_Character->velocity);
	//m_Character->trimMaxSpeed(2.0f);
	//m_Character->position.x = Round(m_Character->position.x, -10.0f, 10.0f);
	//m_Character->position.y = Round(m_Character->position.y, -10.0f, 10.0f);
}

void CMainView::OnFrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	ASSERT(m_d3dSwapChain);

	HRESULT hr;
	Surface BackBuffer;
	V(m_d3dSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer.m_ptr));
	V(pd3dDevice->SetRenderTarget(0, BackBuffer.m_ptr));
	V(pd3dDevice->SetDepthStencilSurface(m_DepthStencil.m_ptr));
	V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0,45,50,170), 1.0f, 0));
	V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE));

	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		CMainFrame * pFrame = CMainFrame::getSingletonPtr();
		ASSERT(pFrame);

		m_Camera.OnFrameMove(0,0);
		V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&Matrix4::identity));
		V(pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera.m_View));
		V(pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera.m_Proj));

		DrawLine(pd3dDevice, Vector3(-10,0,0), Vector3(10,0,0), D3DCOLOR_ARGB(255,0,0,0));
		DrawLine(pd3dDevice, Vector3(0,0,-10), Vector3(0,0,10), D3DCOLOR_ARGB(255,0,0,0));
		for(int i = 1; i <= 10; i++)
		{
			DrawLine(pd3dDevice, Vector3(-10,0, (float)i), Vector3(10,0, (float)i), D3DCOLOR_ARGB(255,127,127,127));
			DrawLine(pd3dDevice, Vector3(-10,0,-(float)i), Vector3(10,0,-(float)i), D3DCOLOR_ARGB(255,127,127,127));
			DrawLine(pd3dDevice, Vector3( (float)i,0,-10), Vector3( (float)i,0,10), D3DCOLOR_ARGB(255,127,127,127));
			DrawLine(pd3dDevice, Vector3(-(float)i,0,-10), Vector3(-(float)i,0,10), D3DCOLOR_ARGB(255,127,127,127));
		}

		pFrame->m_SimpleSample->SetFloat("g_Time", (float)fTime);
		pFrame->m_SimpleSample->SetMatrix("g_World", Matrix4::identity);
		pFrame->m_SimpleSample->SetMatrix("g_ViewProj", m_Camera.m_ViewProj);
		pFrame->m_SimpleSample->SetFloatArray("g_LightDir", &(Vector3(0,0,-1).transform(m_Camera.m_Orientation).x), 3);
		pFrame->m_SimpleSample->SetVector("g_LightDiffuse", Vector4(1,1,1,1));

		COutlinerView * pOutliner = COutlinerView::getSingletonPtr();
		ASSERT(pOutliner);
		switch(m_RenderMode)
		{
		case RenderModeDefault:
			pOutliner->DrawItemNode(pd3dDevice, fElapsedTime, pOutliner->m_TreeCtrl.GetRootItem(), m_RenderMode);
			break;

		case RenderModeWire:
			pOutliner->DrawItemNode(pd3dDevice, fElapsedTime, pOutliner->m_TreeCtrl.GetRootItem(), m_RenderMode);
			break;

		case RenderModePhysics:
			break;
		}

		//Matrix4 CharaTransform = Matrix4::RotationY(m_Character->orientation) * Matrix4::Translation(m_Character->position);
		//DrawSphere(pd3dDevice, 0.05f, D3DCOLOR_ARGB(255,255,0,0), CharaTransform);
		//DrawLine(pd3dDevice, Vector3(0,0,0), Vector3(0,0,0.3f), D3DCOLOR_ARGB(255,255,255,0), CharaTransform);
		//DrawLine(pd3dDevice, m_Character->position, m_Seek.target, D3DCOLOR_ARGB(255,0,255,0));

		DrawTextAtWorld(Vector3(10,0,0), _T("x"), D3DCOLOR_ARGB(255,255,255,0));
		DrawTextAtWorld(Vector3(0,0,10), _T("z"), D3DCOLOR_ARGB(255,255,255,0));

		pFrame->m_UIRender->SetWorld(Matrix4::identity);
		pFrame->m_UIRender->SetViewProj(DialogMgr::m_Camera.m_ViewProj);
		pFrame->m_UIRender->Begin();
		CString strText;
		D3DSURFACE_DESC desc = BackBuffer.GetDesc();
		strText.Format(_T("%d x %d"), desc.Width, desc.Height);
		pFrame->m_Font->DrawString(
			pFrame->m_UIRender.get(), strText, my::Rectangle(10,10,200,200), D3DCOLOR_ARGB(255,255,255,0));
		pFrame->m_UIRender->End();

		V(pd3dDevice->EndScene());
	}

	if(FAILED(hr = m_d3dSwapChain->Present(NULL, NULL, NULL, NULL, 0)))
	{
		if(D3DERR_DEVICELOST == hr || D3DERR_DRIVERINTERNALERROR == hr)
		{
			CMainFrame::getSingleton().ResetD3DDevice();
		}
	}
}

BOOL CMainView::PreTranslateMessage(MSG* pMsg)
{
	switch(pMsg->message)
	{
	case WM_SYSKEYDOWN:
		if(pMsg->wParam == VK_MENU)
		{
			m_bAltDown = TRUE;
		}
		break;

	case WM_SYSKEYUP:
		if(pMsg->wParam == VK_MENU && m_bAltDown)
		{
			m_bAltDown = FALSE;

			if(m_bEatAltUp)
			{
				m_bEatAltUp = FALSE;
				return TRUE;
			}
		}
		break;

	case WM_KEYUP:
		if(pMsg->wParam == VK_MENU && m_bAltDown)
		{
			m_bAltDown = FALSE;
		}
		break;
	}

	return CView::PreTranslateMessage(pMsg);
}

void CMainView::OnKillFocus(CWnd* pNewWnd)
{
	CView::OnKillFocus(pNewWnd);

	m_bAltDown = FALSE;
}

void CMainView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(m_bAltDown && DragCameraNone == m_DragCameraMode)
	{
		m_bEatAltUp = TRUE;
		m_DragCameraMode = DragCameraRotate;
		m_Camera.m_DragPos = point;
		SetCapture();
	}
	else
		m_Tracker.TrackRubberBand(this, point);
}

void CMainView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(DragCameraRotate == m_DragCameraMode)
	{
		m_DragCameraMode = DragCameraNone;
		ReleaseCapture();
	}
}

void CMainView::OnMButtonDown(UINT nFlags, CPoint point)
{
	if(m_bAltDown && DragCameraNone == m_DragCameraMode)
	{
		m_bEatAltUp = TRUE;
		m_DragCameraMode = DragCameraTrack;
		m_Camera.m_DragPos = point;
		SetCapture();
	}
}

void CMainView::OnMButtonUp(UINT nFlags, CPoint point)
{
	if(DragCameraTrack == m_DragCameraMode)
	{
		m_DragCameraMode = DragCameraNone;
		ReleaseCapture();
	}
}

void CMainView::OnRButtonDown(UINT nFlags, CPoint point)
{
	if(m_bAltDown && DragCameraNone == m_DragCameraMode)
	{
		m_bEatAltUp = TRUE;
		m_DragCameraMode = DragCameraZoom;
		m_Camera.m_DragPos = point;
		SetCapture();
	}
}

void CMainView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if(DragCameraZoom == m_DragCameraMode)
	{
		m_DragCameraMode = DragCameraNone;
		ReleaseCapture();
	}
}

void CMainView::OnMouseMove(UINT nFlags, CPoint point)
{
	switch(m_DragCameraMode)
	{
	case DragCameraRotate:
		m_Camera.m_Rotation.x -= D3DXToRadian((point.y - m_Camera.m_DragPos.y) * 0.5f);
		m_Camera.m_Rotation.y -= D3DXToRadian((point.x - m_Camera.m_DragPos.x) * 0.5f);
		m_Camera.m_DragPos = point;
		Invalidate();
		break;

	case DragCameraTrack:
		m_Camera.m_LookAt += Vector3(
			(m_Camera.m_DragPos.x - point.x) * m_Camera.m_Proj._11 * m_Camera.m_Distance * 0.001f,
			(point.y - m_Camera.m_DragPos.y) * m_Camera.m_Proj._11 * m_Camera.m_Distance * 0.001f, 0).transform(m_Camera.m_Orientation);
		m_Camera.m_DragPos = point;
		Invalidate();
		break;

	case DragCameraZoom:
		m_Camera.m_Distance -= (point.x - m_Camera.m_DragPos.x) * 0.02f;
		m_Camera.m_DragPos = point;
		Invalidate();
		break;
	}
}

void CMainView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_DELETE:
		{
			COutlinerView * pOutliner = COutlinerView::getSingletonPtr();
			ASSERT(pOutliner);

			HTREEITEM hSelected = pOutliner->m_TreeCtrl.GetSelectedItem();
			if(hSelected)
			{
				CMainDoc * pDoc = CMainDoc::getSingletonPtr();
				ASSERT(pDoc);
				pDoc->DeleteTreeNode(hSelected);
				pDoc->UpdateAllViews(NULL);
			}
		}
		return;
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}
