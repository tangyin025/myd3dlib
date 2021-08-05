
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "MainApp.h"

#include "ChildView.h"
#include "MainFrm.h"
#include "Animation.h"
#include "DetourDebugDraw.h"
#include "NavigationSerialization.h"
#include "StaticEmitter.h"

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
	, m_UICamera(D3DXToRadian(theApp.default_fov), 1.333333f, 0.1, 3000.0f)
	, m_raycmp(NULL)
	, m_raychunkid(0, 0)
	, m_rayinstid(0)
	, m_duDebugDrawPrimitives(DU_DRAW_QUADS + 1)
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
		CChildView * pView;
		Callback(const my::Frustum & _frustum, RenderPipeline * _pipeline, unsigned int _PassMask, const my::Vector3 & _ViewPos, const my::Vector3 & _TargetPos, CMainFrame * _pFrame, CChildView * _pView)
			: frustum(_frustum)
			, pipeline(_pipeline)
			, PassMask(_PassMask)
			, ViewPos(_ViewPos)
			, TargetPos(_TargetPos)
			, pFrame(_pFrame)
			, pView(_pView)
		{
		}
		virtual void OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
		{
			ASSERT(dynamic_cast<Actor *>(oct_entity));

			Actor * actor = static_cast<Actor *>(oct_entity);

			if (pFrame->GetActiveView() == pView && (PassMask | RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal)))
			{
				if (my::IntersectionTests::IntersectAABBAndAABB(aabb, my::AABB(TargetPos, 1000.0f)) != my::IntersectionTests::IntersectionTypeOutside)
				{
					if (!actor->IsRequested())
					{
						actor->RequestResource();
					}
				}
				else
				{
					if (actor->IsRequested())
					{
						actor->ReleaseResource();
					}
				}
			}

			if (actor->IsRequested())
			{
				actor->AddToPipeline(frustum, pipeline, PassMask, ViewPos, TargetPos);

				if (pView->m_bShowNavigation)
				{
					Actor::ComponentPtrList::const_iterator cmp_iter = actor->m_Cmps.begin();
					for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
					{
						if ((*cmp_iter)->m_Type == Component::ComponentTypeNavigation)
						{
							Navigation* navi = dynamic_cast<Navigation*>(cmp_iter->get());
							duDebugDrawNavMeshWithClosedList(pView, *navi->m_navMesh, *navi->m_navQuery, DU_DRAWNAVMESH_OFFMESHCONS | DU_DRAWNAVMESH_CLOSEDLIST);
						}
					}
				}
			}
		}
	};
	my::ModelViewerCamera * model_view_camera = dynamic_cast<my::ModelViewerCamera *>(m_Camera.get());
	pFrame->QueryEntity(frustum, &Callback(frustum, pipeline, PassMask, m_Camera->m_Eye, model_view_camera->m_LookAt, pFrame, this));
	//pFrame->m_emitter->AddToPipeline(frustum, pipeline, PassMask);
}

void CChildView::RenderSelectedActor(IDirect3DDevice9 * pd3dDevice, Actor * actor)
{
	PushLineAABB(*actor->m_Node, D3DCOLOR_ARGB(255, 255, 255, 0));
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
				D3DXMACRO macro[3] = { {0} };
				Animator* animator = cmp->m_Actor->GetAnimator();
				int j = 0;
				if (animator && mesh_cmp->m_Mesh->m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
				{
					macro[j++].Name = "SKELETON";
				}
				my::Effect* shader = theApp.QueryShader(RenderPipeline::MeshTypeMesh, macro, "shader/mtl_simplecolor.fx", RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeOpaque));
				if (shader)
				{
					shader->SetMatrix(shader->GetParameterByName(NULL, "g_World"), mesh_cmp->m_Actor->m_World);
					shader->SetVector(shader->GetParameterByName(NULL, "g_MeshColor"), my::Vector4(0, 1, 0, 1));
					if (animator && !animator->m_DualQuats.empty() && mesh_cmp->m_Mesh->m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
					{
						shader->SetMatrixArray(shader->GetParameterByName(NULL, "g_dualquat"), &animator->m_DualQuats[0], animator->m_DualQuats.size());
					}
					shader->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
					shader->BeginPass(RenderPipeline::PassTypeOpaque);
					mesh_cmp->m_Mesh->DrawSubset(mesh_cmp->m_MeshSubMeshId);
					shader->EndPass();
					shader->End();
				}
			}
		}
		break;

	case Component::ComponentTypeStaticEmitter:
		{
			if (pFrame->m_selactors.size() > 1 || pFrame->m_selcmp != cmp)
			{
				return;
			}
			StaticEmitter * static_emit_cmp = dynamic_cast<StaticEmitter *>(cmp);
			StaticEmitter::ChunkMap::const_iterator chunk_iter = static_emit_cmp->m_Chunks.find(std::make_pair(pFrame->m_selchunkid.x, pFrame->m_selchunkid.y));
			ASSERT(chunk_iter != static_emit_cmp->m_Chunks.end());
			PushLineAABB(chunk_iter->second.m_OctAabb->transform(cmp->m_Actor->m_World), D3DCOLOR_ARGB(255, 255, 0, 255));
			if (chunk_iter->second.m_buff && pFrame->m_selinstid < chunk_iter->second.m_buff->size())
			{
				my::Matrix4 p2World = GetParticleTransform(static_emit_cmp->m_EmitterFaceType, (*chunk_iter->second.m_buff)[pFrame->m_selinstid], m_Camera->m_View);
				if (static_emit_cmp->m_EmitterSpaceType != EmitterComponent::SpaceTypeWorld)
				{
					p2World *= cmp->m_Actor->m_World;
				}
				void* pvb = theApp.m_ParticleVb.Lock(0, 0, D3DLOCK_READONLY);
				void* pib = theApp.m_ParticleIb.Lock(0, 0, D3DLOCK_READONLY);
				for (int i = 0; i < RenderPipeline::m_ParticlePrimitiveInfo[static_emit_cmp->m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitivePrimitiveCount]; i++)
				{
					unsigned short* pi = (unsigned short*)pib + RenderPipeline::m_ParticlePrimitiveInfo[static_emit_cmp->m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitiveStartIndex] + i * 3;
					unsigned char* pv0 = (unsigned char*)pvb + *(pi + 0) * theApp.m_ParticleVertStride;
					unsigned char* pv1 = (unsigned char*)pvb + *(pi + 1) * theApp.m_ParticleVertStride;
					unsigned char* pv2 = (unsigned char*)pvb + *(pi + 2) * theApp.m_ParticleVertStride;
					my::Vector3 v0 = theApp.m_ParticleVertElems.GetPosition(pv0).transformCoord(p2World);
					my::Vector3 v1 = theApp.m_ParticleVertElems.GetPosition(pv1).transformCoord(p2World);
					my::Vector3 v2 = theApp.m_ParticleVertElems.GetPosition(pv2).transformCoord(p2World);
					PushLine(v0, v1, D3DCOLOR_ARGB(255, 0, 255, 0));
					PushLine(v1, v2, D3DCOLOR_ARGB(255, 0, 255, 0));
					PushLine(v2, v0, D3DCOLOR_ARGB(255, 0, 255, 0));
				}
				theApp.m_ParticleVb.Unlock();
				theApp.m_ParticleIb.Unlock();
			}
		}
		break;

	case Component::ComponentTypeTerrain:
		{
			if (pFrame->m_selactors.size() > 1 || pFrame->m_selcmp != cmp)
			{
				return;
			}
			Terrain * terrain = dynamic_cast<Terrain *>(cmp);
			PushLineAABB(terrain->m_Chunks[pFrame->m_selchunkid.x][pFrame->m_selchunkid.y].m_OctAabb->transform(terrain->m_Actor->m_World), D3DCOLOR_ARGB(255, 255, 0, 255));
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

my::Matrix4 CChildView::GetParticleTransform(DWORD EmitterFaceType, const my::Emitter::Particle & particle, const my::Matrix4 & View)
{
	my::Vector3 sph = View.getColumn<2>().xyz.cartesianToSpherical();
	switch (EmitterFaceType)
	{
	case EmitterComponent::FaceTypeX:
		return my::Matrix4::Compose(
			my::Vector3(particle.m_Size.x, particle.m_Size.y, particle.m_Size.x),
			my::Quaternion::RotationAxis(my::Vector3::unitZ, particle.m_Angle) * my::Quaternion::RotationAxis(my::Vector3::unitY, D3DXToRadian(90)),
			particle.m_Position);
	case EmitterComponent::FaceTypeY:
		return my::Matrix4::Compose(
			my::Vector3(particle.m_Size.x, particle.m_Size.y, particle.m_Size.x),
			my::Quaternion::RotationAxis(my::Vector3::unitZ, particle.m_Angle) * my::Quaternion::RotationAxis(my::Vector3::unitX, D3DXToRadian(-90)),
			particle.m_Position);
	case EmitterComponent::FaceTypeZ:
		return my::Matrix4::Compose(
			my::Vector3(particle.m_Size.x, particle.m_Size.y, particle.m_Size.x),
			my::Quaternion::RotationAxis(my::Vector3::unitZ, particle.m_Angle),
			particle.m_Position);
	case EmitterComponent::FaceTypeCamera:
		return my::Matrix4::Compose(
			my::Vector3(particle.m_Size.x, particle.m_Size.y, particle.m_Size.x),
			my::Quaternion::RotationAxis(my::Vector3::unitZ, particle.m_Angle) * my::Quaternion::RotationAxis(my::Vector3::unitX, -sph.y) * my::Quaternion::RotationAxis(my::Vector3::unitY, D3DXToRadian(90) - sph.z),
			particle.m_Position);
	case EmitterComponent::FaceTypeAngle:
		return my::Matrix4::Compose(
			my::Vector3(particle.m_Size.x, particle.m_Size.y, particle.m_Size.x),
			my::Quaternion::RotationAxis(my::Vector3::unitY, particle.m_Angle),
			particle.m_Position);
	case EmitterComponent::FaceTypeAngleCamera:
		return my::Matrix4::Compose(
			my::Vector3(particle.m_Size.x, particle.m_Size.y, particle.m_Size.x),
			my::Quaternion::RotationAxis(my::Vector3::unitX, particle.m_Angle) * my::Quaternion::RotationAxis(my::Vector3::unitY, D3DXToRadian(90) - sph.z),
			particle.m_Position);
	}
	return my::Matrix4::Identity();
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
			Animator* animator = mesh_cmp->m_Actor->GetAnimator();
			if (animator && !animator->m_DualQuats.empty() && mesh_cmp->m_Mesh->m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
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
					animator->m_DualQuats);
				bool ret = my::Mesh::FrustumTest(local_ftm,
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					mesh_cmp->m_Mesh->LockIndexBuffer(D3DLOCK_READONLY),
					!(mesh_cmp->m_Mesh->GetOptions() & D3DXMESH_32BIT),
					mesh_cmp->m_Mesh->GetNumFaces(),
					elems);
				mesh_cmp->m_Mesh->UnlockVertexBuffer();
				mesh_cmp->m_Mesh->UnlockIndexBuffer();
				if (ret)
				{
					return true;
				}
			}
			else
			{
				bool ret = my::Mesh::FrustumTest(local_ftm,
					mesh_cmp->m_Mesh->LockVertexBuffer(D3DLOCK_READONLY),
					mesh_cmp->m_Mesh->GetNumVertices(),
					mesh_cmp->m_Mesh->GetNumBytesPerVertex(),
					mesh_cmp->m_Mesh->LockIndexBuffer(D3DLOCK_READONLY),
					!(mesh_cmp->m_Mesh->GetOptions() & D3DXMESH_32BIT),
					mesh_cmp->m_Mesh->GetNumFaces(),
					mesh_cmp->m_Mesh->m_VertexElems);
				mesh_cmp->m_Mesh->UnlockVertexBuffer();
				mesh_cmp->m_Mesh->UnlockIndexBuffer();
				if (ret)
				{
					return true;
				}
			}
		}
		break;

	case Component::ComponentTypeStaticEmitter:
		{
			StaticEmitter * static_emit_cmp = dynamic_cast<StaticEmitter*>(cmp);
			struct Callback : public my::OctNode::QueryCallback
			{
				CChildView * pView;
				const my::Frustum & frustum;
				const my::Frustum & local_ftm;
				EmitterComponent * emitter;
				bool ret;
				Callback(CChildView * _pView, const my::Frustum & _frustum, const my::Frustum & _local_ftm, EmitterComponent * _emitter)
					: pView(_pView)
					, frustum(_frustum)
					, local_ftm(_local_ftm)
					, emitter(_emitter)
					, ret(false)
				{
				}
				virtual void OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
				{
					StaticEmitterChunk* chunk = dynamic_cast<StaticEmitterChunk*>(oct_entity);
					if (chunk->m_buff)
					{
						if (!ret && pView->OverlapTestFrustumAndParticles(frustum, local_ftm, emitter, &(*chunk->m_buff)[0], chunk->m_buff->size()))
						{
							ret = true;
						}
					}
				}
			};
			Callback cb(this, frustum, local_ftm, static_emit_cmp);
			static_emit_cmp->QueryEntity(local_ftm, &cb);
			return cb.ret;
		}
		break;

	case Component::ComponentTypeSphericalEmitter:
		{
			SphericalEmitter * sphe_emit_cmp = dynamic_cast<SphericalEmitter*>(cmp);
			if (sphe_emit_cmp->m_ParticleList.empty())
			{
				return false;
			}
			my::Emitter::ParticleList::array_range array_one = sphe_emit_cmp->m_ParticleList.array_one();
			if (OverlapTestFrustumAndParticles(frustum, local_ftm, sphe_emit_cmp, array_one.first, array_one.second))
			{
				return true;
			}
			my::Emitter::ParticleList::array_range array_two = sphe_emit_cmp->m_ParticleList.array_two();
			if (OverlapTestFrustumAndParticles(frustum, local_ftm, sphe_emit_cmp, array_two.first, array_two.second))
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
			Animator* animator = cloth_cmp->m_Actor->GetAnimator();
			if (animator && !animator->m_DualQuats.empty() && cloth_cmp->m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
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
					animator->m_DualQuats);
				bool ret = my::Mesh::FrustumTest(local_ftm,
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
				bool ret = my::Mesh::FrustumTest(local_ftm,
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
	}
	return false;
}

bool CChildView::OverlapTestFrustumAndParticles(const my::Frustum & frustum, const my::Frustum & local_ftm, EmitterComponent * emitter, const my::Emitter::Particle * part_start, int part_num)
{
	void* pvb = theApp.m_ParticleVb.Lock(0, 0, D3DLOCK_READONLY);
	void* pib = theApp.m_ParticleIb.Lock(0, 0, D3DLOCK_READONLY);
	const my::Emitter::Particle* part_iter = part_start;
	for (; part_iter != part_start + part_num; part_iter++)
	{
		my::Matrix4 p2local = GetParticleTransform(emitter->m_EmitterFaceType, *part_iter, m_Camera->m_View);
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
		bool ret = my::Mesh::FrustumTest(particle_ftm, pvb, 0, theApp.m_ParticleVertStride,
			(unsigned short*)pib + RenderPipeline::m_ParticlePrimitiveInfo[emitter->m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitiveStartIndex], true,
			RenderPipeline::m_ParticlePrimitiveInfo[emitter->m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitivePrimitiveCount], theApp.m_ParticleVertElems);
		if (ret)
		{
			theApp.m_ParticleVb.Unlock();
			theApp.m_ParticleIb.Unlock();
			return true;
		}
	}
	theApp.m_ParticleVb.Unlock();
	theApp.m_ParticleIb.Unlock();
	return false;
}

my::RayResult CChildView::OverlapTestRayAndActor(const my::Ray & ray, Actor * actor)
{
	if (actor->m_Cmps.empty())
	{
		m_raycmp = NULL;
		return my::IntersectionTests::rayAndAABB(ray.p, ray.d, *actor->m_OctAabb);
	}

	my::RayResult ret(false, FLT_MAX);
	m_raycmp = NULL;
	CPoint raychunkid;
	int rayinstid;
	my::Ray local_ray = ray.transform(actor->m_World.inverse());
	Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
	for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
	{
		my::RayResult result = OverlapTestRayAndComponent(ray, local_ray, cmp_iter->get(), raychunkid, rayinstid);
		if (result.first && result.second < ret.second)
		{
			ret = result;
			m_raycmp = cmp_iter->get();
			m_raychunkid = raychunkid;
			m_rayinstid = rayinstid;
		}
	}

	if (ret.first)
	{
		ret.second = (local_ray.d * ret.second).transformNormal(actor->m_World).magnitude();
	}
	return ret;
}

my::RayResult CChildView::OverlapTestRayAndComponent(const my::Ray & ray, const my::Ray & local_ray, Component * cmp, CPoint & raychunkid, int & rayinstid)
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
			Animator* animator = mesh_cmp->m_Actor->GetAnimator();
			if (animator && !animator->m_DualQuats.empty() && mesh_cmp->m_Mesh->m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
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
					animator->m_DualQuats);
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
				raychunkid.SetPoint(0, 0);
				rayinstid = 0;
				return ret;
			}
		}
		break;


	case Component::ComponentTypeStaticEmitter:
		{
			StaticEmitter * static_emit_cmp = dynamic_cast<StaticEmitter*>(cmp);
			struct Callback : public my::OctNode::QueryCallback
			{
				CChildView * pView;
				const my::Ray & ray;
				const my::Ray & local_ray;
				EmitterComponent * emitter;
				my::RayResult ret;
				CPoint raychunkid;
				int rayinstid;
				Callback(CChildView * _pView, const my::Ray & _ray, const my::Ray & _local_ray, EmitterComponent * _emitter)
					: pView(_pView)
					, ray(_ray)
					, local_ray(_local_ray)
					, emitter(_emitter)
					, ret(false, FLT_MAX)
					, raychunkid(0, 0)
					, rayinstid(0)
				{
				}
				virtual void OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
				{
					StaticEmitterChunk* chunk = dynamic_cast<StaticEmitterChunk*>(oct_entity);
					if (chunk->m_buff)
					{
						int part_id;
						my::RayResult result = pView->OverlapTestRayAndParticles(ray, local_ray, emitter, &(*chunk->m_buff)[0], chunk->m_buff->size(), part_id);
						if (result.first && result.second < ret.second)
						{
							ret = result;
							raychunkid.SetPoint(chunk->m_Row, chunk->m_Col);
							rayinstid = part_id;
						}
					}
				}
			};
			Callback cb(this, ray, local_ray, static_emit_cmp);
			static_emit_cmp->QueryEntity(local_ray, &cb);
			if (cb.ret.first)
			{
				raychunkid = cb.raychunkid;
				rayinstid = cb.rayinstid;
				return cb.ret;
			}
		}
		break;

	case Component::ComponentTypeSphericalEmitter:
		{
			SphericalEmitter* sphe_emit_cmp = dynamic_cast<SphericalEmitter *>(cmp);
			if (sphe_emit_cmp->m_ParticleList.empty())
			{
				return my::RayResult(false, FLT_MAX);
			}
			int part_id;
			my::Emitter::ParticleList::array_range array_one = sphe_emit_cmp->m_ParticleList.array_one();
			my::RayResult ret = OverlapTestRayAndParticles(ray, local_ray, sphe_emit_cmp, array_one.first, array_one.second, part_id);
			if (ret.first)
			{
				raychunkid.SetPoint(0, 0);
				rayinstid = part_id;
				return ret;
			}
			my::Emitter::ParticleList::array_range array_two = sphe_emit_cmp->m_ParticleList.array_two();
			ret = OverlapTestRayAndParticles(ray, local_ray, sphe_emit_cmp, array_two.first, array_two.second, part_id);
			if (ret.first)
			{
				raychunkid.SetPoint(0, 0);
				rayinstid = part_id;
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
			Animator* animator = cloth_cmp->m_Actor->GetAnimator();
			if (animator && !animator->m_DualQuats.empty() && cloth_cmp->m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
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
					animator->m_DualQuats);
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
				raychunkid.SetPoint(0, 0);
				rayinstid = 0;
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
				CPoint raychunkid;
				Callback(const my::Ray & _ray, const my::Vector3 & _ViewPos, CChildView * _pView, Terrain * _terrain)
					: ray(_ray)
					, pView(_pView)
					, ViewPos(_ViewPos)
					, terrain(_terrain)
					, ret(false, FLT_MAX)
					, raychunkid(0, 0)
				{
				}
				virtual void OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
				{
					TerrainChunk * chunk = dynamic_cast<TerrainChunk *>(oct_entity);
					my::RayResult result;
					if (!chunk->m_Vb)
					{
						std::vector<unsigned short> ib;
						for (int i = chunk->m_Row * terrain->m_MinLodChunkSize; i < chunk->m_Row * terrain->m_MinLodChunkSize + terrain->m_MinLodChunkSize; i++)
						{
							for (int j = chunk->m_Col * terrain->m_MinLodChunkSize; j < chunk->m_Col * terrain->m_MinLodChunkSize + terrain->m_MinLodChunkSize; j++)
							{
								ib.push_back((terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1) * (i + 0) + (j + 0));
								ib.push_back((terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1) * (i + 1) + (j + 0));
								ib.push_back((terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1) * (i + 0) + (j + 1));
								ib.push_back((terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1) * (i + 0) + (j + 1));
								ib.push_back((terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1) * (i + 1) + (j + 0));
								ib.push_back((terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1) * (i + 1) + (j + 1));
							}
						}
						result = my::Mesh::RayTest(
							ray,
							terrain->m_rootVb.Lock(0, 0, D3DLOCK_READONLY),
							(terrain->m_RowChunks + 1) * (terrain->m_ColChunks + 1),
							terrain->m_VertexStride,
							&ib[0],
							true,
							ib.size() / 3,
							terrain->m_VertexElems);
						terrain->m_rootVb.Unlock();
					}
					else
					{
						const Terrain::Fragment& frag = terrain->GetFragment(
							terrain->CalculateLod(*chunk->m_OctAabb, ViewPos),
							terrain->CalculateLod(*terrain->m_Chunks[chunk->m_Row][my::Max(chunk->m_Col - 1, 0)].m_OctAabb, ViewPos),
							terrain->CalculateLod(*terrain->m_Chunks[my::Max(chunk->m_Row - 1, 0)][chunk->m_Col].m_OctAabb, ViewPos),
							terrain->CalculateLod(*terrain->m_Chunks[chunk->m_Row][my::Min(chunk->m_Col + 1, (int)terrain->m_Chunks.shape()[1] - 1)].m_OctAabb, ViewPos),
							terrain->CalculateLod(*terrain->m_Chunks[my::Min(chunk->m_Row + 1, (int)terrain->m_Chunks.shape()[0] - 1)][chunk->m_Col].m_OctAabb, ViewPos));
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
						raychunkid.SetPoint(chunk->m_Row, chunk->m_Col);
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
				raychunkid = cb.raychunkid;
				rayinstid = 0;
				return cb.ret;
			}
		}
		break;
	}
	return my::RayResult(false, FLT_MAX);
}

my::RayResult CChildView::OverlapTestRayAndParticles(const my::Ray & ray, const my::Ray & local_ray, EmitterComponent * emitter, const my::Emitter::Particle * part_start, int part_num, int & part_id)
{
	my::RayResult ret(false, FLT_MAX);
	void* pvb = theApp.m_ParticleVb.Lock(0, 0, D3DLOCK_READONLY);
	void* pib = theApp.m_ParticleIb.Lock(0, 0, D3DLOCK_READONLY);
	const my::Emitter::Particle* part_iter = part_start;
	for (; part_iter != part_start + part_num; part_iter++)
	{
		my::Matrix4 p2local = GetParticleTransform(emitter->m_EmitterFaceType, *part_iter, m_Camera->m_View);
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
		my::RayResult result = my::Mesh::RayTest(particle_ray, pvb, 0, theApp.m_ParticleVertStride,
			(unsigned short*)pib + RenderPipeline::m_ParticlePrimitiveInfo[emitter->m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitiveStartIndex], true,
			RenderPipeline::m_ParticlePrimitiveInfo[emitter->m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitivePrimitiveCount], theApp.m_ParticleVertElems);
		if (result.first)
		{
			result.second = (particle_ray.d * result.second).transformNormal(p2local).magnitude();
			if (result.second < ret.second)
			{
				ret = result;
				part_id = (int)std::distance(part_start, part_iter);
			}
		}
	}
	theApp.m_ParticleVb.Unlock();
	theApp.m_ParticleIb.Unlock();
	if (ret.first && emitter->m_EmitterSpaceType == EmitterComponent::SpaceTypeWorld)
	{
		ret.second = (ray.d * ret.second).transformNormal(emitter->m_Actor->m_World.inverse()).magnitude();
	}
	return ret;
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
	m_duDebugDrawPrimitives = prim;
}

#define DUCOLOR_TO_D3DCOLOR(col) ((col & 0xff00ff00) | (col & 0x00ff0000) >> 16 | (col & 0x000000ff) << 16)

void CChildView::vertex(const float* pos, unsigned int color)
{
	if (m_duDebugDrawPrimitives == DU_DRAW_LINES)
	{
		PushLineVertex(pos[0], pos[1], pos[2], DUCOLOR_TO_D3DCOLOR(color));
	}
	else if (m_duDebugDrawPrimitives == DU_DRAW_TRIS)
	{
		PushTriangleVertex(pos[0], pos[1], pos[2], DUCOLOR_TO_D3DCOLOR(color));
	}
}

void CChildView::vertex(const float x, const float y, const float z, unsigned int color)
{
	if (m_duDebugDrawPrimitives == DU_DRAW_LINES)
	{
		PushLineVertex(x, y, z, DUCOLOR_TO_D3DCOLOR(color));
	}
	else if (m_duDebugDrawPrimitives == DU_DRAW_TRIS)
	{
		PushTriangleVertex(x, y, z, DUCOLOR_TO_D3DCOLOR(color));
	}
}

void CChildView::vertex(const float* pos, unsigned int color, const float* uv)
{
	if (m_duDebugDrawPrimitives == DU_DRAW_LINES)
	{
		PushLineVertex(pos[0], pos[1], pos[2], DUCOLOR_TO_D3DCOLOR(color));
	}
	else if (m_duDebugDrawPrimitives == DU_DRAW_TRIS)
	{
		PushTriangleVertex(pos[0], pos[1], pos[2], DUCOLOR_TO_D3DCOLOR(color));
	}
}

void CChildView::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
{
	if (m_duDebugDrawPrimitives == DU_DRAW_LINES)
	{
		PushLineVertex(x, y, z, DUCOLOR_TO_D3DCOLOR(color));
	}
	else if (m_duDebugDrawPrimitives == DU_DRAW_TRIS)
	{
		PushTriangleVertex(x, y, z, DUCOLOR_TO_D3DCOLOR(color));
	}
}

void CChildView::end()
{
	m_duDebugDrawPrimitives = DU_DRAW_QUADS + 1;
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
				PushLineGrid(12, 5, 5, D3DCOLOR_ARGB(255,127,127,127), D3DCOLOR_ARGB(255,0,0,0), my::Matrix4::RotationX(D3DXToRadian(-90)));
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
					PushLineAABB(pFrame->m_selbox, D3DCOLOR_ARGB(255,255,255,255));
					CMainFrame::SelActorList::const_iterator sel_iter = pFrame->m_selactors.begin();
					for (; sel_iter != pFrame->m_selactors.end(); sel_iter++)
					{
						RenderSelectedActor(theApp.m_d3dDevice, *sel_iter);
					}
				}

				V(theApp.m_d3dDevice->SetVertexShader(NULL));
				V(theApp.m_d3dDevice->SetPixelShader(NULL));
				V(theApp.m_d3dDevice->SetTexture(0, NULL));
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE));
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE));
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID));
				V(theApp.m_d3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera->m_View));
				V(theApp.m_d3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera->m_Proj));
				V(theApp.m_d3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&my::Matrix4::identity));
				DrawHelper::EndLine(theApp.m_d3dDevice);

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
				theApp.m_UIRender->SetViewProj(m_UICamera.m_ViewProj);
				theApp.m_UIRender->SetWorld(my::Matrix4::identity);
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
				my::DialogMgr::DialogList::iterator dlg_iter = ((my::DialogMgr*)pFrame)->m_DlgList.begin();
				for (; dlg_iter != ((my::DialogMgr*)pFrame)->m_DlgList.end(); dlg_iter++)
				{
					theApp.m_UIRender->SetWorld((*dlg_iter)->m_World);

					(*dlg_iter)->Draw(theApp.m_UIRender.get(), theApp.m_fAbsoluteElapsedTime);

					theApp.m_UIRender->Flush();
				}
				theApp.m_UIRender->SetWorld(my::Matrix4::identity);
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

		m_UICamera.m_Fov = D3DXToRadian(theApp.default_fov);
		m_UICamera.m_Euler.x = -D3DX_PI;
		m_UICamera.m_Eye = my::Vector3(cx * 0.5f - 0.5f, cy * 0.5f - 0.5f, -cy * 0.5f * cotf(m_UICamera.m_Fov * 0.5f));
		m_UICamera.OnViewportChanged(my::Vector2((float)cx, (float)cy));
		m_UICamera.UpdateViewProj();
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
		m_PaintTerrainCaptured.reset(new TerrainStream(terrain));
		OnPaintTerrainHeightField(ray, *m_PaintTerrainCaptured);
		SetCapture();
		Invalidate();
		return;
	}

	if (terrain && pFrame->m_PaintType == CMainFrame::PaintTypeTerrainColor)
	{
		m_PaintTerrainCaptured.reset(new TerrainStream(terrain));
		OnPaintTerrainColor(ray, *m_PaintTerrainCaptured);
		SetCapture();
		Invalidate();
		return;
	}

	StaticEmitter* emit = dynamic_cast<StaticEmitter*>(pFrame->GetSelComponent(Component::ComponentTypeStaticEmitter));
	if (emit && pFrame->m_PaintType == CMainFrame::PaintTypeEmitterInstance)
	{
		m_PaintEmitterCaptured.reset(new StaticEmitterStream(emit));
		OnPaintEmitterInstance(ray, *m_PaintEmitterCaptured);
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
		pFrame->m_selcmp = NULL;
		pFrame->m_selchunkid.SetPoint(0, 0);
		pFrame->m_selinstid = 0;
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
		pFrame->m_selcmp = NULL;
		pFrame->m_selchunkid.SetPoint(0, 0);
		pFrame->m_selinstid = 0;
	}
	else
	{
		struct Callback : public my::OctNode::QueryCallback
		{
			const my::Ray & ray;
			CChildView * pView;
			Actor * selact;
			Component * selcmp;
			float seldist;
			CPoint selchunkid;
			int selinstid;
			Callback(const my::Ray & _ray, CChildView * _pView)
				: ray(_ray)
				, pView(_pView)
				, selact(NULL)
				, selcmp(NULL)
				, seldist(FLT_MAX)
				, selchunkid(0, 0)
				, selinstid(0)
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
					selcmp = pView->m_raycmp;
					seldist = ret.second;
					selchunkid = pView->m_raychunkid;
					selinstid = pView->m_rayinstid;
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
				pFrame->m_selcmp = NULL;
				pFrame->m_selchunkid.SetPoint(0, 0);
				pFrame->m_selinstid = 0;
			}
			else
			{
				pFrame->m_selactors.push_back(cb.selact);
				pFrame->m_selcmp = cb.selcmp;
				pFrame->m_selchunkid = cb.selchunkid;
				pFrame->m_selinstid = cb.selinstid;
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
		m_PaintTerrainCaptured->Release();
		m_PaintTerrainCaptured->m_terrain->m_Actor->UpdateAABB();
		m_PaintTerrainCaptured->m_terrain->m_Actor->UpdateOctNode();
		pFrame->UpdateSelBox();
		m_PaintTerrainCaptured.reset();
		ReleaseCapture();
		Invalidate();
		return;
	}

	if (m_PaintEmitterCaptured)
	{
		m_PaintEmitterCaptured->Release();
		m_PaintEmitterCaptured->m_emit->m_Actor->UpdateAABB();
		m_PaintEmitterCaptured->m_emit->m_Actor->UpdateOctNode();
		pFrame->UpdateSelBox();
		m_PaintEmitterCaptured.reset();
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
			(*sel_iter)->UpdateOctNode();

			physx::PxRigidBody * body = NULL;
			if ((*sel_iter)->m_PxActor && (body = (*sel_iter)->m_PxActor->is<physx::PxRigidBody>()) && !body->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
			{
				body->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
				body->setLinearVelocity((physx::PxVec3&)my::Vector3(0, 0, 0));
			}

			//Actor::ComponentPtrList::iterator cmp_iter = (*sel_iter)->m_Cmps.begin();
			//for (; cmp_iter != (*sel_iter)->m_Cmps.end(); cmp_iter++)
			//{
			//	if ((*cmp_iter)->m_Type == Component::ComponentTypeStaticEmitter
			//		&& dynamic_cast<StaticEmitter*>(cmp_iter->get())->m_EmitterSpaceType == EmitterComponent::SpaceTypeWorld)
			//	{
			//		dynamic_cast<StaticEmitter*>(cmp_iter->get())->BuildChunks();
			//	}
			//}
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
		OnPaintEmitterInstance(ray, *m_PaintEmitterCaptured);
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
				pFrame->m_ActorList.push_back(new_actor);
				new_selactors.push_back(new_actor.get());
			}
			pFrame->m_selactors.swap(new_selactors);
			pFrame->m_selcmp = NULL;
			pFrame->m_selchunkid.SetPoint(0, 0);
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

			(*sel_iter)->SetPxPoseOrbyPxThread(physx::PxTransform((physx::PxVec3&)(*sel_iter)->m_Position, (physx::PxQuat&)(*sel_iter)->m_Rotation));
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
						if ((*cmp_iter)->m_Type == Component::ComponentTypeTerrain && cmp_iter->get() == pFrame->m_selcmp)
						{
							Terrain * terrain = dynamic_cast<Terrain *>(cmp_iter->get());
							my::AABB chunk_box = terrain->m_Chunks[pFrame->m_selchunkid.x][pFrame->m_selchunkid.y].m_OctAabb->transform(terrain->m_Actor->m_World);
							float chunk_radius = chunk_box.Extent().magnitude() * 0.5f;
							if ((model_view_camera->m_LookAt - chunk_box.Center()).magnitudeSq() > EPSILON_E6 * EPSILON_E6
								|| fabs(model_view_camera->m_Distance - chunk_radius / asin(model_view_camera->m_Fov * 0.5f)) > EPSILON_E6)
							{
								focus_box = chunk_box;
							}
							break;
						}
						else if ((*cmp_iter)->m_Type == Component::ComponentTypeStaticEmitter && cmp_iter->get() == pFrame->m_selcmp)
						{
							StaticEmitter* static_emit_cmp = dynamic_cast<StaticEmitter*>(cmp_iter->get());
							StaticEmitter::ChunkMap::const_iterator chunk_iter = static_emit_cmp->m_Chunks.find(std::make_pair(pFrame->m_selchunkid.x, pFrame->m_selchunkid.y));
							ASSERT(chunk_iter != static_emit_cmp->m_Chunks.end());
							my::AABB chunk_box = chunk_iter->second.m_OctAabb->transform(static_emit_cmp->m_Actor->m_World);
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
	CPoint raychunkid; int rayinstid;
	my::Ray local_ray = ray.transform(tstr.m_terrain->m_Actor->m_World.inverse());
	my::RayResult res = OverlapTestRayAndComponent(ray, local_ray, tstr.m_terrain, raychunkid, rayinstid);
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
					my::Vector2 offset(j - pt.x, i - pt.z);
					float dist = offset.magnitude() / pFrame->m_PaintRadius;
					if (dist <= 1)
					{
						float height = pFrame->m_PaintHeight * pFrame->m_PaintSpline.Interpolate(1 - dist, 1);
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
	CPoint raychunkid; int rayinstid;
	my::Ray local_ray = ray.transform(tstr.m_terrain->m_Actor->m_World.inverse());
	my::RayResult res = OverlapTestRayAndComponent(ray, local_ray, tstr.m_terrain, raychunkid, rayinstid);
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
					my::Vector2 offset(j - pt.x, i - pt.z);
					float dist = offset.magnitude() / pFrame->m_PaintRadius;
					if (dist <= 1)
					{
						D3DXCOLOR color;
						D3DXColorLerp(&color, &D3DXCOLOR(tstr.GetColor(i, j)), &pFrame->m_PaintColor, pFrame->m_PaintSpline.Interpolate(1 - dist, 1));
						tstr.SetColor(color, i, j);
					}
				}
			}
		}
	}
}

void CChildView::OnPaintEmitterInstance(const my::Ray& ray, StaticEmitterStream& estr)
{
	// TODO: Add your implementation code here.
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);

	struct Callback : public my::OctNode::QueryCallback
	{
		const my::Ray& ray;
		CMainFrame* pFrame;
		StaticEmitterStream& estr;
		Callback(const my::Ray& _ray, CMainFrame* _pFrame, StaticEmitterStream& _estr)
			: ray(_ray)
			, pFrame(_pFrame)
			, estr(_estr)
		{
		}
		virtual void OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
		{
			Actor* actor = dynamic_cast<Actor*>(oct_entity);
			ASSERT(actor);
			my::Ray local_ray = ray.transform(actor->m_World.inverse());
			my::Matrix4 local2emit = actor->m_World * estr.m_emit->m_Actor->m_World.inverse();
			my::Matrix4 emit2local = local2emit.inverse();
			Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
			for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
			{
				if ((*cmp_iter)->m_Type == Component::ComponentTypeTerrain)
				{
					TerrainStream tstr(dynamic_cast<Terrain*>(cmp_iter->get()));
					my::RayResult res = tstr.RayTest(local_ray);
					if (res.first)
					{
						my::Vector3 pt = local_ray.p + local_ray.d * res.second;
						my::Vector3 emit_pt = pt.transformCoord(local2emit);
						if (!estr.m_emit->PtInRect(emit_pt))
						{
							continue;
						}
						std::list<my::Vector2> candidate;
						my::Emitter::Particle * particle = estr.GetNearestParticle2D(emit_pt.x, emit_pt.z, pFrame->m_PaintParticleMinDist);
						if (!particle)
						{
							estr.Spawn(emit_pt, my::Vector3(0, 0, 0), my::Vector4(1, 1, 1, 1), my::Vector2(1, 1), 0.0f, 0.0f);
							candidate.push_back(my::Vector2(emit_pt.x, emit_pt.z));
						}
						else
						{
							candidate.push_back(my::Vector2(particle->m_Position.x, particle->m_Position.z));
						}

						do 
						{
							const my::Vector2 & pos = candidate.front();
							for (int i = 0; i < 30; i++)
							{
								my::Vector2 rand_pos = pos + my::Vector2::RandomUnit() * my::Random(1.0f, 2.0f) * pFrame->m_PaintParticleMinDist;
								if (rand_pos.x >= estr.m_emit->m_min.x && rand_pos.x < estr.m_emit->m_max.x
									&& rand_pos.y >= estr.m_emit->m_min.z && rand_pos.y < estr.m_emit->m_max.z
									&& (rand_pos - my::Vector2(emit_pt.x, emit_pt.z)).magnitudeSq() < pFrame->m_PaintRadius * pFrame->m_PaintRadius)
								{
									if (!estr.GetNearestParticle2D(rand_pos.x, rand_pos.y, pFrame->m_PaintParticleMinDist))
									{
										my::Ray local_ray = my::Ray(my::Vector3(rand_pos.x, estr.m_emit->m_max.y, rand_pos.y), my::Vector3(0, -1, 0)).transform(emit2local);
										my::RayResult res = tstr.RayTest(local_ray);
										if (res.first)
										{
											my::Vector3 emit_pos = (local_ray.p + local_ray.d * res.second).transformCoord(local2emit);
											if (estr.m_emit->PtInRect(emit_pos))
											{
												estr.Spawn(emit_pos, my::Vector3(0, 0, 0), my::Vector4(1, 1, 1, 1), my::Vector2(1, 1), 0.0f, 0.0f);
												candidate.push_back(rand_pos);
											}
										}
									}
								}
							}
							candidate.pop_front();
						}
						while (!candidate.empty());
					}
				}
			}
		}
	};

	Callback cb(ray, pFrame, estr);
	pFrame->QueryEntity(ray, &cb);
}
