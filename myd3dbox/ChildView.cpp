
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "MainApp.h"

#include "MainDoc.h"
#include "ChildView.h"

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
END_MESSAGE_MAP()

// CChildView construction/destruction

CChildView::CChildView()
{
	// TODO: add construction code here

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
	HRESULT hr = theApp.GetD3D9Device()->CreateAdditionalSwapChain(&d3dpp, &m_d3dSwapChain);
	if(FAILED(hr))
	{
		TRACE(my::D3DException::Translate(hr));
		return FALSE;
	}

	my::Surface BackBuffer;
	V(m_d3dSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer.m_ptr));
	m_SwapChainBufferDesc = BackBuffer.GetDesc();

	m_DepthStencil.OnDestroyDevice();
	m_DepthStencil.CreateDepthStencilSurface(
		theApp.GetD3D9Device(), m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height, D3DFMT_D24X8, d3dpp.MultiSampleType, d3dpp.MultiSampleQuality);

	return TRUE;
}

void CChildView::OnFrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	m_mesh_cmp->QueryMesh(&theApp, RenderPipeline::DrawStageCBuffer);

	theApp.m_SimpleSample->SetMatrix("g_View", m_Camera.m_View);
	theApp.m_SimpleSample->SetMatrix("g_ViewProj", m_Camera.m_ViewProj);
	theApp.OnFrameRender(pd3dDevice, fTime, fElapsedTime);

	theApp.m_UIRender->Begin();
	theApp.m_UIRender->SetViewProj(m_ViewProj);
	theApp.m_UIRender->SetWorld(my::Matrix4::Translation(my::Vector3(0.5f,0.5f,0)));
	theApp.m_Font->DrawString(theApp.m_UIRender.get(), L"Hello world!", my::Rectangle::LeftTop(50,50,100,100), D3DCOLOR_ARGB(255,0,0,0), my::Font::AlignLeftTop);
	theApp.m_UIRender->End();
}

void CChildView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
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
		IDirect3DDevice9 * pd3dDevice = theApp.GetD3D9Device();
		_ASSERT(pd3dDevice);

		my::Surface BackBuffer;
		V(m_d3dSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer.m_ptr));
		V(pd3dDevice->SetRenderTarget(0, BackBuffer.m_ptr));
		V(pd3dDevice->SetDepthStencilSurface(m_DepthStencil.m_ptr));
		V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0,161,161,161), 1.0f, 0));

		if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
		{
			OnFrameRender(pd3dDevice, theApp.m_fAbsoluteTime, theApp.m_fElapsedTime);

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
}

BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if(cx > 0 && cy > 0)
	{
		// ! 在初始化窗口时，会被反复创建多次
		ResetD3DSwapChain();
		m_Camera.m_Aspect = (float)m_SwapChainBufferDesc.Width / m_SwapChainBufferDesc.Height;
		m_Camera.OnFrameMove(0,0);
		DialogMgr::SetDlgViewport(my::Vector2((float)cx, (float)cy), D3DXToRadian(75.0f));
	}
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	float k=cos(D3DXToRadian(45));
	float d=20;
	m_Camera.m_Eye=my::Vector3(d*k*k,d*k+1,d*k*k);
	m_Camera.m_Eular=my::Vector3(D3DXToRadian(-45),D3DXToRadian(45),0);
	m_Camera.OnFrameMove(0,0);

	m_mesh_cmp = theApp.CreateMeshComponentFromFile("mesh/tube.mesh.xml");

	return 0;
}

void CChildView::OnDestroy()
{
	CView::OnDestroy();

	// TODO: Add your message handler code here
}
