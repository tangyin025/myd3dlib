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
	m_ParticleInstanceData.CreateVertexBuffer(pd3dDevice, m_ParticleInstanceStride * PARTICLE_INSTANCE_MAX, D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT);

	_ASSERT(!m_MeshInstanceData.m_ptr);
	m_MeshInstanceData.CreateVertexBuffer(pd3dDevice, m_MeshInstanceStride * MESH_INSTANCE_MAX, D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT);
	return S_OK;
}

void RenderPipeline::OnLostDevice(void)
{
	m_ParticleVertexBuffer.OnDestroyDevice();
	m_ParticleIndexBuffer.OnDestroyDevice();
	m_ParticleInstanceData.OnDestroyDevice();
	m_MeshInstanceData.OnDestroyDevice();
}

void RenderPipeline::OnDestroyDevice(void)
{
	_ASSERT(!m_ParticleVertexBuffer.m_ptr);
	_ASSERT(!m_ParticleIndexBuffer.m_ptr);
	_ASSERT(!m_ParticleInstanceData.m_ptr);
	_ASSERT(!m_MeshInstanceData.m_ptr);

	ClearAllRenderObjs();

	//MeshInstanceAtomMap::iterator mesh_inst_iter = m_OpaqueMeshInstanceMap.begin();
	//for (; mesh_inst_iter != m_OpaqueMeshInstanceMap.end(); mesh_inst_iter++)
	//{
	//	mesh_inst_iter->second.m_Decl.Release();
	//}
	m_OpaqueMeshInstanceMap.clear();

	m_ParticleDecl.Release();
}

void RenderPipeline::OnFrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	MeshAtomList::iterator mesh_iter = m_OpaqueMeshList.begin();
	for (; mesh_iter != m_OpaqueMeshList.end(); mesh_iter++)
	{
		DrawOpaqueMesh(mesh_iter->mesh, mesh_iter->AttribId, mesh_iter->shader, mesh_iter->setter);
	}

	MeshInstanceAtomMap::iterator mesh_inst_iter = m_OpaqueMeshInstanceMap.begin();
	for (; mesh_inst_iter != m_OpaqueMeshInstanceMap.end(); mesh_inst_iter++)
	{
		DrawOpaqueMeshInstance(
			pd3dDevice,
			mesh_inst_iter->first.get<0>(),
			mesh_inst_iter->first.get<1>(),
			mesh_inst_iter->first.get<2>(),
			mesh_inst_iter->second.setter,
			mesh_inst_iter->second);
	}

	IndexedPrimitiveUPAtomList::iterator indexed_prim_iter = m_OpaqueIndexedPrimitiveUPList.begin();
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

	EmitterAtomList::iterator emitter_iter = m_QpaqueEmitterList.begin();
	for (; emitter_iter != m_QpaqueEmitterList.end(); emitter_iter++)
	{
		DrawOpaqueEmitter(pd3dDevice, emitter_iter->emitter, emitter_iter->AttribId, emitter_iter->shader, emitter_iter->setter);
	}

	ClearAllRenderObjs();
}

void RenderPipeline::DrawOpaqueMesh(my::Mesh * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter)
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

void RenderPipeline::DrawOpaqueMeshInstance(
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
	setter->OnSetShader(shader, AttribId);
	for (UINT p = 0; p < passes; p++)
	{
		shader->BeginPass(p);
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

void RenderPipeline::DrawOpaqueEmitter(IDirect3DDevice9 * pd3dDevice, my::Emitter * emitter, DWORD AttribId, my::Effect * shader, IShaderSetter * setter)
{
	const DWORD NumInstances = emitter->m_ParticleList.size();
	_ASSERT(NumInstances <= PARTICLE_INSTANCE_MAX);
	unsigned char * pVertices = (unsigned char *)m_ParticleInstanceData.Lock(0, m_ParticleInstanceStride * NumInstances, D3DLOCK_DISCARD);
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
	setter->OnSetShader(shader, AttribId);
	shader->SetFloatArray("g_AnimationColumnRow", &Vector2(emitter->m_ParticleAnimColumn, emitter->m_ParticleAnimRow)[0], 2);
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

void RenderPipeline::PushOpaqueMesh(my::Mesh * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter)
{
	MeshAtom atom;
	atom.mesh = mesh;
	atom.AttribId = AttribId;
	atom.shader = shader;
	atom.setter = setter;
	m_OpaqueMeshList.push_back(atom);
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

void RenderPipeline::PushOpaqueMeshInstance(my::Mesh * mesh, DWORD AttribId, const my::Matrix4 & World, my::Effect * shader, IShaderSetter * setter)
{
	MeshInstanceAtomKey key(mesh, AttribId, shader);
	MeshInstanceAtomMap::iterator atom_iter = m_OpaqueMeshInstanceMap.find(key);
	if (atom_iter == m_OpaqueMeshInstanceMap.end())
	{
		MeshInstanceAtom & atom = m_OpaqueMeshInstanceMap[key];
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
			THROW_CUSEXCEPTION(_T("invalid vertex declaration"));
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
	m_OpaqueMeshInstanceMap[key].m_TransformList.push_back(World);
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
	m_OpaqueIndexedPrimitiveUPList.push_back(atom);
}

void RenderPipeline::PushOpaqueEmitter(my::Emitter * emitter, DWORD AttribId, my::Effect * shader, IShaderSetter * setter)
{
	EmitterAtom atom;
	atom.emitter = emitter;
	atom.AttribId = AttribId;
	atom.shader = shader;
	atom.setter = setter;
	m_QpaqueEmitterList.push_back(atom);
}

void RenderPipeline::ClearAllRenderObjs(void)
{
	m_OpaqueMeshList.clear();
	MeshInstanceAtomMap::iterator mesh_inst_iter = m_OpaqueMeshInstanceMap.begin();
	for (; mesh_inst_iter != m_OpaqueMeshInstanceMap.end(); mesh_inst_iter++)
	{
		mesh_inst_iter->second.m_TransformList.clear();
	}
	m_OpaqueIndexedPrimitiveUPList.clear();
	m_QpaqueEmitterList.clear();
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
	shader->SetTexture("g_MeshTexture", m_DiffuseTexture.second);
}
