
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "MainApp.h"

#include "MainDoc.h"
#include "ChildView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

IMPLEMENT_DYNCREATE(CChildView, CView)

BEGIN_MESSAGE_MAP(CChildView, CView)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

// CChildView construction/destruction

CChildView::CChildView()
{
	// TODO: add construction code here
	m_SwapChainBuffer.reset(new my::Surface());
	ZeroMemory(&m_SwapChainBufferDesc, sizeof(m_SwapChainBufferDesc));
	m_DepthStencil.reset(new my::Surface());
	m_NormalRT.reset(new my::Texture2D());
	m_PositionRT.reset(new my::Texture2D());
	m_LightRT.reset(new my::Texture2D());
	m_OpaqueRT.reset(new my::Texture2D());
	for (unsigned int i = 0; i < _countof(m_DownFilterRT); i++)
	{
		m_DownFilterRT[i].reset(new my::Texture2D());
	}
	m_CameraDragMode = CameraDragNone;
}

CChildView::~CChildView()
{
}

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CChildView drawing

void CChildView::OnDraw(CDC* /*pDC*/)
{
	CMainDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}

BOOL CChildView::ResetD3DSwapChain(void)
{
	D3DPRESENT_PARAMETERS d3dpp = {0};
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.hDeviceWindow = m_hWnd;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	m_d3dSwapChain.Release();
	HRESULT hr = theApp.m_d3dDevice->CreateAdditionalSwapChain(&d3dpp, &m_d3dSwapChain);
	if(FAILED(hr))
	{
		TRACE(my::D3DException::Translate(hr));
		return FALSE;
	}

	m_SwapChainBuffer->OnDestroyDevice();
	V(m_d3dSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &m_SwapChainBuffer->m_ptr));
	m_SwapChainBufferDesc = m_SwapChainBuffer->GetDesc();

	m_DepthStencil->OnDestroyDevice();
	m_DepthStencil->CreateDepthStencilSurface(
		theApp.m_d3dDevice, m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height, D3DFMT_D24X8, d3dpp.MultiSampleType, d3dpp.MultiSampleQuality);

	ResetRenderTargets(theApp.m_d3dDevice, &m_SwapChainBufferDesc);

	return TRUE;
}

BOOL CChildView::ResetRenderTargets(IDirect3DDevice9 * pd3dDevice, const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	m_NormalRT->OnDestroyDevice();
	m_NormalRT->CreateTexture(
		pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	m_PositionRT->OnDestroyDevice();
	m_PositionRT->CreateTexture(
		pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	m_LightRT->OnDestroyDevice();
	m_LightRT->CreateTexture(
		pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);

	m_OpaqueRT->OnDestroyDevice();
	m_OpaqueRT->CreateTexture(
		pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);

	for (unsigned int i = 0; i < _countof(m_DownFilterRT); i++)
	{
		m_DownFilterRT[i]->OnDestroyDevice();
		m_DownFilterRT[i]->CreateTexture(
			pd3dDevice, pBackBufferSurfaceDesc->Width / 4, pBackBufferSurfaceDesc->Height / 4, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);
	}
	return TRUE;
}

IDirect3DSurface9 * CChildView::GetScreenSurface(void)
{
	return m_SwapChainBuffer->m_ptr;
}

IDirect3DSurface9 * CChildView::GetScreenDepthStencilSurface(void)
{
	return m_DepthStencil->m_ptr;
}

IDirect3DSurface9 * CChildView::GetNormalSurface(void)
{
	return m_NormalRT->GetSurfaceLevel(0);
}

my::Texture2D * CChildView::GetNormalTexture(void)
{
	return m_NormalRT.get();
}

IDirect3DSurface9 * CChildView::GetPositionSurface(void)
{
	return m_PositionRT->GetSurfaceLevel(0);
}

my::Texture2D * CChildView::GetPositionTexture(void)
{
	return m_PositionRT.get();
}

IDirect3DSurface9 * CChildView::GetLightSurface(void)
{
	return m_LightRT->GetSurfaceLevel(0);
}

my::Texture2D * CChildView::GetLightTexture(void)
{
	return m_LightRT.get();
}

IDirect3DSurface9 * CChildView::GetOpaqueSurface(void)
{
	return m_OpaqueRT->GetSurfaceLevel(0);
}

my::Texture2D * CChildView::GetOpaqueTexture(void)
{
	return m_OpaqueRT.get();
}

IDirect3DSurface9 * CChildView::GetDownFilterSurface(unsigned int i)
{
	_ASSERT(i < _countof(m_DownFilterRT));
	return m_DownFilterRT[i]->GetSurfaceLevel(0);
}

my::Texture2D * CChildView::GetDownFilterTexture(unsigned int i)
{
	_ASSERT(i < _countof(m_DownFilterRT));
	return m_DownFilterRT[i].get();
}

void CChildView::QueryComponent(const my::Frustum & frustum, unsigned int PassMask)
{
}

void CChildView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}


// CChildView diagnostics

#ifdef _DEBUG
void CChildView::AssertValid() const
{
	CView::AssertValid();
}

void CChildView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMainDoc* CChildView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMainDoc)));
	return (CMainDoc*)m_pDocument;
}
#endif //_DEBUG


// CChildView message handlers

void CChildView::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CView::OnPaint() for painting messages
	if(m_d3dSwapChain)
	{
		if (theApp.m_DeviceObjectsReset)
		{
			m_Camera.OnFrameMove(0,0);

			if(SUCCEEDED(hr = theApp.m_d3dDevice->BeginScene()))
			{
				DrawHelper::BeginLine();

				PushGrid();

				BkColor = D3DCOLOR_ARGB(0,161,161,161);
				theApp.OnFrameRender(theApp.m_d3dDevice, &m_SwapChainBufferDesc, this, theApp.m_fAbsoluteTime, theApp.m_fElapsedTime);

				theApp.m_d3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera.m_View);
				theApp.m_d3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera.m_Proj);
				DrawHelper::EndLine(theApp.m_d3dDevice, my::Matrix4::identity);

				theApp.m_UIRender->Begin();
				theApp.m_UIRender->SetViewProj(DialogMgr::m_ViewProj);
				theApp.m_UIRender->SetWorld(my::Matrix4::Translation(my::Vector3(0.5f,0.5f,0)));
				theApp.m_Font->DrawString(theApp.m_UIRender.get(), L"Hello world!", my::Rectangle::LeftTop(50,50,100,100), D3DCOLOR_ARGB(255,255,255,0), my::Font::AlignLeftTop);
				theApp.m_UIRender->End();

				V(theApp.m_d3dDevice->EndScene());
			}

			if(FAILED(hr = m_d3dSwapChain->Present(NULL, NULL, NULL, NULL, 0)))
			{
				if(D3DERR_DEVICELOST == hr || D3DERR_DRIVERINTERNALERROR == hr)
				{
					theApp.OnLostDevice();
					theApp.m_DeviceObjectsReset = false;
				}
			}
		}
		else if (D3DERR_DEVICENOTRESET == theApp.m_d3dDevice->TestCooperativeLevel())
		{
			theApp.ResetD3DDevice();
		}
	}
}

BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if(cx > 0 && cy > 0 && (cx != m_SwapChainBufferDesc.Width || cy != m_SwapChainBufferDesc.Height))
	{
		// ! 在初始化窗口时，会被反复创建多次
		ResetD3DSwapChain();
		m_Camera.m_Aspect = (float)m_SwapChainBufferDesc.Width / m_SwapChainBufferDesc.Height;
		DialogMgr::SetDlgViewport(my::Vector2((float)cx, (float)cy), D3DXToRadian(75.0f));
	}
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	m_Camera.m_LookAt = my::Vector3(0,0,0);
	m_Camera.m_Eular = my::Vector3(D3DXToRadian(-45),D3DXToRadian(45),0);
	m_Camera.m_Distance = 20.0f;

	return 0;
}

void CChildView::OnDestroy()
{
	CView::OnDestroy();

	// TODO: Add your message handler code here
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if ((GetKeyState(VK_MENU) & 0x8000) && m_CameraDragMode == CameraDragNone)
	{
		m_CameraDragMode = CameraDragRotate;
		m_CameraDragPos = point;
		CMainFrame::getSingleton().m_bEatAltUp = TRUE;
		SetCapture();
	}
}

void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_CameraDragMode == CameraDragRotate)
	{
		m_CameraDragMode = CameraDragNone;
		ReleaseCapture();
	}
}

void CChildView::OnMButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if ((GetKeyState(VK_MENU) & 0x8000) && m_CameraDragMode == CameraDragNone)
	{
		m_CameraDragMode = CameraDragTrake;
		m_CameraDragPos = point;
		CMainFrame::getSingleton().m_bEatAltUp = TRUE;
		SetCapture();
	}
}

void CChildView::OnMButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_CameraDragMode == CameraDragTrake)
	{
		m_CameraDragMode = CameraDragNone;
		ReleaseCapture();
	}
}

void CChildView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if ((GetKeyState(VK_MENU) & 0x8000) && m_CameraDragMode == CameraDragNone)
	{
		m_CameraDragMode = CameraDragMove;
		m_CameraDragPos = point;
		CMainFrame::getSingleton().m_bEatAltUp = TRUE;
		SetCapture();
	}
}

void CChildView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_CameraDragMode == CameraDragMove)
	{
		m_CameraDragMode = CameraDragNone;
		ReleaseCapture();
	}
	else
	{
		ClientToScreen(&point);
		OnContextMenu(this, point);
	}
}

void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	switch(m_CameraDragMode)
	{
	case CameraDragRotate:
		{
			m_Camera.m_Eular.x -= D3DXToRadian((point.y - m_CameraDragPos.y) * 0.5f);
			m_Camera.m_Eular.y -= D3DXToRadian((point.x - m_CameraDragPos.x) * 0.5f);
			m_CameraDragPos = point;
			Invalidate();
		}
		break;

	case CameraDragTrake:
		{
			my::Vector3 Right = m_Camera.m_View.column<0>().xyz;
			my::Vector3 Up = m_Camera.m_View.column<1>().xyz;
			m_Camera.m_LookAt += Right * (float)(m_CameraDragPos.x - point.x) * 0.03f + Up * (float)(point.y - m_CameraDragPos.y) * 0.03f;
			m_CameraDragPos = point;
			Invalidate();
		}
		break;

	case CameraDragMove:
		{
			my::Vector3 Dir = m_Camera.m_View.column<2>().xyz;
			m_Camera.m_LookAt += Dir * (float)(m_CameraDragPos.x - point.x) * 0.03f;
			m_CameraDragPos = point;
			Invalidate();
		}
		break;
	}
}

BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default

	return __super::OnMouseWheel(nFlags, zDelta, pt);
}
