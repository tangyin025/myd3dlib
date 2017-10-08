
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "MainApp.h"

#include "ChildView.h"
#include "MainFrm.h"
#include "Component/Animator.h"

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
	ON_COMMAND(ID_SHOW_GRID, &CChildView::OnShowGrid)
	ON_UPDATE_COMMAND_UI(ID_SHOW_GRID, &CChildView::OnUpdateShowGrid)
	ON_COMMAND(ID_SHOW_CMPHANDLE, &CChildView::OnShowCmphandle)
	ON_UPDATE_COMMAND_UI(ID_SHOW_CMPHANDLE, &CChildView::OnUpdateShowCmphandle)
	ON_COMMAND(ID_RENDERMODE_WIREFRAME, &CChildView::OnRendermodeWireframe)
	ON_UPDATE_COMMAND_UI(ID_RENDERMODE_WIREFRAME, &CChildView::OnUpdateRendermodeWireframe)
	ON_COMMAND(ID_SHOW_COLLISION, &CChildView::OnShowCollisiondebug)
	ON_UPDATE_COMMAND_UI(ID_SHOW_COLLISION, &CChildView::OnUpdateShowCollisiondebug)
	ON_COMMAND(ID_RENDERMODE_DEPTHOFFIELD, &CChildView::OnRendermodeDepthoffield)
	ON_UPDATE_COMMAND_UI(ID_RENDERMODE_DEPTHOFFIELD, &CChildView::OnUpdateRendermodeDepthoffield)
	ON_COMMAND(ID_RENDERMODE_FXAA, &CChildView::OnRendermodeFxaa)
	ON_UPDATE_COMMAND_UI(ID_RENDERMODE_FXAA, &CChildView::OnUpdateRendermodeFxaa)
	ON_COMMAND(ID_RENDERMODE_SSAO, &CChildView::OnRendermodeSsao)
	ON_UPDATE_COMMAND_UI(ID_RENDERMODE_SSAO, &CChildView::OnUpdateRendermodeSsao)
END_MESSAGE_MAP()

// CChildView construction/destruction

CChildView::CChildView()
	: m_PivotScale(1.0f)
	, m_CameraDiagonal(30.0f)
	, m_CameraType(CameraTypeUnknown)
	, m_bShowGrid(TRUE)
	, m_bShowCmpHandle(TRUE)
	, m_bCopyActors(FALSE)
{
	// TODO: add construction code here
	m_SwapChainBuffer.reset(new my::Surface());
	ZeroMemory(&m_SwapChainBufferDesc, sizeof(m_SwapChainBufferDesc));
	m_SwapChainBufferDesc.Width = 100;
	m_SwapChainBufferDesc.Height = 100;
	m_DepthStencil.reset(new my::Surface());
	m_SkyBoxTextures[0] = theApp.LoadTexture("texture/Checker.bmp");
	m_SkyBoxTextures[1] = theApp.LoadTexture("texture/Checker.bmp");
	m_SkyBoxTextures[2] = theApp.LoadTexture("texture/Checker.bmp");
	m_SkyBoxTextures[3] = theApp.LoadTexture("texture/Checker.bmp");
	m_SkyBoxTextures[4] = theApp.LoadTexture("texture/Checker.bmp");
	m_SkyBoxTextures[5] = theApp.LoadTexture("texture/Checker.bmp");
	m_NormalRT.reset(new my::Texture2D());
	m_PositionRT.reset(new my::Texture2D());
	m_LightRT.reset(new my::Texture2D());
	for (unsigned int i = 0; i < RenderPipeline::RTChain::RTArray::static_size; i++)
	{
		m_OpaqueRT.m_RenderTarget[i].reset(new my::Texture2D());
		m_DownFilterRT.m_RenderTarget[i].reset(new my::Texture2D());
	}
	m_SkyLightCam.reset(new my::OrthoCamera(sqrt(30*30*2.0f),1.0f,-100,100));
	m_SkyLightCam->m_Eular = my::Vector3(D3DXToRadian(-45),D3DXToRadian(0),0);
	m_SkyLightCam->UpdateViewProj();
	ZeroMemory(&m_qwTime, sizeof(m_qwTime));
	m_SkyLightAmbient=my::Vector4(0.5,0.5,0.5,0);
	m_SkyLightDiffuse=my::Vector4(0.5,0.5,0.5,0.5);
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
		m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height, D3DFMT_D24X8, d3dpp.MultiSampleType, d3dpp.MultiSampleQuality);

	ResetRenderTargets(theApp.m_d3dDevice, &m_SwapChainBufferDesc);

	return TRUE;
}

BOOL CChildView::ResetRenderTargets(IDirect3DDevice9 * pd3dDevice, const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	m_NormalRT->OnDestroyDevice();
	m_NormalRT->CreateTexture(
		pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	m_PositionRT->OnDestroyDevice();
	m_PositionRT->CreateTexture(
		pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	m_LightRT->OnDestroyDevice();
	m_LightRT->CreateTexture(
		pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);

	for (unsigned int i = 0; i < RenderPipeline::RTChain::RTArray::static_size; i++)
	{
		m_OpaqueRT.m_RenderTarget[i]->OnDestroyDevice();
		m_OpaqueRT.m_RenderTarget[i]->CreateTexture(
			pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);

		m_DownFilterRT.m_RenderTarget[i]->OnDestroyDevice();
		m_DownFilterRT.m_RenderTarget[i]->CreateTexture(
			pBackBufferSurfaceDesc->Width / 4, pBackBufferSurfaceDesc->Height / 4, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);
	}
	return TRUE;
}

void CChildView::QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	my::ModelViewerCamera * model_view_camera = dynamic_cast<my::ModelViewerCamera *>(m_Camera.get());
	//pFrame->m_emitter->m_Emitter->m_ParticleList.clear();
	pFrame->m_WorldL.AddToPipeline(frustum, pipeline, PassMask, model_view_camera->m_LookAt);
	//pFrame->m_emitter->AddToPipeline(frustum, pipeline, PassMask);
}

void CChildView::RenderSelectedComponent(IDirect3DDevice9 * pd3dDevice, Component * cmp)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	switch (cmp->m_Type)
	{
	case Component::ComponentTypeActor:
	case Component::ComponentTypeCharacter:
		{
			Actor * actor = dynamic_cast<Actor *>(cmp);
			PushWireAABB(actor->m_Node->m_aabb.transform(
				my::Matrix4::Translation(actor->GetLevel()->GetOffset())), D3DCOLOR_ARGB(255,255,0,255));
			Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
			for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
			{
				RenderSelectedComponent(pd3dDevice, cmp_iter->get());
			}
		}
		break;
	case Component::ComponentTypeMesh:
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(cmp);
			if (mesh_cmp->m_MeshRes.m_Res)
			{
				theApp.m_SimpleSample->SetMatrix("g_World", mesh_cmp->m_World);
				theApp.m_SimpleSample->SetVector("g_MeshColor", my::Vector4(0,1,0,1));
				theApp.m_SimpleSample->SetTechnique("RenderSceneWireColor");
				UINT passes = theApp.m_SimpleSample->Begin();
				for (unsigned int i = 0; i < mesh_cmp->m_MaterialList.size(); i++)
				{
					theApp.m_SimpleSample->BeginPass(0);
					mesh_cmp->m_MeshRes.m_Res->DrawSubset(i);
					theApp.m_SimpleSample->EndPass();
				}
				theApp.m_SimpleSample->End();
			}
		}
		break;
	case Component::ComponentTypeTerrain:
		{
			Terrain * terrain = dynamic_cast<Terrain *>(cmp);
			for (unsigned int i = 0; i < Terrain::ChunkArray2D::static_size; i++)
			{
				for (unsigned int j = 0; j < Terrain::ChunkArray::static_size; j++)
				{
					PushWireAABB(terrain->m_Chunks[i][j]->m_aabb.transform(terrain->m_World), D3DCOLOR_ARGB(255,255,0,255));
				}
			}
		}
		break;
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
	case Component::ComponentTypeActor:
	case Component::ComponentTypeCharacter:
		{
			Actor * actor = dynamic_cast<Actor *>(cmp);
			if (actor->m_Cmps.empty())
			{
				my::IntersectionTests::IntersectionType intersect_type = my::IntersectionTests::IntersectAABBAndFrustum(actor->m_aabb, local_ftm);
				if (intersect_type != my::IntersectionTests::IntersectionTypeOutside)
				{
					return true;
				}
			}
			Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
			for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
			{
				if (OverlapTestFrustumAndComponent(frustum, cmp_iter->get()))
				{
					return true;
				}
			}
		}
		break;

	case Component::ComponentTypeMesh:
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(cmp);
			my::OgreMeshPtr mesh = mesh_cmp->m_MeshRes.m_Res;
			if (!mesh)
			{
				return false;
			}
			if (mesh_cmp->m_bUseAnimation
				&& mesh_cmp->m_Actor
				&& mesh_cmp->m_Actor->m_Animator
				&& !mesh_cmp->m_Actor->m_Animator->m_DualQuats.empty())
			{
				std::vector<my::Vector3> vertices(mesh->GetNumVertices());
				my::D3DVertexElementSet elems;
				elems.InsertPositionElement(0);
				my::OgreMesh::ComputeDualQuaternionSkinnedVertices(
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					elems,
					mesh->LockVertexBuffer(D3DLOCK_READONLY),
					mesh->GetNumBytesPerVertex(),
					mesh->m_VertexElems,
					mesh_cmp->m_Actor->m_Animator->m_DualQuats);
				bool ret = OverlapTestFrustumAndMesh(local_ftm,
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					mesh->LockIndexBuffer(D3DLOCK_READONLY),
					!(mesh->GetOptions() & D3DXMESH_32BIT),
					mesh->GetNumFaces(),
					elems);
				mesh->UnlockVertexBuffer();
				mesh->UnlockIndexBuffer();
				if (ret)
				{
					return true;
				}
			}
			else
			{
				bool ret = OverlapTestFrustumAndMesh(local_ftm,
					mesh->LockVertexBuffer(D3DLOCK_READONLY),
					mesh->GetNumVertices(),
					mesh->GetNumBytesPerVertex(),
					mesh->LockIndexBuffer(D3DLOCK_READONLY),
					!(mesh->GetOptions() & D3DXMESH_32BIT),
					mesh->GetNumFaces(),
					mesh->m_VertexElems);
				mesh->UnlockVertexBuffer();
				mesh->UnlockIndexBuffer();
				if (ret)
				{
					return true;
				}
			}
		}
		break;

	case Component::ComponentTypeStaticEmitter:
	case Component::ComponentTypeSphericalEmitter:
		{
			const my::Vector3 & Center = cmp->m_World.row<3>().xyz;
			const my::Vector3 Right = m_Camera->m_View.column<0>().xyz.normalize() * 0.5f;
			const my::Vector3 Up = m_Camera->m_View.column<1>().xyz.normalize() * 0.5f;
			const my::Vector3 v[4] = { Center - Right + Up, Center - Right - Up, Center + Right + Up, Center + Right - Up };
			my::IntersectionTests::IntersectionType result = my::IntersectionTests::IntersectTriangleAndFrustum(v[0], v[1], v[2], frustum);
			if (result == my::IntersectionTests::IntersectionTypeInside || result == my::IntersectionTests::IntersectionTypeIntersect)
			{
				return true;
			}
			result = my::IntersectionTests::IntersectTriangleAndFrustum(v[2], v[1], v[3], frustum);
			if (result == my::IntersectionTests::IntersectionTypeInside || result == my::IntersectionTests::IntersectionTypeIntersect)
			{
				return true;
			}
		}
		break;

	case Component::ComponentTypeCloth:
		{
			ClothComponent * cloth_cmp = dynamic_cast<ClothComponent *>(cmp);
			if (cloth_cmp->m_VertexData.empty())
			{
				return false;
			}
			if (cloth_cmp->m_bUseAnimation
				&& cloth_cmp->m_Actor
				&& cloth_cmp->m_Actor->m_Animator
				&& !cloth_cmp->m_Actor->m_Animator->m_DualQuats.empty())
			{
				std::vector<my::Vector3> vertices(cloth_cmp->m_VertexData.size() / cloth_cmp->m_VertexStride);
				my::D3DVertexElementSet elems;
				elems.InsertPositionElement(0);
				my::OgreMesh::ComputeDualQuaternionSkinnedVertices(
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					elems,
					&cloth_cmp->m_VertexData[0],
					cloth_cmp->m_VertexStride,
					cloth_cmp->m_VertexElems,
					cloth_cmp->m_Actor->m_Animator->m_DualQuats);
				bool ret = OverlapTestFrustumAndMesh(local_ftm,
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					&cloth_cmp->m_IndexData[0],
					true,
					cloth_cmp->m_IndexData.size() / 3,
					elems);
				if (ret)
				{
					return true;
				}
			}
			else
			{
				bool ret = OverlapTestFrustumAndMesh(local_ftm,
					&cloth_cmp->m_VertexData[0],
					cloth_cmp->m_VertexData.size() / cloth_cmp->m_VertexStride,
					cloth_cmp->m_VertexStride,
					&cloth_cmp->m_IndexData[0],
					true,
					cloth_cmp->m_IndexData.size() / 3,
					cloth_cmp->m_VertexElems);
				if (ret)
				{
					return true;
				}
			}
		}
		break;

	case Component::ComponentTypeTerrain:
		break;

	//case Component::ComponentTypeRigid:
	//	break;
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
		int i0 = bIndices16 ? *((WORD *)pIndices + face_i * 3 + 0) : *((DWORD *)pIndices + face_i * 3 + 0);
		int i1 = bIndices16 ? *((WORD *)pIndices + face_i * 3 + 1) : *((DWORD *)pIndices + face_i * 3 + 1);
		int i2 = bIndices16 ? *((WORD *)pIndices + face_i * 3 + 2) : *((DWORD *)pIndices + face_i * 3 + 2);

		const my::Vector3 & v0 = VertexElems.GetPosition((unsigned char *)pVertices + i0 * VertexStride);
		const my::Vector3 & v1 = VertexElems.GetPosition((unsigned char *)pVertices + i1 * VertexStride);
		const my::Vector3 & v2 = VertexElems.GetPosition((unsigned char *)pVertices + i2 * VertexStride);

		my::IntersectionTests::IntersectionType result = my::IntersectionTests::IntersectTriangleAndFrustum(v0, v1, v2, frustum);
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
	case Component::ComponentTypeActor:
	case Component::ComponentTypeCharacter:
		{
			Actor * actor = dynamic_cast<Actor *>(cmp);
			if (actor->m_Cmps.empty())
			{
				my::RayResult ret = my::IntersectionTests::rayAndAABB(local_ray.p, local_ray.d, actor->m_aabb);
				if (ret.first)
				{
					ret.second = (local_ray.d * ret.second).transformNormal(actor->m_World).magnitude();
					return ret;
				}
			}
			Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
			for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
			{
				my::RayResult ret = OverlapTestRayAndComponent(ray, cmp_iter->get());
				if (ret.first)
				{
					return ret;
				}
			}
		}
		break;

	case Component::ComponentTypeMesh:
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(cmp);
			my::OgreMeshPtr mesh = mesh_cmp->m_MeshRes.m_Res;
			if (!mesh)
			{
				return my::RayResult(false, FLT_MAX);
			}
			my::RayResult ret;
			if (mesh_cmp->m_bUseAnimation
				&& mesh_cmp->m_Actor
				&& mesh_cmp->m_Actor->m_Animator
				&& !mesh_cmp->m_Actor->m_Animator->m_DualQuats.empty())
			{
				std::vector<my::Vector3> vertices(mesh->GetNumVertices());
				my::D3DVertexElementSet elems;
				elems.InsertPositionElement(0);
				my::OgreMesh::ComputeDualQuaternionSkinnedVertices(
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					elems,
					mesh->LockVertexBuffer(D3DLOCK_READONLY),
					mesh->GetNumBytesPerVertex(),
					mesh->m_VertexElems,
					mesh_cmp->m_Actor->m_Animator->m_DualQuats);
				ret = OverlapTestRayAndMesh(local_ray,
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					mesh->LockIndexBuffer(D3DLOCK_READONLY),
					!(mesh->GetOptions() & D3DXMESH_32BIT),
					mesh->GetNumFaces(),
					elems);
				mesh->UnlockVertexBuffer();
				mesh->UnlockIndexBuffer();
			}
			else
			{
				ret = OverlapTestRayAndMesh(local_ray,
					mesh->LockVertexBuffer(D3DLOCK_READONLY),
					mesh->GetNumVertices(),
					mesh->GetNumBytesPerVertex(),
					mesh->LockIndexBuffer(D3DLOCK_READONLY),
					!(mesh->GetOptions() & D3DXMESH_32BIT),
					mesh->GetNumFaces(),
					mesh->m_VertexElems);
				mesh->UnlockVertexBuffer();
				mesh->UnlockIndexBuffer();
			}
			if (ret.first)
			{
				ret.second = (local_ray.d * ret.second).transformNormal(mesh_cmp->m_World).magnitude();
				return ret;
			}
		}
		break;

	case Component::ComponentTypeStaticEmitter:
	case Component::ComponentTypeSphericalEmitter:
		{
			const my::Vector3 & Center = cmp->m_World.row<3>().xyz;
			const my::Vector3 Right = m_Camera->m_View.column<0>().xyz.normalize() * 0.5f;
			const my::Vector3 Up = m_Camera->m_View.column<1>().xyz.normalize() * 0.5f;
			const my::Vector3 v[4] = { Center - Right + Up, Center - Right - Up, Center + Right + Up, Center + Right - Up };
			my::RayResult ret = my::IntersectionTests::rayAndTriangle(ray.p, ray.d, v[0], v[1], v[2]);
			if (ret.first)
			{
				return ret;
			}
			ret = my::IntersectionTests::rayAndTriangle(ray.p, ray.d, v[2], v[1], v[3]);
			if (ret.first)
			{
				return ret;
			}
		}
		break;

	case Component::ComponentTypeCloth:
		{
			ClothComponent * cloth_cmp = dynamic_cast<ClothComponent *>(cmp);
			if (cloth_cmp->m_VertexData.empty())
			{
				return my::RayResult(false, FLT_MAX);
			}
			my::RayResult ret;
			if (cloth_cmp->m_bUseAnimation
				&& cloth_cmp->m_Actor
				&& cloth_cmp->m_Actor->m_Animator
				&& !cloth_cmp->m_Actor->m_Animator->m_DualQuats.empty())
			{
				std::vector<my::Vector3> vertices(cloth_cmp->m_VertexData.size() / cloth_cmp->m_VertexStride);
				my::D3DVertexElementSet elems;
				elems.InsertPositionElement(0);
				my::OgreMesh::ComputeDualQuaternionSkinnedVertices(
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					elems,
					&cloth_cmp->m_VertexData[0],
					cloth_cmp->m_VertexStride,
					cloth_cmp->m_VertexElems,
					cloth_cmp->m_Actor->m_Animator->m_DualQuats);
				ret = OverlapTestRayAndMesh(local_ray,
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					&cloth_cmp->m_IndexData[0],
					true,
					cloth_cmp->m_IndexData.size() / 3,
					elems);
			}
			else
			{
				ret = OverlapTestRayAndMesh(local_ray,
					&cloth_cmp->m_VertexData[0],
					cloth_cmp->m_VertexData.size() / cloth_cmp->m_VertexStride,
					cloth_cmp->m_VertexStride,
					&cloth_cmp->m_IndexData[0],
					true,
					cloth_cmp->m_IndexData.size() / 3,
					cloth_cmp->m_VertexElems);
			}
			if (ret.first)
			{
				ret.second = (local_ray.d * ret.second).transformNormal(cloth_cmp->m_World).magnitude();
				return ret;
			}
		}
		break;

	case Component::ComponentTypeTerrain:
		{
			struct Callback : public my::OctNodeBase::QueryCallback
			{
				const my::Ray & ray;
				const my::Vector3 & LocalViewPos;
				CChildView * pView;
				Terrain * terrain;
				my::RayResult ret;
				Callback(const my::Ray & _ray, const my::Vector3 & _LocalViewPos, CChildView * _pView, Terrain * _terrain)
					: ray(_ray)
					, pView(_pView)
					, LocalViewPos(_LocalViewPos)
					, terrain(_terrain)
					, ret(false, FLT_MAX)
				{
				}
				void operator() (my::OctActor * oct_actor, my::IntersectionTests::IntersectionType)
				{
					TerrainChunk * chunk = dynamic_cast<TerrainChunk *>(oct_actor);
					const Terrain::Fragment & frag = terrain->GetFragment(
						terrain->CalculateLod(chunk->m_Row, chunk->m_Column, LocalViewPos),
						terrain->CalculateLod(chunk->m_Row, chunk->m_Column - 1, LocalViewPos),
						terrain->CalculateLod(chunk->m_Row - 1, chunk->m_Column, LocalViewPos),
						terrain->CalculateLod(chunk->m_Row, chunk->m_Column + 1, LocalViewPos),
						terrain->CalculateLod(chunk->m_Row + 1, chunk->m_Column, LocalViewPos));
					my::RayResult result = pView->OverlapTestRayAndTerrainChunk(
						ray,
						terrain,
						chunk,
						terrain->m_vb.Lock(0, 0, D3DLOCK_READONLY),
						frag.VertNum,
						terrain->m_VertexStride,
						const_cast<my::IndexBuffer&>(frag.ib).Lock(0, 0, D3DLOCK_READONLY),
						frag.PrimitiveCount);
					terrain->m_vb.Unlock();
					const_cast<my::IndexBuffer&>(frag.ib).Unlock();
					if (result.first && result.second < ret.second)
					{
						ret = result;
					}
				}
			};
			Terrain * terrain = dynamic_cast<Terrain *>(cmp);
			if (!terrain->m_vb.m_ptr)
			{
				return my::RayResult(false, FLT_MAX);
			}
			my::ModelViewerCamera * model_view_camera = dynamic_cast<my::ModelViewerCamera *>(m_Camera.get());
			my::Vector3 loc_viewpos = model_view_camera->m_LookAt.transformCoord(terrain->m_World.inverse());
			Callback cb(local_ray, loc_viewpos, this, terrain);
			terrain->m_Root.QueryActor(local_ray, &cb);
			if (cb.ret.first)
			{
				cb.ret.second = (local_ray.d * cb.ret.second).transformNormal(terrain->m_World).magnitude();
				return cb.ret;
			}
		}
		break;

	//case Component::ComponentTypeRigid:
	//	{
	//		RigidComponent * rigid_cmp = dynamic_cast<RigidComponent *>(cmp);
	//		_ASSERT(rigid_cmp->m_RigidActor);
	//		unsigned int NbShapes = rigid_cmp->m_RigidActor->getNbShapes();
	//		std::vector<PxShape *> shapes(NbShapes);
	//		NbShapes = rigid_cmp->m_RigidActor->getShapes(&shapes[0], shapes.size(), 0);
	//		for (unsigned int i = 0; i < NbShapes; i++)
	//		{
	//			PxRaycastHit hits[1];
	//			if (PxGeometryQuery::raycast(
	//				(PxVec3&)ray.p,
	//				(PxVec3&)ray.d,
	//				shapes[i]->getGeometry().any(),
	//				PxShapeExt::getGlobalPose(*shapes[i]),
	//				3000.0f,
	//				PxSceneQueryFlags(PxSceneQueryFlag::eDISTANCE),
	//				_countof(hits), hits, true))
	//			{
	//				return my::RayResult(true, hits[0].distance);
	//			}
	//		}
	//	}
	//	break;
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
		int i0 = bIndices16 ? *((WORD *)pIndices + face_i * 3 + 0) : *((DWORD *)pIndices + face_i * 3 + 0);
		int i1 = bIndices16 ? *((WORD *)pIndices + face_i * 3 + 1) : *((DWORD *)pIndices + face_i * 3 + 1);
		int i2 = bIndices16 ? *((WORD *)pIndices + face_i * 3 + 2) : *((DWORD *)pIndices + face_i * 3 + 2);

		const my::Vector3 & v0 = VertexElems.GetPosition((unsigned char *)pVertices + i0 * VertexStride);
		const my::Vector3 & v1 = VertexElems.GetPosition((unsigned char *)pVertices + i1 * VertexStride);
		const my::Vector3 & v2 = VertexElems.GetPosition((unsigned char *)pVertices + i2 * VertexStride);

		my::RayResult result = my::CollisionDetector::rayAndTriangle(ray.p, ray.d, v0, v1, v2);
		if (result.first && result.second < ret.second)
		{
			ret = result;
		}
	}
	return ret;
}

my::RayResult CChildView::OverlapTestRayAndTerrainChunk(
	const my::Ray & ray,
	Terrain * terrain,
	TerrainChunk * chunk,
	void * pVertices,
	DWORD NumVerts,
	DWORD VertexStride,
	void * pIndices,
	DWORD NumFaces)
{
	my::RayResult ret(false, FLT_MAX);
	D3DLOCKED_RECT lrc = terrain->m_HeightMap.LockRect(NULL, 0, 0);
	for(unsigned int face_i = 0; face_i < NumFaces; face_i++)
	{
		int i0 = *((WORD *)pIndices + face_i * 3 + 0);
		int i1 = *((WORD *)pIndices + face_i * 3 + 1);
		int i2 = *((WORD *)pIndices + face_i * 3 + 2);

		unsigned char * pv0 = terrain->m_VertexElems.GetVertexValue<unsigned char>((unsigned char *)pVertices + i0 * VertexStride, D3DDECLUSAGE_TEXCOORD, 0);
		unsigned char * pv1 = terrain->m_VertexElems.GetVertexValue<unsigned char>((unsigned char *)pVertices + i1 * VertexStride, D3DDECLUSAGE_TEXCOORD, 0);
		unsigned char * pv2 = terrain->m_VertexElems.GetVertexValue<unsigned char>((unsigned char *)pVertices + i2 * VertexStride, D3DDECLUSAGE_TEXCOORD, 0);

		my::Vector3 v0 = terrain->GetSamplePos(lrc.pBits, lrc.Pitch, chunk->m_Row * terrain->CHUNK_SIZE + pv0[0], chunk->m_Column * terrain->CHUNK_SIZE + pv0[1]);
		my::Vector3 v1 = terrain->GetSamplePos(lrc.pBits, lrc.Pitch, chunk->m_Row * terrain->CHUNK_SIZE + pv1[0], chunk->m_Column * terrain->CHUNK_SIZE + pv1[1]);
		my::Vector3 v2 = terrain->GetSamplePos(lrc.pBits, lrc.Pitch, chunk->m_Row * terrain->CHUNK_SIZE + pv2[0], chunk->m_Column * terrain->CHUNK_SIZE + pv2[1]);

		my::RayResult result = my::CollisionDetector::rayAndTriangle(ray.p, ray.d, v0, v1, v2);
		if (result.first && result.second < ret.second)
		{
			ret = result;
		}
	}
	terrain->m_HeightMap.UnlockRect(0);
	return ret;
}

void CChildView::OnSelectionChanged(EventArgs * arg)
{
	Invalidate();
}

void CChildView::OnSelectionPlaying(EventArgs * arg)
{
	Invalidate();
}

void CChildView::OnPivotModeChanged(EventArgs * arg)
{
	Invalidate();
}

void CChildView::OnCmpAttriChanged(EventArgs * arg)
{
	Invalidate();
}

void CChildView::PostCameraViewChanged(void)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	if (m_CameraType == CameraTypePerspective)
	{
		my::ModelViewerCamera * model_view_camera = dynamic_cast<my::ModelViewerCamera *>(m_Camera.get());
		if (pFrame->m_WorldL.ResetLevelId(model_view_camera->m_LookAt, pFrame))
		{
			model_view_camera->UpdateViewProj();
		}
		pFrame->m_WorldL.ResetViewedActors(model_view_camera->m_LookAt, pFrame, 10, 1);
		pFrame->UpdateSelBox();
		pFrame->UpdatePivotTransform();
	}
	StartPerformanceCount();
	Invalidate();
	CEnvironmentWnd::CameraPropEventArgs arg(this);
	pFrame->m_EventCameraPropChanged(&arg);
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
			theApp.m_SimpleSample->SetFloatArray("g_ScreenDim", (float *)&my::Vector2((float)m_SwapChainBufferDesc.Width, (float)m_SwapChainBufferDesc.Height), 2);

			DrawHelper::BeginLine();

			if (m_bShowGrid)
			{
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
			}

			CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
			ASSERT_VALID(pFrame);
			pFrame->PushRenderBuffer(this);

			if(SUCCEEDED(hr = theApp.m_d3dDevice->BeginScene()))
			{
				m_BkColor = D3DCOLOR_ARGB(0,161,161,161);
				V(theApp.m_d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, m_BkColor, 1.0f, 0)); // ! d3dmultisample will not work
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
				V(theApp.m_d3dDevice->SetRenderTarget(0, m_SwapChainBuffer->m_ptr));
				V(theApp.m_d3dDevice->SetDepthStencilSurface(m_DepthStencil->m_ptr));
				theApp.OnFrameRender(theApp.m_d3dDevice, &m_SwapChainBufferDesc, this, theApp.m_fAbsoluteTime, theApp.m_fElapsedTime);

				swprintf_s(&m_ScrInfos[0][0], m_ScrInfos[0].size(), L"PerformanceSec: %.3f", EndPerformanceCount());
				for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
				{
					swprintf_s(&m_ScrInfos[1+PassID][0], m_ScrInfos[1+PassID].size(), L"%S: %d", RenderPipeline::PassTypeToStr(PassID), theApp.m_PassDrawCall[PassID]);
				}

				V(theApp.m_d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE));
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
				if (!pFrame->m_selactors.empty())
				{
					theApp.m_SimpleSample->SetMatrix("g_View", m_Camera->m_View);
					theApp.m_SimpleSample->SetMatrix("g_ViewProj", m_Camera->m_ViewProj);
					PushWireAABB(pFrame->m_selbox, D3DCOLOR_ARGB(255,255,255,255));
					CMainFrame::ActorSet::const_iterator sel_iter = pFrame->m_selactors.begin();
					for (; sel_iter != pFrame->m_selactors.end(); sel_iter++)
					{
						RenderSelectedComponent(theApp.m_d3dDevice, *sel_iter);
					}
				}

				V(theApp.m_d3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera->m_View));
				V(theApp.m_d3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera->m_Proj));
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE));
				DrawHelper::EndLine(theApp.m_d3dDevice, my::Matrix4::identity);

				if (!pFrame->m_selactors.empty())
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
		m_Camera->UpdateViewProj();
		DialogMgr::SetDlgViewport(my::Vector2((float)cx, (float)cy), D3DXToRadian(75.0f));
	}
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	OnCameratypePerspective();
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventSelectionChanged.connect(boost::bind(&CChildView::OnSelectionChanged, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventSelectionPlaying.connect(boost::bind(&CChildView::OnSelectionPlaying, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventPivotModeChanged.connect(boost::bind(&CChildView::OnPivotModeChanged, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventAttributeChanged.connect(boost::bind(&CChildView::OnCmpAttriChanged, this, _1));
	return 0;
}

void CChildView::OnDestroy()
{
	CView::OnDestroy();

	//// TODO: Add your message handler code here
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventSelectionChanged.disconnect(boost::bind(&CChildView::OnSelectionChanged, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventSelectionPlaying.disconnect(boost::bind(&CChildView::OnSelectionPlaying, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventPivotModeChanged.disconnect(boost::bind(&CChildView::OnPivotModeChanged, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventAttributeChanged.disconnect(boost::bind(&CChildView::OnCmpAttriChanged, this, _1));
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	my::Ray ray = m_Camera->CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height));
	if (!pFrame->m_selactors.empty() && pFrame->m_Pivot.OnLButtonDown(ray, m_PivotScale))
	{
		StartPerformanceCount();
		_ASSERT(m_selactorwlds.empty());
		CMainFrame::ActorSet::iterator sel_iter = pFrame->m_selactors.begin();
		for (; sel_iter != pFrame->m_selactors.end(); sel_iter++)
		{
			m_selactorwlds[*sel_iter] = (*sel_iter)->m_World;
		}
		m_bCopyActors = (nFlags & MK_SHIFT) ? TRUE : FALSE;
		SetCapture();
		Invalidate();
		return;
	}

	pFrame->m_Tracker.TrackRubberBand(this, point, TRUE);
	pFrame->m_Tracker.m_rect.NormalizeRect();

	StartPerformanceCount();
	bool bSelectionChanged = false;
	if (!(nFlags & MK_SHIFT) && !pFrame->m_selactors.empty())
	{
		pFrame->m_selactors.clear();
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
		struct Callback : public WorldL::QueryCallback
		{
			CMainFrame::ActorSet & selacts;
			const my::Frustum & ftm;
			CChildView * pView;
			Callback(CMainFrame::ActorSet & _selacts, const my::Frustum & _ftm, CChildView * _pView)
				: selacts(_selacts)
				, ftm(_ftm)
				, pView(_pView)
			{
			}
			void operator() (Octree * level, const CPoint & level_id)
			{
				struct Callback : public my::OctNodeBase::QueryCallback
				{
					CMainFrame::ActorSet & selacts;
					const my::Frustum & ftm;
					CChildView * pView;
					Callback(CMainFrame::ActorSet & _selacts, const my::Frustum & _ftm, CChildView * _pView)
						: selacts(_selacts)
						, ftm(_ftm)
						, pView(_pView)
					{
					}
					void operator() (my::OctActor * oct_actor, my::IntersectionTests::IntersectionType)
					{
						Actor * actor = dynamic_cast<Actor *>(oct_actor);
						if (actor && pView->OverlapTestFrustumAndComponent(ftm, actor))
						{
							selacts.insert(actor);
						}
					}
				};
				CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
				ASSERT_VALID(pFrame);
				my::Vector3 Offset((float)(level_id.x - pFrame->m_WorldL.m_LevelId.x) * WorldL::LEVEL_SIZE, 0, (float)(level_id.y - pFrame->m_WorldL.m_LevelId.y) * WorldL::LEVEL_SIZE);
				my::Frustum local_ftm = ftm.transform(my::Matrix4::Translation(Offset).transpose());
				level->QueryActor(local_ftm, &Callback(selacts, ftm, pView));
			}
		};
		pFrame->m_WorldL.QueryLevel(pFrame->m_WorldL.m_LevelId, &Callback(pFrame->m_selactors, ftm, this));
	}
	else
	{
		typedef std::map<float, Actor *> ActorMap;
		ActorMap selacts;
		struct Callback : public WorldL::QueryCallback
		{
			ActorMap & selacts;
			const my::Ray & ray;
			CChildView * pView;
			Callback(ActorMap & _selacts, const my::Ray & _ray, CChildView * _pView)
				: selacts(_selacts)
				, ray(_ray)
				, pView(_pView)
			{
			}
			void operator() (Octree * level, const CPoint & level_id)
			{
				struct Callback : public my::OctNodeBase::QueryCallback
				{
					ActorMap & selacts;
					const my::Ray & ray;
					CChildView * pView;
					Callback(ActorMap & _selacts, const my::Ray & _ray, CChildView * _pView)
						: selacts(_selacts)
						, ray(_ray)
						, pView(_pView)
					{
					}
					void operator() (my::OctActor * oct_actor, my::IntersectionTests::IntersectionType)
					{
						Actor * actor = dynamic_cast<Actor *>(oct_actor);
						my::RayResult ret;
						if (actor && (ret = pView->OverlapTestRayAndComponent(ray, actor), ret.first))
						{
							selacts.insert(std::make_pair(ret.second, actor));
						}
					}
				};
				CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
				ASSERT_VALID(pFrame);
				my::Vector3 Offset((float)(level_id.x - pFrame->m_WorldL.m_LevelId.x) * WorldL::LEVEL_SIZE, 0, (float)(level_id.y - pFrame->m_WorldL.m_LevelId.y) * WorldL::LEVEL_SIZE);
				my::Ray local_ray(ray.p - Offset, ray.d);
				level->QueryActor(local_ray, &Callback(selacts, ray, pView));
			}
		};
		pFrame->m_WorldL.QueryLevel(pFrame->m_WorldL.m_LevelId, &Callback(selacts, ray, this));
		ActorMap::iterator cmp_iter = selacts.begin();
		if (cmp_iter != selacts.end())
		{
			CMainFrame::ActorSet::iterator sel_iter = pFrame->m_selactors.find(cmp_iter->second);
			if (sel_iter != pFrame->m_selactors.end())
			{
				pFrame->m_selactors.erase(sel_iter);
				bSelectionChanged = true;
			}
			else
			{
				pFrame->m_selactors.insert(cmp_iter->second);
				bSelectionChanged = true;
			}
		}
	}

	if (bSelectionChanged)
	{
		pFrame->UpdateSelBox();
		pFrame->UpdatePivotTransform();
		EventArgs arg;
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
		ActorWorldMap::iterator actor_world_iter = m_selactorwlds.begin();
		for (; actor_world_iter != m_selactorwlds.end(); actor_world_iter++)
		{
			my::Vector3 Position, Scale; my::Quaternion Rotation;
			actor_world_iter->first->m_World.Decompose(Scale, Rotation, Position);
			pFrame->SafeChangeActorPose(actor_world_iter->first, Position, Rotation, Scale);
			actor_world_iter->first->UpdateRigidActorPose();
		}
		m_selactorwlds.clear();
		pFrame->UpdateSelBox();
		pFrame->UpdatePivotTransform();
		ReleaseCapture();

		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
	}
}

void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	if (pFrame->m_Pivot.m_Captured && pFrame->m_Pivot.OnMouseMove(
		m_Camera->CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height)), m_PivotScale))
	{
		if (m_bCopyActors)
		{
			m_bCopyActors = FALSE;
			CMainFrame::ActorSet new_acts;
			CMainFrame::ActorSet::const_iterator sel_iter = pFrame->m_selactors.begin();
			for (; sel_iter != pFrame->m_selactors.end(); sel_iter++)
			{
				ActorPtr new_actor = boost::dynamic_pointer_cast<Actor>((*sel_iter)->Clone());
				CPoint level_id = (*sel_iter)->GetLevel()->GetId();
				pFrame->m_WorldL.GetLevel(level_id)->AddActor(new_actor, new_actor->m_aabb.transform(new_actor->CalculateLocal()), 0.1f);
				new_actor->RequestResource();
				new_actor->OnEnterPxScene(pFrame);
				pFrame->m_WorldL.m_ViewedActors.insert(new_actor.get());
				new_acts.insert(new_actor.get());
			}
		}
		StartPerformanceCount();
		ActorWorldMap::iterator actor_world_iter = m_selactorwlds.begin();
		for (; actor_world_iter != m_selactorwlds.end(); actor_world_iter++)
		{
			switch (pFrame->m_Pivot.m_Mode)
			{
			case Pivot::PivotModeMove:
				actor_world_iter->first->m_World = actor_world_iter->second * my::Matrix4::Translation(pFrame->m_Pivot.m_DragDeltaPos);
				break;
			case Pivot::PivotModeRot:
				actor_world_iter->first->m_World = actor_world_iter->second
					* my::Matrix4::Translation(-pFrame->m_Pivot.m_Pos)
					* my::Matrix4::RotationQuaternion(pFrame->m_Pivot.m_DragDeltaRot)
					* my::Matrix4::Translation(pFrame->m_Pivot.m_Pos);
				break;
			}
			Actor::ComponentPtrList::iterator cmp_iter = actor_world_iter->first->m_Cmps.begin();
			for (; cmp_iter != actor_world_iter->first->m_Cmps.end(); cmp_iter++)
			{
				(*cmp_iter)->UpdateWorld();
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
		CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
		ASSERT_VALID(pFrame);
		switch (pMsg->message)
		{
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_bEatAltUp = TRUE;
			break;

		case WM_MOUSEMOVE:
			{
				m_Camera->UpdateViewProj();
				PostCameraViewChanged();
			}
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
	m_Camera->m_Eular = my::Vector3(D3DXToRadian(-45),D3DXToRadian(45),0);
	boost::static_pointer_cast<my::ModelViewerCamera>(m_Camera)->m_LookAt = my::Vector3(0,0,0);
	boost::static_pointer_cast<my::ModelViewerCamera>(m_Camera)->m_Distance = cot(fov / 2) * m_CameraDiagonal * 0.5f;
	m_Camera->UpdateViewProj();
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
	m_Camera->m_Eye = my::Vector3::zero;
	m_Camera->m_Eular = my::Vector3::zero;
	m_Camera->UpdateViewProj();
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
	m_Camera->m_Eye = my::Vector3::zero;
	m_Camera->m_Eular = my::Vector3(0,D3DXToRadian(90),0);
	m_Camera->UpdateViewProj();
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
	m_Camera->m_Eye = my::Vector3::zero;
	m_Camera->m_Eular = my::Vector3(D3DXToRadian(-90),0,0);
	m_Camera->UpdateViewProj();
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
	case 'F':
		{
			float fov = D3DXToRadian(75.0f);
			if (!pFrame->m_selactors.empty())
			{
				switch (m_CameraType)
				{
				case CChildView::CameraTypePerspective:
					boost::dynamic_pointer_cast<my::ModelViewerCamera>(m_Camera)->m_LookAt = pFrame->m_selbox.Center();
					boost::dynamic_pointer_cast<my::ModelViewerCamera>(m_Camera)->m_Distance = cot(fov / 2) * m_CameraDiagonal * 0.5f;
					break;
				}
			}
			else
			{
				switch (m_CameraType)
				{
				case CChildView::CameraTypePerspective:
					boost::dynamic_pointer_cast<my::ModelViewerCamera>(m_Camera)->m_LookAt = my::Vector3(0,0,0);
					boost::dynamic_pointer_cast<my::ModelViewerCamera>(m_Camera)->m_Distance = cot(fov / 2) * m_CameraDiagonal * 0.5f;
					break;
				}
			}
			m_Camera->UpdateViewProj();
			PostCameraViewChanged();
		}
		return;
	}

	__super::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChildView::OnShowGrid()
{
	// TODO: Add your command handler code here
	m_bShowGrid = !m_bShowGrid;
	Invalidate();
}

void CChildView::OnUpdateShowGrid(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_bShowGrid);
}

void CChildView::OnShowCmphandle()
{
	// TODO: Add your command handler code here
	m_bShowCmpHandle = !m_bShowCmpHandle;
	Invalidate();
}

void CChildView::OnUpdateShowCmphandle(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_bShowCmpHandle);
}

void CChildView::OnRendermodeWireframe()
{
	// TODO: Add your command handler code here
	m_WireFrame = !m_WireFrame;
	Invalidate();
}

void CChildView::OnUpdateRendermodeWireframe(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_WireFrame);
}

void CChildView::OnShowCollisiondebug()
{
	// TODO: Add your command handler code here
	physx::PxScene * scene = (DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_PxScene.get();
	ASSERT(scene);
	if (scene->getVisualizationParameter(physx::PxVisualizationParameter::eSCALE) > 0)
	{
		scene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 0.0f);
	}
	else
	{
		scene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.0f);
	}
	Invalidate();
}

void CChildView::OnUpdateShowCollisiondebug(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	physx::PxScene * scene = (DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_PxScene.get();
	ASSERT(scene);
	pCmdUI->SetCheck(scene->getVisualizationParameter(physx::PxVisualizationParameter::eSCALE) > 0);
}

void CChildView::OnRendermodeDepthoffield()
{
	// TODO: Add your command handler code here
	m_DofEnable = !m_DofEnable;
	Invalidate();
}

void CChildView::OnUpdateRendermodeDepthoffield(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_DofEnable);
}

void CChildView::OnRendermodeFxaa()
{
	// TODO: Add your command handler code here
	m_FxaaEnable = !m_FxaaEnable;
	Invalidate();
}

void CChildView::OnUpdateRendermodeFxaa(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_FxaaEnable);
}

void CChildView::OnRendermodeSsao()
{
	// TODO: Add your command handler code here
	m_SsaoEnable = !m_SsaoEnable;
	Invalidate();
}

void CChildView::OnUpdateRendermodeSsao(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_SsaoEnable);
}

void CChildView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	// TODO: Add your specialized code here and/or call the base class
	PostCameraViewChanged();

	__super::OnActivateView(bActivate, pActivateView, pDeactiveView);
}
