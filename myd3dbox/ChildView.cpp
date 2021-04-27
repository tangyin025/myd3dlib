
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "MainApp.h"

#include "ChildView.h"
#include "MainFrm.h"
#include "Animation.h"
//#include "DetourDebugDraw.h"

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
	ON_COMMAND(ID_SHOW_NAVIGATION, &CChildView::OnShowNavigation)
	ON_UPDATE_COMMAND_UI(ID_SHOW_NAVIGATION, &CChildView::OnUpdateShowNavigation)
END_MESSAGE_MAP()

// CChildView construction/destruction

CChildView::CChildView()
	: m_PivotScale(1.0f)
	, m_CameraDiagonal(30.0f)
	, m_bShowGrid(TRUE)
	, m_bShowCmpHandle(TRUE)
	, m_bShowNavigation(TRUE)
	, m_bCopyActors(FALSE)
	, m_PaintEmitterCaptured(NULL)
	, m_raychunkid(0, 0)
{
	// TODO: add construction code here
	my::ModelViewerCamera * model_view_camera = new my::ModelViewerCamera(D3DXToRadian(theApp.default_fov), 1.333333f, 0.1f, 3000.0f);
	m_Camera.reset(model_view_camera);
	m_Camera->m_Euler = my::Vector3(D3DXToRadian(-45), D3DXToRadian(45), 0);
	model_view_camera->m_LookAt = my::Vector3(0, 0, 0);
	model_view_camera->m_Distance = cotf(model_view_camera->m_Fov * 0.5f) * m_CameraDiagonal * 0.5f;

	m_SwapChainBuffer.reset(new my::Surface());
	ZeroMemory(&m_SwapChainBufferDesc, sizeof(m_SwapChainBufferDesc));
	m_SwapChainBufferDesc.Width = 100;
	m_SwapChainBufferDesc.Height = 100;
	m_DepthStencil.reset(new my::Surface());
	m_NormalRT.reset(new my::Texture2D());
	m_PositionRT.reset(new my::Texture2D());
	m_LightRT.reset(new my::Texture2D());
	for (unsigned int i = 0; i < RenderPipeline::RTChain::RTArray::static_size; i++)
	{
		m_OpaqueRT.m_RenderTarget[i].reset(new my::Texture2D());
		m_DownFilterRT.m_RenderTarget[i].reset(new my::Texture2D());
	}
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
	//pFrame->m_emitter->m_Emitter->m_ParticleList.clear();
	struct Callback : public my::OctNode::QueryCallback
	{
		const my::Frustum & frustum;
		RenderPipeline * pipeline;
		unsigned int PassMask;
		const my::Vector3 & ViewPos;
		const my::Vector3 & TargetPos;
		CMainFrame * pFrame;
		BOOL UpdateLod;
		Callback(const my::Frustum & _frustum, RenderPipeline * _pipeline, unsigned int _PassMask, const my::Vector3 & _ViewPos, const my::Vector3 & _TargetPos, CMainFrame * _pFrame, BOOL _UpdateLod)
			: frustum(_frustum)
			, pipeline(_pipeline)
			, PassMask(_PassMask)
			, ViewPos(_ViewPos)
			, TargetPos(_TargetPos)
			, pFrame(_pFrame)
			, UpdateLod(_UpdateLod)
		{
		}
		virtual void OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
		{
			ASSERT(dynamic_cast<Actor *>(oct_entity));

			Actor * actor = static_cast<Actor *>(oct_entity);

			if (UpdateLod)
			{
				const unsigned int Lod = actor->CalculateLod(ViewPos, TargetPos);

				if (Lod < Component::LOD_CULLING)
				{
					if (!actor->IsRequested())
					{
						actor->RequestResource();

						actor->EnterPhysxScene(pFrame);
					}

					actor->SetLod(Lod);
				}
				else
				{
					if (actor->IsRequested())
					{
						actor->SetLod(Component::LOD_CULLING);

						actor->LeavePhysxScene(pFrame);

						actor->ReleaseResource();
					}
				}
			}

			if (actor->IsRequested())
			{
				actor->AddToPipeline(frustum, pipeline, PassMask, ViewPos, TargetPos);
			}
		}
	};
	my::ModelViewerCamera * model_view_camera = dynamic_cast<my::ModelViewerCamera *>(m_Camera.get());
	pFrame->QueryEntity(frustum, &Callback(frustum, pipeline, PassMask, m_Camera->m_Eye, model_view_camera->m_LookAt, pFrame,
		(pFrame->GetActiveView() == this && (PassMask | RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal)))));
	//pFrame->m_emitter->AddToPipeline(frustum, pipeline, PassMask);
}

void CChildView::RenderSelectedActor(IDirect3DDevice9 * pd3dDevice, Actor * actor)
{
	PushWireAABB(*actor->m_Node, D3DCOLOR_ARGB(255, 255, 255, 0));
	Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
	for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
	{
		RenderSelectedComponent(pd3dDevice, cmp_iter->get());
	}
}

void CChildView::RenderSelectedComponent(IDirect3DDevice9 * pd3dDevice, Component * cmp)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	switch (cmp->m_Type)
	{
	case Component::ComponentTypeMesh:
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(cmp);
			if (mesh_cmp->m_Mesh)
			{
				theApp.m_SimpleSample->SetTechnique(theApp.technique_RenderSceneColor);
				theApp.m_SimpleSample->SetMatrix(theApp.handle_World, mesh_cmp->m_Actor->m_World);
				theApp.m_SimpleSample->SetVector(theApp.handle_MeshColor, my::Vector4(0,1,0,1));
				UINT passes = theApp.m_SimpleSample->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
				theApp.m_SimpleSample->BeginPass(0);
				mesh_cmp->m_Mesh->DrawSubset(mesh_cmp->m_MeshSubMeshId);
				theApp.m_SimpleSample->EndPass();
				theApp.m_SimpleSample->End();
			}
		}
		break;
	case Component::ComponentTypeTerrain:
		{
			Terrain * terrain = dynamic_cast<Terrain *>(cmp);
			PushWireAABB(terrain->m_Chunks[pFrame->m_selchunkid.x][pFrame->m_selchunkid.y]->m_OctAabb->transform(terrain->m_Actor->m_World), D3DCOLOR_ARGB(255, 255, 0, 255));
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

bool CChildView::OverlapTestFrustumAndActor(const my::Frustum & frustum, Actor * actor)
{
	if (actor->m_Cmps.empty())
	{
		my::IntersectionTests::IntersectionType intersect_type = my::IntersectionTests::IntersectAABBAndFrustum(*actor->m_OctAabb, frustum);
		if (intersect_type != my::IntersectionTests::IntersectionTypeOutside)
		{
			return true;
		}
	}
	my::Frustum local_ftm = frustum.transform(actor->m_World.transpose());
	Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
	for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
	{
		if (OverlapTestFrustumAndComponent(frustum, local_ftm, cmp_iter->get()))
		{
			return true;
		}
	}
	return false;
}

bool CChildView::OverlapTestFrustumAndComponent(const my::Frustum & frustum, const my::Frustum & local_ftm, Component * cmp)
{
	switch (cmp->m_Type)
	{
	case Component::ComponentTypeMesh:
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(cmp);
			if (!mesh_cmp->m_Mesh)
			{
				return false;
			}
			if (mesh_cmp->m_bUseAnimation
				&& mesh_cmp->m_Actor
				&& mesh_cmp->m_Actor->m_Animation
				&& !mesh_cmp->m_Actor->m_Animation->m_DualQuats.empty())
			{
				std::vector<my::Vector3> vertices(mesh_cmp->m_Mesh->GetNumVertices());
				my::D3DVertexElementSet elems;
				elems.InsertPositionElement(0);
				my::OgreMesh::ComputeDualQuaternionSkinnedVertices(
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					elems,
					mesh_cmp->m_Mesh->LockVertexBuffer(D3DLOCK_READONLY),
					mesh_cmp->m_Mesh->GetNumBytesPerVertex(),
					mesh_cmp->m_Mesh->m_VertexElems,
					mesh_cmp->m_Actor->m_Animation->m_DualQuats);
				DWORD ret = my::Mesh::FrustumTest(local_ftm,
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					mesh_cmp->m_Mesh->LockIndexBuffer(D3DLOCK_READONLY),
					!(mesh_cmp->m_Mesh->GetOptions() & D3DXMESH_32BIT),
					mesh_cmp->m_Mesh->GetNumFaces(),
					elems);
				mesh_cmp->m_Mesh->UnlockVertexBuffer();
				mesh_cmp->m_Mesh->UnlockIndexBuffer();
				if (ret == my::IntersectionTests::IntersectionTypeInside || ret == my::IntersectionTests::IntersectionTypeIntersect)
				{
					return true;
				}
			}
			else
			{
				DWORD ret = my::Mesh::FrustumTest(local_ftm,
					mesh_cmp->m_Mesh->LockVertexBuffer(D3DLOCK_READONLY),
					mesh_cmp->m_Mesh->GetNumVertices(),
					mesh_cmp->m_Mesh->GetNumBytesPerVertex(),
					mesh_cmp->m_Mesh->LockIndexBuffer(D3DLOCK_READONLY),
					!(mesh_cmp->m_Mesh->GetOptions() & D3DXMESH_32BIT),
					mesh_cmp->m_Mesh->GetNumFaces(),
					mesh_cmp->m_Mesh->m_VertexElems);
				mesh_cmp->m_Mesh->UnlockVertexBuffer();
				mesh_cmp->m_Mesh->UnlockIndexBuffer();
				if (ret == my::IntersectionTests::IntersectionTypeInside || ret == my::IntersectionTests::IntersectionTypeIntersect)
				{
					return true;
				}
			}
		}
		break;

	case Component::ComponentTypeStaticEmitter:
	case Component::ComponentTypeSphericalEmitter:
		{
			EmitterComponent * emitter = dynamic_cast<EmitterComponent *>(cmp);
			if (emitter->m_ParticleList.empty())
			{
				return true;
			}
			my::Matrix4 p2local;
			my::Vector3 sph = m_Camera->m_View.getColumn<2>().xyz.cartesianToSpherical();
			void * pvb = theApp.m_ParticleQuadVb.Lock(0, theApp.m_ParticleVertStride * RenderPipeline::m_ParticleQuadNumVertices, D3DLOCK_READONLY);
			void * pib = theApp.m_ParticleQuadIb.Lock(0, sizeof(WORD) * RenderPipeline::m_ParticleQuadPrimitiveCount, D3DLOCK_READONLY);
			my::Emitter::ParticleList::const_iterator part_iter = emitter->m_ParticleList.begin();
			for (; part_iter != emitter->m_ParticleList.end(); part_iter++)
			{
				switch (emitter->m_EmitterFaceType)
				{
				case EmitterComponent::FaceTypeX:
					p2local = my::Matrix4::Compose(
						my::Vector3(part_iter->m_Size.x, part_iter->m_Size.y, part_iter->m_Size.x),
						my::Quaternion::RotationAxis(my::Vector3::unitX, part_iter->m_Angle),
						part_iter->m_Position);
					break;
				case EmitterComponent::FaceTypeY:
					p2local = my::Matrix4::Compose(
						my::Vector3(part_iter->m_Size.x, part_iter->m_Size.y, part_iter->m_Size.x),
						my::Quaternion::RotationAxis(my::Vector3::unitX, part_iter->m_Angle) * my::Quaternion::RotationAxis(my::Vector3::unitZ, D3DXToRadian(90)),
						part_iter->m_Position);
					break;
				case EmitterComponent::FaceTypeZ:
					p2local = my::Matrix4::Compose(
						my::Vector3(part_iter->m_Size.x, part_iter->m_Size.y, part_iter->m_Size.x),
						my::Quaternion::RotationAxis(my::Vector3::unitX, part_iter->m_Angle) * my::Quaternion::RotationAxis(my::Vector3::unitY, D3DXToRadian(-90)),
						part_iter->m_Position);
					break;
				case EmitterComponent::FaceTypeCamera:
					p2local = my::Matrix4::Compose(
						my::Vector3(part_iter->m_Size.x, part_iter->m_Size.y, part_iter->m_Size.x),
						my::Quaternion::RotationAxis(my::Vector3::unitX, part_iter->m_Angle) * my::Quaternion::RotationAxis(my::Vector3::unitZ, sph.y) * my::Quaternion::RotationAxis(my::Vector3::unitY, -sph.z),
						part_iter->m_Position);
					break;
				case EmitterComponent::FaceTypeAngle:
					p2local = my::Matrix4::Compose(
						my::Vector3(part_iter->m_Size.x, part_iter->m_Size.y, part_iter->m_Size.x),
						my::Quaternion::RotationAxis(my::Vector3::unitY, part_iter->m_Angle),
						part_iter->m_Position);
					break;
				case EmitterComponent::FaceTypeAngleCamera:
					p2local = my::Matrix4::Compose(
						my::Vector3(part_iter->m_Size.x, part_iter->m_Size.y, part_iter->m_Size.x),
						my::Quaternion::RotationAxis(my::Vector3::unitX, part_iter->m_Angle) * my::Quaternion::RotationAxis(my::Vector3::unitY, -sph.z),
						part_iter->m_Position);
					break;
				}
				my::Frustum particle_ftm;
				switch (emitter->m_EmitterSpaceType)
				{
				case EmitterComponent::SpaceTypeWorld:
					particle_ftm = frustum.transform(p2local.transpose());
					break;
				case EmitterComponent::SpaceTypeLocal:
					particle_ftm = local_ftm.transform(p2local.transpose());
					break;
				}
				DWORD ret = my::Mesh::FrustumTest(particle_ftm,
					pvb, RenderPipeline::m_ParticleQuadNumVertices, theApp.m_ParticleVertStride, pib, true, RenderPipeline::m_ParticleQuadPrimitiveCount, theApp.m_ParticleVertElems);
				if (ret == my::IntersectionTests::IntersectionTypeInside || ret == my::IntersectionTests::IntersectionTypeIntersect)
				{
					theApp.m_ParticleQuadVb.Unlock();
					theApp.m_ParticleQuadIb.Unlock();
					return true;
				}
			}
			theApp.m_ParticleQuadVb.Unlock();
			theApp.m_ParticleQuadIb.Unlock();
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
				&& cloth_cmp->m_Actor->m_Animation
				&& !cloth_cmp->m_Actor->m_Animation->m_DualQuats.empty())
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
					cloth_cmp->m_Actor->m_Animation->m_DualQuats);
				DWORD ret = my::Mesh::FrustumTest(local_ftm,
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					&cloth_cmp->m_IndexData[0],
					true,
					cloth_cmp->m_IndexData.size() / 3,
					elems);
				if (ret == my::IntersectionTests::IntersectionTypeInside || ret == my::IntersectionTests::IntersectionTypeIntersect)
				{
					return true;
				}
			}
			else
			{
				DWORD ret = my::Mesh::FrustumTest(local_ftm,
					&cloth_cmp->m_VertexData[0],
					cloth_cmp->m_VertexData.size() / cloth_cmp->m_VertexStride,
					cloth_cmp->m_VertexStride,
					&cloth_cmp->m_IndexData[0],
					true,
					cloth_cmp->m_IndexData.size() / 3,
					cloth_cmp->m_VertexElems);
				if (ret == my::IntersectionTests::IntersectionTypeInside || ret == my::IntersectionTests::IntersectionTypeIntersect)
				{
					return true;
				}
			}
		}
		break;

	case Component::ComponentTypeTerrain:
		break;
	}
	return false;
}

my::RayResult CChildView::OverlapTestRayAndActor(const my::Ray & ray, Actor * actor)
{
	if (actor->m_Cmps.empty())
	{
		my::RayResult ret = my::IntersectionTests::rayAndAABB(ray.p, ray.d, *actor->m_OctAabb);
		if (ret.first)
		{
			return ret;
		}
	}
	my::Ray local_ray = ray.transform(actor->m_World.inverse());
	Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
	for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
	{
		my::RayResult ret = OverlapTestRayAndComponent(ray, local_ray, cmp_iter->get());
		if (ret.first)
		{
			ret.second = (local_ray.d * ret.second).transformNormal(actor->m_World).magnitude();
			return ret;
		}
	}
	return my::RayResult(false, FLT_MAX);
}

my::RayResult CChildView::OverlapTestRayAndComponent(const my::Ray & ray, const my::Ray & local_ray, Component * cmp)
{
	switch (cmp->m_Type)
	{
	case Component::ComponentTypeMesh:
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(cmp);
			if (!mesh_cmp->m_Mesh)
			{
				return my::RayResult(false, FLT_MAX);
			}
			my::RayResult ret;
			if (mesh_cmp->m_bUseAnimation
				&& mesh_cmp->m_Actor
				&& mesh_cmp->m_Actor->m_Animation
				&& !mesh_cmp->m_Actor->m_Animation->m_DualQuats.empty())
			{
				std::vector<my::Vector3> vertices(mesh_cmp->m_Mesh->GetNumVertices());
				my::D3DVertexElementSet elems;
				elems.InsertPositionElement(0);
				my::OgreMesh::ComputeDualQuaternionSkinnedVertices(
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					elems,
					mesh_cmp->m_Mesh->LockVertexBuffer(D3DLOCK_READONLY),
					mesh_cmp->m_Mesh->GetNumBytesPerVertex(),
					mesh_cmp->m_Mesh->m_VertexElems,
					mesh_cmp->m_Actor->m_Animation->m_DualQuats);
				ret = my::Mesh::RayTest(local_ray,
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					mesh_cmp->m_Mesh->LockIndexBuffer(D3DLOCK_READONLY),
					!(mesh_cmp->m_Mesh->GetOptions() & D3DXMESH_32BIT),
					mesh_cmp->m_Mesh->GetNumFaces(),
					elems);
				mesh_cmp->m_Mesh->UnlockVertexBuffer();
				mesh_cmp->m_Mesh->UnlockIndexBuffer();
			}
			else
			{
				ret = my::Mesh::RayTest(local_ray,
					mesh_cmp->m_Mesh->LockVertexBuffer(D3DLOCK_READONLY),
					mesh_cmp->m_Mesh->GetNumVertices(),
					mesh_cmp->m_Mesh->GetNumBytesPerVertex(),
					mesh_cmp->m_Mesh->LockIndexBuffer(D3DLOCK_READONLY),
					!(mesh_cmp->m_Mesh->GetOptions() & D3DXMESH_32BIT),
					mesh_cmp->m_Mesh->GetNumFaces(),
					mesh_cmp->m_Mesh->m_VertexElems);
				mesh_cmp->m_Mesh->UnlockVertexBuffer();
				mesh_cmp->m_Mesh->UnlockIndexBuffer();
			}
			if (ret.first)
			{
				return ret;
			}
		}
		break;

	case Component::ComponentTypeStaticEmitter:
	case Component::ComponentTypeSphericalEmitter:
		{
			EmitterComponent * emitter = dynamic_cast<EmitterComponent *>(cmp);
			if (emitter->m_ParticleList.empty())
			{
				return my::RayResult(true, local_ray.p.dot(m_Camera->m_View.getColumn<2>().xyz));
			}
			my::Matrix4 p2local;
			my::Vector3 sph = m_Camera->m_View.getColumn<2>().xyz.cartesianToSpherical();
			void * pvb = theApp.m_ParticleQuadVb.Lock(0, theApp.m_ParticleVertStride * RenderPipeline::m_ParticleQuadNumVertices, D3DLOCK_READONLY);
			void * pib = theApp.m_ParticleQuadIb.Lock(0, sizeof(WORD) * RenderPipeline::m_ParticleQuadPrimitiveCount, D3DLOCK_READONLY);
			my::Emitter::ParticleList::const_iterator part_iter = emitter->m_ParticleList.begin();
			for (; part_iter != emitter->m_ParticleList.end(); part_iter++)
			{
				switch (emitter->m_EmitterFaceType)
				{
				case EmitterComponent::FaceTypeX:
					p2local = my::Matrix4::Compose(
						my::Vector3(part_iter->m_Size.x, part_iter->m_Size.y, part_iter->m_Size.x),
						my::Quaternion::RotationAxis(my::Vector3::unitX, part_iter->m_Angle),
						part_iter->m_Position);
					break;
				case EmitterComponent::FaceTypeY:
					p2local = my::Matrix4::Compose(
						my::Vector3(part_iter->m_Size.x, part_iter->m_Size.y, part_iter->m_Size.x),
						my::Quaternion::RotationAxis(my::Vector3::unitX, part_iter->m_Angle) * my::Quaternion::RotationAxis(my::Vector3::unitZ, D3DXToRadian(90)),
						part_iter->m_Position);
					break;
				case EmitterComponent::FaceTypeZ:
					p2local = my::Matrix4::Compose(
						my::Vector3(part_iter->m_Size.x, part_iter->m_Size.y, part_iter->m_Size.x),
						my::Quaternion::RotationAxis(my::Vector3::unitX, part_iter->m_Angle) * my::Quaternion::RotationAxis(my::Vector3::unitY, D3DXToRadian(-90)),
						part_iter->m_Position);
					break;
				case EmitterComponent::FaceTypeCamera:
					p2local = my::Matrix4::Compose(
						my::Vector3(part_iter->m_Size.x, part_iter->m_Size.y, part_iter->m_Size.x),
						my::Quaternion::RotationAxis(my::Vector3::unitX, part_iter->m_Angle) * my::Quaternion::RotationAxis(my::Vector3::unitZ, sph.y) * my::Quaternion::RotationAxis(my::Vector3::unitY, -sph.z),
						part_iter->m_Position);
					break;
				case EmitterComponent::FaceTypeAngle:
					p2local = my::Matrix4::Compose(
						my::Vector3(part_iter->m_Size.x, part_iter->m_Size.y, part_iter->m_Size.x),
						my::Quaternion::RotationAxis(my::Vector3::unitY, part_iter->m_Angle),
						part_iter->m_Position);
					break;
				case EmitterComponent::FaceTypeAngleCamera:
					p2local = my::Matrix4::Compose(
						my::Vector3(part_iter->m_Size.x, part_iter->m_Size.y, part_iter->m_Size.x),
						my::Quaternion::RotationAxis(my::Vector3::unitX, part_iter->m_Angle) * my::Quaternion::RotationAxis(my::Vector3::unitY, -sph.z),
						part_iter->m_Position);
					break;
				}
				my::Ray particle_ray;
				switch (emitter->m_EmitterSpaceType)
				{
				case EmitterComponent::SpaceTypeWorld:
					particle_ray = ray.transform(p2local.inverse());
					break;
				case EmitterComponent::SpaceTypeLocal:
					particle_ray = local_ray.transform(p2local.inverse());
					break;
				}
				my::RayResult ret = my::Mesh::RayTest(particle_ray,
					pvb, RenderPipeline::m_ParticleQuadNumVertices, theApp.m_ParticleVertStride, pib, true, RenderPipeline::m_ParticleQuadPrimitiveCount, theApp.m_ParticleVertElems);
				if (ret.first)
				{
					theApp.m_ParticleQuadVb.Unlock();
					theApp.m_ParticleQuadIb.Unlock();
					return ret;
				}
			}
			theApp.m_ParticleQuadVb.Unlock();
			theApp.m_ParticleQuadIb.Unlock();
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
				&& cloth_cmp->m_Actor->m_Animation
				&& !cloth_cmp->m_Actor->m_Animation->m_DualQuats.empty())
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
					cloth_cmp->m_Actor->m_Animation->m_DualQuats);
				ret = my::Mesh::RayTest(local_ray,
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
				ret = my::Mesh::RayTest(local_ray,
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
				return ret;
			}
		}
		break;

	case Component::ComponentTypeTerrain:
		{
			struct Callback : public my::OctNode::QueryCallback
			{
				const my::Ray & ray;
				const my::Vector3 & ViewPos;
				CChildView * pView;
				Terrain * terrain;
				my::RayResult ret;
				Callback(const my::Ray & _ray, const my::Vector3 & _ViewPos, CChildView * _pView, Terrain * _terrain)
					: ray(_ray)
					, pView(_pView)
					, ViewPos(_ViewPos)
					, terrain(_terrain)
					, ret(false, FLT_MAX)
				{
				}
				virtual void OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
				{
					TerrainChunk * chunk = dynamic_cast<TerrainChunk *>(oct_entity);
					my::RayResult result;
					if (!chunk->m_Vb)
					{
						unsigned short ib[6] = {
							(terrain->m_ColChunks + 1) * (chunk->m_Row + 0) + (chunk->m_Col + 0),
							(terrain->m_ColChunks + 1) * (chunk->m_Row + 1) + (chunk->m_Col + 0),
							(terrain->m_ColChunks + 1) * (chunk->m_Row + 0) + (chunk->m_Col + 1),
							(terrain->m_ColChunks + 1) * (chunk->m_Row + 0) + (chunk->m_Col + 1),
							(terrain->m_ColChunks + 1) * (chunk->m_Row + 1) + (chunk->m_Col + 0),
							(terrain->m_ColChunks + 1) * (chunk->m_Row + 1) + (chunk->m_Col + 1)};
						result = my::Mesh::RayTest(
							ray,
							terrain->m_Vb.Lock(0, 0, D3DLOCK_READONLY),
							(terrain->m_RowChunks + 1) * (terrain->m_ColChunks + 1),
							terrain->m_VertexStride,
							ib,
							true,
							2,
							terrain->m_VertexElems);
						terrain->m_Vb.Unlock();
					}
					else
					{
						const Terrain::Fragment& frag = terrain->GetFragment(
							terrain->CalculateLod(chunk->m_Row, chunk->m_Col, ViewPos),
							terrain->CalculateLod(chunk->m_Row, chunk->m_Col - 1, ViewPos),
							terrain->CalculateLod(chunk->m_Row - 1, chunk->m_Col, ViewPos),
							terrain->CalculateLod(chunk->m_Row, chunk->m_Col + 1, ViewPos),
							terrain->CalculateLod(chunk->m_Row + 1, chunk->m_Col, ViewPos));
						result = my::Mesh::RayTest(
							ray,
							chunk->m_Vb->Lock(0, 0, D3DLOCK_READONLY),
							frag.VertNum,
							terrain->m_VertexStride,
							const_cast<my::IndexBuffer&>(frag.ib).Lock(0, 0, D3DLOCK_READONLY),
							false,
							frag.PrimitiveCount,
							terrain->m_VertexElems);
						chunk->m_Vb->Unlock();
						const_cast<my::IndexBuffer&>(frag.ib).Unlock();
					}
					if (result.first && result.second < ret.second)
					{
						ret = result;
						pView->m_raychunkid.SetPoint(chunk->m_Row, chunk->m_Col);
					}
				}
			};
			Terrain * terrain = dynamic_cast<Terrain *>(cmp);
			my::ModelViewerCamera * model_view_camera = dynamic_cast<my::ModelViewerCamera *>(m_Camera.get());
			my::Vector3 LocalViewPos = model_view_camera->m_LookAt.transformCoord(terrain->m_Actor->m_World.inverse());
			Callback cb(local_ray, LocalViewPos, this, terrain);
			terrain->QueryEntity(local_ray, &cb);
			if (cb.ret.first)
			{
				return cb.ret;
			}
		}
		break;
	}
	return my::RayResult(false, FLT_MAX);
}

void CChildView::OnSelectionChanged(my::EventArg * arg)
{
	Invalidate();
}

void CChildView::OnSelectionPlaying(my::EventArg * arg)
{
	Invalidate();
}

void CChildView::OnPivotModeChanged(my::EventArg * arg)
{
	Invalidate();
}

void CChildView::OnCmpAttriChanged(my::EventArg * arg)
{
	Invalidate();
}

void CChildView::OnCameraPropChanged(my::EventArg * arg)
{
	Invalidate();
}

void CChildView::depthMask(bool state)
{
}

void CChildView::texture(bool state)
{
}

void CChildView::begin(duDebugDrawPrimitives prim, float size)
{
	m_prim = prim;
	m_verts.clear();
}

void CChildView::vertex(const float* pos, unsigned int color)
{
	Vertex v = { pos[0], pos[1], pos[2], color, 0.0f, 0.0f };
	m_verts.push_back(v);
}

void CChildView::vertex(const float x, const float y, const float z, unsigned int color)
{
	Vertex v = { x, y, z, color, 0.0f, 0.0f };
	m_verts.push_back(v);
}

void CChildView::vertex(const float* pos, unsigned int color, const float* uv)
{
	Vertex v = { pos[0], pos[1], pos[2], color, uv[0], uv[1] };
	m_verts.push_back(v);
}

void CChildView::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
{
	Vertex vert = { x, y, z, color, u, v };
	m_verts.push_back(vert);
}

void CChildView::end()
{
	if (m_verts.empty())
	{
		return;
	}
	if (m_prim == DU_DRAW_LINES)
	{
		ASSERT(m_verts.size() % 2 == 0);
		V(theApp.m_d3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE));
		V(theApp.m_d3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, m_verts.size() / 2, &m_verts[0], sizeof(Vertex)));
	}
	else if (m_prim == DU_DRAW_TRIS)
	{
		ASSERT(m_verts.size() % 3 == 0);
		V(theApp.m_d3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE));
		V(theApp.m_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, m_verts.size() / 3, &m_verts[0], sizeof(Vertex)));
	}
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
			DrawHelper::BeginLine();

			if (m_bShowGrid)
			{
				PushGrid(12, 5, 5, D3DCOLOR_ARGB(255,127,127,127), D3DCOLOR_ARGB(255,0,0,0), my::Matrix4::RotationX(D3DXToRadian(-90)));
			}

			CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
			ASSERT_VALID(pFrame);
			pFrame->PushRenderBuffer(this);

			if(SUCCEEDED(hr = theApp.m_d3dDevice->BeginScene()))
			{
				//m_BgColor = D3DCOLOR_ARGB(0,161,161,161);
				//V(theApp.m_d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, m_BgColor, 1.0f, 0)); // ! d3dmultisample will not work
				//V(theApp.m_d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
				V(theApp.m_d3dDevice->SetRenderTarget(0, m_SwapChainBuffer->m_ptr));
				V(theApp.m_d3dDevice->SetDepthStencilSurface(m_DepthStencil->m_ptr));
				my::ModelViewerCamera * model_view_camera = dynamic_cast<my::ModelViewerCamera *>(m_Camera.get());
				theApp.m_SkyLightCam.m_Eye = model_view_camera->m_LookAt;
				theApp.m_SkyLightCam.UpdateViewProj();
				theApp.OnRender(theApp.m_d3dDevice, &m_SwapChainBufferDesc, this, theApp.m_fAbsoluteTime, theApp.m_fElapsedTime);

				swprintf_s(&m_ScrInfo[0][0], m_ScrInfo[0].size(), L"PerformanceSec: %.3f", EndPerformanceCount());
				for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
				{
					swprintf_s(&m_ScrInfo[1+PassID][0], m_ScrInfo[1+PassID].size(), L"%S: %d", RenderPipeline::PassTypeToStr(PassID), theApp.m_PassDrawCall[PassID]);
				}

				V(theApp.m_d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE));
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME));
				if (m_bShowCmpHandle && !pFrame->m_selactors.empty())
				{
					theApp.m_SimpleSample->SetMatrix(theApp.handle_View, m_Camera->m_View);
					theApp.m_SimpleSample->SetMatrix(theApp.handle_ViewProj, m_Camera->m_ViewProj);
					PushWireAABB(pFrame->m_selbox, D3DCOLOR_ARGB(255,255,255,255));
					CMainFrame::SelActorList::const_iterator sel_iter = pFrame->m_selactors.begin();
					for (; sel_iter != pFrame->m_selactors.end(); sel_iter++)
					{
						RenderSelectedActor(theApp.m_d3dDevice, *sel_iter);
					}
				}

				V(theApp.m_d3dDevice->SetVertexShader(NULL));
				V(theApp.m_d3dDevice->SetPixelShader(NULL));
				V(theApp.m_d3dDevice->SetTexture(0, NULL));
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE));
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE));
				V(theApp.m_d3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera->m_View));
				V(theApp.m_d3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera->m_Proj));
				V(theApp.m_d3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&my::Matrix4::identity));
				DrawHelper::EndLine(theApp.m_d3dDevice);

				V(theApp.m_d3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID));
				//if (m_bShowNavigation && pFrame->m_navMesh && pFrame->m_navQuery)
				//{
				//	duDebugDrawNavMesh(this, *pFrame->m_navMesh, DU_DRAWNAVMESH_OFFMESHCONS | DU_DRAWNAVMESH_CLOSEDLIST);
				//}

				V(theApp.m_d3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
				if (m_bShowCmpHandle && !pFrame->m_selactors.empty())
				{
					if (pFrame->m_PaintType == CMainFrame::PaintTypeTerrainHeightField)
					{
					}
					else if (pFrame->m_PaintType == CMainFrame::PaintTypeTerrainColor)
					{
					}
					else if (pFrame->m_PaintType == CMainFrame::PaintTypeEmitterInstance)
					{
					}
					else
					{
						m_PivotScale = m_Camera->CalculateViewportScaler(pFrame->m_Pivot.m_Pos) * 40.0f / m_SwapChainBufferDesc.Height;
						pFrame->m_Pivot.Draw(theApp.m_d3dDevice, m_Camera.get(), &m_SwapChainBufferDesc, m_PivotScale);
					}
				}

				theApp.m_UIRender->Begin();
				theApp.m_UIRender->SetViewProj(DialogMgr::m_ViewProj);
				theApp.m_UIRender->SetWorld(my::Matrix4::Translation(my::Vector3(0.5f, 0.5f, 0)));
				if (m_bShowGrid)
				{
					my::Vector3 pt = m_Camera->WorldToScreen(my::Vector3(12, 0, 0), my::Vector2((float)m_SwapChainBufferDesc.Width, (float)m_SwapChainBufferDesc.Height));
					if (pt.z > 0.0f && pt.z < 1.0f)
					{
						theApp.m_Font->PushString(theApp.m_UIRender.get(), L"x", my::Rectangle(pt.xy, pt.xy), D3DCOLOR_ARGB(255, 255, 255, 0), my::Font::AlignCenterMiddle);
					}

					pt = m_Camera->WorldToScreen(my::Vector3(0, 0, 12), my::Vector2((float)m_SwapChainBufferDesc.Width, (float)m_SwapChainBufferDesc.Height));
					if (pt.z > 0.0f && pt.z < 1.0f)
					{
						theApp.m_Font->PushString(theApp.m_UIRender.get(), L"z", my::Rectangle(pt.xy, pt.xy), D3DCOLOR_ARGB(255, 255, 255, 0), my::Font::AlignCenterMiddle);
					}
				}
				ScrInfoMap::const_iterator info_iter = m_ScrInfo.begin();
				for (int y = 5; info_iter != m_ScrInfo.end(); info_iter++, y += theApp.m_Font->m_LineHeight)
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
		DialogMgr::SetDlgViewport(my::Vector2((float)cx, (float)cy), D3DXToRadian(theApp.default_fov));
	}
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventSelectionChanged.connect(boost::bind(&CChildView::OnSelectionChanged, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventSelectionPlaying.connect(boost::bind(&CChildView::OnSelectionPlaying, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventPivotModeChanged.connect(boost::bind(&CChildView::OnPivotModeChanged, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventAttributeChanged.connect(boost::bind(&CChildView::OnCmpAttriChanged, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventCameraPropChanged.connect(boost::bind(&CChildView::OnCameraPropChanged, this, _1));
	return 0;
}

void CChildView::OnDestroy()
{
	CView::OnDestroy();

	// TODO: Add your message handler code here
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventSelectionChanged.disconnect(boost::bind(&CChildView::OnSelectionChanged, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventSelectionPlaying.disconnect(boost::bind(&CChildView::OnSelectionPlaying, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventPivotModeChanged.disconnect(boost::bind(&CChildView::OnPivotModeChanged, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventAttributeChanged.disconnect(boost::bind(&CChildView::OnCmpAttriChanged, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventCameraPropChanged.disconnect(boost::bind(&CChildView::OnCameraPropChanged, this, _1));
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	my::Ray ray = m_Camera->CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height));
	Terrain* terrain = dynamic_cast<Terrain *>(pFrame->GetSelComponent(Component::ComponentTypeTerrain));
	if (terrain && pFrame->m_PaintType == CMainFrame::PaintTypeTerrainHeightField)
	{
		m_PaintTerrainCaptured.reset(new TerrainStream(terrain, false));
		OnPaintTerrainHeightField(ray, *m_PaintTerrainCaptured);
		SetCapture();
		Invalidate();
		return;
	}

	if (terrain && pFrame->m_PaintType == CMainFrame::PaintTypeTerrainColor)
	{
		m_PaintTerrainCaptured.reset(new TerrainStream(terrain, false));
		OnPaintTerrainColor(ray, *m_PaintTerrainCaptured);
		SetCapture();
		Invalidate();
		return;
	}

	StaticEmitterComponent* emit = dynamic_cast<StaticEmitterComponent*>(pFrame->GetSelComponent(Component::ComponentTypeStaticEmitter));
	if (emit && pFrame->m_PaintType == CMainFrame::PaintTypeEmitterInstance)
	{
		m_PaintEmitterCaptured = emit;
		OnPaintEmitterInstance(ray, emit);
		SetCapture();
		Invalidate();
		return;
	}

	if (!pFrame->m_selactors.empty() && pFrame->m_Pivot.OnLButtonDown(ray, m_PivotScale))
	{
		StartPerformanceCount();
		CMainFrame::SelActorList::iterator sel_iter = pFrame->m_selactors.begin();
		for (; sel_iter != pFrame->m_selactors.end(); sel_iter++)
		{
			physx::PxRigidBody * body = NULL;
			if ((*sel_iter)->m_PxActor && (body = (*sel_iter)->m_PxActor->is<physx::PxRigidBody>()))
			{
				body->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, true);
			}
		}
		m_bCopyActors = (nFlags & MK_SHIFT) ? TRUE : FALSE;
		SetCapture();
		Invalidate();
		return;
	}

	CRectTracker tracker;
	tracker.TrackRubberBand(this, point, TRUE);
	tracker.m_rect.NormalizeRect();

	StartPerformanceCount();
	if (!(nFlags & MK_SHIFT) && !pFrame->m_selactors.empty())
	{
		pFrame->m_selactors.clear();
	}

	if (!tracker.m_rect.IsRectEmpty())
	{
		my::Rectangle rc(
			(float)tracker.m_rect.left,
			(float)tracker.m_rect.top,
			(float)tracker.m_rect.right,
			(float)tracker.m_rect.bottom);
		my::Frustum ftm = m_Camera->CalculateFrustum(rc, CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height));
		struct Callback : public my::OctNode::QueryCallback
		{
			CMainFrame::SelActorList & selacts;
			const my::Frustum & ftm;
			CChildView * pView;
			Callback(CMainFrame::SelActorList & _selacts, const my::Frustum & _ftm, CChildView * _pView)
				: selacts(_selacts)
				, ftm(_ftm)
				, pView(_pView)
			{
			}
			virtual void OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
			{
				Actor * actor = dynamic_cast<Actor *>(oct_entity);
				ASSERT(actor);
				if (pView->OverlapTestFrustumAndActor(ftm, actor))
				{
					selacts.push_back(actor);
				}
			}
		};
		pFrame->QueryEntity(ftm, &Callback(pFrame->m_selactors, ftm, this));
	}
	else
	{
		struct Callback : public my::OctNode::QueryCallback
		{
			const my::Ray & ray;
			CChildView * pView;
			Actor * selact;
			float seldist;
			CPoint selchunkid;
			Callback(const my::Ray & _ray, CChildView * _pView)
				: ray(_ray)
				, pView(_pView)
				, selact(NULL)
				, seldist(FLT_MAX)
				, selchunkid(0, 0)
			{
			}
			virtual void OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
			{
				Actor * actor = dynamic_cast<Actor *>(oct_entity);
				ASSERT(actor);
				my::RayResult ret = pView->OverlapTestRayAndActor(ray, actor);
				if (ret.first && ret.second < seldist)
				{
					selact = actor;
					seldist = ret.second;
					selchunkid = pView->m_raychunkid;
				}
			}
		};
		Callback cb(ray, this);
		pFrame->QueryEntity(ray, &cb);
		if (cb.selact)
		{
			CMainFrame::SelActorList::iterator sel_iter = std::find(pFrame->m_selactors.begin(), pFrame->m_selactors.end(), cb.selact);
			if (sel_iter != pFrame->m_selactors.end())
			{
				pFrame->m_selactors.erase(sel_iter);
			}
			else
			{
				pFrame->m_selactors.push_back(cb.selact);
				pFrame->m_selchunkid = cb.selchunkid;
			}
		}
	}
	pFrame->OnSelChanged();
}

void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	if (m_PaintTerrainCaptured)
	{
		m_PaintTerrainCaptured.reset();
		ReleaseCapture();
		Invalidate();
		return;
	}

	if (m_PaintEmitterCaptured)
	{
		m_PaintEmitterCaptured->m_Actor->UpdateAABB();
		m_PaintEmitterCaptured->m_Actor->UpdateOctNode();
		pFrame->UpdateSelBox();
		m_PaintEmitterCaptured = NULL;
		ReleaseCapture();
		Invalidate();
		return;
	}

	if (pFrame->m_Pivot.m_Captured && pFrame->m_Pivot.OnLButtonUp(
		m_Camera->CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height))))
	{
		StartPerformanceCount();
		CMainFrame::SelActorList::iterator sel_iter = pFrame->m_selactors.begin();
		for (; sel_iter != pFrame->m_selactors.end(); sel_iter++)
		{
			(*sel_iter)->UpdateAABB();
			(*sel_iter)->UpdateOctNode();

			physx::PxRigidBody * body = NULL;
			if ((*sel_iter)->m_PxActor && (body = (*sel_iter)->m_PxActor->is<physx::PxRigidBody>()) && !body->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
			{
				body->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
				body->setLinearVelocity((physx::PxVec3&)my::Vector3(0, 0, 0));
			}
		}
		pFrame->UpdateSelBox();
		ReleaseCapture();
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		return;
	}
}

void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	if (m_PaintTerrainCaptured && pFrame->m_PaintType == CMainFrame::PaintTypeTerrainHeightField)
	{
		my::Ray ray = m_Camera->CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height));
		OnPaintTerrainHeightField(ray, *m_PaintTerrainCaptured);
		Invalidate();
		UpdateWindow();
		return;
	}

	if (m_PaintTerrainCaptured && pFrame->m_PaintType == CMainFrame::PaintTypeTerrainColor)
	{
		my::Ray ray = m_Camera->CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height));
		OnPaintTerrainColor(ray, *m_PaintTerrainCaptured);
		Invalidate();
		UpdateWindow();
		return;
	}

	if (m_PaintEmitterCaptured && pFrame->m_PaintType == CMainFrame::PaintTypeEmitterInstance)
	{
		my::Ray ray = m_Camera->CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height));
		OnPaintEmitterInstance(ray, m_PaintEmitterCaptured);
		Invalidate();
		UpdateWindow();
		return;
	}

	if (pFrame->m_Pivot.m_Captured && pFrame->m_Pivot.OnMouseMove(
		m_Camera->CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height)), m_PivotScale))
	{
		if (m_bCopyActors)
		{
			m_bCopyActors = FALSE;
			CMainFrame::SelActorList new_selactors;
			CMainFrame::SelActorList::const_iterator sel_iter = pFrame->m_selactors.begin();
			for (; sel_iter != pFrame->m_selactors.end(); sel_iter++)
			{
				ActorPtr new_actor = (*sel_iter)->Clone();
				pFrame->AddEntity(new_actor.get(), new_actor->m_aabb.transform(new_actor->m_World), Actor::MinBlock, Actor::Threshold);
				pFrame->m_ActorList.insert(new_actor);
				new_selactors.push_back(new_actor.get());
			}
			pFrame->m_selactors.swap(new_selactors);
		}
		StartPerformanceCount();
		CMainFrame::SelActorList::iterator sel_iter = pFrame->m_selactors.begin();
		for (; sel_iter != pFrame->m_selactors.end(); sel_iter++)
		{
			my::Matrix4 trans = my::Matrix4::Identity();
			switch (pFrame->m_Pivot.m_Mode)
			{
			case Pivot::PivotModeMove:
				(*sel_iter)->m_Position += pFrame->m_Pivot.m_DragDeltaPos;
				(*sel_iter)->UpdateWorld();
				break;
			case Pivot::PivotModeRot:
				trans = my::Matrix4::AffineTransformation(1,
					pFrame->m_Pivot.m_Pos, pFrame->m_Pivot.m_Rot.inverse() * pFrame->m_Pivot.m_DragDeltaRot * pFrame->m_Pivot.m_Rot, my::Vector3(0, 0, 0));
				(*sel_iter)->m_World *= trans;
				(*sel_iter)->m_World.Decompose((*sel_iter)->m_Scale, (*sel_iter)->m_Rotation, (*sel_iter)->m_Position);
				break;
			}

			if ((*sel_iter)->m_PxActor)
			{
				(*sel_iter)->m_PxActor->setGlobalPose(physx::PxTransform(
					(physx::PxVec3&)(*sel_iter)->m_Position, (physx::PxQuat&)(*sel_iter)->m_Rotation), true);
			}

			Actor::ComponentPtrList::iterator cmp_iter = (*sel_iter)->m_Cmps.begin();
			for (; cmp_iter != (*sel_iter)->m_Cmps.end(); cmp_iter++)
			{
				(*cmp_iter)->OnSetPose();
			}
		}
		Invalidate();
		UpdateWindow();
		return;
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
		{
			pFrame->m_bEatAltUp = TRUE;
			break;
		}
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		{
			CEnvironmentWnd::CameraPropEventArgs arg(this);
			pFrame->m_EventCameraPropChanged(&arg);
			break;
		}
		case WM_MOUSEMOVE:
		{
			m_Camera->UpdateViewProj();
			StartPerformanceCount();
			//my::ModelViewerCamera * model_view_camera = dynamic_cast<my::ModelViewerCamera *>(m_Camera.get());
			//pFrame->CheckViewedActor(pFrame,
			//	my::AABB(model_view_camera->m_LookAt, 1000.0f), my::AABB(model_view_camera->m_LookAt, 1000.0f));
			Invalidate();
			break;
		}
		}
		return TRUE;
	}
	return __super::PreTranslateMessage(pMsg);
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	switch (nChar)
	{
	case 'W':
		if (pFrame->m_PaintType != CMainFrame::PaintTypeNone || pFrame->m_Pivot.m_Mode != Pivot::PivotModeMove)
		{
			pFrame->OnCmdMsg(ID_PIVOT_MOVE, 0, NULL, NULL);
		}
		return;
	case 'E':
		if (pFrame->m_PaintType != CMainFrame::PaintTypeNone || pFrame->m_Pivot.m_Mode != Pivot::PivotModeRot)
		{
			pFrame->OnCmdMsg(ID_PIVOT_ROTATE, 0, NULL, NULL);
		}
		return;
	case 'T':
		if (pFrame->GetSelComponent(Component::ComponentTypeTerrain) && pFrame->m_PaintType != CMainFrame::PaintTypeTerrainHeightField)
		{
			pFrame->OnCmdMsg(ID_PAINT_TERRAINHEIGHTFIELD, 0, NULL, NULL);
		}
		return;
	case 'Y':
		if (pFrame->GetSelComponent(Component::ComponentTypeTerrain) && pFrame->m_PaintType != CMainFrame::PaintTypeTerrainColor)
		{
			pFrame->OnCmdMsg(ID_PAINT_TERRAINCOLOR, 0, NULL, NULL);
		}
		return;
	case 'U':
		if (pFrame->GetSelComponent(Component::ComponentTypeStaticEmitter) && pFrame->m_PaintType != CMainFrame::PaintTypeEmitterInstance)
		{
			pFrame->OnCmdMsg(ID_PAINT_EMITTERINSTANCE, 0, NULL, NULL);
		}
		return;
	case 'F':
		{
			my::ModelViewerCamera * model_view_camera = dynamic_cast<my::ModelViewerCamera *>(m_Camera.get());
			if (!pFrame->m_selactors.empty())
			{
				my::AABB focus_box = pFrame->m_selbox;
				if (pFrame->m_selactors.size() == 1)
				{
					Actor::ComponentPtrList::iterator cmp_iter = pFrame->m_selactors.front()->m_Cmps.begin();
					for (; cmp_iter != pFrame->m_selactors.front()->m_Cmps.end(); cmp_iter++)
					{
						if ((*cmp_iter)->m_Type == Component::ComponentTypeTerrain)
						{
							Terrain * terrain = dynamic_cast<Terrain *>(cmp_iter->get());
							my::AABB chunk_box = terrain->m_Chunks[pFrame->m_selchunkid.x][pFrame->m_selchunkid.y]->m_OctAabb->transform(terrain->m_Actor->m_World);
							float chunk_radius = chunk_box.Extent().magnitude() * 0.5f;
							if ((model_view_camera->m_LookAt - chunk_box.Center()).magnitudeSq() > EPSILON_E6 * EPSILON_E6
								|| fabs(model_view_camera->m_Distance - chunk_radius / asin(model_view_camera->m_Fov * 0.5f)) > EPSILON_E6)
							{
								focus_box = chunk_box;
							}
							break;
						}
					}
				}
				float radius = focus_box.Extent().magnitude() * 0.5f;
				model_view_camera->m_LookAt = focus_box.Center();
				model_view_camera->m_Distance = radius / asin(model_view_camera->m_Fov * 0.5f);
			}
			else
			{
				model_view_camera->m_LookAt = my::Vector3(0, 0, 0);
				model_view_camera->m_Distance = cotf(model_view_camera->m_Fov * 0.5f) * m_CameraDiagonal * 0.5f;
			}
			m_Camera->UpdateViewProj();
			StartPerformanceCount();
			Invalidate();
			CEnvironmentWnd::CameraPropEventArgs arg(this);
			pFrame->m_EventCameraPropChanged(&arg);
		}
		return;
	case VK_DELETE:
		if (!pFrame->m_selactors.empty())
		{
			pFrame->OnCmdMsg(ID_EDIT_DELETE, 0, NULL, NULL);
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
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	if (pFrame->m_PxScene->getVisualizationParameter(physx::PxVisualizationParameter::eSCALE) > 0)
	{
		pFrame->m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 0.0f);
	}
	else
	{
		pFrame->m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.0f);
	}
	Invalidate();
}

void CChildView::OnUpdateShowCollisiondebug(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	pCmdUI->SetCheck(pFrame->m_PxScene->getVisualizationParameter(physx::PxVisualizationParameter::eSCALE) > 0);
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
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CEnvironmentWnd::CameraPropEventArgs arg(this);
	pFrame->m_EventCameraPropChanged(&arg);

	__super::OnActivateView(bActivate, pActivateView, pDeactiveView);
}


void CChildView::OnShowNavigation()
{
	// TODO: Add your command handler code here
	m_bShowNavigation = !m_bShowNavigation;
	Invalidate();
}


void CChildView::OnUpdateShowNavigation(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_bShowNavigation);
}


void CChildView::OnPaintTerrainHeightField(const my::Ray& ray, TerrainStream& tstr)
{
	// TODO: Add your implementation code here.
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	my::Ray local_ray = ray.transform(tstr.m_terrain->m_Actor->m_World.inverse());
	my::RayResult res = OverlapTestRayAndComponent(ray, local_ray, tstr.m_terrain);
	if (res.first)
	{
		my::Vector3 pt = local_ray.p + local_ray.d * res.second;
		for (int i = my::Max(0, (int)roundf(pt.z - pFrame->m_PaintRadius));
			i <= my::Min(tstr.m_terrain->m_RowChunks * tstr.m_terrain->m_ChunkSize, (int)roundf(pt.z + pFrame->m_PaintRadius)); i++)
		{
			for (int j = my::Max(0, (int)roundf(pt.x - pFrame->m_PaintRadius));
				j <= my::Min(tstr.m_terrain->m_ColChunks * tstr.m_terrain->m_ChunkSize, (int)roundf(pt.x + pFrame->m_PaintRadius)); j++)
			{
				if (pFrame->m_PaintShape == CMainFrame::PaintShapeCircle)
				{
					my::Vector2 diff(j - pt.x, i - pt.z);
					float alpha = diff.magnitude() / pFrame->m_PaintRadius;
					if (alpha <= 1)
					{
						float height = pFrame->m_PaintHeight * pFrame->m_PaintSpline.Interpolate(1 - alpha, 1);
						switch (pFrame->m_PaintMode)
						{
						case CMainFrame::PaintModeGreater:
							height = my::Max(height, tstr.GetPos(i, j).y);
							break;
						}
						tstr.SetPos(my::Vector3(j, height, i), i, j, true);
					}
				}
			}
		}
	}
}


void CChildView::OnPaintTerrainColor(const my::Ray& ray, TerrainStream& tstr)
{
	// TODO: Add your implementation code here.
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	my::Ray local_ray = ray.transform(tstr.m_terrain->m_Actor->m_World.inverse());
	my::RayResult res = OverlapTestRayAndComponent(ray, local_ray, tstr.m_terrain);
	if (res.first)
	{
		my::Vector3 pt = local_ray.p + local_ray.d * res.second;
		for (int i = my::Max(0, (int)roundf(pt.z - pFrame->m_PaintRadius));
			i <= my::Min(tstr.m_terrain->m_RowChunks * tstr.m_terrain->m_ChunkSize, (int)roundf(pt.z + pFrame->m_PaintRadius)); i++)
		{
			for (int j = my::Max(0, (int)roundf(pt.x - pFrame->m_PaintRadius));
				j <= my::Min(tstr.m_terrain->m_ColChunks * tstr.m_terrain->m_ChunkSize, (int)roundf(pt.x + pFrame->m_PaintRadius)); j++)
			{
				if (pFrame->m_PaintShape == CMainFrame::PaintShapeCircle)
				{
					my::Vector2 diff(j - pt.x, i - pt.z);
					float alpha = diff.magnitude() / pFrame->m_PaintRadius;
					if (alpha <= 1)
					{
						D3DXCOLOR color = pFrame->m_PaintColor * pFrame->m_PaintSpline.Interpolate(1 - alpha, 1);
						switch (pFrame->m_PaintMode)
						{
						case CMainFrame::PaintModeGreater:
							color.r = my::Max(color.r, D3DXCOLOR(tstr.GetColor(i, j)).r);
							color.g = my::Max(color.g, D3DXCOLOR(tstr.GetColor(i, j)).g);
							color.b = my::Max(color.b, D3DXCOLOR(tstr.GetColor(i, j)).b);
							color.a = my::Max(color.a, D3DXCOLOR(tstr.GetColor(i, j)).a);
							break;
						}
						tstr.SetColor(color, i, j);
					}
				}
			}
		}
	}
}

void CChildView::OnPaintEmitterInstance(const my::Ray& ray, StaticEmitterComponent* emit)
{
	// TODO: Add your implementation code here.
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);

	struct Callback : public my::OctNode::QueryCallback
	{
		const my::Ray& ray;
		CMainFrame* pFrame;
		StaticEmitterComponent* emit;
		Callback(const my::Ray& _ray, CMainFrame* _pFrame, StaticEmitterComponent* _emit)
			: ray(_ray)
			, pFrame(_pFrame)
			, emit(_emit)
		{
		}
		virtual void OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
		{
			Actor* actor = dynamic_cast<Actor*>(oct_entity);
			ASSERT(actor);
			my::Ray local_ray = ray.transform(actor->m_World.inverse());
			my::Matrix4 terrain2emit = actor->m_World * emit->m_Actor->m_World.inverse();
			Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
			for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
			{
				if ((*cmp_iter)->m_Type == Component::ComponentTypeTerrain)
				{
					TerrainStream tstr(dynamic_cast<Terrain*>(cmp_iter->get()), false);
					my::RayResult res = tstr.RayTest(local_ray);
					if (res.first)
					{
						if (emit->m_ParticleList.full())
						{
							emit->m_ParticleList.set_capacity(emit->m_ParticleList.capacity() + 1024);
						}

						my::Vector3 pt = local_ray.p + local_ray.d * res.second;

						for (int i = 0; i < pFrame->m_PaintDensity; i++)
						{
							if (pFrame->m_PaintShape == CMainFrame::PaintShapeCircle)
							{
								my::Vector2 rand_circle = (pFrame->m_PaintDensity > 1 ? my::Vector2::RandomUnitCircle() * pFrame->m_PaintRadius : my::Vector2::zero);
								my::Ray spawn_ray(my::Vector3(pt.x + rand_circle.x, pt.y + 1000, pt.z + rand_circle.y), my::Vector3(0, -1, 0));
								my::RayResult spawn_res = tstr.RayTest(spawn_ray);
								if (spawn_res.first)
								{
									emit->Spawn((spawn_ray.p + spawn_ray.d * spawn_res.second).transformCoord(terrain2emit),
										my::Vector3(0, 0, 0), my::Vector4(1, 1, 1, 1), my::Vector2(10, 10), 0.0f, 0.0f);
								}
							}
						}
					}
				}
			}
		}
	};

	Callback cb(ray, pFrame, emit);
	pFrame->QueryEntity(ray, &cb);
	emit->BuildChunks();
}
