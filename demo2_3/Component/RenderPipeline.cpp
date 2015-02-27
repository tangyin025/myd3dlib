#include "stdafx.h"
#include "RenderPipeline.h"

using namespace my;

HRESULT RenderPipeline::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	m_ParticleVertexElems.InsertTexcoordElement(0);

	m_ParticleInstanceElems.InsertPositionElement(0);
	WORD offset = sizeof(Vector3);
	m_ParticleInstanceElems.InsertColorElement(offset);
	offset += sizeof(D3DCOLOR);
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

	_ASSERT(!m_ParticleVertexBuffer.m_ptr);
	_ASSERT(!m_ParticleIndexBuffer.m_ptr);
	_ASSERT(!m_ParticleInstanceData.m_ptr);
	return S_OK;
}

HRESULT RenderPipeline::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	_ASSERT(!m_ParticleVertexBuffer.m_ptr);
	m_ParticleVertexBuffer.CreateVertexBuffer(pd3dDevice, m_ParticleVertexStride * 4);
	unsigned char * pVertices = (unsigned char *)m_ParticleVertexBuffer.Lock(0, m_ParticleVertexStride * 4);
	m_ParticleVertexElems.SetTexcoord(pVertices + m_ParticleVertexStride * 0, Vector2(0,0));
	m_ParticleVertexElems.SetTexcoord(pVertices + m_ParticleVertexStride * 1, Vector2(1,0));
	m_ParticleVertexElems.SetTexcoord(pVertices + m_ParticleVertexStride * 2, Vector2(1,1));
	m_ParticleVertexElems.SetTexcoord(pVertices + m_ParticleVertexStride * 3, Vector2(0,1));
	m_ParticleVertexBuffer.Unlock();

	_ASSERT(!m_ParticleIndexBuffer.m_ptr);
	m_ParticleIndexBuffer.CreateIndexBuffer(pd3dDevice, sizeof(WORD) * 4);
	WORD * pIndices = (WORD *)m_ParticleIndexBuffer.Lock(0, sizeof(WORD) * 4);
	pIndices[0] = 0;
	pIndices[1] = 1;
	pIndices[2] = 2;
	pIndices[3] = 3;
	m_ParticleIndexBuffer.Unlock();

	_ASSERT(!m_ParticleInstanceData.m_ptr);
	m_ParticleInstanceData.CreateVertexBuffer(pd3dDevice, m_ParticleInstanceStride * PARTICLE_INSTANCE_MAX, 0, 0, D3DPOOL_DEFAULT);
	return S_OK;
}

void RenderPipeline::OnLostDevice(void)
{
	m_ParticleVertexBuffer.OnDestroyDevice();
	m_ParticleIndexBuffer.OnDestroyDevice();
	m_ParticleInstanceData.OnDestroyDevice();
}

void RenderPipeline::OnDestroyDevice(void)
{
	_ASSERT(!m_ParticleVertexBuffer.m_ptr);
	_ASSERT(!m_ParticleIndexBuffer.m_ptr);
	_ASSERT(!m_ParticleInstanceData.m_ptr);

	m_ParticleDecl.Release();
}

void RenderPipeline::OnFrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	OpaqueMeshList::iterator mesh_iter = m_OpaqueMeshList.begin();
	for (; mesh_iter != m_OpaqueMeshList.end(); mesh_iter++)
	{
		DrawOpaqueMesh(mesh_iter->mesh, mesh_iter->AttribId, mesh_iter->shader, mesh_iter->setter);
	}

	OpaqueMeshInstanceMap::iterator mesh_inst_iter = m_OpaqueMeshInstanceMap.begin();
	for (; mesh_inst_iter != m_OpaqueMeshInstanceMap.end(); mesh_inst_iter++)
	{
		DrawOpaqueMeshInstance(mesh_inst_iter->first.mesh, mesh_inst_iter->first.AttribId, mesh_inst_iter->second, mesh_inst_iter->first.shader, mesh_inst_iter->first.setter);
	}

	OpaqueIndexedPrimitiveUPList::iterator indexed_prim_iter = m_OpaqueIndexedPrimitiveUPList.begin();
	for (; indexed_prim_iter != m_OpaqueIndexedPrimitiveUPList.end(); indexed_prim_iter++)
	{
		DrawOpaqueIndexedPrimitiveUP(
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
	}

	EmitterAtomList::iterator emitter_iter = m_EmitterAtomList.begin();
	for (; emitter_iter != m_EmitterAtomList.end(); emitter_iter++)
	{
		DrawEmitterAtom(pd3dDevice, emitter_iter->emitter, emitter_iter->shader, emitter_iter->setter);
	}

	ClearAllRenderObjs();
}

void RenderPipeline::DrawOpaqueMesh(my::MeshInstance * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter)
{
	shader->SetTechnique("RenderScene");
	const UINT passes = shader->Begin(0);
	setter->OnSetShader(shader, AttribId);
	for (UINT p = 0; p < passes; p++)
	{
		shader->BeginPass(p);
		mesh->DrawSubset(AttribId);
		shader->EndPass();
	}
	shader->End();
}

void RenderPipeline::DrawOpaqueMeshInstance(my::MeshInstance * mesh, DWORD AttribId, const my::TransformList & worlds, my::Effect * shader, IShaderSetter * setter)
{
	Matrix4 * mat = mesh->LockInstanceData(worlds.size());
	memcpy(mat, &worlds[0], sizeof(Matrix4) * worlds.size());
	mesh->UnlockInstanceData();

	shader->SetTechnique("RenderScene");
	const UINT passes = shader->Begin(0);
	setter->OnSetShader(shader, AttribId);
	for (UINT p = 0; p < passes; p++)
	{
		shader->BeginPass(p);
		mesh->DrawSubsetInstance(AttribId, worlds.size());
		shader->EndPass();
	}
	shader->End();
}

static size_t hash_value(const RenderPipeline::OpaqueMesh & key)
{
	size_t seed = 0;
	boost::hash_combine(seed, key.mesh);
	boost::hash_combine(seed, key.AttribId);
	boost::hash_combine(seed, key.shader);
	//boost::hash_combine(seed, key.setter); // ! setter was not a key, the first will always be used
	return seed;
}

void RenderPipeline::DrawOpaqueIndexedPrimitiveUP(
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
	setter->OnSetShader(shader, AttribId);
	for (UINT p = 0; p < passes; p++)
	{
		shader->BeginPass(p);
		HRESULT hr;
		V(pd3dDevice->SetVertexDeclaration(pDecl));
		V(pd3dDevice->DrawIndexedPrimitiveUP(
			PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride));
		shader->EndPass();
	}
	shader->End();
}

void RenderPipeline::DrawEmitterAtom(IDirect3DDevice9 * pd3dDevice, my::Emitter * emitter, my::Effect * shader, IShaderSetter * setter)
{
	const DWORD NumInstances = emitter->m_ParticleList.size();
	_ASSERT(NumInstances <= PARTICLE_INSTANCE_MAX);
	unsigned char * pVertices = (unsigned char *)m_ParticleInstanceData.Lock(0, m_ParticleInstanceStride * NumInstances, 0);
	_ASSERT(pVertices);
	for(DWORD i = 0; i < NumInstances; i++)
	{
		// ! Can optimize, because all point offset are constant
		unsigned char * pVertex = pVertices + i * m_ParticleInstanceStride;
		const my::Emitter::Particle & particle = emitter->m_ParticleList[i].second;
		m_ParticleInstanceElems.SetPosition(pVertex, particle.m_Position);
		m_ParticleInstanceElems.SetColor(pVertex, particle.m_Color);
		m_ParticleInstanceElems.SetVertexValue(pVertex, D3DDECLUSAGE_TEXCOORD, 1, particle.m_Texcoord1);
		m_ParticleInstanceElems.SetVertexValue(pVertex, D3DDECLUSAGE_TEXCOORD, 2, particle.m_Texcoord2);
	}
	m_ParticleInstanceData.Unlock();

	shader->SetTechnique("RenderScene");
	const UINT passes = shader->Begin(0);
	setter->OnSetShader(shader, 0);
	for (UINT p = 0; p < passes; p++)
	{
		shader->BeginPass(p);
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

void RenderPipeline::PushOpaqueMesh(my::MeshInstance * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter)
{
	OpaqueMesh atom;
	atom.mesh = mesh;
	atom.AttribId = AttribId;
	atom.shader = shader;
	atom.setter = setter;
	m_OpaqueMeshList.push_back(atom);
}

void RenderPipeline::PushOpaqueMeshInstance(my::MeshInstance * mesh, DWORD AttribId, const my::Matrix4 & World, my::Effect * shader, IShaderSetter * setter)
{
	OpaqueMesh atom;
	atom.mesh = mesh;
	atom.AttribId = AttribId;
	atom.shader = shader;
	atom.setter = setter;
	m_OpaqueMeshInstanceMap[atom].push_back(World);
}

void RenderPipeline::PushOpaqueIndexedPrimitiveUP(
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
	OpaqueIndexedPrimitiveUP atom;
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
	m_OpaqueIndexedPrimitiveUPList.push_back(atom);
}

void RenderPipeline::PushEmitter(my::Emitter * emitter, my::Effect * shader, IShaderSetter * setter)
{
	EmitterAtom atom;
	atom.emitter = emitter;
	atom.shader = shader;
	atom.setter = setter;
	m_EmitterAtomList.push_back(atom);
}

void RenderPipeline::ClearAllRenderObjs(void)
{
	m_OpaqueMeshList.clear();
	OpaqueMeshInstanceMap::iterator mesh_inst_iter = m_OpaqueMeshInstanceMap.begin();
	for (; mesh_inst_iter != m_OpaqueMeshInstanceMap.end(); mesh_inst_iter++)
	{
		mesh_inst_iter->second.clear();
	}
	m_OpaqueIndexedPrimitiveUPList.clear();
	m_EmitterAtomList.clear();
}

my::Effect * Material::QueryShader(
	RenderPipeline * pipeline,
	RenderPipeline::DrawStage stage,
	RenderPipeline::MeshType mesh_type,
	bool bInstance)
{
	return pipeline->QueryShader(mesh_type, stage, bInstance, this);
}

void Material::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	shader->SetTexture("g_MeshTexture", m_DiffuseTexture);
}
