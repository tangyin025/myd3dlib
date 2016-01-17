
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "MainApp.h"

#include "ChildView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

IMPLEMENT_DYNCREATE(CChildView, CView)

BEGIN_MESSAGE_MAP(CChildView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_CAMERATYPE_PERSPECTIVE, &CChildView::OnCameratypePerspective)
	ON_UPDATE_COMMAND_UI(ID_CAMERATYPE_PERSPECTIVE, &CChildView::OnUpdateCameratypePerspective)
	ON_COMMAND(ID_CAMERATYPE_FRONT, &CChildView::OnCameratypeFront)
	ON_UPDATE_COMMAND_UI(ID_CAMERATYPE_FRONT, &CChildView::OnUpdateCameratypeFront)
	ON_COMMAND(ID_CAMERATYPE_SIDE, &CChildView::OnCameratypeSide)
	ON_UPDATE_COMMAND_UI(ID_CAMERATYPE_SIDE, &CChildView::OnUpdateCameratypeSide)
	ON_COMMAND(ID_CAMERATYPE_TOP, &CChildView::OnCameratypeTop)
	ON_UPDATE_COMMAND_UI(ID_CAMERATYPE_TOP, &CChildView::OnUpdateCameratypeTop)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// CChildView construction/destruction

CChildView::CChildView()
	: m_PivotScale(1.0f)
	, m_CameraType(CameraTypeUnknown)
	, m_CameraDiagonal(30.0f)
{
	// TODO: add construction code here
	m_SwapChainBuffer.reset(new my::Surface());
	ZeroMemory(&m_SwapChainBufferDesc, sizeof(m_SwapChainBufferDesc));
	m_SwapChainBufferDesc.Width = 100;
	m_SwapChainBufferDesc.Height = 100;
	m_DepthStencil.reset(new my::Surface());
	m_NormalRT.reset(new my::Texture2D());
	m_PositionRT.reset(new my::Texture2D());
	m_LightRT.reset(new my::Texture2D());
	m_OpaqueRT.reset(new my::Texture2D());
	for (unsigned int i = 0; i < _countof(m_DownFilterRT); i++)
	{
		m_DownFilterRT[i].reset(new my::Texture2D());
	}
	m_SkyLightCam.reset(new my::OrthoCamera(sqrt(30*30*2.0f),1.0f,-100,100));
	boost::static_pointer_cast<my::OrthoCamera>(m_SkyLightCam)->m_Eular = my::Vector3(D3DXToRadian(-45),D3DXToRadian(0),0);
	ZeroMemory(&m_qwTime, sizeof(m_qwTime));
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
	//CMainDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc);
	//if (!pDoc)
	//	return;

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

void CChildView::QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	struct CallBack : public my::IQueryCallback
	{
		const my::Frustum & frustum;
		RenderPipeline * pipeline;
		unsigned int PassMask;
		CallBack(const my::Frustum & _frustum, RenderPipeline * _pipeline, unsigned int _PassMask)
			: frustum(_frustum)
			, pipeline(_pipeline)
			, PassMask(_PassMask)
		{
		}
		void operator() (my::OctComponent * oct_cmp, my::IntersectionTests::IntersectionType)
		{
			RenderComponent * render_cmp = static_cast<RenderComponent *>(oct_cmp);
			if (render_cmp)
			{
				if (!render_cmp->IsRequested())
				{
					render_cmp->RequestResource();
				}
				render_cmp->AddToPipeline(pipeline, PassMask);
			}
		}
	};

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	pFrame->m_Root.QueryComponent(frustum, &CallBack(frustum, pipeline, PassMask));
}

void CChildView::RenderSelectedObject(IDirect3DDevice9 * pd3dDevice)
{
	theApp.m_SimpleSample->SetMatrix("g_View", m_Camera->m_View);
	theApp.m_SimpleSample->SetMatrix("g_ViewProj", m_Camera->m_ViewProj);
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	if (!pFrame->m_selcmps.empty())
	{
		CMainFrame::ComponentSet::const_iterator sel_iter = pFrame->m_selcmps.begin();
		for (; sel_iter != pFrame->m_selcmps.end(); sel_iter++)
		{
			switch ((*sel_iter)->m_Type)
			{
			case Component::ComponentTypeMesh:
				{
					MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(*sel_iter);
					if (mesh_cmp->m_MeshRes.m_Res)
					{
						theApp.m_SimpleSample->SetMatrix("g_World", mesh_cmp->m_World);
						UINT passes = theApp.m_SimpleSample->Begin();
						for (unsigned int i = 0; i < mesh_cmp->m_MaterialList.size(); i++)
						{
							theApp.m_SimpleSample->BeginPass(0);
							mesh_cmp->m_MeshRes.m_Res->DrawSubset(i);
							theApp.m_SimpleSample->EndPass();
						}
						theApp.m_SimpleSample->End();
					}
					PushWireAABB(mesh_cmp->GetOctAABB(), D3DCOLOR_ARGB(2555,255,0,255));
				}
				break;
			}
		}
		PushWireAABB(pFrame->m_selbox, D3DCOLOR_ARGB(255,255,255,255));
	}
}

void CChildView::StartPerformanceCount(void)
{
	QueryPerformanceCounter(&m_qwTime[0]);
}

double CChildView::EndPerformanceCount(void)
{
	QueryPerformanceCounter(&m_qwTime[1]);
	return (double)(m_qwTime[1].QuadPart - m_qwTime[0].QuadPart) / theApp.m_llQPFTicksPerSec;
}

bool CChildView::OverlapTestFrustumAndComponent(const my::Frustum & frustum, Component * cmp)
{
	my::Frustum local_ftm = frustum.transform(cmp->m_World.transpose());
	switch (cmp->m_Type)
	{
	case Component::ComponentTypeMesh:
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(cmp);
			my::OgreMeshPtr mesh = boost::dynamic_pointer_cast<my::OgreMesh>(mesh_cmp->m_MeshRes.m_Res);
			if (!mesh)
			{
				return false;
			}
			if (mesh_cmp->m_Animator && !mesh_cmp->m_Animator->m_DualQuats.empty())
			{
				std::vector<my::Vector3> vertices(mesh->GetNumVertices());
				my::D3DVertexElementSet elems;
				elems.InsertPositionElement(0);
				my::OgreMesh::ComputeDualQuaternionSkinnedVertices(
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					elems,
					mesh->LockVertexBuffer(),
					mesh->GetNumBytesPerVertex(),
					mesh->m_VertexElems,
					mesh_cmp->m_Animator->m_DualQuats);
				bool ret = OverlapTestFrustumAndMesh(local_ftm,
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					mesh->LockIndexBuffer(),
					!(mesh->GetOptions() & D3DXMESH_32BIT),
					mesh->GetNumFaces(),
					elems);
				mesh->UnlockVertexBuffer();
				mesh->UnlockIndexBuffer();
				return ret;
			}
			else
			{
				bool ret = OverlapTestFrustumAndMesh(local_ftm,
					mesh->LockVertexBuffer(),
					mesh->GetNumVertices(),
					mesh->GetNumBytesPerVertex(),
					mesh->LockIndexBuffer(),
					!(mesh->GetOptions() & D3DXMESH_32BIT),
					mesh->GetNumFaces(),
					mesh->m_VertexElems);
				mesh->UnlockVertexBuffer();
				mesh->UnlockIndexBuffer();
				return ret;
			}
		}
		break;
	}
	return false;
}

bool CChildView::OverlapTestFrustumAndMesh(
	const my::Frustum & frustum,
	void * pVertices,
	DWORD NumVerts,
	DWORD VertexStride,
	void * pIndices,
	bool bIndices16,
	DWORD NumFaces,
	const my::D3DVertexElementSet & VertexElems)
{
	for(unsigned int face_i = 0; face_i < NumFaces; face_i++)
	{
		int i1, i2, i3;
		if(bIndices16)
		{
			i1 = *((WORD *)pIndices + face_i * 3 + 0);
			i2 = *((WORD *)pIndices + face_i * 3 + 1);
			i3 = *((WORD *)pIndices + face_i * 3 + 2);
		}
		else
		{
			i1 = *((DWORD *)pIndices + face_i * 3 + 0);
			i2 = *((DWORD *)pIndices + face_i * 3 + 1);
			i3 = *((DWORD *)pIndices + face_i * 3 + 2);
		}

		unsigned char * pv1 = (unsigned char *)pVertices + i1 * VertexStride;
		unsigned char * pv2 = (unsigned char *)pVertices + i2 * VertexStride;
		unsigned char * pv3 = (unsigned char *)pVertices + i3 * VertexStride;

		const my::Vector3 & v1 = VertexElems.GetPosition(pv1);
		const my::Vector3 & v2 = VertexElems.GetPosition(pv2);
		const my::Vector3 & v3 = VertexElems.GetPosition(pv3);

		my::IntersectionTests::IntersectionType result = my::IntersectionTests::IntersectTriangleAndFrustum(v1, v2, v3, frustum);
		if (result == my::IntersectionTests::IntersectionTypeInside || result == my::IntersectionTests::IntersectionTypeIntersect)
		{
			return true;
		}
	}
	return false;
}

my::RayResult CChildView::OverlapTestRayAndComponent(const my::Ray & ray, Component * cmp)
{
	my::Ray local_ray = ray.transform(cmp->m_World.inverse());
	switch (cmp->m_Type)
	{
	case Component::ComponentTypeMesh:
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(cmp);
			my::OgreMeshPtr mesh = boost::dynamic_pointer_cast<my::OgreMesh>(mesh_cmp->m_MeshRes.m_Res);
			if (!mesh)
			{
				return my::RayResult(false, FLT_MAX);
			}
			if (mesh_cmp->m_Animator && !mesh_cmp->m_Animator->m_DualQuats.empty())
			{
				std::vector<my::Vector3> vertices(mesh->GetNumVertices());
				my::D3DVertexElementSet elems;
				elems.InsertPositionElement(0);
				my::OgreMesh::ComputeDualQuaternionSkinnedVertices(
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					elems,
					mesh->LockVertexBuffer(),
					mesh->GetNumBytesPerVertex(),
					mesh->m_VertexElems,
					mesh_cmp->m_Animator->m_DualQuats);
				my::RayResult ret = OverlapTestRayAndMesh(local_ray,
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					mesh->LockIndexBuffer(),
					!(mesh->GetOptions() & D3DXMESH_32BIT),
					mesh->GetNumFaces(),
					elems);
				mesh->UnlockVertexBuffer();
				mesh->UnlockIndexBuffer();
				return ret;
			}
			else
			{
				my::RayResult ret = OverlapTestRayAndMesh(local_ray,
					mesh->LockVertexBuffer(),
					mesh->GetNumVertices(),
					mesh->GetNumBytesPerVertex(),
					mesh->LockIndexBuffer(),
					!(mesh->GetOptions() & D3DXMESH_32BIT),
					mesh->GetNumFaces(),
					mesh->m_VertexElems);
				mesh->UnlockVertexBuffer();
				mesh->UnlockIndexBuffer();
				return ret;
			}
		}
		break;
	}
	return my::RayResult(false, FLT_MAX);
}

my::RayResult CChildView::OverlapTestRayAndMesh(
	const my::Ray & ray,
	void * pVertices,
	DWORD NumVerts,
	DWORD VertexStride,
	void * pIndices,
	bool bIndices16,
	DWORD NumFaces,
	const my::D3DVertexElementSet & VertexElems)
{
	my::RayResult ret(false, FLT_MAX);
	for(unsigned int face_i = 0; face_i < NumFaces; face_i++)
	{
		int i1, i2, i3;
		if(bIndices16)
		{
			i1 = *((WORD *)pIndices + face_i * 3 + 0);
			i2 = *((WORD *)pIndices + face_i * 3 + 1);
			i3 = *((WORD *)pIndices + face_i * 3 + 2);
		}
		else
		{
			i1 = *((DWORD *)pIndices + face_i * 3 + 0);
			i2 = *((DWORD *)pIndices + face_i * 3 + 1);
			i3 = *((DWORD *)pIndices + face_i * 3 + 2);
		}

		unsigned char * pv1 = (unsigned char *)pVertices + i1 * VertexStride;
		unsigned char * pv2 = (unsigned char *)pVertices + i2 * VertexStride;
		unsigned char * pv3 = (unsigned char *)pVertices + i3 * VertexStride;

		const my::Vector3 & v1 = VertexElems.GetPosition(pv1);
		const my::Vector3 & v2 = VertexElems.GetPosition(pv2);
		const my::Vector3 & v3 = VertexElems.GetPosition(pv3);

		my::RayResult result = my::CollisionDetector::rayAndTriangle(ray.p, ray.d, v1, v2, v3);
		if (result.first && result.second < ret.second)
		{
			ret = result;
		}
	}
	return ret;
}

void CChildView::OnSelectionChanged(EventArg * arg)
{
	Invalidate();
}

void CChildView::OnPivotModeChanged(EventArg * arg)
{
	Invalidate();
}

void CChildView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_VIEW, point.x, point.y, this, TRUE);
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

//CMainDoc* CChildView::GetDocument() const // non-debug version is inline
//{
//	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMainDoc)));
//	return (CMainDoc*)m_pDocument;
//}
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
			m_Camera->Update(theApp.m_fAbsoluteTime, 0.0f);
			m_SkyLightCam->Update(theApp.m_fAbsoluteTime, 0.0f);
			theApp.m_SimpleSample->SetFloat("g_Time", (float)theApp.m_fAbsoluteTime);
			theApp.m_SimpleSample->SetFloatArray("g_ScreenDim", (float *)&my::Vector2((float)m_SwapChainBufferDesc.Width, (float)m_SwapChainBufferDesc.Height), 2);
			DrawHelper::BeginLine();
			switch (m_CameraType)
			{
			case CameraTypePerspective:
				PushGrid(12, 5, 5, D3DCOLOR_ARGB(255,127,127,127), D3DCOLOR_ARGB(255,0,0,0), my::Matrix4::RotationX(D3DXToRadian(-90)));
				break;
			case CameraTypeFront:
				PushGrid(12, 5, 5, D3DCOLOR_ARGB(255,127,127,127), D3DCOLOR_ARGB(255,0,0,0), my::Matrix4::Identity());
				break;
			case CameraTypeSide:
				PushGrid(12, 5, 5, D3DCOLOR_ARGB(255,127,127,127), D3DCOLOR_ARGB(255,0,0,0), my::Matrix4::RotationY(D3DXToRadian(90)));
				break;
			case CameraTypeTop:
				PushGrid(12, 5, 5, D3DCOLOR_ARGB(255,127,127,127), D3DCOLOR_ARGB(255,0,0,0), my::Matrix4::RotationX(D3DXToRadian(-90)));
				break;
			}
			if(SUCCEEDED(hr = theApp.m_d3dDevice->BeginScene()))
			{
				m_BkColor = D3DCOLOR_ARGB(0,161,161,161);
				V(theApp.m_d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, m_BkColor, 1.0f, 0)); // ! d3dmultisample will not work
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
				V(theApp.m_d3dDevice->SetRenderTarget(0, GetScreenSurface()));
				V(theApp.m_d3dDevice->SetDepthStencilSurface(GetScreenDepthStencilSurface()));
				theApp.OnFrameRender(theApp.m_d3dDevice, &m_SwapChainBufferDesc, this, theApp.m_fAbsoluteTime, theApp.m_fElapsedTime);

				swprintf_s(&m_ScrInfos[0][0], m_ScrInfos[0].size(), L"PerformanceSec: %.3f", EndPerformanceCount());
				for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
				{
					swprintf_s(&m_ScrInfos[1+PassID][0], m_ScrInfos[1+PassID].size(), L"%S: %d", RenderPipeline::PassTypeToStr(PassID), theApp.m_PassDrawCall[PassID]);
				}

				RenderSelectedObject(theApp.m_d3dDevice);

				V(theApp.m_d3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera->m_View));
				V(theApp.m_d3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera->m_Proj));
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE));
				DrawHelper::EndLine(theApp.m_d3dDevice, my::Matrix4::identity);

				CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
				ASSERT_VALID(pFrame);
				if (!pFrame->m_selcmps.empty())
				{
					m_PivotScale = m_Camera->CalculateViewportScaler(pFrame->m_Pivot.m_Pos) * 50.0f / m_SwapChainBufferDesc.Width;
					V(theApp.m_d3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
					pFrame->m_Pivot.Draw(theApp.m_d3dDevice, m_Camera.get(), &m_SwapChainBufferDesc, m_PivotScale);
					V(theApp.m_d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE));
				}

				theApp.m_UIRender->Begin();
				theApp.m_UIRender->SetViewProj(DialogMgr::m_ViewProj);
				theApp.m_UIRender->SetWorld(my::Matrix4::Translation(my::Vector3(0.5f,0.5f,0)));
				ScrInfoType::const_iterator info_iter = m_ScrInfos.begin();
				for (int y = 5; info_iter != m_ScrInfos.end(); info_iter++, y += theApp.m_Font->m_LineHeight)
				{
					theApp.m_Font->PushString(theApp.m_UIRender.get(), &info_iter->second[0], my::Rectangle::LeftTop(5,(float)y,500,10), D3DCOLOR_ARGB(255,255,255,0));
				}
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
		StartPerformanceCount();
		ResetD3DSwapChain();
		_ASSERT(m_Camera);
		m_Camera->OnViewportChanged(my::Vector2((float)cx, (float)cy) * 0.1f);
		DialogMgr::SetDlgViewport(my::Vector2((float)cx, (float)cy), D3DXToRadian(75.0f));
	}
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	OnCameratypePerspective();
	CMainFrame::getSingleton().m_EventSelectionChanged.connect(boost::bind(&CChildView::OnSelectionChanged, this, _1));
	CMainFrame::getSingleton().m_EventPivotModeChanged.connect(boost::bind(&CChildView::OnPivotModeChanged, this, _1));

	return 0;
}

void CChildView::OnDestroy()
{
	CView::OnDestroy();

	// TODO: Add your message handler code here
	CMainFrame::getSingleton().m_EventSelectionChanged.disconnect(boost::bind(&CChildView::OnSelectionChanged, this, _1));
	CMainFrame::getSingleton().m_EventPivotModeChanged.disconnect(boost::bind(&CChildView::OnPivotModeChanged, this, _1));
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	my::Ray ray = m_Camera->CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height));
	if (!pFrame->m_selcmps.empty() && pFrame->m_Pivot.OnLButtonDown(ray, m_PivotScale))
	{
		StartPerformanceCount();
		_ASSERT(m_selcmpwlds.empty());
		CMainFrame::ComponentSet::iterator sel_iter = pFrame->m_selcmps.begin();
		for (; sel_iter != pFrame->m_selcmps.end(); sel_iter++)
		{
			m_selcmpwlds.insert(std::make_pair(*sel_iter, (*sel_iter)->m_World));
		}
		SetCapture();
		Invalidate();
		return;
	}

	pFrame->m_Tracker.TrackRubberBand(this, point, TRUE);
	pFrame->m_Tracker.m_rect.NormalizeRect();

	StartPerformanceCount();
	bool bSelectionChanged = false;
	if (!(nFlags & (MK_CONTROL|MK_SHIFT)) && !pFrame->m_selcmps.empty())
	{
		pFrame->m_selcmps.clear();
		bSelectionChanged = true;
	}

	if (!pFrame->m_Tracker.m_rect.IsRectEmpty())
	{
		my::Rectangle rc(
			(float)pFrame->m_Tracker.m_rect.left,
			(float)pFrame->m_Tracker.m_rect.top,
			(float)pFrame->m_Tracker.m_rect.right,
			(float)pFrame->m_Tracker.m_rect.bottom);
		my::Frustum ftm = m_Camera->CalculateFrustum(rc, CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height));
		struct Callback : public my::IQueryCallback
		{
			CMainFrame::ComponentSet selcmps;
			const my::Frustum & ftm;
			Callback(const my::Frustum & _ftm)
				: ftm(_ftm)
			{
			}
			void operator() (my::OctComponent * oct_cmp, my::IntersectionTests::IntersectionType)
			{
				Component * cmp = dynamic_cast<Component *>(oct_cmp);
				if (cmp && CChildView::OverlapTestFrustumAndComponent(ftm, cmp))
				{
					selcmps.insert(cmp);
				}
			}
		};
		Callback cb(ftm);
		pFrame->m_Root.QueryComponent(ftm, &cb);
		CMainFrame::ComponentSet::iterator cmp_iter = cb.selcmps.begin();
		for (; cmp_iter != cb.selcmps.end(); cmp_iter++)
		{
			pFrame->m_selcmps.insert(*cmp_iter);
			bSelectionChanged = true;
		}
	}
	else
	{
		struct Callback : public my::IQueryCallback
		{
			typedef std::map<float, Component *> ComponentMap;
			ComponentMap selcmps;
			const my::Ray & ray;
			Callback(const my::Ray & _ray)
				: ray(_ray)
			{
			}
			void operator() (my::OctComponent * oct_cmp, my::IntersectionTests::IntersectionType)
			{
				Component * cmp = dynamic_cast<Component *>(oct_cmp);
				my::RayResult ret;
				if (cmp && (ret = CChildView::OverlapTestRayAndComponent(ray, cmp), ret.first))
				{
					selcmps.insert(std::make_pair(ret.second, cmp));
				}
			}
		};
		Callback cb(ray);
		pFrame->m_Root.QueryComponent(ray, &cb);
		Callback::ComponentMap::iterator cmp_iter = cb.selcmps.begin();
		if (cmp_iter != cb.selcmps.end())
		{
			CMainFrame::ComponentSet::iterator sel_iter = pFrame->m_selcmps.find(cmp_iter->second);
			if (sel_iter != pFrame->m_selcmps.end())
			{
				pFrame->m_selcmps.erase(sel_iter);
				bSelectionChanged = true;
			}
			else
			{
				pFrame->m_selcmps.insert(cmp_iter->second);
				bSelectionChanged = true;
			}
		}
	}

	if (bSelectionChanged)
	{
		pFrame->UpdateSelBox();
		pFrame->UpdatePivotTransform();
		EventArg arg;
		pFrame->m_EventSelectionChanged(&arg);
	}

	Invalidate();
}

void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	if (pFrame->m_Pivot.m_Captured && pFrame->m_Pivot.OnLButtonUp(
		m_Camera->CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height))))
	{
		StartPerformanceCount();
		ComponentWorldMap::iterator cmp_world_iter = m_selcmpwlds.begin();
		for (; cmp_world_iter != m_selcmpwlds.end(); cmp_world_iter++)
		{
			VERIFY(pFrame->m_Root.RemoveComponent(cmp_world_iter->first));
			pFrame->m_Root.AddComponent(cmp_world_iter->first, cmp_world_iter->first->m_aabb.transform(cmp_world_iter->first->m_World), 0.1f);
		}
		m_selcmpwlds.clear();
		pFrame->UpdateSelBox();
		ReleaseCapture();
		Invalidate();
	}
}

void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	if (pFrame->m_Pivot.m_Captured && pFrame->m_Pivot.OnMouseMove(
		m_Camera->CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height)), m_PivotScale))
	{
		StartPerformanceCount();
		ComponentWorldMap::iterator cmp_world_iter = m_selcmpwlds.begin();
		for (; cmp_world_iter != m_selcmpwlds.end(); cmp_world_iter++)
		{
			switch (pFrame->m_Pivot.m_Mode)
			{
			case Pivot::PivotModeMove:
				cmp_world_iter->first->m_World = cmp_world_iter->second * my::Matrix4::Translation(pFrame->m_Pivot.m_DragDeltaPos);
				break;
			case Pivot::PivotModeRot:
				cmp_world_iter->first->m_World = cmp_world_iter->second
					* my::Matrix4::Translation(-pFrame->m_Pivot.m_Pos)
					* my::Matrix4::RotationQuaternion(pFrame->m_Pivot.m_Rot.inverse() * pFrame->m_Pivot.m_DragDeltaRot * pFrame->m_Pivot.m_Rot)
					* my::Matrix4::Translation(pFrame->m_Pivot.m_Pos);
				break;
			}
		}
		Invalidate();
	}
}

BOOL CChildView::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	ASSERT(pMsg->hwnd == m_hWnd);
	bool bNoFurtherProcessing = false;
	m_Camera->MsgProc(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam, &bNoFurtherProcessing);
	if (bNoFurtherProcessing)
	{
		switch (pMsg->message)
		{
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_bEatAltUp = TRUE;
			break;

		case WM_MOUSEMOVE:
			StartPerformanceCount();
			Invalidate();
			break;
		}
		return TRUE;
	}

	return __super::PreTranslateMessage(pMsg);
}

void CChildView::OnCameratypePerspective()
{
	// TODO: Add your command handler code here
	StartPerformanceCount();
	float fov = D3DXToRadian(75.0f);
	m_Camera.reset(new my::ModelViewerCamera(fov, m_SwapChainBufferDesc.Width / (float)m_SwapChainBufferDesc.Height, 0.1f, 3000.0f));
	boost::static_pointer_cast<my::ModelViewerCamera>(m_Camera)->m_LookAt = my::Vector3(0,0,0);
	boost::static_pointer_cast<my::ModelViewerCamera>(m_Camera)->m_Eular = my::Vector3(D3DXToRadian(-45),D3DXToRadian(45),0);
	boost::static_pointer_cast<my::ModelViewerCamera>(m_Camera)->m_Distance = cot(fov / 2) * m_CameraDiagonal * 0.5f;
	m_CameraType = CameraTypePerspective;
	Invalidate();
}

void CChildView::OnUpdateCameratypePerspective(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_CameraType == CameraTypePerspective);
}

void CChildView::OnCameratypeFront()
{
	StartPerformanceCount();
	m_Camera.reset(new my::OrthoCamera(m_CameraDiagonal, m_SwapChainBufferDesc.Width / (float)m_SwapChainBufferDesc.Height, -1500, 1500));
	boost::static_pointer_cast<my::OrthoCamera>(m_Camera)->m_Eye = my::Vector3::zero;
	boost::static_pointer_cast<my::OrthoCamera>(m_Camera)->m_Eular = my::Vector3::zero;
	m_CameraType = CameraTypeFront;
	Invalidate();
}

void CChildView::OnUpdateCameratypeFront(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_CameraType == CameraTypeFront);
}

void CChildView::OnCameratypeSide()
{
	StartPerformanceCount();
	m_Camera.reset(new my::OrthoCamera(m_CameraDiagonal, m_SwapChainBufferDesc.Width / (float)m_SwapChainBufferDesc.Height, -1500, 1500));
	boost::static_pointer_cast<my::OrthoCamera>(m_Camera)->m_Eye = my::Vector3::zero;
	boost::static_pointer_cast<my::OrthoCamera>(m_Camera)->m_Eular = my::Vector3(0,D3DXToRadian(90),0);
	m_CameraType = CameraTypeSide;
	Invalidate();
}

void CChildView::OnUpdateCameratypeSide(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_CameraType == CameraTypeSide);
}

void CChildView::OnCameratypeTop()
{
	StartPerformanceCount();
	m_Camera.reset(new my::OrthoCamera(
		m_CameraDiagonal,m_SwapChainBufferDesc.Width/(float)m_SwapChainBufferDesc.Height,-1500,1500));
	boost::static_pointer_cast<my::OrthoCamera>(m_Camera)->m_Eye = my::Vector3::zero;
	boost::static_pointer_cast<my::OrthoCamera>(m_Camera)->m_Eular = my::Vector3(D3DXToRadian(-90),0,0);
	m_CameraType = CameraTypeTop;
	Invalidate();
}

void CChildView::OnUpdateCameratypeTop(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_CameraType == CameraTypeTop);
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	switch (nChar)
	{
	case 'W':
		if (pFrame->m_Pivot.m_Mode != Pivot::PivotModeMove)
		{
			pFrame->OnCmdMsg(ID_PIVOT_MOVE, 0, NULL, NULL);
		}
		return;
	case 'E':
		if (pFrame->m_Pivot.m_Mode != Pivot::PivotModeRot)
		{
			pFrame->OnCmdMsg(ID_PIVOT_ROTATE, 0, NULL, NULL);
		}
		return;
	}

	__super::OnKeyDown(nChar, nRepCnt, nFlags);
}
