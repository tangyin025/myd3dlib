#include "stdafx.h"
#include "RenderPipeline.h"

using namespace my;
RenderPipeline::IRenderContext::IRenderContext(void)
	: m_BkColor(D3DCOLOR_ARGB(255,45,50,170))
	, m_SkyLightDiffuse(1.0f,1.0f,1.0f,1.0f)
	, m_SkyLightAmbient(0.3f,0.3f,0.3f,0.0f)
	, m_WireFrame(false)
	, m_DofEnable(false)
	, m_DofParams(5.0f,15.0f,25.0f,1.0f)
	, m_FxaaEnable(false)
{
}

RenderPipeline::RenderPipeline(void)
	: m_ParticleVertexStride(0)
	, m_ParticleInstanceStride(0)
	, m_MeshInstanceStride(0)
	, SHADOW_MAP_SIZE(1024)
	, SHADOW_EPSILON(0.001f)
	, m_ShadowRT(new Texture2D())
	, m_ShadowDS(new Surface())
{
}

RenderPipeline::~RenderPipeline(void)
{
}

const char * RenderPipeline::PassTypeToStr(unsigned int pass_type)
{
	switch (pass_type)
	{
	case PassTypeShadow: return "PassTypeShadow";
	case PassTypeNormal: return "PassTypeNormal";
	case PassTypeLight: return "PassTypeLight";
	case PassTypeOpaque: return "PassTypeOpaque";
	case PassTypeTransparent: return "PassTypeTransparent";
	}
	return "unknown pass type";
}

HRESULT RenderPipeline::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	m_ParticleVertexElems.InsertTexcoordElement(0);

	m_ParticleInstanceElems.InsertPositionElement(0);
	WORD offset = sizeof(Vector3);
	m_ParticleInstanceElems.InsertNormalElement(offset);
	offset += sizeof(Vector3);
	m_ParticleInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 1);
	offset += sizeof(Vector4);
	m_ParticleInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2);
	offset += sizeof(Vector4);

	m_ParticleVEList = m_ParticleVertexElems.BuildVertexElementList(0);
	std::vector<D3DVERTEXELEMENT9> IEList = m_ParticleInstanceElems.BuildVertexElementList(1);
	m_ParticleVEList.insert(m_ParticleVEList.end(), IEList.begin(), IEList.end());
	D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
	m_ParticleVEList.push_back(ve_end);

	m_ParticleVertexStride = D3DXGetDeclVertexSize(&m_ParticleVEList[0], 0);
	m_ParticleInstanceStride = D3DXGetDeclVertexSize(&m_ParticleVEList[0], 1);

	HRESULT hr;
	if(FAILED(hr = pd3dDevice->CreateVertexDeclaration(&m_ParticleVEList[0], &m_ParticleDecl)))
	{
		THROW_D3DEXCEPTION(hr);
	}

	offset = 0;
	m_MeshInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 1);
	offset += sizeof(Vector4);
	m_MeshInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 2);
	offset += sizeof(Vector4);
	m_MeshInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 3);
	offset += sizeof(Vector4);
	m_MeshInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 4);
	offset += sizeof(Vector4);

	m_MeshIEList = m_MeshInstanceElems.BuildVertexElementList(1);
	m_MeshInstanceStride = offset;

	if (!(m_SimpleSample = my::ResourceMgr::getSingleton().LoadEffect("shader/SimpleSample.fx", "")))
	{
		THROW_CUSEXCEPTION("create m_SimpleSample failed");
	}

	if (!(m_DofEffect = my::ResourceMgr::getSingleton().LoadEffect("shader/DofEffect.fx", "")))
	{
		THROW_CUSEXCEPTION("create m_DofEffect failed");
	}

	if (!(m_FxaaEffect = my::ResourceMgr::getSingleton().LoadEffect("shader/FXAA.fx", "")))
	{
		THROW_CUSEXCEPTION("create m_FxaaEffect failed");
	}
	return S_OK;
}

HRESULT RenderPipeline::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	_ASSERT(!m_ParticleVertexBuffer.m_ptr);
	m_ParticleVertexBuffer.CreateVertexBuffer(pd3dDevice, m_ParticleVertexStride * 4, 0, 0, D3DPOOL_DEFAULT);
	unsigned char * pVertices = (unsigned char *)m_ParticleVertexBuffer.Lock(0, m_ParticleVertexStride * 4);
	m_ParticleVertexElems.SetTexcoord(pVertices + m_ParticleVertexStride * 0, Vector2(0,0));
	m_ParticleVertexElems.SetTexcoord(pVertices + m_ParticleVertexStride * 1, Vector2(0,1));
	m_ParticleVertexElems.SetTexcoord(pVertices + m_ParticleVertexStride * 2, Vector2(1,1));
	m_ParticleVertexElems.SetTexcoord(pVertices + m_ParticleVertexStride * 3, Vector2(1,0));
	m_ParticleVertexBuffer.Unlock();

	_ASSERT(!m_ParticleIndexBuffer.m_ptr);
	m_ParticleIndexBuffer.CreateIndexBuffer(pd3dDevice, sizeof(WORD) * 4, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT);
	WORD * pIndices = (WORD *)m_ParticleIndexBuffer.Lock(0, sizeof(WORD) * 4);
	pIndices[0] = 0;
	pIndices[1] = 1;
	pIndices[2] = 2;
	pIndices[3] = 3;
	m_ParticleIndexBuffer.Unlock();

	_ASSERT(!m_ParticleInstanceData.m_ptr);
	m_ParticleInstanceData.CreateVertexBuffer(pd3dDevice, m_ParticleInstanceStride * PARTICLE_INSTANCE_MAX, D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT);

	_ASSERT(!m_MeshInstanceData.m_ptr);
	m_MeshInstanceData.CreateVertexBuffer(pd3dDevice, m_MeshInstanceStride * MESH_INSTANCE_MAX, D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT);

	m_ShadowRT->CreateAdjustedTexture(
		pd3dDevice, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT);

	// ! 所有的 render target必须使用具有相同 multisample的 depth stencil
	m_ShadowDS->CreateDepthStencilSurface(
		pd3dDevice, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, D3DFMT_D24X8);

	return S_OK;
}

void RenderPipeline::OnLostDevice(void)
{
	m_ParticleVertexBuffer.OnDestroyDevice();
	m_ParticleIndexBuffer.OnDestroyDevice();
	m_ParticleInstanceData.OnDestroyDevice();
	m_MeshInstanceData.OnDestroyDevice();
	m_ShadowRT->OnDestroyDevice();
	m_ShadowDS->OnDestroyDevice();
}

void RenderPipeline::OnDestroyDevice(void)
{
	_ASSERT(!m_ParticleVertexBuffer.m_ptr);
	_ASSERT(!m_ParticleIndexBuffer.m_ptr);
	_ASSERT(!m_ParticleInstanceData.m_ptr);
	_ASSERT(!m_MeshInstanceData.m_ptr);

	ClearAllObjects();

	unsigned int PassID = 0;
	for (; PassID < m_Pass.size(); PassID++)
	{
		//MeshInstanceAtomMap::iterator mesh_inst_iter = m_MeshInstanceMap.begin();
		//for (; mesh_inst_iter != m_MeshInstanceMap.end(); mesh_inst_iter++)
		//{
		//	mesh_inst_iter->second.m_Decl.Release();
		//}
		m_Pass[PassID].m_MeshInstanceMap.clear();
	}

	m_ParticleDecl.Release();
}

void RenderPipeline::OnFrameRender(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
	IRenderContext * pRC,
	double fTime,
	float fElapsedTime)
{
	UpdateQuads(pBackBufferSurfaceDesc);

	HRESULT hr;
	CComPtr<IDirect3DSurface9> ScreenSurf;
	CComPtr<IDirect3DSurface9> ScreenDepthStencilSurf;
	V(pd3dDevice->GetRenderTarget(0, &ScreenSurf));
	V(pd3dDevice->GetDepthStencilSurface(&ScreenDepthStencilSurf));

	// ! Ogre & Apex模型都是顺时针，右手系应该是逆时针
	V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
	V(pd3dDevice->SetRenderState(D3DRS_FILLMODE, pRC->m_WireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID));

	pRC->QueryRenderComponent(Frustum::ExtractMatrix(pRC->m_SkyLightCam->m_ViewProj), this, PassTypeToMask(PassTypeShadow));

	CComPtr<IDirect3DSurface9> ShadowSurf = m_ShadowRT->GetSurfaceLevel(0);
	m_SimpleSample->SetMatrix("g_View", pRC->m_SkyLightCam->m_View);
	m_SimpleSample->SetMatrix("g_ViewProj", pRC->m_SkyLightCam->m_ViewProj);
	V(pd3dDevice->SetRenderTarget(0, ShadowSurf));
	V(pd3dDevice->SetDepthStencilSurface(m_ShadowDS->m_ptr));
	V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00ffffff, 1.0f, 0));
	RenderAllObjects(PassTypeShadow, pd3dDevice, fTime, fElapsedTime);
	ShadowSurf.Release();

	pRC->QueryRenderComponent(Frustum::ExtractMatrix(pRC->m_Camera->m_ViewProj), this, PassTypeToMask(PassTypeNormal) | PassTypeToMask(PassTypeLight) | PassTypeToMask(PassTypeOpaque) | PassTypeToMask(PassTypeTransparent));

	CComPtr<IDirect3DSurface9> NormalSurf = pRC->m_NormalRT->GetSurfaceLevel(0);
	CComPtr<IDirect3DSurface9> PositionSurf = pRC->m_PositionRT->GetSurfaceLevel(0);
	m_SimpleSample->SetMatrix("g_View", pRC->m_Camera->m_View);
	m_SimpleSample->SetMatrix("g_ViewProj", pRC->m_Camera->m_ViewProj);
	m_SimpleSample->SetMatrix("g_InvViewProj", pRC->m_Camera->m_InverseViewProj);
	m_SimpleSample->SetVector("g_Eye", pRC->m_Camera->m_Eye);
	m_SimpleSample->SetVector("g_SkyLightDir", -pRC->m_SkyLightCam->m_View.column<2>().xyz.normalize()); // ! RH -z
	m_SimpleSample->SetMatrix("g_SkyLightViewProj", pRC->m_SkyLightCam->m_ViewProj);
	m_SimpleSample->SetVector("g_SkyLightDiffuse", pRC->m_SkyLightDiffuse);
	m_SimpleSample->SetVector("g_SkyLightAmbient", pRC->m_SkyLightAmbient);
	m_SimpleSample->SetTexture("g_ShadowRT", m_ShadowRT.get());
	V(pd3dDevice->SetRenderTarget(0, NormalSurf));
	V(pd3dDevice->SetRenderTarget(1, PositionSurf));
	V(pd3dDevice->SetDepthStencilSurface(ScreenDepthStencilSurf));
	V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00ffffff, 1.0f, 0));
	RenderAllObjects(PassTypeNormal, pd3dDevice, fTime, fElapsedTime);
	NormalSurf.Release();
	PositionSurf.Release();

	CComPtr<IDirect3DSurface9> LightSurf = pRC->m_LightRT->GetSurfaceLevel(0);
	m_SimpleSample->SetTexture("g_NormalRT", pRC->m_NormalRT.get());
	m_SimpleSample->SetTexture("g_PositionRT", pRC->m_PositionRT.get());
	V(pd3dDevice->SetRenderTarget(0, LightSurf));
	V(pd3dDevice->SetRenderTarget(1, NULL));
	V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0));
	V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
	V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
	V(pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));
	V(pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR));
	V(pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));
	RenderAllObjects(PassTypeLight, pd3dDevice, fTime, fElapsedTime);
	V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE));
	V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
	LightSurf.Release();

	CComPtr<IDirect3DSurface9> OpaqueSurf = pRC->m_OpaqueRT.GetNextTarget()->GetSurfaceLevel(0);
	m_SimpleSample->SetTexture("g_LightRT", pRC->m_LightRT.get());
	V(pd3dDevice->SetRenderTarget(0, OpaqueSurf));
	V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, pRC->m_BkColor, 1.0f, 0)); // ! d3dmultisample will not work
	RenderAllObjects(PassTypeOpaque, pd3dDevice, fTime, fElapsedTime);
	OpaqueSurf.Release();

	V(pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
	V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
	V(pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));
	V(pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR));
	V(pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));
	RenderAllObjects(PassTypeTransparent, pd3dDevice, fTime, fElapsedTime);
	V(pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
	V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
	pRC->m_OpaqueRT.Flip();

	//if (0)
	//{
	//	D3DXSaveTextureToFileA("d:/aaa.bmp", D3DXIFF_BMP, pRC->m_OpaqueRT.GetNextSource()->m_ptr, NULL);
	//}

	V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));

	if (pRC->m_FxaaEnable)
	{
		RenderFXAA(pd3dDevice, pBackBufferSurfaceDesc, pRC);
	}

	if (pRC->m_DofEnable)
	{
		RenderDof(pd3dDevice, pBackBufferSurfaceDesc, pRC);
	}

	V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
	V(pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID));
	V(pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE));
	V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
	V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
	V(pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	V(pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	V(pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));
	V(pd3dDevice->SetRenderTarget(0, ScreenSurf));
	V(pd3dDevice->SetTexture(0, pRC->m_OpaqueRT.GetNextSource()->m_ptr));
	V(pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
	V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad, sizeof(quad[0])));
	V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE));

	ClearAllObjects();
}

void RenderPipeline::UpdateQuads(const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	quad[0].x = -0.5f;
	quad[0].y = -0.5f;
	quad[0].z = 1.0f;
	quad[0].rhw = 1.0f;
	quad[0].u = 0.0f;
	quad[0].v = 0.0f;

	quad[1].x = -0.5f;
	quad[1].y = pBackBufferSurfaceDesc->Height - 0.5f;
	quad[1].z = 1.0f;
	quad[1].rhw = 1.0f;
	quad[1].u = 0.0f;
	quad[1].v = 1.0f;

	quad[2].x = pBackBufferSurfaceDesc->Width - 0.5f;
	quad[2].y = pBackBufferSurfaceDesc->Height - 0.5f;
	quad[2].z = 1.0f;
	quad[2].rhw = 1.0f;
	quad[2].u = 1.0f;
	quad[2].v = 1.0f;

	quad[3].x = pBackBufferSurfaceDesc->Width - 0.5f;
	quad[3].y = -0.5f;
	quad[3].z = 1.0f;
	quad[3].rhw = 1.0f;
	quad[3].u = 1.0f;
	quad[3].v = 0.0f;

	quadDownFilter[0].x = -0.5f;
	quadDownFilter[0].y = -0.5f;
	quadDownFilter[0].z = 1.0f;
	quadDownFilter[0].rhw = 1.0f;
	quadDownFilter[0].u = 0.0f;
	quadDownFilter[0].v = 0.0f;

	quadDownFilter[1].x = -0.5f;
	quadDownFilter[1].y = pBackBufferSurfaceDesc->Height / 4 - 0.5f;
	quadDownFilter[1].z = 1.0f;
	quadDownFilter[1].rhw = 1.0f;
	quadDownFilter[1].u = 0.0f;
	quadDownFilter[1].v = 1.0f;

	quadDownFilter[2].x = pBackBufferSurfaceDesc->Width / 4 - 0.5f;
	quadDownFilter[2].y = pBackBufferSurfaceDesc->Height / 4 - 0.5f;
	quadDownFilter[2].z = 1.0f;
	quadDownFilter[2].rhw = 1.0f;
	quadDownFilter[2].u = 1.0f;
	quadDownFilter[2].v = 1.0f;

	quadDownFilter[3].x = pBackBufferSurfaceDesc->Width / 4 - 0.5f;
	quadDownFilter[3].y = -0.5f;
	quadDownFilter[3].z = 1.0f;
	quadDownFilter[3].rhw = 1.0f;
	quadDownFilter[3].u = 1.0f;
	quadDownFilter[3].v = 0.0f;
}

void RenderPipeline::RenderAllObjects(
	unsigned int PassID,
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	m_PassDrawCall[PassID] = 0;

	IndexedPrimitiveAtomList::iterator prim_iter = m_Pass[PassID].m_IndexedPrimitiveList.begin();
	for (; prim_iter != m_Pass[PassID].m_IndexedPrimitiveList.end(); prim_iter++)
	{
		DrawIndexedPrimitive(
			PassID,
			pd3dDevice,
			prim_iter->pDecl,
			prim_iter->pVB,
			prim_iter->pIB,
			prim_iter->PrimitiveType,
			prim_iter->BaseVertexIndex,
			prim_iter->MinVertexIndex,
			prim_iter->NumVertices,
			prim_iter->VertexStride,
			prim_iter->StartIndex,
			prim_iter->PrimitiveCount,
			prim_iter->AttribId,
			prim_iter->shader,
			prim_iter->setter);
		m_PassDrawCall[PassID]++;
	}

	IndexedPrimitiveUPAtomList::iterator indexed_prim_iter = m_Pass[PassID].m_IndexedPrimitiveUPList.begin();
	for (; indexed_prim_iter != m_Pass[PassID].m_IndexedPrimitiveUPList.end(); indexed_prim_iter++)
	{
		DrawIndexedPrimitiveUP(
			PassID,
			pd3dDevice,
			indexed_prim_iter->pDecl,
			indexed_prim_iter->PrimitiveType,
			indexed_prim_iter->MinVertexIndex,
			indexed_prim_iter->NumVertices,
			indexed_prim_iter->PrimitiveCount,
			indexed_prim_iter->pIndexData,
			indexed_prim_iter->IndexDataFormat,
			indexed_prim_iter->pVertexStreamZeroData,
			indexed_prim_iter->VertexStreamZeroStride,
			indexed_prim_iter->AttribId,
			indexed_prim_iter->shader,
			indexed_prim_iter->setter);
		m_PassDrawCall[PassID]++;
	}

	MeshAtomList::iterator mesh_iter = m_Pass[PassID].m_MeshList.begin();
	for (; mesh_iter != m_Pass[PassID].m_MeshList.end(); mesh_iter++)
	{
		DrawMesh(PassID, mesh_iter->mesh, mesh_iter->AttribId, mesh_iter->shader, mesh_iter->setter);
		m_PassDrawCall[PassID]++;
	}

	MeshInstanceAtomMap::iterator mesh_inst_iter = m_Pass[PassID].m_MeshInstanceMap.begin();
	for (; mesh_inst_iter != m_Pass[PassID].m_MeshInstanceMap.end(); mesh_inst_iter++)
	{
		if (!mesh_inst_iter->second.m_TransformList.empty())
		{
			DrawMeshInstance(
				PassID,
				pd3dDevice,
				mesh_inst_iter->first.get<0>(),
				mesh_inst_iter->first.get<1>(),
				mesh_inst_iter->first.get<2>(),
				mesh_inst_iter->second.setter,
				mesh_inst_iter->second);
			m_PassDrawCall[PassID]++;
		}
	}

	EmitterAtomList::iterator emitter_iter = m_Pass[PassID].m_EmitterList.begin();
	for (; emitter_iter != m_Pass[PassID].m_EmitterList.end(); emitter_iter++)
	{
		if (!emitter_iter->emitter->m_ParticleList.empty())
		{
			DrawEmitter(PassID, pd3dDevice, emitter_iter->emitter, emitter_iter->AttribId, emitter_iter->shader, emitter_iter->setter);
			m_PassDrawCall[PassID]++;
		}
	}
}

void RenderPipeline::ClearAllObjects(void)
{
	unsigned int PassID = 0;
	for (; PassID < m_Pass.size(); PassID++)
	{
		m_Pass[PassID].m_IndexedPrimitiveList.clear();
		m_Pass[PassID].m_IndexedPrimitiveUPList.clear();
		m_Pass[PassID].m_MeshList.clear();
		MeshInstanceAtomMap::iterator mesh_inst_iter = m_Pass[PassID].m_MeshInstanceMap.begin();
		for (; mesh_inst_iter != m_Pass[PassID].m_MeshInstanceMap.end(); mesh_inst_iter++)
		{
			mesh_inst_iter->second.m_TransformList.clear();
		}
		m_Pass[PassID].m_EmitterList.clear();
	}
}

void RenderPipeline::RenderDof(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
	IRenderContext * pRC)
{
	HRESULT hr;
	CComPtr<IDirect3DSurface9> OpaqueSurf = pRC->m_OpaqueRT.GetNextSource()->GetSurfaceLevel(0);
	V(pd3dDevice->StretchRect(OpaqueSurf, NULL, pRC->m_DownFilterRT.GetNextTarget()->GetSurfaceLevel(0), NULL, D3DTEXF_NONE)); // ! d3dref only support none
	OpaqueSurf.Release();
	pRC->m_DownFilterRT.Flip();

	m_DofEffect->SetVector("g_DofParams", pRC->m_DofParams);
	m_DofEffect->SetTexture("g_OpaqueRT", pRC->m_OpaqueRT.GetNextSource().get());
	m_DofEffect->SetTexture("g_DownFilterRT", pRC->m_DownFilterRT.GetNextSource().get());
	V(pd3dDevice->SetRenderTarget(0, pRC->m_DownFilterRT.GetNextTarget()->GetSurfaceLevel(0)));
	V(pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
	UINT passes = m_DofEffect->Begin();
	m_DofEffect->BeginPass(0);
	V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quadDownFilter, sizeof(quad[0])));
	m_DofEffect->EndPass();
	pRC->m_DownFilterRT.Flip();

	m_DofEffect->SetTexture("g_DownFilterRT", pRC->m_DownFilterRT.GetNextSource().get());
	V(pd3dDevice->SetRenderTarget(0, pRC->m_DownFilterRT.GetNextTarget()->GetSurfaceLevel(0)));
	m_DofEffect->BeginPass(1);
	V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quadDownFilter, sizeof(quad[0])));
	m_DofEffect->EndPass();
	pRC->m_DownFilterRT.Flip();

	m_DofEffect->SetTexture("g_DownFilterRT", pRC->m_DownFilterRT.GetNextSource().get());
	V(pd3dDevice->SetRenderTarget(0, pRC->m_OpaqueRT.GetNextTarget()->GetSurfaceLevel(0)));
	m_DofEffect->BeginPass(2);
	V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad, sizeof(quad[0])));
	m_DofEffect->EndPass();
	m_DofEffect->End();
	pRC->m_OpaqueRT.Flip();
}

void RenderPipeline::RenderFXAA(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
	IRenderContext * pRC)
{
	HRESULT hr;
	V(pd3dDevice->SetRenderTarget(0, pRC->m_OpaqueRT.GetNextTarget()->GetSurfaceLevel(0)));
	V(pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
	m_FxaaEffect->SetTexture("InputTexture", pRC->m_OpaqueRT.GetNextSource().get());
	Vector4 RCPFrame(1.0f / pBackBufferSurfaceDesc->Width, 1.0f / pBackBufferSurfaceDesc->Height, 0.0f, 0.0f);
	m_FxaaEffect->SetFloatArray("RCPFrame", &RCPFrame.x, sizeof(RCPFrame) / sizeof(float));
	m_FxaaEffect->Begin();
	m_FxaaEffect->BeginPass(0);
	V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad, sizeof(quad[0])));
	m_FxaaEffect->EndPass();
	m_FxaaEffect->End();
	pRC->m_OpaqueRT.Flip();
}

void RenderPipeline::DrawIndexedPrimitive(
	unsigned int PassID,
	IDirect3DDevice9 * pd3dDevice,
	IDirect3DVertexDeclaration9* pDecl,
	IDirect3DVertexBuffer9 * pVB,
	IDirect3DIndexBuffer9 * pIB,
	D3DPRIMITIVETYPE PrimitiveType,
	INT BaseVertexIndex,
	UINT MinVertexIndex,
	UINT NumVertices,
	UINT VertexStride,
	UINT StartIndex,
	UINT PrimitiveCount,
	DWORD AttribId,
	my::Effect * shader,
	IShaderSetter * setter)
{
	HRESULT hr;
	V(pd3dDevice->SetStreamSource(0, pVB, 0, VertexStride));
	V(pd3dDevice->SetVertexDeclaration(pDecl));
	V(pd3dDevice->SetIndices(pIB));

	shader->SetTechnique("RenderScene");
	const UINT passes = shader->Begin(0);
	_ASSERT(PassID < passes);
	setter->OnSetShader(shader, AttribId);
	{
		shader->BeginPass(PassID);
		V(pd3dDevice->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, StartIndex, PrimitiveCount));
		shader->EndPass();
	}
	shader->End();
}

void RenderPipeline::DrawIndexedPrimitiveUP(
	unsigned int PassID,
	IDirect3DDevice9 * pd3dDevice,
	IDirect3DVertexDeclaration9* pDecl,
	D3DPRIMITIVETYPE PrimitiveType,
	UINT MinVertexIndex,
	UINT NumVertices,
	UINT PrimitiveCount,
	CONST void* pIndexData,
	D3DFORMAT IndexDataFormat,
	CONST void* pVertexStreamZeroData,
	UINT VertexStreamZeroStride,
	DWORD AttribId,
	my::Effect * shader,
	IShaderSetter * setter)
{
	shader->SetTechnique("RenderScene");
	const UINT passes = shader->Begin(0);
	_ASSERT(PassID < passes);
	setter->OnSetShader(shader, AttribId);
	{
		shader->BeginPass(PassID);
		HRESULT hr;
		V(pd3dDevice->SetVertexDeclaration(pDecl));
		V(pd3dDevice->DrawIndexedPrimitiveUP(
			PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride));
		shader->EndPass();
	}
	shader->End();
}

void RenderPipeline::DrawMesh(unsigned int PassID, my::Mesh * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter)
{
	shader->SetTechnique("RenderScene");
	const UINT passes = shader->Begin(0);
	_ASSERT(PassID < passes);
	setter->OnSetShader(shader, AttribId);
	{
		shader->BeginPass(PassID);
		mesh->DrawSubset(AttribId);
		shader->EndPass();
	}
	shader->End();
}

void RenderPipeline::DrawMeshInstance(
	unsigned int PassID,
	IDirect3DDevice9 * pd3dDevice,
	my::Mesh * mesh,
	DWORD AttribId,
	my::Effect * shader,
	IShaderSetter * setter,
	MeshInstanceAtom & atom)
{
	_ASSERT(AttribId < atom.m_AttribTable.size());
	const DWORD NumInstances = atom.m_TransformList.size();
	_ASSERT(NumInstances <= MESH_INSTANCE_MAX);

	Matrix4 * mat = (Matrix4 *)m_MeshInstanceData.Lock(0, NumInstances * m_MeshInstanceStride, D3DLOCK_DISCARD);
	memcpy(mat, &atom.m_TransformList[0], NumInstances * m_MeshInstanceStride);
	m_MeshInstanceData.Unlock();

	CComPtr<IDirect3DVertexBuffer9> vb = mesh->GetVertexBuffer();
	CComPtr<IDirect3DIndexBuffer9> ib = mesh->GetIndexBuffer();

	HRESULT hr;
	V(pd3dDevice->SetStreamSource(0, vb, 0, atom.m_VertexStride));
	V(pd3dDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | NumInstances));
	V(pd3dDevice->SetStreamSource(1, m_MeshInstanceData.m_ptr, 0, m_MeshInstanceStride));
	V(pd3dDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1));
	V(pd3dDevice->SetVertexDeclaration(atom.m_Decl));
	V(pd3dDevice->SetIndices(ib));

	shader->SetTechnique("RenderScene");
	const UINT passes = shader->Begin(0);
	_ASSERT(PassID < passes);
	setter->OnSetShader(shader, AttribId);
	{
		shader->BeginPass(PassID);
		V(pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0,
			atom.m_AttribTable[AttribId].VertexStart,
			atom.m_AttribTable[AttribId].VertexCount,
			atom.m_AttribTable[AttribId].FaceStart * 3,
			atom.m_AttribTable[AttribId].FaceCount));
		shader->EndPass();
	}
	shader->End();

	V(pd3dDevice->SetStreamSourceFreq(0,1));
	V(pd3dDevice->SetStreamSourceFreq(1,1));
}

void RenderPipeline::DrawEmitter(unsigned int PassID, IDirect3DDevice9 * pd3dDevice, my::Emitter * emitter, DWORD AttribId, my::Effect * shader, IShaderSetter * setter)
{
	const DWORD NumInstances = emitter->m_ParticleList.size();
	_ASSERT(NumInstances <= PARTICLE_INSTANCE_MAX);
	unsigned char * pVertices = (unsigned char *)m_ParticleInstanceData.Lock(0, m_ParticleInstanceStride * NumInstances, D3DLOCK_DISCARD);
	_ASSERT(pVertices);
	for(DWORD i = 0; i < NumInstances; i++)
	{
		// ! Can optimize, because all point offset are constant
		unsigned char * pVertex = pVertices + i * m_ParticleInstanceStride;
		const my::Emitter::Particle & particle = emitter->m_ParticleList[i];
		//m_ParticleInstanceElems.SetPosition(pVertex, particle.m_Position);
		//m_ParticleInstanceElems.SetNormal(pVertex, particle.m_Velocity);
		//m_ParticleInstanceElems.SetVertexValue(pVertex, D3DDECLUSAGE_TEXCOORD, 1, particle.m_Color);
		//m_ParticleInstanceElems.SetVertexValue(pVertex, D3DDECLUSAGE_TEXCOORD, 2, Vector4(particle.m_Size.x, particle.m_Size.y, particle.m_Angle, particle.m_Time));
		memcpy(pVertex, &particle, sizeof(particle));
	}
	m_ParticleInstanceData.Unlock();

	shader->SetTechnique("RenderScene");
	const UINT passes = shader->Begin(0);
	_ASSERT(PassID < passes);
	setter->OnSetShader(shader, AttribId);
	{
		shader->BeginPass(PassID);
		HRESULT hr;
		V(pd3dDevice->SetStreamSource(0, m_ParticleVertexBuffer.m_ptr, 0, m_ParticleVertexStride));
		V(pd3dDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | NumInstances));

		V(pd3dDevice->SetStreamSource(1, m_ParticleInstanceData.m_ptr, 0, m_ParticleInstanceStride));
		V(pd3dDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1));

		V(pd3dDevice->SetVertexDeclaration(m_ParticleDecl));
		V(pd3dDevice->SetIndices(m_ParticleIndexBuffer.m_ptr));
		V(pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, 4, 0, 2));

		V(pd3dDevice->SetStreamSourceFreq(0,1));
		V(pd3dDevice->SetStreamSourceFreq(1,1));
		shader->EndPass();
	}
	shader->End();
}

void RenderPipeline::PushIndexedPrimitive(
	unsigned int PassID,
	IDirect3DVertexDeclaration9* pDecl,
	IDirect3DVertexBuffer9 * pVB,
	IDirect3DIndexBuffer9 * pIB,
	D3DPRIMITIVETYPE PrimitiveType,
	INT BaseVertexIndex,
	UINT MinVertexIndex,
	UINT NumVertices,
	UINT VertexStride,
	UINT StartIndex,
	UINT PrimitiveCount,
	DWORD AttribId,
	my::Effect * shader,
	IShaderSetter * setter)
{
	IndexedPrimitiveAtom atom;
	atom.pDecl = pDecl;
	atom.pVB = pVB;
	atom.pIB = pIB;
	atom.PrimitiveType = PrimitiveType;
	atom.BaseVertexIndex = BaseVertexIndex;
	atom.MinVertexIndex = MinVertexIndex;
	atom.NumVertices = NumVertices;
	atom.VertexStride = VertexStride;
	atom.StartIndex = StartIndex;
	atom.PrimitiveCount = PrimitiveCount;
	atom.AttribId = AttribId;
	atom.shader = shader;
	atom.setter = setter;
	m_Pass[PassID].m_IndexedPrimitiveList.push_back(atom);
}

void RenderPipeline::PushIndexedPrimitiveUP(
	unsigned int PassID,
	IDirect3DVertexDeclaration9* pDecl,
	D3DPRIMITIVETYPE PrimitiveType,
	UINT MinVertexIndex,
	UINT NumVertices,
	UINT PrimitiveCount,
	CONST void* pIndexData,
	D3DFORMAT IndexDataFormat,
	CONST void* pVertexStreamZeroData,
	UINT VertexStreamZeroStride,
	DWORD AttribId,
	my::Effect * shader,
	IShaderSetter * setter)
{
	IndexedPrimitiveUPAtom atom;
	atom.pDecl = pDecl;
	atom.PrimitiveType = PrimitiveType;
	atom.PrimitiveCount = PrimitiveCount;
	atom.MinVertexIndex = MinVertexIndex;
	atom.NumVertices = NumVertices;
	atom.PrimitiveCount = PrimitiveCount;
	atom.pIndexData = pIndexData;
	atom.IndexDataFormat = IndexDataFormat;
	atom.pVertexStreamZeroData = pVertexStreamZeroData;
	atom.VertexStreamZeroStride = VertexStreamZeroStride;
	atom.AttribId = AttribId;
	atom.shader = shader;
	atom.setter = setter;
	m_Pass[PassID].m_IndexedPrimitiveUPList.push_back(atom);
}

void RenderPipeline::PushMesh(unsigned int PassID, my::Mesh * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter)
{
	MeshAtom atom;
	atom.mesh = mesh;
	atom.AttribId = AttribId;
	atom.shader = shader;
	atom.setter = setter;
	m_Pass[PassID].m_MeshList.push_back(atom);
}

namespace boost
{
	static size_t hash_value(const RenderPipeline::MeshInstanceAtomKey & key)
	{
		size_t seed = 0;
		boost::hash_combine(seed, key.get<0>());
		boost::hash_combine(seed, key.get<1>());
		boost::hash_combine(seed, key.get<2>());
		return seed;
	}
}

void RenderPipeline::PushMeshInstance(unsigned int PassID, my::Mesh * mesh, DWORD AttribId, const my::Matrix4 & World, my::Effect * shader, IShaderSetter * setter)
{
	MeshInstanceAtomKey key(mesh, AttribId, shader);
	MeshInstanceAtomMap::iterator atom_iter = m_Pass[PassID].m_MeshInstanceMap.find(key);
	if (atom_iter == m_Pass[PassID].m_MeshInstanceMap.end())
	{
		MeshInstanceAtom & atom = m_Pass[PassID].m_MeshInstanceMap[key];
		atom.setter = setter;
		DWORD submeshes = 0;
		mesh->GetAttributeTable(NULL, &submeshes);
		atom.m_AttribTable.resize(submeshes);
		mesh->GetAttributeTable(&atom.m_AttribTable[0], &submeshes);

		atom.m_velist.resize(MAX_FVF_DECL_SIZE);
		mesh->GetDeclaration(&atom.m_velist[0]);
		unsigned int i = 0;
		for (; i < atom.m_velist.size(); i++)
		{
			if (atom.m_velist[i].Stream == 0xff || atom.m_velist[i].Type == D3DDECLTYPE_UNUSED)
			{
				break;
			}
		}
		if (i >= atom.m_velist.size())
		{
			THROW_CUSEXCEPTION("invalid vertex declaration");
		}
		atom.m_velist.insert(atom.m_velist.begin() + i, m_MeshIEList.begin(), m_MeshIEList.end());
		atom.m_VertexStride = D3DXGetDeclVertexSize(&atom.m_velist[0], 0);
		_ASSERT(m_MeshInstanceStride == D3DXGetDeclVertexSize(&atom.m_velist[0], 1));

		HRESULT hr;
		CComPtr<IDirect3DDevice9> Device = mesh->GetDevice();
		if (FAILED(hr = Device->CreateVertexDeclaration(&atom.m_velist[0], &atom.m_Decl)))
		{
			THROW_D3DEXCEPTION(hr);
		}
	}
	m_Pass[PassID].m_MeshInstanceMap[key].m_TransformList.push_back(World);
}

void RenderPipeline::PushEmitter(unsigned int PassID, my::Emitter * emitter, DWORD AttribId, my::Effect * shader, IShaderSetter * setter)
{
	EmitterAtom atom;
	atom.emitter = emitter;
	atom.AttribId = AttribId;
	atom.shader = shader;
	atom.setter = setter;
	m_Pass[PassID].m_EmitterList.push_back(atom);
}
