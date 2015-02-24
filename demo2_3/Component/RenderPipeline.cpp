#include "stdafx.h"
#include "RenderPipeline.h"

using namespace my;

void RenderPipeline::OnRender(IDirect3DDevice9 * pd3dDevice, double fTime, float fElapsedTime)
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
	memcpy(mat, &worlds[0], worlds.size());
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
	boost::hash_combine(seed, key.setter);
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

void RenderPipeline::ClearAllOpaqueObjs(void)
{
	m_OpaqueMeshList.clear();
	m_OpaqueMeshInstanceMap.clear();
	m_OpaqueIndexedPrimitiveUPList.clear();
}

void Material::OnQueryMesh(
	RenderPipeline * pipeline,
	RenderPipeline::DrawStage stage,
	RenderPipeline::MeshType mesh_type,
	my::MeshInstance * mesh,
	DWORD AttribId,
	RenderPipeline::IShaderSetter * setter)
{
	my::Effect * shader = pipeline->QueryShader(mesh_type, stage, false, this);
	if (shader)
	{
		pipeline->PushOpaqueMesh(mesh, AttribId, shader, setter);
	}
}

void Material::OnQueryMeshInstance(
	RenderPipeline * pipeline,
	RenderPipeline::DrawStage stage,
	RenderPipeline::MeshType mesh_type,
	my::MeshInstance * mesh,
	DWORD AttribId,
	const my::Matrix4 & World,
	RenderPipeline::IShaderSetter * setter)
{
	my::Effect * shader = pipeline->QueryShader(mesh_type, stage, true, this);
	if (shader)
	{
		pipeline->PushOpaqueMeshInstance(mesh, AttribId, World, shader, setter);
	}
}

void Material::OnQueryIndexedPrimitiveUP(
	RenderPipeline * pipeline,
	RenderPipeline::DrawStage stage,
	RenderPipeline::MeshType mesh_type,
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
	RenderPipeline::IShaderSetter * setter)
{
	my::Effect * shader = pipeline->QueryShader(mesh_type, stage, false, this);
	if (shader)
	{
		pipeline->PushOpaqueIndexedPrimitiveUP(
			pDecl, PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride, AttribId, shader, setter);
	}
}

void Material::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	shader->SetTexture("g_MeshTexture", m_DiffuseTexture);
}
