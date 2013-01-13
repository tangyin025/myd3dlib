#include "StdAfx.h"
#include "MainView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMainView, CView)

CMainView::CMainView(void)
{
}

BEGIN_MESSAGE_MAP(CMainView, CView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

int CMainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, GetParent());
	ASSERT(pFrame);

	LPDIRECT3DDEVICE9 pd3dDevice = pFrame->m_d3dDevice;
	m_font.CreateFontFromFile(pd3dDevice, "../demo2_3/Media/font/wqy-microhei.ttc", 13);

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

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, GetParent());
	ASSERT(pFrame);

	if(m_d3dSwapChain)
	{
		LPDIRECT3DDEVICE9 pd3dDevice = pFrame->m_d3dDevice;

		HRESULT hr;
		my::Surface BackBuffer;
		V(m_d3dSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer.m_ptr));
		V(pd3dDevice->SetRenderTarget(0, BackBuffer.m_ptr));
		V(pd3dDevice->SetDepthStencilSurface(m_DepthStencil.m_ptr));
		V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0,45,50,170), 1.0f, 0));

		if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
		{
			D3DSURFACE_DESC desc = BackBuffer.GetDesc();
			my::UIRender ui_render(pd3dDevice);
			ui_render.SetTransform(my::Matrix4::Identity(),
				my::UIRender::PerspectiveView(D3DXToRadian(75.0f), (float)desc.Width, (float)desc.Height),
				my::UIRender::PerspectiveProj(D3DXToRadian(75.0f), (float)desc.Width, (float)desc.Height));
			ui_render.Begin();

			CString strText;
			strText.Format(_T("%d x %d"), desc.Width, desc.Height);
			m_font.DrawString(&ui_render, strText, my::Rectangle(10,10,100,100), D3DCOLOR_ARGB(255,255,255,0));

			ui_render.End();
			V(pd3dDevice->EndScene());
		}

		V(m_d3dSwapChain->Present(NULL, NULL, NULL, NULL, 0));
	}
}

void CMainView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, GetParent());
	ASSERT(pFrame);

	if(cx > 0 && cy > 0)
	{
		LPDIRECT3DDEVICE9 pd3dDevice = pFrame->m_d3dDevice;

		D3DPRESENT_PARAMETERS d3dpp = {0};
		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		d3dpp.hDeviceWindow = m_hWnd;

		m_d3dSwapChain.Release();
		HRESULT hres = pd3dDevice->CreateAdditionalSwapChain(&d3dpp, &m_d3dSwapChain);
		if(FAILED(hres))
		{
			my::D3DException e(hres, __FILE__, __LINE__);
			TRACE(e.GetFullDescription().c_str());
		}

		m_DepthStencil.OnDestroyDevice();
		m_DepthStencil.CreateDepthStencilSurface(
			pd3dDevice, cx, cy, D3DFMT_D24X8, d3dpp.MultiSampleType, d3dpp.MultiSampleQuality);
	}
}

BOOL CMainView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}
