
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "MainApp.h"

#include "ChildView.h"
#include "MainFrm.h"
#include "Animator.h"
#include "NavigationSerialization.h"
#include "StaticEmitter.h"
#include <boost/scope_exit.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include "DebugDraw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const float ctl_handle_size = 3;

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
	ON_WM_LBUTTONDBLCLK()
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
	ON_COMMAND(ID_RENDERMODE_BLOOM, &CChildView::OnRendermodeBloom)
	ON_UPDATE_COMMAND_UI(ID_RENDERMODE_BLOOM, &CChildView::OnUpdateRendermodeBloom)
	ON_COMMAND(ID_RENDERMODE_FXAA, &CChildView::OnRendermodeFxaa)
	ON_UPDATE_COMMAND_UI(ID_RENDERMODE_FXAA, &CChildView::OnUpdateRendermodeFxaa)
	ON_COMMAND(ID_RENDERMODE_SSAO, &CChildView::OnRendermodeSsao)
	ON_UPDATE_COMMAND_UI(ID_RENDERMODE_SSAO, &CChildView::OnUpdateRendermodeSsao)
	ON_COMMAND(ID_SHOW_NAVIGATION, &CChildView::OnShowNavigation)
	ON_UPDATE_COMMAND_UI(ID_SHOW_NAVIGATION, &CChildView::OnUpdateShowNavigation)
	ON_COMMAND(ID_RENDERTARGET_NORMAL, &CChildView::OnRendertargetNormal)
	ON_UPDATE_COMMAND_UI(ID_RENDERTARGET_NORMAL, &CChildView::OnUpdateRendertargetNormal)
	ON_COMMAND(ID_RENDERTARGET_POSITION, &CChildView::OnRendertargetPosition)
	ON_UPDATE_COMMAND_UI(ID_RENDERTARGET_POSITION, &CChildView::OnUpdateRendertargetPosition)
	ON_COMMAND(ID_RENDERTARGET_LIGHT, &CChildView::OnRendertargetLight)
	ON_UPDATE_COMMAND_UI(ID_RENDERTARGET_LIGHT, &CChildView::OnUpdateRendertargetLight)
	ON_COMMAND(ID_RENDERTARGET_OPAQUE, &CChildView::OnRendertargetOpaque)
	ON_UPDATE_COMMAND_UI(ID_RENDERTARGET_OPAQUE, &CChildView::OnUpdateRendertargetOpaque)
	ON_COMMAND(ID_RENDERTARGET_DOWNFILTER, &CChildView::OnRendertargetDownfilter)
	ON_UPDATE_COMMAND_UI(ID_RENDERTARGET_DOWNFILTER, &CChildView::OnUpdateRendertargetDownfilter)
	ON_COMMAND(ID_RENDERTARGET_SPECULAR, &CChildView::OnRendertargetSpecular)
	ON_UPDATE_COMMAND_UI(ID_RENDERTARGET_SPECULAR, &CChildView::OnUpdateRendertargetSpecular)
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
	m_SpecularRT.reset(new my::Texture2D());
	m_PositionRT.reset(new my::Texture2D());
	m_LightRT.reset(new my::Texture2D());
	for (unsigned int i = 0; i < RenderPipeline::RTChain::RTArray::static_size; i++)
	{
		m_OpaqueRT.m_RenderTarget[i].reset(new my::Texture2D());
		m_DownFilterRT.m_RenderTarget[i].reset(new my::Texture2D());
	}
	m_OffscreenPositionRT.reset(new my::Surface());
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

	m_NormalRT->OnDestroyDevice();
	m_NormalRT->CreateTexture(
		m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	m_SpecularRT->OnDestroyDevice();
	m_SpecularRT->CreateTexture(
		m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	m_PositionRT->OnDestroyDevice();
	m_PositionRT->CreateTexture(
		m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	m_LightRT->OnDestroyDevice();
	m_LightRT->CreateTexture(
		m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);

	for (unsigned int i = 0; i < RenderPipeline::RTChain::RTArray::static_size; i++)
	{
		m_OpaqueRT.m_RenderTarget[i]->OnDestroyDevice();
		m_OpaqueRT.m_RenderTarget[i]->CreateTexture(
			m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);

		m_DownFilterRT.m_RenderTarget[i]->OnDestroyDevice();
		m_DownFilterRT.m_RenderTarget[i]->CreateTexture(
			m_SwapChainBufferDesc.Width / 4, m_SwapChainBufferDesc.Height / 4, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);
	}

	m_OffscreenPositionRT->OnDestroyDevice();
	m_OffscreenPositionRT->CreateOffscreenPlainSurface(
		m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height, D3DFMT_A32B32G32R32F, D3DPOOL_SYSTEMMEM);

	return TRUE;
}

#define DUCOLOR_TO_D3DCOLOR(col) ((col & 0xff00ff00) | (col & 0x00ff0000) >> 16 | (col & 0x000000ff) << 16)

void CChildView::QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);

	struct Callback : public my::OctNode::QueryCallback
		, public duDebugDraw
	{
		const my::Frustum & frustum;
		RenderPipeline * pipeline;
		unsigned int PassMask;
		const my::Vector3 & ViewPos;
		const my::Vector3 & TargetPos;
		CMainFrame * pFrame;
		CChildView * pView;
		CMainFrame::ViewedActorSet::iterator insert_actor_iter;
		DWORD m_duDebugDrawPrimitives;

		Callback(const my::Frustum& _frustum, RenderPipeline* _pipeline, unsigned int _PassMask, const my::Vector3& _ViewPos, const my::Vector3& _TargetPos, CMainFrame* _pFrame, CChildView* _pView)
			: frustum(_frustum)
			, pipeline(_pipeline)
			, PassMask(_PassMask)
			, ViewPos(_ViewPos)
			, TargetPos(_TargetPos)
			, pFrame(_pFrame)
			, pView(_pView)
			, insert_actor_iter(pFrame->m_ViewedActors.begin())
			, m_duDebugDrawPrimitives(DU_DRAW_QUADS + 1)
		{
		}

		void depthMask(bool state)
		{
		}

		void texture(bool state)
		{
		}

		void begin(duDebugDrawPrimitives prim, float size)
		{
			m_duDebugDrawPrimitives = prim;
		}

		void vertex(const float* pos, unsigned int color)
		{
			if (m_duDebugDrawPrimitives == DU_DRAW_LINES)
			{
				pView->PushLineVertex(pos[0], pos[1], pos[2], DUCOLOR_TO_D3DCOLOR(color));
			}
			else if (m_duDebugDrawPrimitives == DU_DRAW_TRIS)
			{
				pView->PushTriangleVertex(pos[0], pos[1], pos[2], DUCOLOR_TO_D3DCOLOR(color));
			}
		}

		void vertex(const float x, const float y, const float z, unsigned int color)
		{
			if (m_duDebugDrawPrimitives == DU_DRAW_LINES)
			{
				pView->PushLineVertex(x, y, z, DUCOLOR_TO_D3DCOLOR(color));
			}
			else if (m_duDebugDrawPrimitives == DU_DRAW_TRIS)
			{
				pView->PushTriangleVertex(x, y, z, DUCOLOR_TO_D3DCOLOR(color));
			}
		}

		void vertex(const float* pos, unsigned int color, const float* uv)
		{
			if (m_duDebugDrawPrimitives == DU_DRAW_LINES)
			{
				pView->PushLineVertex(pos[0], pos[1], pos[2], DUCOLOR_TO_D3DCOLOR(color));
			}
			else if (m_duDebugDrawPrimitives == DU_DRAW_TRIS)
			{
				pView->PushTriangleVertex(pos[0], pos[1], pos[2], DUCOLOR_TO_D3DCOLOR(color));
			}
		}

		void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
		{
			if (m_duDebugDrawPrimitives == DU_DRAW_LINES)
			{
				pView->PushLineVertex(x, y, z, DUCOLOR_TO_D3DCOLOR(color));
			}
			else if (m_duDebugDrawPrimitives == DU_DRAW_TRIS)
			{
				pView->PushTriangleVertex(x, y, z, DUCOLOR_TO_D3DCOLOR(color));
			}
		}

		void end()
		{
			m_duDebugDrawPrimitives = DU_DRAW_QUADS + 1;
		}

		unsigned int areaToCol(unsigned int area)
		{
			return duDebugDraw::areaToCol(area);
		}

		virtual bool OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
		{
			ASSERT(dynamic_cast<Actor *>(oct_entity));

			Actor * actor = static_cast<Actor *>(oct_entity);

			if (pFrame->GetActiveView() == pView && (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal)))
			{
				if ((actor->m_OctAabb->Center() - TargetPos).magnitudeSq() > actor->m_CullingDistSq) // ! derestrict update sequence of attached actors
				{
					return true;
				}

				if (actor->is_linked())
				{
					CMainFrame::ViewedActorSet::iterator actor_iter = pFrame->m_ViewedActors.iterator_to(*actor);
					if (actor_iter != insert_actor_iter)
					{
						pFrame->m_ViewedActors.splice(insert_actor_iter, pFrame->m_ViewedActors, actor_iter);
					}
					else
					{
						ASSERT(insert_actor_iter != pFrame->m_ViewedActors.end());

						insert_actor_iter++;
					}
				}
				else
				{
					ASSERT(!actor->IsRequested());

					actor->RequestResource();

					pFrame->m_ViewedActors.insert(insert_actor_iter, *actor);
				}

				actor->UpdateLod(ViewPos, TargetPos);
			}

			if (actor->IsRequested())
			{
				actor->AddToPipeline(frustum, pipeline, PassMask, ViewPos, TargetPos);
			}

			if (pView->m_bShowNavigation && (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal)))
			{
				Actor::ComponentPtrList::const_iterator cmp_iter = actor->m_Cmps.begin();
				for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
				{
					if ((*cmp_iter)->GetComponentType() == Component::ComponentTypeNavigation)
					{
						Navigation* navi = dynamic_cast<Navigation*>(cmp_iter->get());
						navi->DebugDraw(this, frustum, ViewPos, TargetPos);
					}
				}
			}
			return true;
		}
	};

	//pFrame->m_emitter->m_Emitter->m_ParticleList.clear();
	my::ModelViewerCamera * model_view_camera = dynamic_cast<my::ModelViewerCamera *>(m_Camera.get());
	Callback cb(frustum, pipeline, PassMask, m_Camera->m_Eye, model_view_camera->m_LookAt, pFrame, this);
	pFrame->QueryEntity(frustum, &cb);
	//pFrame->m_emitter->AddToPipeline(frustum, pipeline, PassMask);

	if (pFrame->GetActiveView() == this && (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal)))
	{
		CMainFrame::ViewedActorSet::iterator actor_iter = cb.insert_actor_iter;
		for (; actor_iter != pFrame->m_ViewedActors.end(); )
		{
			ASSERT(actor_iter->IsRequested());

			actor_iter->ReleaseResource();

			actor_iter = pFrame->m_ViewedActors.erase(actor_iter);
		}
	}
}

void CChildView::RenderSelectedActor(IDirect3DDevice9 * pd3dDevice, Actor * actor, D3DCOLOR color)
{
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	PushLineAABB(*actor->m_Node, D3DCOLOR_ARGB(255, 255, 255, 0));
	Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
	for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
	{
		if (cmp_iter->get() == pFrame->m_selcmp)
		{
			RenderSelectedComponent(pd3dDevice, cmp_iter->get(), color);
		}
	}

	my::Vector3 pt = m_Camera->WorldToScreen(actor->m_World.getRow<3>().xyz, my::Vector2((float)m_SwapChainBufferDesc.Width, (float)m_SwapChainBufferDesc.Height));
	if (pt.z > 0.0f && pt.z < 1.0f)
	{
		theApp.m_UIRender->PushString(my::Rectangle::RightBottom(floorf(pt.x), floorf(pt.y), 1, 1), ms2ts(actor->GetName()).c_str(), D3DCOLOR_ARGB(255, 255, 255, 0), my::Font::AlignRightBottom, theApp.m_Font.get());
	}

	Actor::ActorList::iterator att_iter = actor->m_Attaches.begin();
	for (; att_iter != actor->m_Attaches.end(); att_iter++)
	{
		RenderSelectedActor(pd3dDevice, *att_iter, color);
	}
}

void CChildView::RenderSelectedComponent(IDirect3DDevice9 * pd3dDevice, Component * cmp, D3DCOLOR color)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	switch (cmp->GetComponentType())
	{
	case Component::ComponentTypeMesh:
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(cmp);
			if (mesh_cmp->m_Mesh)
			{
				D3DXMACRO macro[3] = { {0} };
				Animator* animator = cmp->m_Actor->GetFirstComponent<Animator>();
				int j = 0;
				if (animator && mesh_cmp->m_Mesh->m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
				{
					macro[j++].Name = "SKELETON";
				}
				my::Effect* shader = theApp.QueryShader(RenderPipeline::MeshTypeMesh, macro, "shader/mtl_simplecolor.fx", RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeOpaque));
				if (shader)
				{
					shader->SetMatrix(shader->GetParameterByName(NULL, "g_World"), mesh_cmp->m_Actor->m_World);
					shader->SetVector(shader->GetParameterByName(NULL, "g_MeshColor"), (my::Vector4&)D3DXCOLOR(color));
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
			ASSERT(pFrame->m_selactors.size() >= 1 && pFrame->m_selcmp == cmp);
			StaticEmitter * static_emit_cmp = dynamic_cast<StaticEmitter *>(cmp);
			StaticEmitter::ChunkMap::const_iterator chunk_iter = static_emit_cmp->m_Chunks.find(std::make_pair(pFrame->m_selchunkid.x, pFrame->m_selchunkid.y));
			ASSERT(chunk_iter != static_emit_cmp->m_Chunks.end());
			PushLineAABB(chunk_iter->second.m_OctAabb->transform(cmp->m_Actor->m_World), D3DCOLOR_ARGB(255, 255, 0, 255));
			if (chunk_iter->second.m_buff && pFrame->m_selinstid < chunk_iter->second.m_buff->size())
			{
				const my::Emitter::Particle & particle = (*chunk_iter->second.m_buff)[pFrame->m_selinstid];
				my::Matrix4 p2World;
				switch (static_emit_cmp->m_EmitterSpaceType)
				{
				case EmitterComponent::SpaceTypeWorld:
					p2World = GetParticleTransform(static_emit_cmp->m_EmitterFaceType, particle, my::Matrix4::Identity(), m_Camera->m_View);
					break;
				case EmitterComponent::SpaceTypeLocal:
					p2World = GetParticleTransform(static_emit_cmp->m_EmitterFaceType, particle, static_emit_cmp->m_Actor->m_World, m_Camera->m_View);
					break;
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
					PushLine(v0, v1, color);
					PushLine(v1, v2, color);
					PushLine(v2, v0, color);
				}
				theApp.m_ParticleVb.Unlock();
				theApp.m_ParticleIb.Unlock();
			}
		}
		break;

	case Component::ComponentTypeTerrain:
		{
			ASSERT(pFrame->m_selactors.size() >= 1 && pFrame->m_selcmp == cmp);
			Terrain * terrain = dynamic_cast<Terrain *>(cmp);
			PushLineAABB(terrain->m_Chunks[pFrame->m_selchunkid.x][pFrame->m_selchunkid.y].m_OctAabb->transform(terrain->m_Actor->m_World), D3DCOLOR_ARGB(255, 255, 0, 255));
		}
		break;
	}
}

void CChildView::RenderSelectedControl(IDirect3DDevice9 * pd3dDevice, my::Control * ctl, D3DCOLOR color, bool subhandle)
{
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);

	my::Dialog * dlg = ctl->GetTopControl();
	ASSERT(dlg);

	Vertex v[] = {
		{ctl->m_Rect.l + (subhandle ? ctl_handle_size : 0), ctl->m_Rect.t, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleCenterTop ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.r - (subhandle ? ctl_handle_size : 0), ctl->m_Rect.t, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleCenterTop ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.r, ctl->m_Rect.t + (subhandle ? ctl_handle_size : 0), 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleRightMiddle ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.r, ctl->m_Rect.b - (subhandle ? ctl_handle_size : 0), 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleRightMiddle ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.r - (subhandle ? ctl_handle_size : 0), ctl->m_Rect.b, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleCenterBottom ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.l + (subhandle ? ctl_handle_size : 0), ctl->m_Rect.b, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleCenterBottom ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.l, ctl->m_Rect.b - (subhandle ? ctl_handle_size : 0), 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleLeftMiddle ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.l, ctl->m_Rect.t + (subhandle ? ctl_handle_size : 0), 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleLeftMiddle ? D3DCOLOR_ARGB(255,255,255,0) : color},

		{ctl->m_Rect.l - ctl_handle_size, ctl->m_Rect.t - ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleLeftTop ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.l + ctl_handle_size, ctl->m_Rect.t - ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleLeftTop ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.l + ctl_handle_size, ctl->m_Rect.t - ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleLeftTop ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.l + ctl_handle_size, ctl->m_Rect.t + ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleLeftTop ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.l + ctl_handle_size, ctl->m_Rect.t + ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleLeftTop ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.l - ctl_handle_size, ctl->m_Rect.t + ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleLeftTop ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.l - ctl_handle_size, ctl->m_Rect.t + ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleLeftTop ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.l - ctl_handle_size, ctl->m_Rect.t - ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleLeftTop ? D3DCOLOR_ARGB(255,255,255,0) : color},

		{ctl->m_Rect.r - ctl_handle_size, ctl->m_Rect.t - ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleRightTop ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.r + ctl_handle_size, ctl->m_Rect.t - ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleRightTop ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.r + ctl_handle_size, ctl->m_Rect.t - ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleRightTop ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.r + ctl_handle_size, ctl->m_Rect.t + ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleRightTop ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.r + ctl_handle_size, ctl->m_Rect.t + ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleRightTop ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.r - ctl_handle_size, ctl->m_Rect.t + ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleRightTop ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.r - ctl_handle_size, ctl->m_Rect.t + ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleRightTop ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.r - ctl_handle_size, ctl->m_Rect.t - ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleRightTop ? D3DCOLOR_ARGB(255,255,255,0) : color},

		{ctl->m_Rect.r - ctl_handle_size, ctl->m_Rect.b - ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleRightBottom ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.r + ctl_handle_size, ctl->m_Rect.b - ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleRightBottom ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.r + ctl_handle_size, ctl->m_Rect.b - ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleRightBottom ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.r + ctl_handle_size, ctl->m_Rect.b + ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleRightBottom ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.r + ctl_handle_size, ctl->m_Rect.b + ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleRightBottom ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.r - ctl_handle_size, ctl->m_Rect.b + ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleRightBottom ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.r - ctl_handle_size, ctl->m_Rect.b + ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleRightBottom ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.r - ctl_handle_size, ctl->m_Rect.b - ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleRightBottom ? D3DCOLOR_ARGB(255,255,255,0) : color},

		{ctl->m_Rect.l - ctl_handle_size, ctl->m_Rect.b - ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleLeftBottom ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.l + ctl_handle_size, ctl->m_Rect.b - ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleLeftBottom ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.l + ctl_handle_size, ctl->m_Rect.b - ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleLeftBottom ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.l + ctl_handle_size, ctl->m_Rect.b + ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleLeftBottom ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.l + ctl_handle_size, ctl->m_Rect.b + ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleLeftBottom ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.l - ctl_handle_size, ctl->m_Rect.b + ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleLeftBottom ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.l - ctl_handle_size, ctl->m_Rect.b + ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleLeftBottom ? D3DCOLOR_ARGB(255,255,255,0) : color},
		{ctl->m_Rect.l - ctl_handle_size, ctl->m_Rect.b - ctl_handle_size, 0, subhandle && pFrame->m_ctlhandle == CMainFrame::ControlHandleLeftBottom ? D3DCOLOR_ARGB(255,255,255,0) : color},
	};

	V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&dlg->m_World));
	V(pd3dDevice->SetTexture(0, NULL));
	V(pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE));
	V(pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, subhandle ? _countof(v) / 2 : 4, v, sizeof(v[0])));
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

my::Matrix4 CChildView::GetParticleTransform(DWORD EmitterFaceType, const my::Emitter::Particle & particle, const my::Matrix4 & World, const my::Matrix4 & View)
{
	my::Vector3 polar = View.getColumn<2>().xyz.cartesianToPolar();
	switch (EmitterFaceType)
	{
	case EmitterComponent::FaceTypeX:
		return my::Matrix4::Compose(
			my::Vector3(particle.m_Size.x, particle.m_Size.y, particle.m_Size.x),
			my::Quaternion::RotationAxis(my::Vector3::unitZ, particle.m_Angle) * my::Quaternion::RotationAxis(my::Vector3::unitY, D3DXToRadian(90)),
			particle.m_Position.transform(World).xyz);
	case EmitterComponent::FaceTypeY:
		return my::Matrix4::Compose(
			my::Vector3(particle.m_Size.x, particle.m_Size.y, particle.m_Size.x),
			my::Quaternion::RotationAxis(my::Vector3::unitZ, particle.m_Angle) * my::Quaternion::RotationAxis(my::Vector3::unitX, D3DXToRadian(-90)),
			particle.m_Position.transform(World).xyz);
	case EmitterComponent::FaceTypeZ:
		return my::Matrix4::Compose(
			my::Vector3(particle.m_Size.x, particle.m_Size.y, particle.m_Size.x),
			my::Quaternion::RotationAxis(my::Vector3::unitZ, particle.m_Angle),
			particle.m_Position.transform(World).xyz);
	case EmitterComponent::FaceTypeCamera:
		return my::Matrix4::Compose(
			my::Vector3(particle.m_Size.x, particle.m_Size.y, particle.m_Size.x),
			my::Quaternion::RotationAxis(my::Vector3::unitZ, particle.m_Angle) * my::Quaternion::RotationAxis(my::Vector3::unitX, -polar.y) * my::Quaternion::RotationAxis(my::Vector3::unitY, D3DXToRadian(90) - polar.z),
			particle.m_Position.transform(World).xyz);
	case EmitterComponent::FaceTypeAngle:
		return my::Matrix4::Compose(
			my::Vector3(particle.m_Size.x, particle.m_Size.y, particle.m_Size.x),
			my::Quaternion::RotationAxis(my::Vector3::unitY, particle.m_Angle),
			particle.m_Position.transform(World).xyz);
	case EmitterComponent::FaceTypeAngleCamera:
		return my::Matrix4::Compose(
			my::Vector3(particle.m_Size.x, particle.m_Size.y, particle.m_Size.x),
			my::Quaternion::RotationAxis(my::Vector3::unitX, particle.m_Angle) * my::Quaternion::RotationAxis(my::Vector3::unitY, D3DXToRadian(90) - polar.z),
			particle.m_Position.transform(World).xyz);
	}
	return my::Matrix4::Identity();
}

bool CChildView::OverlapTestFrustumAndActor(const my::Frustum & frustum, Actor * actor)
{
	if (actor->m_Cmps.empty())
	{
		m_raycmp = NULL;
		my::IntersectionTests::IntersectionType intersect_type = my::IntersectionTests::IntersectAABBAndFrustum(*actor->m_OctAabb, frustum);
		if (intersect_type != my::IntersectionTests::IntersectionTypeOutside)
		{
			return true;
		}
		return false;
	}
	m_raycmp = NULL;
	my::Frustum local_ftm = frustum.transform(actor->m_World.transpose());
	Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
	for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
	{
		if (OverlapTestFrustumAndComponent(frustum, local_ftm, cmp_iter->get()))
		{
			m_raycmp = cmp_iter->get();
			return true;
		}
	}
	return false;
}

bool CChildView::OverlapTestFrustumAndComponent(const my::Frustum & frustum, const my::Frustum & local_ftm, Component * cmp)
{
	switch (cmp->GetComponentType())
	{
	case Component::ComponentTypeMesh:
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(cmp);
			if (!mesh_cmp->m_Mesh)
			{
				return false;
			}
			Animator* animator = mesh_cmp->m_Actor->GetFirstComponent<Animator>();
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
					animator->m_DualQuats.data(),
					animator->m_DualQuats.size());
				bool ret = my::Mesh::FrustumTest(local_ftm,
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					mesh_cmp->m_Mesh->LockIndexBuffer(D3DLOCK_READONLY),
					!(mesh_cmp->m_Mesh->GetOptions() & D3DXMESH_32BIT),
					mesh_cmp->m_Mesh->m_AttribTable[mesh_cmp->m_MeshSubMeshId].FaceStart,
					mesh_cmp->m_Mesh->m_AttribTable[mesh_cmp->m_MeshSubMeshId].FaceCount,
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
					mesh_cmp->m_Mesh->m_AttribTable[mesh_cmp->m_MeshSubMeshId].FaceStart,
					mesh_cmp->m_Mesh->m_AttribTable[mesh_cmp->m_MeshSubMeshId].FaceCount,
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
				virtual bool OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
				{
					StaticEmitterChunk* chunk = dynamic_cast<StaticEmitterChunk*>(oct_entity);
					if (chunk->m_buff)
					{
						if (pView->OverlapTestFrustumAndParticles(frustum, local_ftm, emitter, &(*chunk->m_buff)[0], chunk->m_buff->size()))
						{
							ret = true;
							return false;
						}
					}
					return true;
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
			Animator* animator = cloth_cmp->m_Actor->GetFirstComponent<Animator>();
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
					animator->m_DualQuats.data(),
					animator->m_DualQuats.size());
				bool ret = my::Mesh::FrustumTest(local_ftm,
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					&cloth_cmp->m_IndexData[0],
					true,
					0,
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
					0,
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
		my::Matrix4 p2World;
		switch (emitter->m_EmitterSpaceType)
		{
		case EmitterComponent::SpaceTypeWorld:
			p2World = GetParticleTransform(emitter->m_EmitterFaceType, *part_iter, my::Matrix4::Identity(), m_Camera->m_View);
			break;
		case EmitterComponent::SpaceTypeLocal:
			p2World = GetParticleTransform(emitter->m_EmitterFaceType, *part_iter, emitter->m_Actor->m_World, m_Camera->m_View);
			break;
		}
		my::Frustum particle_ftm = frustum.transform(p2World.transpose());
		bool ret = my::Mesh::FrustumTest(particle_ftm, pvb, 0, theApp.m_ParticleVertStride,
			(unsigned short*)pib + RenderPipeline::m_ParticlePrimitiveInfo[emitter->m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitiveStartIndex], true,
			0, RenderPipeline::m_ParticlePrimitiveInfo[emitter->m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitivePrimitiveCount], theApp.m_ParticleVertElems);
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
	switch (cmp->GetComponentType())
	{
	case Component::ComponentTypeMesh:
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(cmp);
			if (!mesh_cmp->m_Mesh)
			{
				return my::RayResult(false, FLT_MAX);
			}
			my::RayResult ret;
			Animator* animator = mesh_cmp->m_Actor->GetFirstComponent<Animator>();
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
					animator->m_DualQuats.data(),
					animator->m_DualQuats.size());
				ret = my::Mesh::RayTest(local_ray,
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					mesh_cmp->m_Mesh->LockIndexBuffer(D3DLOCK_READONLY),
					!(mesh_cmp->m_Mesh->GetOptions() & D3DXMESH_32BIT),
					mesh_cmp->m_Mesh->m_AttribTable[mesh_cmp->m_MeshSubMeshId].FaceStart,
					mesh_cmp->m_Mesh->m_AttribTable[mesh_cmp->m_MeshSubMeshId].FaceCount,
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
					mesh_cmp->m_Mesh->m_AttribTable[mesh_cmp->m_MeshSubMeshId].FaceStart,
					mesh_cmp->m_Mesh->m_AttribTable[mesh_cmp->m_MeshSubMeshId].FaceCount,
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
				virtual bool OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
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
					return true;
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
			Animator* animator = cloth_cmp->m_Actor->GetFirstComponent<Animator>();
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
					animator->m_DualQuats.data(),
					animator->m_DualQuats.size());
				ret = my::Mesh::RayTest(local_ray,
					&vertices[0],
					vertices.size(),
					sizeof(vertices[0]),
					&cloth_cmp->m_IndexData[0],
					true,
					0,
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
					0,
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
			Terrain * terrain = dynamic_cast<Terrain *>(cmp);
			my::ModelViewerCamera * model_view_camera = dynamic_cast<my::ModelViewerCamera *>(m_Camera.get());
			my::Vector3 LocalViewPos = model_view_camera->m_LookAt.transformCoord(terrain->m_Actor->m_World.inverse());
			my::RayResult ret = terrain->RayTest(local_ray, LocalViewPos, raychunkid);
			if (ret.first)
			{
				raychunkid = raychunkid;
				rayinstid = 0;
				return ret;
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
		my::Matrix4 p2World;
		switch (emitter->m_EmitterSpaceType)
		{
		case EmitterComponent::SpaceTypeWorld:
			p2World = GetParticleTransform(emitter->m_EmitterFaceType, *part_iter, my::Matrix4::Identity(), m_Camera->m_View);
			break;
		case EmitterComponent::SpaceTypeLocal:
			p2World = GetParticleTransform(emitter->m_EmitterFaceType, *part_iter, emitter->m_Actor->m_World, m_Camera->m_View);
			break;
		}
		my::Ray particle_ray = ray.transform(p2World.inverse());
		my::RayResult result = my::Mesh::RayTest(particle_ray, pvb, 0, theApp.m_ParticleVertStride,
			(unsigned short*)pib + RenderPipeline::m_ParticlePrimitiveInfo[emitter->m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitiveStartIndex], true,
			0, RenderPipeline::m_ParticlePrimitiveInfo[emitter->m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitivePrimitiveCount], theApp.m_ParticleVertElems);
		if (result.first)
		{
			result.second = (particle_ray.d * result.second).transformNormal(p2World).magnitude();
			if (result.second < ret.second)
			{
				ret = result;
				part_id = (int)std::distance(part_start, part_iter);
			}
		}
	}
	theApp.m_ParticleVb.Unlock();
	theApp.m_ParticleIb.Unlock();
	if (ret.first)
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

void CChildView::DrawTerrainHeightFieldHandle(Terrain* terrain)
{
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);

	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);
	my::Ray ray = m_Camera->CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height));
	my::Ray local_ray = ray.transform(terrain->m_Actor->m_World.inverse());
	my::ModelViewerCamera* model_view_camera = dynamic_cast<my::ModelViewerCamera*>(m_Camera.get());
	my::Vector3 LocalViewPos = model_view_camera->m_LookAt.transformCoord(terrain->m_Actor->m_World.inverse());
	CPoint chunkid;
	my::RayResult res = terrain->RayTest(local_ray, LocalViewPos, chunkid);
	if (res.first)
	{
		float LocalPaintRadius = pFrame->m_PaintRadius / terrain->m_Actor->m_Scale.x;
		my::Vector3 pt = local_ray.p + local_ray.d * res.second;
		my::Vector3 last_handle_pt;
		for (int i = 0; i <= 30; i++)
		{
			const my::Vector2 pos = my::Vector2(pt.x, pt.z) + my::Vector2::PolarToCartesian(LocalPaintRadius, (float)i / 30 * 2 * D3DX_PI);
			const my::Vector3 handle_pt= my::Vector3(pos, terrain->RayTest2D(pos.x, pos.y)).transformCoord(terrain->m_Actor->m_World);
			if (i > 0)
			{
				DrawHelper::PushLine(last_handle_pt, handle_pt, D3DCOLOR_ARGB(255, 255, 255, 0));
			}
			//else
			//{
			//	DrawHelper::PushLine(handle_pt, my::Vector3(handle_pt.xz(), handle_pt.y + 10), D3DCOLOR_ARGB(255, 255, 0, 0));
			//}
			last_handle_pt = handle_pt;
		}
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
			if (m_bShowGrid)
			{
				PushLineGrid(theApp.default_grid_length, theApp.default_grid_lines_every, theApp.default_grid_subdivisions,
					theApp.default_grid_color, theApp.default_grid_axis_color, my::Matrix4::RotationX(D3DXToRadian(-90)));
			}

			CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
			ASSERT_VALID(pFrame);
			pFrame->PhysxScene::PushRenderBuffer(this);
			pFrame->m_RenderingView = this;
			BOOST_SCOPE_EXIT(&pFrame)
			{
				pFrame->m_RenderingView = FALSE;
			}
			BOOST_SCOPE_EXIT_END

			if(SUCCEEDED(hr = theApp.m_d3dDevice->BeginScene()))
			{
				//m_BgColor = D3DCOLOR_ARGB(0,161,161,161);
				//V(theApp.m_d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, m_BgColor, 1.0f, 0)); // ! d3dmultisample will not work
				//V(theApp.m_d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
				//V(theApp.m_d3dDevice->SetRenderTarget(0, m_SwapChainBuffer->m_ptr));
				//V(theApp.m_d3dDevice->SetDepthStencilSurface(m_DepthStencil->m_ptr));
				my::ModelViewerCamera * model_view_camera = dynamic_cast<my::ModelViewerCamera *>(m_Camera.get());
				theApp.m_SkyLightCam->m_Eye = model_view_camera->m_LookAt;
				theApp.m_SkyLightCam->UpdateViewProj();
				D3DVIEWPORT9 vp = { 0, 0, m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height, 0.0f, 1.0f };
				theApp.OnRender(theApp.m_d3dDevice, m_SwapChainBuffer->m_ptr, m_DepthStencil->m_ptr, &vp, this, theApp.m_fAbsoluteTime, theApp.m_fElapsedTime);
				V(theApp.m_d3dDevice->GetRenderTargetData(m_PositionRT->GetSurfaceLevel(0), m_OffscreenPositionRT->m_ptr));

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
					CMainFrame::ActorList::const_iterator sel_iter = pFrame->m_selactors.begin();
					for (; sel_iter != pFrame->m_selactors.end(); sel_iter++)
					{
						RenderSelectedActor(theApp.m_d3dDevice, *sel_iter, D3DCOLOR_ARGB(255, 0, 255, 0));
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
				V(theApp.m_d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
				V(theApp.m_d3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera->m_View));
				V(theApp.m_d3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera->m_Proj));
				V(theApp.m_d3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&my::Matrix4::identity));
				DrawHelper::FlushLine(theApp.m_d3dDevice);

				V(theApp.m_d3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
				if (m_bShowCmpHandle && !pFrame->m_selactors.empty())
				{
					if (pFrame->m_PaintType == CMainFrame::PaintTypeTerrainHeightField
						|| pFrame->m_PaintType == CMainFrame::PaintTypeTerrainColor
						|| pFrame->m_PaintType == CMainFrame::PaintTypeEmitterInstance)
					{
						DrawTerrainHeightFieldHandle(pFrame->GetSelComponent<Terrain>());
					}
					else
					{
						m_PivotScale = m_Camera->CalculateViewportScaler(pFrame->m_Pivot.m_Pos) * 40.0f / m_SwapChainBufferDesc.Height;
						pFrame->m_Pivot.Draw(theApp.m_d3dDevice, m_Camera.get(), &m_SwapChainBufferDesc, m_PivotScale);
					}
				}
				V(theApp.m_d3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&my::Matrix4::identity));
				DrawHelper::FlushLine(theApp.m_d3dDevice);

				theApp.m_UIRender->Begin();
				theApp.m_UIRender->SetViewProj(m_UICamera.m_ViewProj);
				theApp.m_UIRender->SetWorld(my::Matrix4::identity);
				if (m_bShowGrid)
				{
					my::Vector3 pt = m_Camera->WorldToScreen(my::Vector3(12, 0, 0), my::Vector2((float)m_SwapChainBufferDesc.Width, (float)m_SwapChainBufferDesc.Height));
					if (pt.z > 0.0f && pt.z < 1.0f)
					{
						theApp.m_UIRender->PushString(my::Rectangle::LeftTop(floorf(pt.x), floorf(pt.y), 1, 1), L"x", D3DCOLOR_ARGB(255, 255, 255, 0), my::Font::AlignCenterMiddle, theApp.m_Font.get());
					}

					pt = m_Camera->WorldToScreen(my::Vector3(0, 0, 12), my::Vector2((float)m_SwapChainBufferDesc.Width, (float)m_SwapChainBufferDesc.Height));
					if (pt.z > 0.0f && pt.z < 1.0f)
					{
						theApp.m_UIRender->PushString(my::Rectangle::LeftTop(floorf(pt.x), floorf(pt.y), 1, 1), L"z", D3DCOLOR_ARGB(255, 255, 255, 0), my::Font::AlignCenterMiddle, theApp.m_Font.get());
					}
					theApp.m_UIRender->Flush();
				}
				pFrame->DialogMgr::Draw(theApp.m_UIRender.get(), theApp.m_fTotalTime, theApp.m_fAbsoluteElapsedTime, my::Vector2(-m_UICamera.m_View._41 * 2, m_UICamera.m_View._42 * 2));
				if (!pFrame->m_selctls.empty())
				{
					RenderSelectedControl(theApp.m_d3dDevice, pFrame->m_selctls.front(), D3DCOLOR_ARGB(255, 0, 255, 0), true);
					CMainFrame::ControlList::iterator ctrl_iter = pFrame->m_selctls.begin() + 1;
					for (; ctrl_iter != pFrame->m_selctls.end(); ctrl_iter++)
					{
						RenderSelectedControl(theApp.m_d3dDevice, *ctrl_iter, D3DCOLOR_ARGB(255, 255, 255, 255), false);
					}
				}
				theApp.m_UIRender->SetWorld(my::Matrix4::identity);
				ScrInfoMap::const_iterator info_iter = m_ScrInfo.begin();
				for (int y = 5; info_iter != m_ScrInfo.end(); info_iter++, y += theApp.m_Font->m_LineHeight)
				{
					theApp.m_UIRender->PushString(my::Rectangle::LeftTop(5, (float)y, 500, 10), &info_iter->second[0], D3DCOLOR_ARGB(255,255,255,0), my::Font::AlignLeftTop, theApp.m_Font.get());
				}
				theApp.m_UIRender->End();

				swprintf_s(&m_ScrInfo[1 + RenderPipeline::PassTypeNum][0], m_ScrInfo[1 + RenderPipeline::PassTypeNum].size(), L"PassTypeUILayer: %d", theApp.m_UIRender->m_LayerDrawCall);

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
		ASSERT(m_Camera);
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
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventSelectionChanged.connect(boost::bind(&CChildView::OnSelectionChanged, this, boost::placeholders::_1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventSelectionPlaying.connect(boost::bind(&CChildView::OnSelectionPlaying, this, boost::placeholders::_1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventPivotModeChanged.connect(boost::bind(&CChildView::OnPivotModeChanged, this, boost::placeholders::_1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventAttributeChanged.connect(boost::bind(&CChildView::OnCmpAttriChanged, this, boost::placeholders::_1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventCameraPropChanged.connect(boost::bind(&CChildView::OnCameraPropChanged, this, boost::placeholders::_1));
	return 0;
}

void CChildView::OnDestroy()
{
	CView::OnDestroy();

	// TODO: Add your message handler code here
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventSelectionChanged.disconnect(boost::bind(&CChildView::OnSelectionChanged, this, boost::placeholders::_1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventSelectionPlaying.disconnect(boost::bind(&CChildView::OnSelectionPlaying, this, boost::placeholders::_1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventPivotModeChanged.disconnect(boost::bind(&CChildView::OnPivotModeChanged, this, boost::placeholders::_1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventAttributeChanged.disconnect(boost::bind(&CChildView::OnCmpAttriChanged, this, boost::placeholders::_1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventCameraPropChanged.disconnect(boost::bind(&CChildView::OnCameraPropChanged, this, boost::placeholders::_1));
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	my::Ray ray = m_Camera->CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height));
	Terrain* terrain = pFrame->GetSelComponent<Terrain>();
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

	StaticEmitter* emit = pFrame->GetSelComponent<StaticEmitter>();
	if (terrain && emit && pFrame->m_PaintType == CMainFrame::PaintTypeEmitterInstance)
	{
		m_PaintTerrainCaptured.reset(new TerrainStream(terrain));
		m_PaintEmitterCaptured.reset(new StaticEmitterStream(emit));
		OnPaintEmitterInstance(ray, *m_PaintTerrainCaptured, *m_PaintEmitterCaptured);
		SetCapture();
		Invalidate();
		return;
	}

	if (!pFrame->m_selactors.empty() && pFrame->m_Pivot.OnLButtonDown(ray, m_PivotScale))
	{
		StartPerformanceCount();
		CMainFrame::ActorList::iterator sel_iter = pFrame->m_selactors.begin();
		for (; sel_iter != pFrame->m_selactors.end(); sel_iter++)
		{
			physx::PxRigidBody * body = NULL;
			if ((*sel_iter)->m_PxActor && (body = (*sel_iter)->m_PxActor->is<physx::PxRigidBody>()) && !body->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
			{
				body->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, true);
			}
		}
		m_bCopyActors = (nFlags & MK_SHIFT) ? TRUE : FALSE;
		SetCapture();
		Invalidate();
		return;
	}

	if (!pFrame->m_selctls.empty() && !(nFlags & MK_CONTROL))
	{
		CMainFrame::ControlList::iterator ctrl_iter = pFrame->m_selctls.begin();
		my::Ray ui_ray = m_UICamera.CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height));
		my::Dialog * dlg = (*ctrl_iter)->GetTopControl();
		my::Vector2 pt;
		if (dlg->RayToWorld(ui_ray, pt))
		{
			if (my::Rectangle(
				(*ctrl_iter)->m_Rect.l - ctl_handle_size,
				(*ctrl_iter)->m_Rect.t - ctl_handle_size,
				(*ctrl_iter)->m_Rect.l + ctl_handle_size,
				(*ctrl_iter)->m_Rect.t + ctl_handle_size).PtInRect(pt))
			{
				pFrame->m_selactors.clear();
				pFrame->m_selcmp = NULL;
				pFrame->m_selchunkid.SetPoint(0, 0);
				pFrame->m_selinstid = 0;
				pFrame->m_ctlcaptured = true;
				pFrame->m_ctlhandle = CMainFrame::ControlHandleLeftTop;
				pFrame->m_ctlhandleoff.x = pt.x - (*ctrl_iter)->m_x.offset;
				pFrame->m_ctlhandleoff.y = pt.y - (*ctrl_iter)->m_y.offset;
				pFrame->m_ctlhandlesz.x = (*ctrl_iter)->m_Width.offset + (*ctrl_iter)->m_x.offset;
				pFrame->m_ctlhandlesz.y = (*ctrl_iter)->m_Height.offset + (*ctrl_iter)->m_y.offset;
				SetCapture();
				Invalidate();
				return;
			}

			if (my::Rectangle(
				(*ctrl_iter)->m_Rect.l + ctl_handle_size,
				(*ctrl_iter)->m_Rect.t - ctl_handle_size,
				(*ctrl_iter)->m_Rect.r - ctl_handle_size,
				(*ctrl_iter)->m_Rect.t + ctl_handle_size).PtInRect(pt))
			{
				pFrame->m_selactors.clear();
				pFrame->m_selcmp = NULL;
				pFrame->m_selchunkid.SetPoint(0, 0);
				pFrame->m_selinstid = 0;
				pFrame->m_ctlcaptured = true;
				pFrame->m_ctlhandle = CMainFrame::ControlHandleCenterTop;
				pFrame->m_ctlhandleoff.y = pt.y - (*ctrl_iter)->m_y.offset;
				pFrame->m_ctlhandlesz.y = (*ctrl_iter)->m_Height.offset + (*ctrl_iter)->m_y.offset;
				SetCapture();
				Invalidate();
				return;
			}

			if (my::Rectangle(
				(*ctrl_iter)->m_Rect.r - ctl_handle_size,
				(*ctrl_iter)->m_Rect.t - ctl_handle_size,
				(*ctrl_iter)->m_Rect.r + ctl_handle_size,
				(*ctrl_iter)->m_Rect.t + ctl_handle_size).PtInRect(pt))
			{
				pFrame->m_selactors.clear();
				pFrame->m_selcmp = NULL;
				pFrame->m_selchunkid.SetPoint(0, 0);
				pFrame->m_selinstid = 0;
				pFrame->m_ctlcaptured = true;
				pFrame->m_ctlhandle = CMainFrame::ControlHandleRightTop;
				pFrame->m_ctlhandleoff.y = pt.y - (*ctrl_iter)->m_y.offset;
				pFrame->m_ctlhandlesz.x = (*ctrl_iter)->m_Width.offset - pt.x;
				pFrame->m_ctlhandlesz.y = (*ctrl_iter)->m_Height.offset + (*ctrl_iter)->m_y.offset;
				SetCapture();
				Invalidate();
				return;
			}

			if (my::Rectangle(
				(*ctrl_iter)->m_Rect.l - ctl_handle_size,
				(*ctrl_iter)->m_Rect.t + ctl_handle_size,
				(*ctrl_iter)->m_Rect.l + ctl_handle_size,
				(*ctrl_iter)->m_Rect.b - ctl_handle_size).PtInRect(pt))
			{
				pFrame->m_selactors.clear();
				pFrame->m_selcmp = NULL;
				pFrame->m_selchunkid.SetPoint(0, 0);
				pFrame->m_selinstid = 0;
				pFrame->m_ctlcaptured = true;
				pFrame->m_ctlhandle = CMainFrame::ControlHandleLeftMiddle;
				pFrame->m_ctlhandleoff.x = pt.x - (*ctrl_iter)->m_x.offset;
				pFrame->m_ctlhandlesz.x = (*ctrl_iter)->m_Width.offset + (*ctrl_iter)->m_x.offset;
				SetCapture();
				Invalidate();
				return;
			}

			if (my::Rectangle(
				(*ctrl_iter)->m_Rect.l + ctl_handle_size,
				(*ctrl_iter)->m_Rect.t + ctl_handle_size,
				(*ctrl_iter)->m_Rect.r - ctl_handle_size,
				(*ctrl_iter)->m_Rect.b - ctl_handle_size).PtInRect(pt))
			{
				my::Ray ui_ray = m_UICamera.CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height));
				my::DialogMgr::DialogList::reverse_iterator dlg_iter = pFrame->m_DlgList.rbegin();
				for (; dlg_iter != pFrame->m_DlgList.rend(); dlg_iter++)
				{
					my::Vector2 pt;
					if ((*dlg_iter)->RayToWorld(ui_ray, pt))
					{
						my::Control* ControlPtd = (*dlg_iter)->GetChildAtPoint(pt, true);
						if (ControlPtd)
						{
							if (ControlPtd != (*ctrl_iter))
								goto ctrl_handle_end;
							break;
						}
					}
				}

				pFrame->m_selactors.clear();
				pFrame->m_selcmp = NULL;
				pFrame->m_selchunkid.SetPoint(0, 0);
				pFrame->m_selinstid = 0;
				pFrame->m_ctlcaptured = true;
				pFrame->m_ctlhandle = CMainFrame::ControlHandleCenterMiddle;
				pFrame->m_ctlhandleoff.x = pt.x - (*ctrl_iter)->m_x.offset;
				pFrame->m_ctlhandleoff.y = pt.y - (*ctrl_iter)->m_y.offset;
				SetCapture();
				Invalidate();
				return;
			}

			if (my::Rectangle(
				(*ctrl_iter)->m_Rect.r - ctl_handle_size,
				(*ctrl_iter)->m_Rect.t + ctl_handle_size,
				(*ctrl_iter)->m_Rect.r + ctl_handle_size,
				(*ctrl_iter)->m_Rect.b - ctl_handle_size).PtInRect(pt))
			{
				pFrame->m_selactors.clear();
				pFrame->m_selcmp = NULL;
				pFrame->m_selchunkid.SetPoint(0, 0);
				pFrame->m_selinstid = 0;
				pFrame->m_ctlcaptured = true;
				pFrame->m_ctlhandle = CMainFrame::ControlHandleRightMiddle;
				pFrame->m_ctlhandlesz.x = (*ctrl_iter)->m_Width.offset - pt.x;
				SetCapture();
				Invalidate();
				return;
			}

			if (my::Rectangle(
				(*ctrl_iter)->m_Rect.l - ctl_handle_size,
				(*ctrl_iter)->m_Rect.b - ctl_handle_size,
				(*ctrl_iter)->m_Rect.l + ctl_handle_size,
				(*ctrl_iter)->m_Rect.b + ctl_handle_size).PtInRect(pt))
			{
				pFrame->m_selactors.clear();
				pFrame->m_selcmp = NULL;
				pFrame->m_selchunkid.SetPoint(0, 0);
				pFrame->m_selinstid = 0;
				pFrame->m_ctlcaptured = true;
				pFrame->m_ctlhandle = CMainFrame::ControlHandleLeftBottom;
				pFrame->m_ctlhandleoff.x = pt.x - (*ctrl_iter)->m_x.offset;
				pFrame->m_ctlhandlesz.x = (*ctrl_iter)->m_Width.offset + (*ctrl_iter)->m_x.offset;
				pFrame->m_ctlhandlesz.y = (*ctrl_iter)->m_Height.offset - pt.y;
				SetCapture();
				Invalidate();
				return;
			}

			if (my::Rectangle(
				(*ctrl_iter)->m_Rect.l + ctl_handle_size,
				(*ctrl_iter)->m_Rect.b - ctl_handle_size,
				(*ctrl_iter)->m_Rect.r - ctl_handle_size,
				(*ctrl_iter)->m_Rect.b + ctl_handle_size).PtInRect(pt))
			{
				pFrame->m_selactors.clear();
				pFrame->m_selcmp = NULL;
				pFrame->m_selchunkid.SetPoint(0, 0);
				pFrame->m_selinstid = 0;
				pFrame->m_ctlcaptured = true;
				pFrame->m_ctlhandle = CMainFrame::ControlHandleCenterBottom;
				pFrame->m_ctlhandlesz.y = (*ctrl_iter)->m_Height.offset - pt.y;
				SetCapture();
				Invalidate();
				return;
			}

			if (my::Rectangle(
				(*ctrl_iter)->m_Rect.r - ctl_handle_size,
				(*ctrl_iter)->m_Rect.b - ctl_handle_size,
				(*ctrl_iter)->m_Rect.r + ctl_handle_size,
				(*ctrl_iter)->m_Rect.b + ctl_handle_size).PtInRect(pt))
			{
				pFrame->m_selactors.clear();
				pFrame->m_selcmp = NULL;
				pFrame->m_selchunkid.SetPoint(0, 0);
				pFrame->m_selinstid = 0;
				pFrame->m_ctlcaptured = true;
				pFrame->m_ctlhandle = CMainFrame::ControlHandleRightBottom;
				pFrame->m_ctlhandlesz.x = (*ctrl_iter)->m_Width.offset - pt.x;
				pFrame->m_ctlhandlesz.y = (*ctrl_iter)->m_Height.offset - pt.y;
				SetCapture();
				Invalidate();
				return;
			}
		}
	}
ctrl_handle_end:

	CRectTracker tracker;
	tracker.TrackRubberBand(this, point, TRUE);
	tracker.m_rect.NormalizeRect();

	StartPerformanceCount();
	if (!(nFlags & MK_SHIFT) && !(nFlags & MK_CONTROL) && (!pFrame->m_selactors.empty() || !pFrame->m_selctls.empty()))
	{
		pFrame->m_selactors.clear();
		pFrame->m_selcmp = NULL;
		pFrame->m_selchunkid.SetPoint(0, 0);
		pFrame->m_selinstid = 0;
		pFrame->m_selctls.clear();
	}

	if (!tracker.m_rect.IsRectEmpty())
	{
		my::Rectangle rc(
			(float)tracker.m_rect.left,
			(float)tracker.m_rect.top,
			(float)tracker.m_rect.right,
			(float)tracker.m_rect.bottom);
		my::Frustum ui_ftm = m_UICamera.CalculateFrustum(rc, CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height));
		my::DialogMgr::DialogList::reverse_iterator dlg_iter = pFrame->m_DlgList.rbegin();
		for (; dlg_iter != pFrame->m_DlgList.rend(); dlg_iter++)
		{
			std::vector<my::Control*> ctrl_list;
			if ((*dlg_iter)->GetChildAtFrustum(ui_ftm, false, ctrl_list))
			{
				pFrame->m_selactors.clear();
				pFrame->m_selcmp = NULL;
				pFrame->m_selchunkid.SetPoint(0, 0);
				pFrame->m_selinstid = 0;
				if (nFlags & MK_CONTROL)
				{
					my::intersection_remove(pFrame->m_selctls, ctrl_list.begin(), ctrl_list.end());
				}
				else
				{
					my::union_insert(pFrame->m_selctls, ctrl_list.begin(), ctrl_list.end());
				}
				pFrame->m_ctlhandle = CMainFrame::ControlHandleNone;
				pFrame->OnSelChanged();
				return;
			}
		}

		my::Frustum ftm = m_Camera->CalculateFrustum(rc, CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height));
		struct Callback : public my::OctNode::QueryCallback
		{
			CMainFrame::ActorList selacts;
			Component* selcmp;
			const my::Frustum& ftm;
			CChildView* pView;
			Callback(const my::Frustum& _ftm, CChildView* _pView)
				: selcmp(NULL)
				, ftm(_ftm)
				, pView(_pView)
			{
			}
			virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
			{
				Actor* actor = dynamic_cast<Actor*>(oct_entity);
				ASSERT(actor);
				if (pView->OverlapTestFrustumAndActor(ftm, actor))
				{
					selacts.push_back(actor);
					if (!selcmp)
					{
						selcmp = pView->m_raycmp;
					}
				}
				return true;
			}
		};
		Callback cb(ftm, this);
		pFrame->QueryEntity(ftm, &cb);
		if (nFlags & MK_CONTROL)
		{
			my::intersection_remove(pFrame->m_selactors, cb.selacts.begin(), cb.selacts.end());
			CMainFrame::ActorList::iterator sel_iter = cb.selacts.begin();
			for (; sel_iter != cb.selacts.end(); sel_iter++)
			{
				if ((*sel_iter)->m_Cmps.end() != boost::find_if((*sel_iter)->m_Cmps,
					boost::bind(std::equal_to<Component*>(), pFrame->m_selcmp, boost::bind(&ComponentPtr::get, boost::placeholders::_1))))
				{
					pFrame->m_selcmp = NULL;
				}
			}
		}
		else
		{
			my::union_insert(pFrame->m_selactors, cb.selacts.begin(), cb.selacts.end());
			pFrame->m_selcmp = cb.selcmp;
		}
		if (!pFrame->m_selcmp && !pFrame->m_selactors.empty() && pFrame->m_selactors.front()->GetComponentNum() > 0)
		{
			pFrame->m_selcmp = pFrame->m_selactors.front()->m_Cmps.front().get();
		}
		pFrame->m_selchunkid.SetPoint(0, 0);
		pFrame->m_selinstid = 0;
		pFrame->m_selctls.clear();
	}
	else
	{
		my::Ray ui_ray = m_UICamera.CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height));
		my::DialogMgr::DialogList::reverse_iterator dlg_iter = pFrame->m_DlgList.rbegin();
		for (; dlg_iter != pFrame->m_DlgList.rend(); dlg_iter++)
		{
			my::Vector2 pt;
			if ((*dlg_iter)->RayToWorld(ui_ray, pt))
			{
				my::Control* ControlPtd = (*dlg_iter)->GetChildAtPoint(pt, false);
				if (ControlPtd)
				{
					CMainFrame::ControlList::iterator ctrl_iter = std::find(pFrame->m_selctls.begin(), pFrame->m_selctls.end(), ControlPtd);
					if (ctrl_iter != pFrame->m_selctls.end())
					{
						pFrame->m_selactors.clear();
						pFrame->m_selcmp = NULL;
						pFrame->m_selchunkid.SetPoint(0, 0);
						pFrame->m_selinstid = 0;
						pFrame->m_selctls.erase(ctrl_iter);
						pFrame->m_ctlhandle = CMainFrame::ControlHandleNone;
						pFrame->OnSelChanged();
						return;
					}
					else
					{
						pFrame->m_selactors.clear();
						pFrame->m_selcmp = NULL;
						pFrame->m_selchunkid.SetPoint(0, 0);
						pFrame->m_selinstid = 0;
						pFrame->m_selctls.insert(pFrame->m_selctls.begin(), ControlPtd);
						pFrame->m_ctlhandle = CMainFrame::ControlHandleNone;
						pFrame->OnSelChanged();
						return;
					}
				}
			}
		}

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
			virtual bool OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
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
				return true;
			}
		};
		Callback cb(ray, this);
		pFrame->QueryEntity(ray, &cb);
		if (cb.selact)
		{
			CMainFrame::ActorList::iterator sel_iter = std::find(pFrame->m_selactors.begin(), pFrame->m_selactors.end(), cb.selact);
			if (sel_iter != pFrame->m_selactors.end())
			{
				pFrame->m_selactors.erase(sel_iter);
				if (pFrame->m_selcmp && pFrame->m_selcmp->m_Actor == cb.selact)
				{
					if (!pFrame->m_selactors.empty() && pFrame->m_selactors.front()->GetComponentNum() > 0)
					{
						pFrame->m_selcmp = pFrame->m_selactors.front()->m_Cmps.front().get();
					}
					else
					{
						pFrame->m_selcmp = NULL;
					}
				}
				pFrame->m_selchunkid.SetPoint(0, 0);
				pFrame->m_selinstid = 0;
				pFrame->m_selctls.clear();
			}
			else
			{
				pFrame->m_selactors.insert(pFrame->m_selactors.begin(), cb.selact);
				pFrame->m_selcmp = cb.selcmp;
				pFrame->m_selchunkid = cb.selchunkid;
				pFrame->m_selinstid = cb.selinstid;
				pFrame->m_selctls.clear();
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
	if (m_PaintTerrainCaptured || m_PaintEmitterCaptured)
	{
		if (m_PaintTerrainCaptured)
		{
			m_PaintTerrainCaptured->Flush();
			m_PaintTerrainCaptured->m_terrain->m_Actor->UpdateAABB();
			m_PaintTerrainCaptured->m_terrain->m_Actor->UpdateOctNode();
			m_PaintTerrainCaptured.reset();
		}

		if (m_PaintEmitterCaptured)
		{
			m_PaintEmitterCaptured->Flush();
			m_PaintEmitterCaptured->m_emit->m_Actor->UpdateAABB();
			m_PaintEmitterCaptured->m_emit->m_Actor->UpdateOctNode();
			m_PaintEmitterCaptured.reset();
		}
		pFrame->UpdateSelBox();
		ReleaseCapture();
		Invalidate();
		return;
	}

	if (pFrame->m_ctlcaptured)
	{
		ASSERT(!pFrame->m_selctls.empty());
		pFrame->m_ctlcaptured = false;
		ReleaseCapture();
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		return;
	}

	if (pFrame->m_Pivot.m_Captured && pFrame->m_Pivot.OnLButtonUp(
		m_Camera->CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height))))
	{
		StartPerformanceCount();
		CMainFrame::ActorList::iterator sel_iter = pFrame->m_selactors.begin();
		for (; sel_iter != pFrame->m_selactors.end(); sel_iter++)
		{
			switch (pFrame->m_Pivot.m_Mode)
			{
			case Pivot::PivotModeMove:
				(*sel_iter)->m_Position = (*sel_iter)->m_Position + pFrame->m_Pivot.m_Pos - pFrame->m_Pivot.m_DragRot.xyz;
				break;
			case Pivot::PivotModeRot:
				(*sel_iter)->m_World.Decompose((*sel_iter)->m_Scale, (*sel_iter)->m_Rotation, (*sel_iter)->m_Position);
				break;
			}

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
			//	if ((*cmp_iter)->GetComponentType() == Component::ComponentTypeStaticEmitter
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

void CChildView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	if (!pFrame->m_selctls.empty())
	{
		OnLButtonDown(nFlags, point);
		return;
	}

	__super::OnLButtonDblClk(nFlags, point);
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

	if (m_PaintTerrainCaptured && m_PaintEmitterCaptured && pFrame->m_PaintType == CMainFrame::PaintTypeEmitterInstance)
	{
		my::Ray ray = m_Camera->CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height));
		OnPaintEmitterInstance(ray, *m_PaintTerrainCaptured, *m_PaintEmitterCaptured);
		Invalidate();
		UpdateWindow();
		return;
	}

	if (pFrame->m_ctlcaptured)
	{
		ASSERT(!pFrame->m_selctls.empty());
		CMainFrame::ControlList::iterator ctrl_iter = pFrame->m_selctls.begin();
		my::Ray ray = m_UICamera.CalculateRay(my::Vector2((float)point.x, (float)point.y), CSize(m_SwapChainBufferDesc.Width, m_SwapChainBufferDesc.Height));
		my::Vector2 pt;
		if ((*ctrl_iter)->RayToWorld(ray, pt))
		{
			switch (pFrame->m_ctlhandle)
			{
			case CMainFrame::ControlHandleLeftTop:
				(*ctrl_iter)->m_x.offset = pt.x - pFrame->m_ctlhandleoff.x;
				(*ctrl_iter)->m_y.offset = pt.y - pFrame->m_ctlhandleoff.y;
				ALIGN_TO_GRID((*ctrl_iter)->m_x.offset);
				ALIGN_TO_GRID((*ctrl_iter)->m_y.offset);
				(*ctrl_iter)->m_Width.offset = pFrame->m_ctlhandlesz.x - (*ctrl_iter)->m_x.offset;
				(*ctrl_iter)->m_Height.offset = pFrame->m_ctlhandlesz.y - (*ctrl_iter)->m_y.offset;
				(*ctrl_iter)->OnLayout();
				break;
			case CMainFrame::ControlHandleCenterTop:
				(*ctrl_iter)->m_y.offset = pt.y - pFrame->m_ctlhandleoff.y;
				ALIGN_TO_GRID((*ctrl_iter)->m_y.offset);
				(*ctrl_iter)->m_Height.offset = pFrame->m_ctlhandlesz.y - (*ctrl_iter)->m_y.offset;
				(*ctrl_iter)->OnLayout();
				break;
			case CMainFrame::ControlHandleRightTop:
				(*ctrl_iter)->m_y.offset = pt.y - pFrame->m_ctlhandleoff.y;
				(*ctrl_iter)->m_Width.offset = pFrame->m_ctlhandlesz.x + pt.x;
				ALIGN_TO_GRID((*ctrl_iter)->m_y.offset);
				ALIGN_TO_GRID((*ctrl_iter)->m_Width.offset);
				(*ctrl_iter)->m_Height.offset = pFrame->m_ctlhandlesz.y - (*ctrl_iter)->m_y.offset;
				(*ctrl_iter)->OnLayout();
				break;
			case CMainFrame::ControlHandleLeftMiddle:
				(*ctrl_iter)->m_x.offset = pt.x - pFrame->m_ctlhandleoff.x;
				ALIGN_TO_GRID((*ctrl_iter)->m_x.offset);
				(*ctrl_iter)->m_Width.offset = pFrame->m_ctlhandlesz.x - (*ctrl_iter)->m_x.offset;
				(*ctrl_iter)->OnLayout();
				break;
			case CMainFrame::ControlHandleCenterMiddle:
				(*ctrl_iter)->m_x.offset = pt.x - pFrame->m_ctlhandleoff.x;
				(*ctrl_iter)->m_y.offset = pt.y - pFrame->m_ctlhandleoff.y;
				ALIGN_TO_GRID((*ctrl_iter)->m_x.offset);
				ALIGN_TO_GRID((*ctrl_iter)->m_y.offset);
				break;
			case CMainFrame::ControlHandleRightMiddle:
				(*ctrl_iter)->m_Width.offset = pFrame->m_ctlhandlesz.x + pt.x;
				ALIGN_TO_GRID((*ctrl_iter)->m_Width.offset);
				(*ctrl_iter)->OnLayout();
				break;
			case CMainFrame::ControlHandleLeftBottom:
				(*ctrl_iter)->m_x.offset = pt.x - pFrame->m_ctlhandleoff.x;
				(*ctrl_iter)->m_Height.offset = pFrame->m_ctlhandlesz.y + pt.y;
				ALIGN_TO_GRID((*ctrl_iter)->m_x.offset);
				ALIGN_TO_GRID((*ctrl_iter)->m_Height.offset);
				(*ctrl_iter)->m_Width.offset = pFrame->m_ctlhandlesz.x - (*ctrl_iter)->m_x.offset;
				(*ctrl_iter)->OnLayout();
				break;
			case CMainFrame::ControlHandleCenterBottom:
				(*ctrl_iter)->m_Height.offset = pFrame->m_ctlhandlesz.y + pt.y;
				ALIGN_TO_GRID((*ctrl_iter)->m_Height.offset);
				(*ctrl_iter)->OnLayout();
				break;
			case CMainFrame::ControlHandleRightBottom:
				(*ctrl_iter)->m_Width.offset = pFrame->m_ctlhandlesz.x + pt.x;
				(*ctrl_iter)->m_Height.offset = pFrame->m_ctlhandlesz.y + pt.y;
				ALIGN_TO_GRID((*ctrl_iter)->m_Width.offset);
				ALIGN_TO_GRID((*ctrl_iter)->m_Height.offset);
				(*ctrl_iter)->OnLayout();
				break;
			}
		}
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
			CMainFrame::ActorList new_selactors;
			CMainFrame::ActorList::const_iterator sel_iter = pFrame->m_selactors.begin();
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
			pFrame->m_selctls.clear();
		}
		StartPerformanceCount();
		CMainFrame::ActorList::iterator sel_iter = pFrame->m_selactors.begin();
		for (; sel_iter != pFrame->m_selactors.end(); sel_iter++)
		{
			my::Matrix4 trans = my::Matrix4::Identity();
			switch (pFrame->m_Pivot.m_Mode)
			{
			case Pivot::PivotModeMove:
				(*sel_iter)->m_World = my::Matrix4::Compose((*sel_iter)->m_Scale, (*sel_iter)->m_Rotation, (*sel_iter)->m_Position + pFrame->m_Pivot.m_Pos - pFrame->m_Pivot.m_DragRot.xyz);
				break;
			case Pivot::PivotModeRot:
				trans = my::Matrix4::AffineTransformation(1,
					pFrame->m_Pivot.m_Pos, pFrame->m_Pivot.m_Rot.inverse() * pFrame->m_Pivot.m_DragDeltaRot * pFrame->m_Pivot.m_Rot, my::Vector3(0, 0, 0));
				(*sel_iter)->m_World *= trans;
				break;
			}

			my::Vector3 Pos, Scale; my::Quaternion Rot;
			(*sel_iter)->m_World.Decompose(Scale, Rot, Pos);
			(*sel_iter)->SetPxPoseOrbyPxThread(Pos, Rot, NULL);
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
		if (pFrame->GetSelComponent<Terrain>() && pFrame->m_PaintType != CMainFrame::PaintTypeTerrainHeightField)
		{
			pFrame->OnCmdMsg(ID_PAINT_TERRAINHEIGHTFIELD, 0, NULL, NULL);
		}
		return;
	case 'Y':
		if (pFrame->GetSelComponent<Terrain>() && pFrame->m_PaintType != CMainFrame::PaintTypeTerrainColor)
		{
			pFrame->OnCmdMsg(ID_PAINT_TERRAINCOLOR, 0, NULL, NULL);
		}
		return;
	case 'U':
		if (pFrame->GetSelComponent<StaticEmitter>() && pFrame->m_PaintType != CMainFrame::PaintTypeEmitterInstance)
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
						if ((*cmp_iter)->GetComponentType() == Component::ComponentTypeTerrain && cmp_iter->get() == pFrame->m_selcmp)
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
						else if ((*cmp_iter)->GetComponentType() == Component::ComponentTypeStaticEmitter && cmp_iter->get() == pFrame->m_selcmp)
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
		if (!pFrame->m_selactors.empty() || !pFrame->m_selctls.empty())
		{
			pFrame->SendMessage(WM_COMMAND, ID_EDIT_DELETE);
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

void CChildView::OnRendermodeBloom()
{
	// TODO: Add your command handler code here
	m_BloomEnable = !m_BloomEnable;
	Invalidate();
}

void CChildView::OnUpdateRendermodeBloom(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_BloomEnable);
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
	CPoint raychunkid;
	my::Ray local_ray = ray.transform(tstr.m_terrain->m_Actor->m_World.inverse());
	my::ModelViewerCamera* model_view_camera = dynamic_cast<my::ModelViewerCamera*>(m_Camera.get());
	my::Vector3 LocalViewPos = model_view_camera->m_LookAt.transformCoord(tstr.m_terrain->m_Actor->m_World.inverse());
	my::RayResult res = tstr.m_terrain->RayTest(local_ray, LocalViewPos, raychunkid);
	if (res.first)
	{
		float LocalPaintRadius = pFrame->m_PaintRadius / tstr.m_terrain->m_Actor->m_Scale.x;
		my::Vector3 pt = local_ray.p + local_ray.d * res.second;
		for (int i = my::Max(0, (int)roundf(pt.z - LocalPaintRadius));
			i <= my::Min(tstr.m_terrain->m_RowChunks * tstr.m_terrain->m_ChunkSize, (int)roundf(pt.z + LocalPaintRadius)); i++)
		{
			for (int j = my::Max(0, (int)roundf(pt.x - LocalPaintRadius));
				j <= my::Min(tstr.m_terrain->m_ColChunks * tstr.m_terrain->m_ChunkSize, (int)roundf(pt.x + LocalPaintRadius)); j++)
			{
				if (pFrame->m_PaintShape == CMainFrame::PaintShapeCircle)
				{
					my::Vector2 offset(j - pt.x, i - pt.z);
					float dist = offset.magnitude() / LocalPaintRadius;
					if (dist <= 1)
					{
						float LocalHeight = pFrame->m_PaintHeight * pFrame->m_PaintSpline.Interpolate(1 - dist, 1) / tstr.m_terrain->m_Actor->m_Scale.y;
						switch (pFrame->m_PaintMode)
						{
						case CMainFrame::PaintModeGreater:
							LocalHeight = my::Max(LocalHeight, tstr.GetPos(i, j).y);
							break;
						}
						tstr.SetPos(i, j, my::Vector3(j, LocalHeight, i));
					}
				}
			}
		}
		tstr.UpdateNormal();
	}
}


void CChildView::OnPaintTerrainColor(const my::Ray& ray, TerrainStream& tstr)
{
	// TODO: Add your implementation code here.
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CPoint raychunkid;
	my::Ray local_ray = ray.transform(tstr.m_terrain->m_Actor->m_World.inverse());
	my::ModelViewerCamera* model_view_camera = dynamic_cast<my::ModelViewerCamera*>(m_Camera.get());
	my::Vector3 LocalViewPos = model_view_camera->m_LookAt.transformCoord(tstr.m_terrain->m_Actor->m_World.inverse());
	my::RayResult res = tstr.m_terrain->RayTest(local_ray, LocalViewPos, raychunkid);
	if (res.first)
	{
		float LocalPaintRadius = pFrame->m_PaintRadius / tstr.m_terrain->m_Actor->m_Scale.x;
		my::Vector3 pt = local_ray.p + local_ray.d * res.second;
		for (int i = my::Max(0, (int)roundf(pt.z - LocalPaintRadius));
			i <= my::Min(tstr.m_terrain->m_RowChunks * tstr.m_terrain->m_ChunkSize, (int)roundf(pt.z + LocalPaintRadius)); i++)
		{
			for (int j = my::Max(0, (int)roundf(pt.x - LocalPaintRadius));
				j <= my::Min(tstr.m_terrain->m_ColChunks * tstr.m_terrain->m_ChunkSize, (int)roundf(pt.x + LocalPaintRadius)); j++)
			{
				if (pFrame->m_PaintShape == CMainFrame::PaintShapeCircle)
				{
					my::Vector2 offset(j - pt.x, i - pt.z);
					float dist = offset.magnitude() / LocalPaintRadius;
					if (dist <= 1)
					{
						D3DXCOLOR color;
						D3DXColorLerp(&color, &D3DXCOLOR(tstr.GetColor(i, j)), &pFrame->m_PaintColor, pFrame->m_PaintSpline.Interpolate(1 - dist, 1));
						tstr.SetColor(i, j, color);
					}
				}
			}
		}
	}
}

void CChildView::OnPaintEmitterInstance(const my::Ray& ray, TerrainStream& tstr, StaticEmitterStream& estr)
{
	// TODO: Add your implementation code here.
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CPoint raychunkid;
	my::Ray local_ray = ray.transform(tstr.m_terrain->m_Actor->m_World.inverse());
	my::ModelViewerCamera* model_view_camera = dynamic_cast<my::ModelViewerCamera*>(m_Camera.get());
	my::Vector3 LocalViewPos = model_view_camera->m_LookAt.transformCoord(tstr.m_terrain->m_Actor->m_World.inverse());
	my::RayResult res = tstr.m_terrain->RayTest(local_ray, LocalViewPos, raychunkid);
	if (res.first)
	{
		float LocalPaintRadius = pFrame->m_PaintRadius / tstr.m_terrain->m_Actor->m_Scale.x;
		float LocalPaintParticleMinDist = pFrame->m_PaintParticleMinDist / tstr.m_terrain->m_Actor->m_Scale.x;
		my::Vector3 pt = local_ray.p + local_ray.d * res.second;
		std::list<my::Vector2> candidate;
		my::Emitter::Particle* particle = estr.GetFirstNearParticle2D(pt, LocalPaintParticleMinDist);
		if (!particle)
		{
			estr.Spawn(my::Vector4(pt.x, tstr.RayTest2D(pt.x, pt.z), pt.z, 1.0f), my::Vector4(0, 0, 0, 1), my::Vector4(1, 1, 1, 1), my::Vector2(1, 1), 0.0f, 0.0f);
			candidate.push_back(my::Vector2(pt.x, pt.z));
		}
		else
		{
			candidate.push_back(my::Vector2(particle->m_Position.x, particle->m_Position.z));
		}

		for (; !candidate.empty(); candidate.pop_front())
		{
			const my::Vector2& pos = candidate.front();
			for (int i = 0; i < 30; i++)
			{
				my::Vector2 rand_pos = pos + my::Vector2::RandomUnit() * my::Random(1.0f, 2.0f) * LocalPaintParticleMinDist;
				if (rand_pos.x >= estr.m_emit->m_min.x && rand_pos.x < estr.m_emit->m_max.x
					&& rand_pos.y >= estr.m_emit->m_min.z && rand_pos.y < estr.m_emit->m_max.z
					&& (rand_pos - my::Vector2(pt.x, pt.z)).magnitudeSq() < LocalPaintRadius * LocalPaintRadius)
				{
					if (!estr.GetFirstNearParticle2D(my::Vector3(rand_pos.x, 0, rand_pos.y), LocalPaintParticleMinDist))
					{
						my::Vector3 emit_pos(rand_pos, tstr.RayTest2D(rand_pos.x, rand_pos.y));
						if (estr.m_emit->Intersect(emit_pos))
						{
							estr.Spawn(my::Vector4(emit_pos, 1.0f), my::Vector4(0, 0, 0, 1), my::Vector4(1, 1, 1, 1), my::Vector2(1, 1), 0.0f, 0.0f);
							candidate.push_back(rand_pos);
						}
					}
				}
			}
		}
	}
}


void CChildView::OnRendertargetNormal()
{
	// TODO: Add your command handler code here
	m_RTType = RenderPipeline::RenderTargetNormal;
	Invalidate();
}


void CChildView::OnUpdateRendertargetNormal(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(RenderPipeline::RenderTargetNormal == m_RTType);
}


void CChildView::OnRendertargetPosition()
{
	// TODO: Add your command handler code here
	m_RTType = RenderPipeline::RenderTargetPosition;
	Invalidate();
}


void CChildView::OnUpdateRendertargetPosition(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(RenderPipeline::RenderTargetPosition == m_RTType);
}


void CChildView::OnRendertargetLight()
{
	// TODO: Add your command handler code here
	m_RTType = RenderPipeline::RenderTargetLight;
	Invalidate();
}


void CChildView::OnUpdateRendertargetLight(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(RenderPipeline::RenderTargetLight == m_RTType);
}


void CChildView::OnRendertargetOpaque()
{
	// TODO: Add your command handler code here
	m_RTType = RenderPipeline::RenderTargetOpaque;
	Invalidate();
}


void CChildView::OnUpdateRendertargetOpaque(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(RenderPipeline::RenderTargetOpaque == m_RTType);
}


void CChildView::OnRendertargetDownfilter()
{
	// TODO: Add your command handler code here
	m_RTType = RenderPipeline::RenderTargetDownFilter;
	Invalidate();
}


void CChildView::OnUpdateRendertargetDownfilter(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(RenderPipeline::RenderTargetDownFilter == m_RTType);
}


void CChildView::OnRendertargetSpecular()
{
	// TODO: Add your command handler code here
	m_RTType = RenderPipeline::RenderTargetSpecular;
	Invalidate();
}


void CChildView::OnUpdateRendertargetSpecular(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(RenderPipeline::RenderTargetSpecular == m_RTType);
}
