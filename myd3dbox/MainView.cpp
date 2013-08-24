#include "StdAfx.h"
#include "MainView.h"
#include "MainFrm.h"
#include "MainApp.h"

using namespace my;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void EffectUIRender::Begin(void)
{
	CRect rectClient;
	CMainView::getSingleton().GetClientRect(&rectClient);
	m_UIEffect->SetVector("g_ScreenDim", Vector4((float)rectClient.Width(), (float)rectClient.Height(), 0, 0));

	m_Passes = m_UIEffect->Begin();
}

void EffectUIRender::End(void)
{
	m_UIEffect->End();

	m_Passes = 0;
}

void EffectUIRender::SetWorld(const Matrix4 & World)
{
	m_UIEffect->SetMatrix("g_World", World);
}

void EffectUIRender::SetViewProj(const my::Matrix4 & ViewProj)
{
	m_UIEffect->SetMatrix("g_ViewProj", ViewProj);
}

void EffectUIRender::SetTexture(IDirect3DBaseTexture9 * pTexture)
{
	_ASSERT(CMainView::getSingleton().m_WhiteTex);

	m_UIEffect->SetTexture("g_MeshTexture", pTexture ? pTexture : CMainView::getSingleton().m_WhiteTex->m_ptr);
}

void EffectUIRender::DrawVertexList(void)
{
	if(vertex_count > 0)
	{
		for(UINT p = 0; p < m_Passes; p++)
		{
			m_UIEffect->BeginPass(p);
			V(m_Device->SetFVF(D3DFVF_CUSTOMVERTEX));
			V(m_Device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertex_count / 3, vertex_list, sizeof(vertex_list[0])));
			m_UIEffect->EndPass();
		}
	}
}

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
	m_UIRender->SetWorld(Matrix4::identity);
	m_UIRender->SetViewProj(DialogMgr::m_Camera.m_ViewProj);
	m_UIRender->Begin();

	Vector3 ptProj = pos.transformCoord(m_Camera.m_ViewProj);

	Vector2 vp = DialogMgr::GetDlgViewport();

	Vector2 ptVp(Lerp(0.0f, vp.x, (ptProj.x + 1) / 2), Lerp(0.0f, vp.y, (1 - ptProj.y) / 2));

	m_Font->DrawString(m_UIRender.get(), lpszText, my::Rectangle(ptVp, ptVp), Color, align);

	m_UIRender->End();
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

	m_UIRender.reset(new EffectUIRender(theApp.GetD3D9Device(), theApp.LoadEffect("shader/UIEffect.fx", std::vector<std::pair<std::string, std::string> >())));

	m_WhiteTex = theApp.LoadTexture("texture/white.bmp");

	m_Font = theApp.LoadFont("font/wqy-microhei.ttc", 13);

	m_SimpleSample = theApp.LoadEffect("shader/SimpleSample.fx", std::vector<std::pair<std::string, std::string> >());

	m_Camera.m_Rotation = Vector3(D3DXToRadian(-30),D3DXToRadian(0),D3DXToRadian(0));
	m_Camera.m_LookAt = Vector3(0,0,0);
	m_Camera.m_Distance = 20;

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
		theApp.UpdateClock();

		OnFrameRender(theApp.GetD3D9Device(), theApp.m_fAbsoluteTime, theApp.m_fElapsedTime);

		m_Tracker.Draw(&dc);
	}
}

void CMainView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if(cx > 0 && cy > 0)
	{
		OnDeviceLost();

		// ! �ڳ�ʼ������ʱ���ᱻ�����������
		ResetD3DSwapChain();
	}
}

BOOL CMainView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

BOOL CMainView::ResetD3DSwapChain(void)
{
	D3DPRESENT_PARAMETERS d3dpp = {0};
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.hDeviceWindow = m_hWnd;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	HRESULT hr = theApp.GetD3D9Device()->CreateAdditionalSwapChain(&d3dpp, &m_d3dSwapChain);
	if(FAILED(hr))
	{
		TRACE(my::D3DException::Translate(hr).c_str());
		return FALSE;
	}

	Surface BackBuffer;
	V(m_d3dSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer.m_ptr));
	D3DSURFACE_DESC desc = BackBuffer.GetDesc();

	DialogMgr::SetDlgViewport(Vector2((float)desc.Width, (float)desc.Height));

	m_DepthStencil.CreateDepthStencilSurface(
		theApp.GetD3D9Device(), desc.Width, desc.Height, D3DFMT_D24X8, d3dpp.MultiSampleType, d3dpp.MultiSampleQuality);

	m_Camera.m_Aspect = (float)desc.Width / desc.Height;

	return TRUE;
}

void CMainView::OnDeviceLost(void)
{
	m_DepthStencil.OnDestroyDevice();

	m_d3dSwapChain.Release();
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

	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE));
		V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));

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

		m_SimpleSample->SetFloat("g_Time", (float)fTime);
		m_SimpleSample->SetMatrix("g_World", Matrix4::identity);
		m_SimpleSample->SetMatrix("g_ViewProj", m_Camera.m_ViewProj);
		m_SimpleSample->SetFloatArray("g_LightDir", &(Vector3(0,0,-1).transform(m_Camera.m_Orientation).x), 3);
		m_SimpleSample->SetVector("g_LightDiffuse", Vector4(1,1,1,1));

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

		DrawTextAtWorld(Vector3(10,0,0), L"x", D3DCOLOR_ARGB(255,255,255,0));
		DrawTextAtWorld(Vector3(0,0,10), L"z", D3DCOLOR_ARGB(255,255,255,0));

		m_UIRender->SetWorld(Matrix4::identity);
		m_UIRender->SetViewProj(DialogMgr::m_Camera.m_ViewProj);
		m_UIRender->Begin();
		CString strText;
		D3DSURFACE_DESC desc = BackBuffer.GetDesc();
		strText.Format(_T("%d x %d"), desc.Width, desc.Height);
		m_Font->DrawString(
			m_UIRender.get(), ts2ws(strText).c_str(), my::Rectangle(10,10,200,200), D3DCOLOR_ARGB(255,255,255,0));
		m_UIRender->End();

		V(pd3dDevice->EndScene());
	}

	if(FAILED(hr = m_d3dSwapChain->Present(NULL, NULL, NULL, NULL, 0)))
	{
		if(D3DERR_DEVICELOST == hr || D3DERR_DRIVERINTERNALERROR == hr)
		{
			theApp.ResetD3DDevice();
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
