// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "myMesh.h"
#include "mySkeleton.h"
#include "myCollision.h"
#include "myResource.h"
#include "myDxutApp.h"
#include "myUtility.h"
#include "libc.h"
#include "rapidxml.hpp"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/fusion/tuple.hpp>
#include <boost/fusion/include/hash.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/multi_array.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/tuple/tuple_comparison.hpp>

using namespace my;

D3DVertexElementSet::D3DVertexElementSet(D3DVERTEXELEMENT9* elems)
{
	D3DVERTEXELEMENT9* elem_iter = elems;
	for (; elem_iter->Type != D3DDECLTYPE_UNUSED; elem_iter++)
	{
		InsertVertexElement(elem_iter->Offset, (D3DDECLTYPE)elem_iter->Type, (D3DDECLUSAGE)elem_iter->Usage, elem_iter->UsageIndex, (D3DDECLMETHOD)elem_iter->Method);
	}
}

void D3DVertexElementSet::InsertVertexElement(WORD Offset, D3DDECLTYPE Type, D3DDECLUSAGE Usage, BYTE UsageIndex, D3DDECLMETHOD Method)
{
	_ASSERT(Type != D3DDECLTYPE_UNUSED);

	_ASSERT(Usage < MAX_USAGE && UsageIndex < MAX_USAGE_INDEX);

	elems[Usage][UsageIndex] = D3DVertexElement(Offset, Type, Method);
}

std::vector<D3DVERTEXELEMENT9> D3DVertexElementSet::BuildVertexElementList(WORD Stream) const
{
	std::vector<D3DVERTEXELEMENT9> ret;
	for(unsigned int Usage = 0; Usage < MAX_USAGE; Usage++)
	{
		for(unsigned int UsageIndex = 0; UsageIndex < MAX_USAGE_INDEX; UsageIndex++)
		{
			const D3DVertexElement & elem = elems[Usage][UsageIndex];
			if(elem.Type != D3DDECLTYPE_UNUSED)
			{
				D3DVERTEXELEMENT9 ve = {Stream, elem.Offset, (BYTE)elem.Type, (BYTE)elem.Method, (BYTE)Usage, (BYTE)UsageIndex};
				ret.push_back(ve);
			}
		}
	}
	return ret;
}

unsigned int D3DVertexElementSet::CalcTextureCoords(void) const
{
	unsigned int texture_coords = 0;
	for(unsigned int UsageIndex = 0; UsageIndex < MAX_USAGE_INDEX; UsageIndex++)
	{
		const D3DVertexElement & elem = elems[D3DDECLUSAGE_TEXCOORD][UsageIndex];
		if(elem.Type != D3DDECLTYPE_UNUSED)
		{
			texture_coords++;
		}
	}
	return texture_coords;
}

void D3DVertexElementSet::InsertPositionElement(WORD Offset, BYTE UsageIndex, D3DDECLMETHOD Method)
{
	InsertVertexElement(Offset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION, UsageIndex, Method);
}

Vector3 & D3DVertexElementSet::GetPosition(void * pVertex, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_POSITION][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

	return *GetVertexValue<Vector3>(pVertex, D3DDECLUSAGE_POSITION, UsageIndex);
}

void D3DVertexElementSet::SetPosition(void * pVertex, const Vector3 & Position, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_POSITION][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

	return SetVertexValue<Vector3>(pVertex, D3DDECLUSAGE_POSITION, UsageIndex, Position);
}

void D3DVertexElementSet::InsertBlendWeightElement(WORD Offset, BYTE UsageIndex, D3DDECLMETHOD Method)
{
	InsertVertexElement(Offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_BLENDWEIGHT, UsageIndex, Method);
}

Vector4 & D3DVertexElementSet::GetBlendWeight(void * pVertex, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_BLENDWEIGHT][UsageIndex].Type == D3DDECLTYPE_FLOAT4);

	return *GetVertexValue<Vector4>(pVertex, D3DDECLUSAGE_BLENDWEIGHT, UsageIndex);
}

void D3DVertexElementSet::SetBlendWeight(void * pVertex, const Vector4 & BlendWeight, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_BLENDWEIGHT][UsageIndex].Type == D3DDECLTYPE_FLOAT4);

	return SetVertexValue<Vector4>(pVertex, D3DDECLUSAGE_BLENDWEIGHT, UsageIndex, BlendWeight);
}

void D3DVertexElementSet::InsertBlendIndicesElement(WORD Offset, BYTE UsageIndex, D3DDECLMETHOD Method)
{
	InsertVertexElement(Offset, D3DDECLTYPE_UBYTE4, D3DDECLUSAGE_BLENDINDICES, UsageIndex, Method);
}

DWORD & D3DVertexElementSet::GetBlendIndices(void * pVertex, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_BLENDINDICES][UsageIndex].Type == D3DDECLTYPE_UBYTE4);

	return *GetVertexValue<DWORD>(pVertex, D3DDECLUSAGE_BLENDINDICES, UsageIndex);
}

void D3DVertexElementSet::SetBlendIndices(void * pVertex, const DWORD & BlendIndices, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_BLENDINDICES][UsageIndex].Type == D3DDECLTYPE_UBYTE4);

	return SetVertexValue<DWORD>(pVertex, D3DDECLUSAGE_BLENDINDICES, UsageIndex, BlendIndices);
}

void D3DVertexElementSet::InsertNormalElement(WORD Offset, BYTE UsageIndex, D3DDECLMETHOD Method)
{
	InsertVertexElement(Offset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL, UsageIndex, Method);
}

Vector3 & D3DVertexElementSet::GetNormal(void * pVertex, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_NORMAL][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

	return *GetVertexValue<Vector3>(pVertex, D3DDECLUSAGE_NORMAL, UsageIndex);
}

void D3DVertexElementSet::SetNormal(void * pVertex, const Vector3 & Normal, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_NORMAL][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

	return SetVertexValue<Vector3>(pVertex, D3DDECLUSAGE_NORMAL, UsageIndex, Normal);
}

void D3DVertexElementSet::InsertTexcoordElement(WORD Offset, BYTE UsageIndex, D3DDECLMETHOD Method)
{
	InsertVertexElement(Offset, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, UsageIndex, Method);
}

Vector2 & D3DVertexElementSet::GetTexcoord(void * pVertex, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_TEXCOORD][UsageIndex].Type == D3DDECLTYPE_FLOAT2);

	return *GetVertexValue<Vector2>(pVertex, D3DDECLUSAGE_TEXCOORD, UsageIndex);
}

void D3DVertexElementSet::SetTexcoord(void * pVertex, const Vector2 & Texcoord, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_TEXCOORD][UsageIndex].Type == D3DDECLTYPE_FLOAT2);

	return SetVertexValue<Vector2>(pVertex, D3DDECLUSAGE_TEXCOORD, UsageIndex, Texcoord);
}

void D3DVertexElementSet::InsertTangentElement(WORD Offset, BYTE UsageIndex, D3DDECLMETHOD Method)
{
	InsertVertexElement(Offset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_TANGENT, UsageIndex, Method);
}

Vector3 & D3DVertexElementSet::GetTangent(void * pVertex, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_TANGENT][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

	return *GetVertexValue<Vector3>(pVertex, D3DDECLUSAGE_TANGENT, UsageIndex);
}

void D3DVertexElementSet::SetTangent(void * pVertex, const Vector3 & Tangent, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_TANGENT][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

	return SetVertexValue<Vector3>(pVertex, D3DDECLUSAGE_TANGENT, UsageIndex, Tangent);
}

void D3DVertexElementSet::InsertBinormalElement(WORD Offset, BYTE UsageIndex, D3DDECLMETHOD Method)
{
	InsertVertexElement(Offset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_BINORMAL, UsageIndex, Method);
}

Vector3 & D3DVertexElementSet::GetBinormal(void * pVertex, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_BINORMAL][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

	return *GetVertexValue<Vector3>(pVertex, D3DDECLUSAGE_BINORMAL, UsageIndex);
}

void D3DVertexElementSet::SetBinormal(void * pVertex, const Vector3 & Binormal, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_BINORMAL][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

	return SetVertexValue<Vector3>(pVertex, D3DDECLUSAGE_BINORMAL, UsageIndex, Binormal);
}

void D3DVertexElementSet::InsertColorElement(WORD Offset, BYTE UsageIndex, D3DDECLMETHOD Method)
{
	InsertVertexElement(Offset, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR, UsageIndex, Method);
}

D3DCOLOR & D3DVertexElementSet::GetColor(void * pVertex, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_COLOR][UsageIndex].Type == D3DDECLTYPE_D3DCOLOR);

	return *GetVertexValue<D3DCOLOR>(pVertex, D3DDECLUSAGE_COLOR, UsageIndex);
}

void D3DVertexElementSet::SetColor(void * pVertex, const D3DCOLOR & Color, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_COLOR][UsageIndex].Type == D3DDECLTYPE_D3DCOLOR);

	return SetVertexValue<D3DCOLOR>(pVertex, D3DDECLUSAGE_COLOR, UsageIndex, Color);
}

void VertexBuffer::OnResetDevice(void)
{

}

void VertexBuffer::OnLostDevice(void)
{
	_ASSERT(!m_ptr || GetDesc().Pool == D3DPOOL_MANAGED);
}

void VertexBuffer::Create(IDirect3DVertexBuffer9 * ptr)
{
	_ASSERT(!m_ptr);

	m_ptr = ptr;
}

void VertexBuffer::CreateVertexBuffer(
	UINT Length,
	DWORD Usage,
	DWORD FVF,
	D3DPOOL Pool)
{
	IDirect3DVertexBuffer9 * pVB;
	if(FAILED(hr = my::D3DContext::getSingleton().m_d3dDevice->CreateVertexBuffer(Length, Usage, FVF, Pool, &pVB, 0)))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pVB);
}

void VertexBuffer::SaveVertexBuffer(const char* path)
{
	std::ofstream ofs(path);
	D3DVERTEXBUFFER_DESC desc = GetDesc();
	ofs.write((char*)Lock(0, 0, D3DLOCK_READONLY), desc.Size);
	ofs.flush();
}

D3DVERTEXBUFFER_DESC VertexBuffer::GetDesc(void)
{
	D3DVERTEXBUFFER_DESC desc;
	V(m_ptr->GetDesc(&desc));
	return desc;
}

void * VertexBuffer::Lock(UINT OffsetToLock, UINT SizeToLock, DWORD Flags)
{
	void * ret;
	V(m_ptr->Lock(OffsetToLock, SizeToLock, &ret, Flags));
	return ret;
}

void VertexBuffer::Unlock(void)
{
	V(m_ptr->Unlock());
}

void IndexBuffer::Create(IDirect3DIndexBuffer9 * ptr)
{
	_ASSERT(!m_ptr);

	m_ptr = ptr;
}

void IndexBuffer::CreateIndexBuffer(
	UINT Length,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool)
{
	IDirect3DIndexBuffer9 * pIB;
	if(FAILED(hr = my::D3DContext::getSingleton().m_d3dDevice->CreateIndexBuffer(Length, Usage, Format, Pool, &pIB, 0)))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pIB);
}

D3DINDEXBUFFER_DESC IndexBuffer::GetDesc(void)
{
	D3DINDEXBUFFER_DESC desc;
	V(m_ptr->GetDesc(&desc));
	return desc;
}

void * IndexBuffer::Lock(UINT OffsetToLock, UINT SizeToLock, DWORD Flags)
{
	void * ret;
	V(m_ptr->Lock(OffsetToLock, SizeToLock, &ret, Flags));
	return ret;
}

void IndexBuffer::Unlock(void)
{
	V(m_ptr->Unlock());
}

void Mesh::Create(ID3DXMesh * pMesh)
{
	_ASSERT(!m_ptr);

	m_ptr = pMesh;
}

void Mesh::CreateMesh(
	DWORD NumFaces,
	DWORD NumVertices,
	CONST LPD3DVERTEXELEMENT9 pDeclaration,
	DWORD Options)
{
	LPD3DXMESH pMesh = NULL;
	hr = D3DXCreateMesh(NumFaces, NumVertices, Options, pDeclaration, my::D3DContext::getSingleton().m_d3dDevice, &pMesh);
	if(FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pMesh);
}

void Mesh::CreateMeshFVF(
	DWORD NumFaces,
	DWORD NumVertices,
	DWORD FVF,
	DWORD Options)
{
	LPD3DXMESH pMesh = NULL;
	hr = D3DXCreateMeshFVF(NumFaces, NumVertices, Options, FVF, my::D3DContext::getSingleton().m_d3dDevice, &pMesh);
	if(FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pMesh);
}

void Mesh::CreateMeshFromX(
	LPCTSTR pFilename,
	DWORD Options,
	LPD3DXBUFFER * ppAdjacency,
	LPD3DXBUFFER * ppMaterials,
	LPD3DXBUFFER * ppEffectInstances,
	DWORD * pNumMaterials)
{
	LPD3DXMESH pMesh = NULL;
	hr = D3DXLoadMeshFromX(
		pFilename, Options, my::D3DContext::getSingleton().m_d3dDevice, ppAdjacency, ppMaterials, ppEffectInstances, pNumMaterials, &pMesh);
	if(FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pMesh);
}

void Mesh::CreateMeshFromXInMemory(
	LPCVOID Memory,
	DWORD SizeOfMemory,
	DWORD Options,
	LPD3DXBUFFER * ppAdjacency,
	LPD3DXBUFFER * ppMaterials,
	LPD3DXBUFFER * ppEffectInstances,
	DWORD * pNumMaterials)
{
	LPD3DXMESH pMesh = NULL;
	hr = D3DXLoadMeshFromXInMemory(
		Memory, SizeOfMemory, Options, my::D3DContext::getSingleton().m_d3dDevice, ppAdjacency, ppMaterials, ppEffectInstances, pNumMaterials, &pMesh);
	if(FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pMesh);
}

void Mesh::CreateBox(
	FLOAT Width,
	FLOAT Height,
	FLOAT Depth,
	LPD3DXBUFFER * ppAdjacency)
{
	LPD3DXMESH pMesh = NULL;
	hr = D3DXCreateBox(my::D3DContext::getSingleton().m_d3dDevice, Width, Height, Depth, &pMesh, ppAdjacency);
	if(FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pMesh);
}

void Mesh::CreateCylinder(
	FLOAT Radius1,
	FLOAT Radius2,
	FLOAT Length,
	UINT Slices,
	UINT Stacks,
	LPD3DXBUFFER * ppAdjacency)
{
	LPD3DXMESH pMesh = NULL;
	hr = D3DXCreateCylinder(my::D3DContext::getSingleton().m_d3dDevice, Radius1, Radius2, Length, Slices, Stacks, &pMesh, ppAdjacency);
	if(FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pMesh);
}

void Mesh::CreatePolygon(
	FLOAT Length,
	UINT Sides,
	LPD3DXBUFFER * ppAdjacency)
{
	LPD3DXMESH pMesh = NULL;
	hr = D3DXCreatePolygon(my::D3DContext::getSingleton().m_d3dDevice, Length, Sides, &pMesh, ppAdjacency);
	if(FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pMesh);
}

void Mesh::CreateSphere(
	FLOAT Radius,
	UINT Slices,
	UINT Stacks,
	LPD3DXBUFFER * ppAdjacency)
{
	LPD3DXMESH pMesh = NULL;
	hr = D3DXCreateSphere(my::D3DContext::getSingleton().m_d3dDevice, Radius, Slices, Stacks, &pMesh, ppAdjacency);
	if(FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pMesh);
}

void Mesh::CreateTeapot(
	LPD3DXBUFFER * ppAdjacency)
{
	LPD3DXMESH pMesh = NULL;
	hr = D3DXCreateTeapot(my::D3DContext::getSingleton().m_d3dDevice, &pMesh, ppAdjacency);
	if(FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pMesh);
}

void Mesh::CreateTorus(
	FLOAT InnerRadius,
	FLOAT OuterRadius,
	UINT Sides,
	UINT Rings,
	LPD3DXBUFFER * ppAdjacency)
{
	LPD3DXMESH pMesh = NULL;
	hr = D3DXCreateTorus(my::D3DContext::getSingleton().m_d3dDevice, InnerRadius, OuterRadius, Sides, Rings, &pMesh, ppAdjacency);
	if(FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pMesh);
}

CComPtr<ID3DXMesh> Mesh::CloneMesh(DWORD Options, CONST D3DVERTEXELEMENT9 * pDeclaration)
{
	CComPtr<ID3DXMesh> ret;
	V(m_ptr->CloneMesh(Options, pDeclaration, my::D3DContext::getSingleton().m_d3dDevice, &ret));
	return ret;
}

CComPtr<ID3DXMesh> Mesh::CloneMeshFVF(DWORD Options, DWORD FVF)
{
	CComPtr<ID3DXMesh> ret;
	V(m_ptr->CloneMeshFVF(Options, FVF, my::D3DContext::getSingleton().m_d3dDevice, &ret));
	return ret;
}

CComPtr<ID3DXMesh> Mesh::CleanMesh(D3DXCLEANTYPE CleanType, const DWORD *pAdjacencyIn, DWORD *pAdjacencyOut)
{
	CComPtr<ID3DXMesh> ret;
	CComPtr<ID3DXBuffer> ErrorMsgs;
	hr = D3DXCleanMesh(CleanType, m_ptr, pAdjacencyIn, &ret, pAdjacencyOut, &ErrorMsgs);
	if (FAILED(hr))
	{
		if (ErrorMsgs)
		{
			THROW_CUSEXCEPTION((char *)ErrorMsgs->GetBufferPointer());
		}
		THROW_D3DEXCEPTION(hr);
	}
	return ret;
}

CComPtr<ID3DXMesh> Mesh::SimplifyMesh(
	const DWORD *pAdjacency,
	DWORD MinValue,
	DWORD Options,
	const D3DXATTRIBUTEWEIGHTS *pVertexAttributeWeights,
	const FLOAT *pVertexWeights)
{
	std::vector<DWORD> outAdjacency(GetNumFaces() * 3);
	CComPtr<ID3DXMesh> mesh = CleanMesh(D3DXCLEAN_BOWTIES, pAdjacency, &outAdjacency[0]);
	CComPtr<ID3DXMesh> ret = NULL;
	hr = D3DXSimplifyMesh(mesh, &outAdjacency[0], pVertexAttributeWeights, pVertexWeights, MinValue, Options, &ret);
	if (FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}
	return ret;
}

void Mesh::ConvertAdjacencyToPointReps(CONST DWORD * pAdjacency, DWORD * pPRep)
{
	V(m_ptr->ConvertAdjacencyToPointReps(pAdjacency, pPRep));
}

void Mesh::ConvertPointRepsToAdjacency(CONST DWORD* pPRep, DWORD* pAdjacency)
{
	V(m_ptr->ConvertPointRepsToAdjacency(pPRep, pAdjacency));
}

void Mesh::DrawSubset(DWORD AttribId)
{
	V(m_ptr->DrawSubset(AttribId));
}

void Mesh::GenerateAdjacency(FLOAT Epsilon, DWORD * pAdjacency)
{
	V(m_ptr->GenerateAdjacency(Epsilon, pAdjacency));
}

void Mesh::GetAttributeTable(D3DXATTRIBUTERANGE * pAttribTable, DWORD * pAttribTableSize)
{
	V(m_ptr->GetAttributeTable(pAttribTable, pAttribTableSize));
}

DWORD Mesh::GetNumAttributes(void)
{
	DWORD AttribTableSize;
	GetAttributeTable(NULL, &AttribTableSize);
	return AttribTableSize;
}

void Mesh::GetDeclaration(D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE])
{
	V(m_ptr->GetDeclaration(Declaration));
}

CComPtr<IDirect3DDevice9> Mesh::GetDevice(void)
{
	CComPtr<IDirect3DDevice9> Device;
	V(m_ptr->GetDevice(&Device));
	return Device;
}

DWORD Mesh::GetFVF(void)
{
	return m_ptr->GetFVF();
}

CComPtr<IDirect3DIndexBuffer9> Mesh::GetIndexBuffer(void)
{
	CComPtr<IDirect3DIndexBuffer9> IndexBuffer;
	V(m_ptr->GetIndexBuffer(&IndexBuffer));
	return IndexBuffer;
}

DWORD Mesh::GetNumBytesPerVertex(void)
{
	return m_ptr->GetNumBytesPerVertex();
}

DWORD Mesh::GetNumFaces(void)
{
	return m_ptr->GetNumFaces();
}

DWORD Mesh::GetNumVertices(void)
{
	return m_ptr->GetNumVertices();
}

DWORD Mesh::GetOptions(void)
{
	return m_ptr->GetOptions();
}

CComPtr<IDirect3DVertexBuffer9> Mesh::GetVertexBuffer(void)
{
	CComPtr<IDirect3DVertexBuffer9> VertexBuffer;
	V(m_ptr->GetVertexBuffer(&VertexBuffer));
	return VertexBuffer;
}

LPVOID Mesh::LockIndexBuffer(DWORD Flags)
{
	LPVOID pData = NULL;
	if(FAILED(hr = m_ptr->LockIndexBuffer(Flags, &pData)))
	{
		THROW_D3DEXCEPTION(hr);
	}
	return pData;
}

LPVOID Mesh::LockVertexBuffer(DWORD Flags)
{
	LPVOID pData = NULL;
	if(FAILED(hr = m_ptr->LockVertexBuffer(Flags, &pData)))
	{
		THROW_D3DEXCEPTION(hr);
	}
	return pData;
}

void Mesh::UnlockIndexBuffer(void)
{
	V(m_ptr->UnlockIndexBuffer());
}

void Mesh::UnlockVertexBuffer(void)
{
	V(m_ptr->UnlockVertexBuffer());
}

void Mesh::UpdateSemantics(D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE])
{
	V(m_ptr->UpdateSemantics(Declaration));
}

DWORD * Mesh::LockAttributeBuffer(DWORD Flags)
{
	DWORD * pData = NULL;
	if(FAILED(hr = m_ptr->LockAttributeBuffer(Flags, &pData)))
	{
		THROW_D3DEXCEPTION(hr);
	}
	return pData;
}

CComPtr<ID3DXMesh> Mesh::Optimize(
	DWORD Flags,
	CONST DWORD * pAdjacencyIn,
	DWORD * pAdjacencyOut,
	DWORD * pFaceRemap,
	LPD3DXBUFFER * ppVertexRemap)
{
	CComPtr<ID3DXMesh> ret;
	V(m_ptr->Optimize(Flags, pAdjacencyIn, pAdjacencyOut, pFaceRemap, ppVertexRemap, &ret));
	return ret;
}

void Mesh::OptimizeInplace(DWORD Flags,
	CONST DWORD * pAdjacencyIn,
	DWORD * pAdjacencyOut,
	DWORD * pFaceRemap,
	LPD3DXBUFFER * ppVertexRemap)
{
	V(m_ptr->OptimizeInplace(Flags, pAdjacencyIn, pAdjacencyOut, pFaceRemap, ppVertexRemap));
}

void Mesh::SetAttributeTable(CONST D3DXATTRIBUTERANGE * pAttribTable, DWORD cAttribTableSize)
{
	V(m_ptr->SetAttributeTable(pAttribTable, cAttribTableSize));
}

void Mesh::UnlockAttributeBuffer(void)
{
	V(m_ptr->UnlockAttributeBuffer());
}

DWORD Mesh::GetAttributeIdFromInternalFaceIndex(unsigned int face_i)
{
	boost::multi_array_ref<DWORD, 1> attribids(LockAttributeBuffer(D3DLOCK_READONLY), boost::extents[GetNumFaces()]);
	unsigned int ret = attribids[face_i];
	UnlockAttributeBuffer();
	return ret;
}

void Mesh::ComputeDualQuaternionSkinnedVertices(
	void * pDstVertices,
	DWORD NumVerts,
	DWORD DstVertexStride,
	const D3DVertexElementSet & DstVertexElems,
	void * pSrcVertices,
	DWORD SrcVertexStride,
	const D3DVertexElementSet & SrcVertexElems,
	const Matrix4 * DualQuaternions,
	DWORD DualQuaternionSize)
{
	for (unsigned int i = 0; i < NumVerts; i++)
	{
		unsigned char * pDstVertex = (unsigned char *)pDstVertices + i * DstVertexStride;
		unsigned char * pSrcVertex = (unsigned char *)pSrcVertices + i * SrcVertexStride;
		DstVertexElems.SetPosition(pDstVertex, TransformList::TransformVertexWithDualQuaternion(
			SrcVertexElems.GetPosition(pSrcVertex), TransformList::BuildSkinnedDualQuaternion(
				DualQuaternions, DualQuaternionSize, SrcVertexElems.GetBlendIndices(pSrcVertex), SrcVertexElems.GetBlendWeight(pSrcVertex))));
	}
}

void Mesh::ComputeNormalFrame(
	void * pVertices,
	DWORD NumVerts,
	DWORD VertexStride,
	const void * pIndices,
	bool bIndices16,
	DWORD NumFaces,
	const D3DVertexElementSet & VertexElems)
{
	std::vector<Vector3> FNormals(NumFaces);
	for (unsigned int face_i = 0; face_i < NumFaces; face_i++)
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

		const Vector3 & v1 = VertexElems.GetPosition(pv1);
		const Vector3 & v2 = VertexElems.GetPosition(pv2);
		const Vector3 & v3 = VertexElems.GetPosition(pv3);

		FNormals[face_i] = (v2 - v1).cross(v3 - v1).normalize(Vector3(1, 0, 0));
	}

	for (unsigned int vert_i = 0; vert_i < NumVerts; vert_i++)
	{
		unsigned char * pVertex = (unsigned char *)pVertices + vert_i * VertexStride;
		VertexElems.SetNormal(pVertex, Vector3::zero);
	}

	for (unsigned int face_i = 0; face_i < NumFaces; face_i++)
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

		const Vector3 & v1 = VertexElems.GetPosition(pv1);
		const Vector3 & v2 = VertexElems.GetPosition(pv2);
		const Vector3 & v3 = VertexElems.GetPosition(pv3);

		VertexElems.GetNormal(pv1) += FNormals[face_i] * (v3 - v1).angle(v2 - v1);
		VertexElems.GetNormal(pv2) += FNormals[face_i] * (v3 - v2).angle(v1 - v2);
		VertexElems.GetNormal(pv3) += FNormals[face_i] * (v1 - v3).angle(v2 - v3);
	}

	for (unsigned int vert_i = 0; vert_i < NumVerts; vert_i++)
	{
		unsigned char * pVertex = (unsigned char *)pVertices + vert_i * VertexStride;
		VertexElems.SetNormal(pVertex, VertexElems.GetNormal(pVertex).normalize(Vector3(1, 0, 0)));
	}
}

void Mesh::ComputeTangentFrame(
	void * pVertices,
	DWORD NumVerts,
	DWORD VertexStride,
	const void * pIndices,
	bool bIndices16,
	DWORD NumFaces,
	const D3DVertexElementSet & VertexElems)
{
	std::vector<Vector3> tan1(NumVerts, Vector3::zero);
	std::vector<Vector3> tan2(NumVerts, Vector3::zero);
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

		const Vector3 & v1 = VertexElems.GetPosition(pv1);
		const Vector3 & v2 = VertexElems.GetPosition(pv2);
		const Vector3 & v3 = VertexElems.GetPosition(pv3);

		const Vector2 & w1 = VertexElems.GetTexcoord(pv1);
		const Vector2 & w2 = VertexElems.GetTexcoord(pv2);
		const Vector2 & w3 = VertexElems.GetTexcoord(pv3);

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float d = s1 * t2 - s2 * t1;
		float r = fabs(d) <= EPSILON_E12 ? 1.0f : 1.0F / d;
		Vector3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
		Vector3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

		tan1[i1] += sdir;
		tan1[i2] += sdir;
		tan1[i3] += sdir;

		tan2[i1] += tdir;
		tan2[i2] += tdir;
		tan2[i3] += tdir;
	}

	for(DWORD vertex_i = 0; vertex_i < NumVerts; vertex_i++)
	{
		unsigned char * pVertex = (unsigned char *)pVertices + vertex_i * VertexStride;
		const Vector3 & n = VertexElems.GetNormal(pVertex);
		const Vector3 & t = tan1[vertex_i];

		// Gram-Schmidt orthogonalize
		Vector3 perp = n.perpendicular(t);
		float l = perp.magnitude();
		VertexElems.SetTangent(pVertex, l > EPSILON_E12 ? perp / l : Vector3(0, 0, 1), 0);
	}
}

RayResult Mesh::RayTest(
	const Ray& ray,
	void* pVertices,
	DWORD NumVerts,
	DWORD VertexStride,
	void* pIndices,
	bool bIndices16,
	DWORD FaceStart,
	DWORD NumFaces,
	const D3DVertexElementSet& VertexElems)
{
	RayResult ret(false, FLT_MAX);
	for (unsigned int face_i = FaceStart; face_i < FaceStart + NumFaces; face_i++)
	{
		int i0 = bIndices16 ? *((WORD*)pIndices + face_i * 3 + 0) : *((DWORD*)pIndices + face_i * 3 + 0);
		int i1 = bIndices16 ? *((WORD*)pIndices + face_i * 3 + 1) : *((DWORD*)pIndices + face_i * 3 + 1);
		int i2 = bIndices16 ? *((WORD*)pIndices + face_i * 3 + 2) : *((DWORD*)pIndices + face_i * 3 + 2);

		const Vector3& v0 = VertexElems.GetPosition((unsigned char*)pVertices + i0 * VertexStride);
		const Vector3& v1 = VertexElems.GetPosition((unsigned char*)pVertices + i1 * VertexStride);
		const Vector3& v2 = VertexElems.GetPosition((unsigned char*)pVertices + i2 * VertexStride);

		if (IntersectionTests::isValidTriangle(v0, v1, v2))
		{
			RayResult result = CollisionDetector::rayAndTriangle(ray.p, ray.d, v0, v1, v2);
			if (result.first && result.second < ret.second)
			{
				ret = result;
			}
		}
	}
	return ret;
}

bool Mesh::FrustumTest(
	const Frustum& frustum,
	void* pVertices,
	DWORD NumVerts,
	DWORD VertexStride,
	void* pIndices,
	bool bIndices16,
	DWORD FaceStart,
	DWORD NumFaces,
	const D3DVertexElementSet& VertexElems)
{
	for (unsigned int face_i = FaceStart; face_i < FaceStart + NumFaces; face_i++)
	{
		int i0 = bIndices16 ? *((WORD*)pIndices + face_i * 3 + 0) : *((DWORD*)pIndices + face_i * 3 + 0);
		int i1 = bIndices16 ? *((WORD*)pIndices + face_i * 3 + 1) : *((DWORD*)pIndices + face_i * 3 + 1);
		int i2 = bIndices16 ? *((WORD*)pIndices + face_i * 3 + 2) : *((DWORD*)pIndices + face_i * 3 + 2);

		const Vector3& v0 = VertexElems.GetPosition((unsigned char*)pVertices + i0 * VertexStride);
		const Vector3& v1 = VertexElems.GetPosition((unsigned char*)pVertices + i1 * VertexStride);
		const Vector3& v2 = VertexElems.GetPosition((unsigned char*)pVertices + i2 * VertexStride);

		if (IntersectionTests::isValidTriangle(v0, v1, v2))
		{
			IntersectionTests::IntersectionType result = IntersectionTests::IntersectTriangleAndFrustum(v0, v1, v2, frustum);
			if (result == IntersectionTests::IntersectionTypeInside)
			{
				return true;
			}
			else if (result == IntersectionTests::IntersectionTypeIntersect)
			{
				return true;
			}
		}
	}
	return false;
}

void OgreMesh::CreateMeshFromOgreXmlInFile(
	LPCTSTR pFilename,
	bool bComputeTangentFrame,
	DWORD dwMeshOptions,
	unsigned int reserveVertices,
	unsigned int reserveFaces)
{
	CachePtr cache = FileIStream::Open(pFilename)->GetWholeCache();
	cache->push_back(0);

	CreateMeshFromOgreXmlInMemory((char *)&(*cache)[0], cache->size(), bComputeTangentFrame, dwMeshOptions, reserveVertices, reserveFaces);
}

void OgreMesh::CreateMeshFromOgreXmlInMemory(
	LPSTR pSrcData,
	UINT srcDataLen,
	bool bComputeTangentFrame,
	DWORD dwMeshOptions,
	unsigned int reserveVertices,
	unsigned int reserveFaces)
{
	_ASSERT(0 == pSrcData[srcDataLen-1]);

	rapidxml::xml_document<char> doc;
	try
	{
		doc.parse<0>(pSrcData);
	}
	catch(rapidxml::parse_error & e)
	{
		THROW_CUSEXCEPTION(e.what());
	}

	CreateMeshFromOgreXml(&doc, bComputeTangentFrame, dwMeshOptions, reserveVertices, reserveFaces);
}

void OgreMesh::CreateMeshFromOgreXml(
	const rapidxml::xml_node<char> * node_root,
	bool bComputeTangentFrame,
	DWORD dwMeshOptions,
	unsigned int reserveVertices,
	unsigned int reserveFaces)
{
	DEFINE_XML_NODE_SIMPLE(mesh, root);
	DEFINE_XML_NODE_SIMPLE(submeshes, mesh);
	DEFINE_XML_NODE_SIMPLE(submesh, submeshes);
	rapidxml::xml_node<char>* node_sharedgeometry = node_mesh->first_node("sharedgeometry");
	rapidxml::xml_node<char>* node_vertexbuffer = NULL, * node_boneassignments = NULL;
	bool positions = false, normals = false, colours_diffuse = false;
	int texture_coords = 0;
	rapidxml::xml_attribute<char>* attr_positions = NULL, * attr_normals = NULL, * attr_colours_diffuse = NULL, * attr_texture_coords = NULL;
	unsigned int total_vertices = 0, total_faces = 0;
	if (node_sharedgeometry)
	{
		DEFINE_XML_ATTRIBUTE_INT_SIMPLE(vertexcount, sharedgeometry);
		total_vertices += vertexcount;

		DEFINE_XML_NODE_SIMPLE(vertexbuffer, sharedgeometry);
		node_boneassignments = node_mesh->first_node("boneassignments");
		DEFINE_XML_ATTRIBUTE_BOOL(positions, attr_positions, node_vertexbuffer, positions);
		DEFINE_XML_ATTRIBUTE_BOOL(normals, attr_normals, node_vertexbuffer, normals);
		DEFINE_XML_ATTRIBUTE_BOOL(colours_diffuse, attr_colours_diffuse, node_vertexbuffer, colours_diffuse);
		DEFINE_XML_ATTRIBUTE_INT(texture_coords, attr_texture_coords, node_vertexbuffer, texture_coords);

		for (int submesh_i = 0; node_submesh != NULL; node_submesh = node_submesh->next_sibling(), submesh_i++)
		{
			DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(use32bitindexes, submesh);
			DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(usesharedvertices, submesh);
			if (!usesharedvertices)
			{
				THROW_CUSEXCEPTION("invalid usesharedvertices");
			}

			rapidxml::xml_attribute<char>* attr_operationtype = node_submesh->first_attribute("operationtype");
			if (attr_operationtype && 0 != _stricmp(attr_operationtype->value(), "triangle_list"))
			{
				THROW_CUSEXCEPTION("!triangle_list");
			}

			DEFINE_XML_NODE_SIMPLE(faces, submesh);
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(count, faces);
			D3DXATTRIBUTERANGE rang;
			rang.AttribId = submesh_i;
			rang.VertexStart = 0;
			rang.VertexCount = vertexcount;
			rang.FaceStart = total_faces;
			rang.FaceCount = count;
			m_AttribTable.push_back(rang);

			total_faces += count;
		}
	}
	else
	{
		DEFINE_XML_NODE_SIMPLE(geometry, submesh);
		DEFINE_XML_NODE_SIMPLE(vertexbuffer, geometry);
		node_boneassignments = node_submesh->first_node("boneassignments");
		DEFINE_XML_ATTRIBUTE_BOOL(positions, attr_positions, node_vertexbuffer, positions);
		DEFINE_XML_ATTRIBUTE_BOOL(normals, attr_normals, node_vertexbuffer, normals);
		DEFINE_XML_ATTRIBUTE_BOOL(colours_diffuse, attr_colours_diffuse, node_vertexbuffer, colours_diffuse);
		DEFINE_XML_ATTRIBUTE_INT(texture_coords, attr_texture_coords, node_vertexbuffer, texture_coords);

		for (int submesh_i = 0; node_submesh != NULL; node_submesh = node_submesh->next_sibling(), submesh_i++)
		{
			DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(use32bitindexes, submesh);
			DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(usesharedvertices, submesh);
			if (usesharedvertices)
			{
				THROW_CUSEXCEPTION("invalid usesharedvertices");
			}

			rapidxml::xml_attribute<char>* attr_operationtype = node_submesh->first_attribute("operationtype");
			if (attr_operationtype && 0 != _stricmp(attr_operationtype->value(), "triangle_list"))
			{
				THROW_CUSEXCEPTION("!triangle_list");
			}

			DEFINE_XML_NODE(node_geometry, node_submesh, geometry);
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(vertexcount, geometry);

			DEFINE_XML_NODE_SIMPLE(faces, submesh);
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(count, faces);
			D3DXATTRIBUTERANGE rang;
			rang.AttribId = submesh_i;
			rang.VertexStart = total_vertices;
			rang.VertexCount = vertexcount;
			rang.FaceStart = total_faces;
			rang.FaceCount = count;
			m_AttribTable.push_back(rang);

			total_vertices += vertexcount;
			total_faces += count;
		}
	}

	if (!(dwMeshOptions & D3DXMESH_32BIT) && (Max(reserveVertices, total_vertices) >= USHRT_MAX || Max(reserveFaces, total_faces) >= USHRT_MAX))
	{
		D3DContext::getSingleton().m_EventLog("facecount overflow ( >= 65535 )");
		dwMeshOptions |= D3DXMESH_32BIT;
	}

	if (!positions)
	{
		THROW_CUSEXCEPTION("cannot process non-position vertex");
	}

	m_VertexElems.InsertPositionElement(0);
	WORD offset = sizeof(Vector3);

	if (normals || bComputeTangentFrame)
	{
		m_VertexElems.InsertNormalElement(offset);
		offset += sizeof(Vector3);
	}

	if (bComputeTangentFrame)
	{
		m_VertexElems.InsertTangentElement(offset);
		offset += sizeof(Vector3);
	}

	if (colours_diffuse)
	{
		m_VertexElems.InsertColorElement(offset);
		offset += sizeof(D3DCOLOR);
	}

	if (texture_coords > D3DVertexElementSet::MAX_USAGE_INDEX)
	{
		texture_coords = D3DVertexElementSet::MAX_USAGE_INDEX;
	}

	for (int i = 0; i < texture_coords; i++)
	{
		m_VertexElems.InsertTexcoordElement(offset, i);
		offset += sizeof(Vector2);
	}

	WORD indicesOffset = 0, weightsOffset = 0;
	if (node_boneassignments != NULL && node_boneassignments->first_node("vertexboneassignment"))
	{
		m_VertexElems.InsertBlendIndicesElement(offset);
		offset += sizeof(DWORD);

		m_VertexElems.InsertBlendWeightElement(offset);
		offset += sizeof(Vector4);
	}

	std::vector<D3DVERTEXELEMENT9> velist = m_VertexElems.BuildVertexElementList(0);
	D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
	velist.push_back(ve_end);

	D3DContext::getSingleton().m_d3dDeviceSec.Enter();
	if (FAILED(hr = my::D3DContext::getSingleton().m_d3dDevice->CreateVertexDeclaration(velist.data(), &m_Decl)))
	{
		THROW_D3DEXCEPTION(hr);
	}
	D3DContext::getSingleton().m_d3dDeviceSec.Leave();

	D3DContext::getSingleton().m_d3dDeviceSec.Enter();
	CreateMesh(Max(reserveFaces, total_faces), Max(reserveVertices, total_vertices), velist.data(), dwMeshOptions);
	D3DContext::getSingleton().m_d3dDeviceSec.Leave();

	D3DContext::getSingleton().m_d3dDeviceSec.Enter();
	VOID* pVertices = LockVertexBuffer();
	D3DContext::getSingleton().m_d3dDeviceSec.Leave();

	int vertex_i = 0;
	DEFINE_XML_NODE(node_submesh, node_submeshes, submesh);
	for (int submesh_i = 0; node_submesh != NULL; node_submesh = node_submesh->next_sibling(), submesh_i++)
	{
		const int start_vertex_i = vertex_i;
		if (node_sharedgeometry)
		{
			DEFINE_XML_NODE(node_vertexbuffer, node_sharedgeometry, vertexbuffer);
			node_boneassignments = node_mesh->first_node("boneassignments");
		}
		else
		{
			DEFINE_XML_NODE_SIMPLE(geometry, submesh);
			DEFINE_XML_NODE(node_vertexbuffer, node_geometry, vertexbuffer);
			node_boneassignments = node_submesh->first_node("boneassignments");
		}

		DEFINE_XML_NODE_SIMPLE(vertex, vertexbuffer);
		for (; node_vertex != NULL; node_vertex = node_vertex->next_sibling(), vertex_i++)
		{
			_ASSERT(vertex_i < total_vertices);
			unsigned char* pVertex = (unsigned char*)pVertices + vertex_i * offset;
			if (positions)
			{
				DEFINE_XML_NODE_SIMPLE(position, vertex);
				Vector3& Position = m_VertexElems.GetPosition(pVertex);
				rapidxml::xml_attribute<char>* attr_tmp;
				DEFINE_XML_ATTRIBUTE_FLOAT(Position.x, attr_tmp, node_position, x);
				DEFINE_XML_ATTRIBUTE_FLOAT(Position.y, attr_tmp, node_position, y);
				DEFINE_XML_ATTRIBUTE_FLOAT(Position.z, attr_tmp, node_position, z);
			}

			if (normals)
			{
				DEFINE_XML_NODE_SIMPLE(normal, vertex);
				Vector3& Normal = m_VertexElems.GetNormal(pVertex);
				rapidxml::xml_attribute<char>* attr_tmp;
				DEFINE_XML_ATTRIBUTE_FLOAT(Normal.x, attr_tmp, node_normal, x);
				DEFINE_XML_ATTRIBUTE_FLOAT(Normal.y, attr_tmp, node_normal, y);
				DEFINE_XML_ATTRIBUTE_FLOAT(Normal.z, attr_tmp, node_normal, z);
			}

			if (colours_diffuse)
			{
				DEFINE_XML_NODE_SIMPLE(colour_diffuse, vertex);
				DEFINE_XML_ATTRIBUTE_SIMPLE(value, colour_diffuse);
				const char* color_value = attr_value->value();
				std::vector<std::string> color_set;
				boost::algorithm::split(color_set, color_value, boost::is_any_of(" "), boost::algorithm::token_compress_off);
				D3DXCOLOR Color(
					boost::lexical_cast<float>(color_set[0]),
					boost::lexical_cast<float>(color_set[1]),
					boost::lexical_cast<float>(color_set[2]),
					boost::lexical_cast<float>(color_set[3]));
				m_VertexElems.SetColor(pVertex, Color);
			}

			rapidxml::xml_node<char>* node_texcoord = node_vertex->first_node("texcoord");
			for (int i = 0; i < texture_coords && node_texcoord != NULL; i++, node_texcoord = node_texcoord->next_sibling())
			{
				Vector2& Texcoord = m_VertexElems.GetTexcoord(pVertex, i);
				rapidxml::xml_attribute<char>* attr_tmp;
				DEFINE_XML_ATTRIBUTE_FLOAT(Texcoord.x, attr_tmp, node_texcoord, u);
				DEFINE_XML_ATTRIBUTE_FLOAT(Texcoord.y, attr_tmp, node_texcoord, v);
			}

			if (node_boneassignments != NULL && node_boneassignments->first_node("vertexboneassignment"))
			{
				m_VertexElems.SetBlendIndices(pVertex, 0);
				m_VertexElems.SetBlendWeight(pVertex, Vector4::zero);
			}
		}

		if (node_boneassignments != NULL)
		{
			rapidxml::xml_node<char>* node_vertexboneassignment = node_boneassignments->first_node("vertexboneassignment");
			for (; node_vertexboneassignment != NULL; node_vertexboneassignment = node_vertexboneassignment->next_sibling())
			{
				DEFINE_XML_ATTRIBUTE_INT_SIMPLE(vertexindex, vertexboneassignment);
				DEFINE_XML_ATTRIBUTE_INT_SIMPLE(boneindex, vertexboneassignment);
				DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(weight, vertexboneassignment);

				if (start_vertex_i + vertexindex >= total_vertices)
				{
					THROW_CUSEXCEPTION(str_printf("invalid vertex index: %d", vertexindex));
				}

				if (boneindex >= 0xff)
				{
					THROW_CUSEXCEPTION(str_printf("invalid bone index: %d", boneindex));
				}

				unsigned char* pVertex = (unsigned char*)pVertices + (start_vertex_i + vertexindex) * offset;
				unsigned char* pIndices = (unsigned char*)&m_VertexElems.GetBlendIndices(pVertex);
				float* pWeights = (float*)&m_VertexElems.GetBlendWeight(pVertex);

				int i = std::distance(pWeights, boost::find_if(boost::make_iterator_range_n(
					pWeights, D3DVertexElementSet::MAX_BONE_INDICES), boost::bind(std::equal_to<float>(), boost::placeholders::_1, 0.0f)));
				if (i < D3DVertexElementSet::MAX_BONE_INDICES)
				{
					pIndices[i] = boneindex;
					pWeights[i] = weight;
				}
				else
				{
					THROW_CUSEXCEPTION("too much bone assignment");
				}
			}
		}

		if (node_sharedgeometry)
		{
			break;
		}
	}

	D3DContext::getSingleton().m_d3dDeviceSec.Enter();
	UnlockVertexBuffer();
	D3DContext::getSingleton().m_d3dDeviceSec.Leave();

	D3DContext::getSingleton().m_d3dDeviceSec.Enter();
	VOID* pIndices = LockIndexBuffer();
	DWORD* pAttrBuffer = LockAttributeBuffer();
	D3DContext::getSingleton().m_d3dDeviceSec.Leave();

	DEFINE_XML_NODE(node_submesh, node_submeshes, submesh);
	int face_i = 0;
	for (int submesh_i = 0; node_submesh != NULL; node_submesh = node_submesh->next_sibling(), submesh_i++)
	{
		DEFINE_XML_NODE_SIMPLE(faces, submesh);
		DEFINE_XML_ATTRIBUTE_INT_SIMPLE(count, faces);
		DEFINE_XML_NODE_SIMPLE(face, faces);

		D3DXATTRIBUTERANGE& rang = m_AttribTable[submesh_i];
		int vmin = INT_MAX, vmax = 0;
		for (; node_face != NULL && face_i < total_faces; node_face = node_face->next_sibling(), face_i++)
		{
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(v1, face);
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(v2, face);
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(v3, face);

			_ASSERT(!node_sharedgeometry || rang.VertexStart == 0);
			v1 += rang.VertexStart;
			v2 += rang.VertexStart;
			v3 += rang.VertexStart;

			if (dwMeshOptions & D3DXMESH_32BIT)
			{
				*((DWORD*)pIndices + face_i * 3 + 0) = v1;
				*((DWORD*)pIndices + face_i * 3 + 1) = v2;
				*((DWORD*)pIndices + face_i * 3 + 2) = v3;
			}
			else
			{
				*((WORD*)pIndices + face_i * 3 + 0) = v1;
				*((WORD*)pIndices + face_i * 3 + 1) = v2;
				*((WORD*)pIndices + face_i * 3 + 2) = v3;
			}
			pAttrBuffer[face_i] = submesh_i;
			vmin = Min(vmin, Min(v1, Min(v2, v3)));
			vmax = Max(vmax, Max(v1, Max(v2, v3)));
			_ASSERT(vmax < total_vertices);
		}
		rang.VertexStart = vmin;
		rang.VertexCount = vmax - vmin + 1;
	}

	D3DContext::getSingleton().m_d3dDeviceSec.Enter();
	UnlockAttributeBuffer();
	UnlockIndexBuffer();
	D3DContext::getSingleton().m_d3dDeviceSec.Leave();

	D3DContext::getSingleton().m_d3dDeviceSec.Enter();
	SetAttributeTable(&m_AttribTable[0], m_AttribTable.size());
	D3DContext::getSingleton().m_d3dDeviceSec.Leave();

	m_Vb.Create(GetVertexBuffer().Detach());
	m_Ib.Create(GetIndexBuffer().Detach());

	if (bComputeTangentFrame)
	{
		//std::vector<DWORD> adjacency(GetNumFaces() * 3);
		//D3DContext::getSingleton().m_d3dDeviceSec.Enter();
		//GenerateAdjacency((float)EPSILON_E6, &adjacency[0]);
		//D3DContext::getSingleton().m_d3dDeviceSec.Leave();

		//DWORD dwOptions = D3DXTANGENT_GENERATE_IN_PLACE;
		//if(!normals)
		//	dwOptions |= D3DXTANGENT_CALCULATE_NORMALS;
		//D3DContext::getSingleton().m_d3dDeviceSec.Enter();
		//hr = D3DXComputeTangentFrameEx(
		//	m_ptr, D3DDECLUSAGE_TEXCOORD, 0, D3DDECLUSAGE_TANGENT, 0, D3DX_DEFAULT, 0, D3DDECLUSAGE_NORMAL, 0, dwOptions, &adjacency[0], -1.01f, -0.01f, -1.01f, NULL, NULL);
		//D3DContext::getSingleton().m_d3dDeviceSec.Leave();
		//if(FAILED(hr))
		//{
		//	THROW_D3DEXCEPTION(hr);
		//}

		D3DContext::getSingleton().m_d3dDeviceSec.Enter();
		VOID* pVertices = LockVertexBuffer();
		VOID* pIndices = LockIndexBuffer();
		D3DContext::getSingleton().m_d3dDeviceSec.Leave();

		ComputeTangentFrame(
			pVertices, total_vertices, offset, pIndices, !(dwMeshOptions& D3DXMESH_32BIT), total_faces, m_VertexElems);

		D3DContext::getSingleton().m_d3dDeviceSec.Enter();
		UnlockVertexBuffer();
		UnlockIndexBuffer();
		D3DContext::getSingleton().m_d3dDeviceSec.Leave();
	}

	//std::vector<DWORD> adjacency(GetNumFaces() * 3);
	//D3DContext::getSingleton().m_d3dDeviceSec.Enter();
	//GenerateAdjacency((float)EPSILON_E6, &adjacency[0]);
	//D3DContext::getSingleton().m_d3dDeviceSec.Leave();

	//D3DContext::getSingleton().m_d3dDeviceSec.Enter();
	//OptimizeInplace(D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, &adjacency[0], &m_Adjacency[0], NULL, NULL);
	//D3DContext::getSingleton().m_d3dDeviceSec.Leave();
}
//
//void OgreMesh::CreateMeshFromObjInFile(
//	LPCTSTR pFilename,
//	bool bComputeTangentFrame,
//	DWORD dwMeshOptions)
//{
//	my::IStreamBuff<char> buff(my::FileIStream::Open(pFilename));
//	std::istream ifs(&buff);
//	CreateMeshFromObjInStream(ifs, bComputeTangentFrame, dwMeshOptions);
//}
//
//void OgreMesh::CreateMeshFromObjInStream(
//	std::istream & is,
//	bool bComputeTangentFrame,
//	DWORD dwMeshOptions)
//{
//	//                1    2      3             4      5             6      7
//	boost::regex reg("(v\\s(-?\\d+(\\.\\d+)?)\\s(-?\\d+(\\.\\d+)?)\\s(-?\\d+(\\.\\d+)?))"
//		//8     9      10            11     12
//		"|(vt\\s(-?\\d+(\\.\\d+)?)\\s(-?\\d+(\\.\\d+)?))"
//		//13   14     15       16     17       18     19
//		"|(f\\s(\\d+)/(\\d+)\\s(\\d+)/(\\d+)\\s(\\d+)/(\\d+))");
//
//	char line[2048];
//	boost::match_results<const char*> what;
//	std::vector<Vector3> verts;
//	std::vector<Vector2> uvs;
//	std::vector<int> faces;
//	while (!is.eof())
//	{
//		is.getline(line, _countof(line));
//		if (boost::regex_search(line, what, reg, boost::match_default))
//		{
//			if (what[1].matched)
//			{
//				Vector3 vert(boost::lexical_cast<float>(what[2]), boost::lexical_cast<float>(what[4]), boost::lexical_cast<float>(what[6]));
//				verts.push_back(vert);
//			}
//			else if (what[8].matched)
//			{
//				Vector2 uv(boost::lexical_cast<float>(what[9]), boost::lexical_cast<float>(what[11]));
//				uvs.push_back(uv);
//			}
//			else if (what[13].matched)
//			{
//				int face[] = {
//					boost::lexical_cast<int>(what[14]) - 1,
//					boost::lexical_cast<int>(what[16]) - 1,
//					boost::lexical_cast<int>(what[18]) - 1 };
//				faces.insert(faces.end(), &face[0], &face[3]);
//			}
//		}
//	}
//
//	if (!verts.empty() && verts.size() == uvs.size())
//	{
//		_ASSERT(m_VertexElems.elems[D3DDECLUSAGE_POSITION][0].Type == D3DDECLTYPE_UNUSED);
//		m_VertexElems.InsertPositionElement(0);
//		WORD offset = sizeof(Vector3);
//
//		m_VertexElems.InsertTexcoordElement(offset, 0);
//		offset += sizeof(Vector2);
//
//		m_VertexElems.InsertNormalElement(offset, 0);
//		offset += sizeof(Vector3);
//
//		if (bComputeTangentFrame)
//		{
//			m_VertexElems.InsertTangentElement(offset, 0);
//			offset += sizeof(Vector3);
//		}
//
//		std::vector<D3DVERTEXELEMENT9> velist = m_VertexElems.BuildVertexElementList(0);
//		D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
//		velist.push_back(ve_end);
//
//		if (!(dwMeshOptions & D3DXMESH_32BIT) && verts.size() >= USHRT_MAX)
//		{
//			//THROW_CUSEXCEPTION("facecount overflow ( >= 65535 )");
//			dwMeshOptions |= D3DXMESH_32BIT;
//		}
//
//		CreateMesh(faces.size() / 3, verts.size(), velist.data(), dwMeshOptions);
//
//		VOID* pVertices = LockVertexBuffer();
//		VOID* pIndices = LockIndexBuffer();
//		DWORD* pAttrBuffer = LockAttributeBuffer();
//
//		for (int i = 0; i < verts.size(); i++)
//		{
//			unsigned char* pVertex = (unsigned char*)pVertices + i * offset;
//			m_VertexElems.SetPosition(pVertex, verts[i]);
//			m_VertexElems.SetTexcoord(pVertex, uvs[i]);
//		}
//
//		int vmin = INT_MAX, vmax = 0;
//		for (int i = 0; i < faces.size() / 3; i++)
//		{
//			if (dwMeshOptions & D3DXMESH_32BIT)
//			{
//				*((DWORD*)pIndices + i * 3 + 0) = faces[i * 3 + 0];
//				*((DWORD*)pIndices + i * 3 + 1) = faces[i * 3 + 1];
//				*((DWORD*)pIndices + i * 3 + 2) = faces[i * 3 + 2];
//			}
//			else
//			{
//				*((WORD*)pIndices + i * 3 + 0) = faces[i * 3 + 0];
//				*((WORD*)pIndices + i * 3 + 1) = faces[i * 3 + 1];
//				*((WORD*)pIndices + i * 3 + 2) = faces[i * 3 + 2];
//			}
//			pAttrBuffer[i] = 0;
//			vmin = Min(vmin, Min(faces[i * 3 + 0], Min(faces[i * 3 + 1], faces[i * 3 + 2])));
//			vmax = Max(vmax, Max(faces[i * 3 + 0], Max(faces[i * 3 + 1], faces[i * 3 + 2])));
//		}
//		m_MaterialNameList.push_back("aaa");
//
//		ComputeNormalFrame(
//			pVertices, GetNumVertices(), GetNumBytesPerVertex(), pIndices, !(dwMeshOptions & D3DXMESH_32BIT), GetNumFaces(), m_VertexElems);
//
//		if (bComputeTangentFrame)
//		{
//			ComputeTangentFrame(
//				pVertices, GetNumVertices(), GetNumBytesPerVertex(), pIndices, !(dwMeshOptions & D3DXMESH_32BIT), GetNumFaces(), m_VertexElems);
//		}
//
//		UnlockVertexBuffer();
//		UnlockIndexBuffer();
//		UnlockAttributeBuffer();
//
//		D3DXATTRIBUTERANGE rang = { 0, 0, faces.size() / 3, vmin, vmax - vmin + 1 };
//		m_AttribTable.push_back(rang);
//		SetAttributeTable(&m_AttribTable[0], m_AttribTable.size());
//
//		//std::vector<DWORD> adjacency(GetNumFaces() * 3);
//		//GenerateAdjacency((float)EPSILON_E6, &adjacency[0]);
//		//m_Adjacency.resize(adjacency.size());
//		//OptimizeInplace(D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, &adjacency[0], &m_Adjacency[0], NULL, NULL);
//
//		//DWORD AttribTblCount = 0;
//		//GetAttributeTable(NULL, &AttribTblCount);
//		//m_AttribTable.resize(AttribTblCount);
//		//GetAttributeTable(&m_AttribTable[0], &AttribTblCount);
//	}
//}

void OgreMesh::CreateMeshFromOther(OgreMesh* other, DWORD AttribId, const Matrix4& trans, const Matrix4& uv_trans, unsigned int reserveVertices, unsigned int reserveFaces)
{
	DWORD dwMeshOptions = other->GetOptions();
	if (!(dwMeshOptions & D3DXMESH_32BIT) && Max<DWORD>(reserveVertices, other->GetNumVertices()) >= USHRT_MAX)
	{
		dwMeshOptions |= D3DXMESH_32BIT;
	}

	m_VertexElems = other->m_VertexElems;

	std::vector<D3DVERTEXELEMENT9> velist = m_VertexElems.BuildVertexElementList(0);
	D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
	velist.push_back(ve_end);

	if (FAILED(hr = my::D3DContext::getSingleton().m_d3dDevice->CreateVertexDeclaration(velist.data(), &m_Decl)))
	{
		THROW_D3DEXCEPTION(hr);
	}

	CreateMesh(Max<DWORD>(reserveFaces, other->GetNumFaces()), Max<DWORD>(reserveVertices, other->GetNumVertices()), velist.data(), dwMeshOptions);

	m_Vb.Create(GetVertexBuffer().Detach());
	m_Ib.Create(GetIndexBuffer().Detach());

	AppendMesh(other, AttribId, trans, uv_trans);
}

void OgreMesh::AppendMesh(OgreMesh* other, DWORD AttribId, const Matrix4& trans, const Matrix4& uv_trans)
{
	D3DXATTRIBUTERANGE rang = { m_AttribTable.size(), 0, 0, 0, 0 };
	for (int i = 0; i < m_AttribTable.size(); i++)
	{
		rang.VertexStart = Max(rang.VertexStart, m_AttribTable[i].VertexStart + m_AttribTable[i].VertexCount);
		rang.FaceStart = Max(rang.FaceStart, m_AttribTable[i].FaceStart + m_AttribTable[i].FaceCount);
	}
	AppendToAttrib(rang, other, AttribId, trans, uv_trans);

	rang.FaceCount = other->m_AttribTable[AttribId].FaceCount;
	rang.VertexCount = other->m_AttribTable[AttribId].VertexCount;
	m_AttribTable.push_back(rang);
	SetAttributeTable(&m_AttribTable[0], m_AttribTable.size());
}

void OgreMesh::CombineMesh(OgreMesh* other, DWORD AttribId, const Matrix4& trans, const Matrix4& uv_trans)
{
	if (m_AttribTable.empty())
	{
		THROW_CUSEXCEPTION("OgreMesh::CombineMesh: m_AttribTable is empty");
	}
	D3DXATTRIBUTERANGE& rang = m_AttribTable.back();
	AppendToAttrib(rang, other, AttribId, trans, uv_trans);

	rang.FaceCount += other->m_AttribTable[AttribId].FaceCount;
	rang.VertexCount += other->m_AttribTable[AttribId].VertexCount;
	SetAttributeTable(&m_AttribTable[0], m_AttribTable.size());
}

const D3DXATTRIBUTERANGE& OgreMesh::AppendToAttrib(const D3DXATTRIBUTERANGE& rang, OgreMesh* other, DWORD AttribId, const Matrix4& trans, const Matrix4& uv_trans)
{
	if (rang.VertexStart + rang.VertexCount + other->m_AttribTable[AttribId].VertexCount > GetNumVertices()
		|| rang.FaceStart + rang.FaceCount + other->m_AttribTable[AttribId].FaceCount > GetNumFaces())
	{
		THROW_CUSEXCEPTION(str_printf("OgreMesh::AppendToAttrib: vertex(%u + %u) or face(%u + %u) overflow",
			rang.VertexStart + rang.VertexCount, other->m_AttribTable[AttribId].VertexCount, rang.FaceStart + rang.FaceCount, other->m_AttribTable[AttribId].FaceCount));
	}

	VOID* pVertices = LockVertexBuffer();
	VOID* pOtherVertices = other->LockVertexBuffer();
	for (int i = 0; i < other->m_AttribTable[AttribId].VertexCount; i++)
	{
		unsigned char* pVertex = (unsigned char*)pVertices + (rang.VertexStart + rang.VertexCount + i) * GetNumBytesPerVertex();
		unsigned char* pOtherVertex = (unsigned char*)pOtherVertices + (other->m_AttribTable[AttribId].VertexStart + i) * other->GetNumBytesPerVertex();
		m_VertexElems.SetPosition(pVertex, other->m_VertexElems.GetPosition(pOtherVertex).transformCoord(trans));

		if (m_VertexElems.elems[D3DDECLUSAGE_NORMAL][0].Type != D3DDECLTYPE_UNUSED)
		{
			m_VertexElems.SetNormal(pVertex, other->m_VertexElems.GetNormal(pOtherVertex).transformNormal(trans));
		}

		if (m_VertexElems.elems[D3DDECLUSAGE_TANGENT][0].Type != D3DDECLTYPE_UNUSED)
		{
			m_VertexElems.SetTangent(pVertex, other->m_VertexElems.GetTangent(pOtherVertex).transformNormal(trans));
		}

		if (m_VertexElems.elems[D3DDECLUSAGE_COLOR][0].Type != D3DDECLTYPE_UNUSED)
		{
			m_VertexElems.SetColor(pVertex, other->m_VertexElems.GetColor(pOtherVertex));
		}

		for (int j = 0; j < D3DVertexElementSet::MAX_USAGE_INDEX; j++)
		{
			if (m_VertexElems.elems[D3DDECLUSAGE_TEXCOORD][j].Type != D3DDECLTYPE_UNUSED)
			{
				m_VertexElems.SetTexcoord(pVertex, other->m_VertexElems.GetTexcoord(pOtherVertex, j).transformCoord(uv_trans), j);
			}
		}
	}
	other->UnlockVertexBuffer();
	UnlockVertexBuffer();

	VOID* pIndices = LockIndexBuffer();
	VOID* pOtherIndices = other->LockIndexBuffer();
	DWORD* pAttrBuffer = LockAttributeBuffer();
	for (int i = 0; i < other->m_AttribTable[AttribId].FaceCount; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			int idx = (other->GetOptions() & D3DXMESH_32BIT) ?
				*((DWORD*)pOtherIndices + (other->m_AttribTable[AttribId].FaceStart + i) * 3 + j) :
				*((WORD*)pOtherIndices + (other->m_AttribTable[AttribId].FaceStart + i) * 3 + j);
			idx = idx - other->m_AttribTable[AttribId].VertexStart + rang.VertexStart + rang.VertexCount;
			if (GetOptions() & D3DXMESH_32BIT)
			{
				*((DWORD*)pIndices + (rang.FaceStart + rang.FaceCount + i) * 3 + j) = idx;
			}
			else
			{
				*((WORD*)pIndices + (rang.FaceStart + rang.FaceCount + i) * 3 + j) = idx;
			}
		}
		pAttrBuffer[rang.FaceStart + rang.FaceCount + i] = rang.AttribId;
	}
	other->UnlockIndexBuffer();
	UnlockIndexBuffer();
	UnlockAttributeBuffer();
	return rang;
}

void OgreMesh::AppendProgressiveMesh(ProgressiveMesh* pmesh)
{
	_ASSERT(GetNumBytesPerVertex() == pmesh->m_Mesh->GetNumBytesPerVertex());

	DWORD FaceStart = 0;
	for (int i = 0; i < m_AttribTable.size(); i++)
	{
		const D3DXATTRIBUTERANGE & rang = m_AttribTable[i];
		if (rang.FaceStart + rang.FaceCount > FaceStart)
		{
			FaceStart = rang.FaceStart + rang.FaceCount;
		}
	}

	DWORD NumFaces = pmesh->GetNumFaces();
	if (FaceStart + NumFaces > GetNumFaces())
	{
		THROW_CUSEXCEPTION(str_printf("OgreMesh::AppendProgressiveMesh: face(%u + %u) overflow", FaceStart, NumFaces));
	}

	VOID* pIndices = LockIndexBuffer();
	DWORD* pAttrBuffer = LockAttributeBuffer();
	int face_i = FaceStart, face_start = FaceStart;
	for (int i = 0; i < pmesh->m_NumAttribs; i++, face_start = face_i)
	{
		D3DXATTRIBUTERANGE rang = m_AttribTable[i];
		rang.AttribId = m_AttribTable.size();

		std::vector<ProgressiveMesh::PMTriangle>::iterator tri_iter = pmesh->m_Tris.begin();
		for (; tri_iter != pmesh->m_Tris.end(); tri_iter++)
		{
			if (tri_iter->AttribId == i)
			{
				if (GetOptions() & D3DXMESH_32BIT)
				{
					boost::multi_array_ref<DWORD, 1> idx((DWORD*)pIndices, boost::extents[(FaceStart + NumFaces) * 3]);
					idx[face_i * 3 + 0] = tri_iter->vi[0];
					idx[face_i * 3 + 1] = tri_iter->vi[1];
					idx[face_i * 3 + 2] = tri_iter->vi[2];
				}
				else
				{
					boost::multi_array_ref<WORD, 1> idx((WORD*)pIndices, boost::extents[(FaceStart + NumFaces) * 3]);
					idx[face_i * 3 + 0] = tri_iter->vi[0];
					idx[face_i * 3 + 1] = tri_iter->vi[1];
					idx[face_i * 3 + 2] = tri_iter->vi[2];
				}
				pAttrBuffer[face_i++] = rang.AttribId;
			}
		}
		rang.FaceStart = face_start;
		rang.FaceCount = face_i - face_start;
		m_AttribTable.push_back(rang);
	}
	_ASSERT(face_i - FaceStart == NumFaces);
	UnlockIndexBuffer();
	UnlockAttributeBuffer();

	SetAttributeTable(&m_AttribTable[0], m_AttribTable.size());
}

void OgreMesh::SaveOgreMesh(LPCTSTR path, bool useSharedGeom)
{
	std::ofstream ofs(path);
	_ASSERT(ofs.is_open());
	// start mesh description
	ofs << "<mesh>\n";
	void* pVertices = LockVertexBuffer();
	DWORD VertexStride = GetNumBytesPerVertex();
	// write shared geometry (if used)
	unsigned int vertexcount = 0;
	if (useSharedGeom)
	{
		for (int i = 0; i < m_AttribTable.size(); i++)
		{
			if (m_AttribTable[i].VertexStart + m_AttribTable[i].VertexCount > vertexcount)
			{
				vertexcount = m_AttribTable[i].VertexStart + m_AttribTable[i].VertexCount;
			}
		}
		ofs << "\t<sharedgeometry vertexcount=\"" << vertexcount << "\">\n";
		ofs << "\t\t<vertexbuffer positions=\"true\" normals=";
		bool normals = m_VertexElems.elems[D3DDECLUSAGE_NORMAL][0].Type == D3DDECLTYPE_FLOAT3;
		if (normals)
			ofs << "\"true\"";
		else
			ofs << "\"false\"";
		ofs << " colours_diffuse=";
		bool colours_diffuse = m_VertexElems.elems[D3DDECLUSAGE_COLOR][0].Type == D3DDECLTYPE_D3DCOLOR;
		if (colours_diffuse)
			ofs << "\"true\"";
		else
			ofs << "\"false\"";
		ofs << " colours_specular=\"false\" texture_coords=\"";
		unsigned int texture_coords = m_VertexElems.CalcTextureCoords();
		if (texture_coords)
			ofs << texture_coords << "\">\n";
		else
			ofs << 0 << "\">\n";
		// write vertex data
		for (DWORD i = 0; i < vertexcount; i++)
		{
			ofs << "\t\t\t<vertex>\n";
			//write vertex position
			unsigned char* pVertex = (unsigned char*)pVertices + i * VertexStride;
			const Vector3& vertex = m_VertexElems.GetPosition(pVertex);
			ofs << "\t\t\t\t<position x=\"" << vertex.x << "\" y=\"" << vertex.y << "\" " << "z=\"" << vertex.z << "\"/>\n";
			//write vertex normal
			if (normals)
			{
				const Vector3& normal = m_VertexElems.GetNormal(pVertex);
				ofs << "\t\t\t\t<normal x=\"" << normal.x << "\" y=\"" << normal.y << "\" " << "z=\"" << normal.z << "\"/>\n";
			}
			//write vertex color
			if (colours_diffuse)
			{
				const D3DXCOLOR color(m_VertexElems.GetColor(pVertex));
				ofs << "\t\t\t\t<colour_diffuse value=\"" << color.r << " " << color.g << " " << color.b << " " << color.a << "\"/>\n";
			}
			//write vertex texture coordinates
			if (texture_coords)
			{
				for (DWORD j = 0; j < texture_coords; j++)
				{
					const Vector2 texcoord = m_VertexElems.GetTexcoord(pVertex, (BYTE)j);
					ofs << "\t\t\t\t<texcoord u=\"" << texcoord.x << "\" v=\"" << texcoord.y << "\"/>\n";
				}
			}
			ofs << "\t\t\t</vertex>\n";
		}
		ofs << "\t\t</vertexbuffer>\n";
		ofs << "\t</sharedgeometry>\n";
	}
	VOID * pIndices = LockIndexBuffer();
	// write submeshes data
	ofs << "\t<submeshes>\n";
	for (DWORD i=0; i < m_AttribTable.size(); i++)
	{
		// Start submesh description
		ofs << "\t\t<submesh ";
		//// Write material name
		//ofs << "material=\"" << m_MaterialNameList[m_AttribTable[i].AttribId] << "\" ";
		DWORD use32bitindexes = GetOptions() & D3DXMESH_32BIT;
		// Write use32bitIndexes flag
		ofs << "use32bitindexes=\"";
		if (use32bitindexes)
			ofs << "true";
		else
			ofs << "false";
		ofs << "\" ";
		// Write use32bitIndexes flag
		ofs << "usesharedvertices=\"";
		if (useSharedGeom)
			ofs << "true";
		else
			ofs << "false";
		ofs << "\" ";
		// Write operation type flag
		ofs << "operationtype=\"triangle_list\">\n";

		// Write submesh polygons
		ofs << "\t\t\t<faces count=\"" << m_AttribTable[i].FaceCount << "\">\n";
		for (DWORD j = m_AttribTable[i].FaceStart; j < m_AttribTable[i].FaceStart + m_AttribTable[i].FaceCount; j++)
		{
			int v[3];
			for (int k = 0; k < 3; k++)
			{
				if (use32bitindexes)
					v[k] = *((DWORD*)pIndices + j * 3 + k);
				else
					v[k] = *((WORD*)pIndices + j * 3 + k);

				if (!useSharedGeom)
					v[k] -= m_AttribTable[i].VertexStart;
			}
			ofs << "\t\t\t\t<face v1=\"" << v[0] << "\" v2=\"" << v[1] << "\" "
				<< "v3=\"" << v[2] << "\"/>\n";
		}
		ofs << "\t\t\t</faces>\n";

		// Write mesh geometry
		if (!useSharedGeom)
		{
			ofs << "\t\t\t<geometry vertexcount=\"" << m_AttribTable[i].VertexCount
				<< "\" transform=\"0 0 0 0 0 0 1 1 1\">\n";
			ofs << "\t\t\t\t<vertexbuffer positions=\"true\" normals=";
			bool normals = m_VertexElems.elems[D3DDECLUSAGE_NORMAL][0].Type == D3DDECLTYPE_FLOAT3;
			if (normals)
				ofs << "\"true\"";
			else
				ofs << "\"false\"";
			ofs << " colours_diffuse=";
			bool colours_diffuse = m_VertexElems.elems[D3DDECLUSAGE_COLOR][0].Type == D3DDECLTYPE_D3DCOLOR;
			if (colours_diffuse)
				ofs << "\"true\"";
			else
				ofs << "\"false\"";
			ofs << " colours_specular=\"false\" texture_coords=\"";
			unsigned int texture_coords = m_VertexElems.CalcTextureCoords();
			if (texture_coords)
				ofs << texture_coords << "\">\n";
			else
				ofs << 0 << "\">\n";
			//write vertex data
			for (int j = m_AttribTable[i].VertexStart; j < m_AttribTable[i].VertexStart + m_AttribTable[i].VertexCount; j++)
			{
				ofs << "\t\t\t\t\t<vertex>\n";
				//write vertex position
				unsigned char* pVertex = (unsigned char*)pVertices + j * VertexStride;
				const Vector3 vertex = m_VertexElems.GetPosition(pVertex);
				ofs << "\t\t\t\t\t\t<position x=\"" << vertex.x << "\" y=\""
					<< vertex.y << "\" " << "z=\"" << vertex.z << "\"/>\n";
				//write vertex normal
				if (normals)
				{
					const Vector3 normal = m_VertexElems.GetNormal(pVertex);
					ofs << "\t\t\t\t\t\t<normal x=\"" << normal.x << "\" y=\""
						<< normal.y << "\" " << "z=\"" << normal.z << "\"/>\n";
				}
				//write vertex colour
				if (colours_diffuse)
				{
					const D3DXCOLOR color(m_VertexElems.GetColor(pVertex));
					ofs << "\t\t\t\t\t\t<colour_diffuse value=\"" << color.r << " " << color.g
						<< " " << color.b << " " << color.a << "\"/>\n";
				}//write vertex texture coordinates
				if (texture_coords)
				{
					for (int k = 0; k < texture_coords; k++)
					{
						const Vector2 texcoord = m_VertexElems.GetTexcoord(pVertex, (BYTE)k);
						ofs << "\t\t\t\t\t\t<texcoord u=\"" << texcoord.x << "\" v=\"" <<
							texcoord.y << "\"/>\n";
					}
				}
				ofs << "\t\t\t\t\t</vertex>\n";
			}
			//end vertex data
			ofs << "\t\t\t\t</vertexbuffer>\n";
			//end geometry description
			ofs << "\t\t\t</geometry>\n";

			// Write bone assignments
			ofs << "\t\t\t<boneassignments>\n";
			if (m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
			{
				for (int j = 0; j < m_AttribTable[i].VertexCount; j++)
				{
					unsigned char* pVertex = (unsigned char*)pVertices + m_AttribTable[i].VertexStart * VertexStride + j * VertexStride;
					for (int k = 0; k < D3DVertexElementSet::MAX_BONE_INDICES; k++)
					{
						if (m_VertexElems.GetBlendWeight(pVertex)[k] > 0.001)
						{
							ofs << "\t\t\t\t<vertexboneassignment vertexindex=\"" << j
								<< "\" boneindex=\"" << (int)((unsigned char*)&m_VertexElems.GetBlendIndices(pVertex))[k] - m_AttribTable[i].VertexStart << "\" weight=\""
								<< m_VertexElems.GetBlendWeight(pVertex)[k] << "\"/>\n";
						}
					}
				}
			}
			ofs << "\t\t\t</boneassignments>\n";
		}
		// End submesh description
		ofs << "\t\t</submesh>\n";
	}
	UnlockIndexBuffer();
	ofs << "\t</submeshes>\n";
	//// write skeleton link
	//ofs << "\t<skeletonlink name=\"" << m_skeletonlink << "\"/>\n";
	// Write shared geometry bone assignments
	if (useSharedGeom)
	{
		ofs << "\t<boneassignments>\n";
		if (m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
		{
			for (DWORD i = 0; i < vertexcount; i++)
			{
				unsigned char* pVertex = (unsigned char*)pVertices + i * VertexStride;
				unsigned char* pIndices = (unsigned char*)&m_VertexElems.GetBlendIndices(pVertex);
				float* pWeights = (float*)&m_VertexElems.GetBlendWeight(pVertex);
				for (int j = 0; j < D3DVertexElementSet::MAX_BONE_INDICES; j++)
				{
					if (pWeights[j] > 0)
					{
						ofs << "\t\t<vertexboneassignment vertexindex=\"" << i << "\" boneindex=\"" << (int)pIndices[j] << "\" weight=\"" << pWeights[j] << "\"/>\n";
					}
				}
			}
		}
		ofs << "\t</boneassignments>\n";
	}
	UnlockVertexBuffer();
	// write submesh names
	ofs << "\t<submeshnames>\n";
	for (int i = 0; i < m_AttribTable.size(); i++)
	{
		ofs << "\t\t<submeshname name=\"submesh" << i << "\" index=\"" << i << "\"/>\n";
	}
	ofs << "\t</submeshnames>\n";
	// end mesh description
	ofs << "</mesh>\n";
}

boost::shared_ptr<OgreMesh> OgreMesh::Optimize(DWORD Flags)
{
	_ASSERT(!(Flags & (D3DXMESH_32BIT | D3DXMESH_IB_WRITEONLY | D3DXMESH_WRITEONLY)));

	std::vector<DWORD> adjacency(GetNumFaces() * 3);
	GenerateAdjacency((float)EPSILON_E6, &adjacency[0]);
	std::vector<DWORD> adjacency_out(GetNumFaces() * 3);
	OgreMeshPtr optimized_mesh(new OgreMesh());
	optimized_mesh->Create(Mesh::Optimize(Flags, &adjacency[0], &adjacency_out[0]).Detach());
	//optimized_mesh->m_Adjacency = m_Adjacency;
	optimized_mesh->m_VertexElems = m_VertexElems;
	DWORD AttribTblCount = 0;
	optimized_mesh->GetAttributeTable(NULL, &AttribTblCount);
	optimized_mesh->m_AttribTable.resize(AttribTblCount);
	optimized_mesh->GetAttributeTable(&optimized_mesh->m_AttribTable[0], &AttribTblCount);
	return optimized_mesh;
}

void OgreMesh::OptimizeInplace(DWORD Flags)
{
	_ASSERT(!(Flags & (D3DXMESH_32BIT | D3DXMESH_IB_WRITEONLY | D3DXMESH_WRITEONLY)));

	std::vector<DWORD> adjacency(GetNumFaces() * 3);
	GenerateAdjacency((float)EPSILON_E6, &adjacency[0]);
	std::vector<DWORD> adjacency_out(GetNumFaces() * 3);
	Mesh::OptimizeInplace(Flags, &adjacency[0], &adjacency_out[0]);
	DWORD AttribTblCount = 0;
	GetAttributeTable(NULL, &AttribTblCount);
	m_AttribTable.resize(AttribTblCount);
	GetAttributeTable(&m_AttribTable[0], &AttribTblCount);
}

boost::shared_ptr<OgreMesh> OgreMesh::SimplifyMesh(DWORD MinValue, DWORD Options)
{
	std::vector<DWORD> adjacency(GetNumFaces() * 3);
	GenerateAdjacency((float)EPSILON_E6, &adjacency[0]);
	OgreMeshPtr simplified_mesh(new OgreMesh());
	simplified_mesh->Create(Mesh::SimplifyMesh(&adjacency[0], MinValue, Options).Detach());
	//simplified_mesh->m_Adjacency = m_Adjacency;
	simplified_mesh->m_VertexElems = m_VertexElems;
	DWORD AttribTblCount = 0;
	simplified_mesh->GetAttributeTable(NULL, &AttribTblCount);
	simplified_mesh->m_AttribTable.resize(AttribTblCount);
	simplified_mesh->GetAttributeTable(&simplified_mesh->m_AttribTable[0], &AttribTblCount);
	return simplified_mesh;
}

void OgreMesh::SaveObj(const char* path)
{
	std::ofstream ofs(path);
	_ASSERT(ofs.is_open());
	void* pVertices = LockVertexBuffer();
	DWORD VertexStride = GetNumBytesPerVertex();
	boost::unordered_map<boost::fusion::tuple<float, float, float>, int> verts, texcoords, normals;
	for (int i = 0; i < GetNumVertices(); i++)
	{
		unsigned char* pVertex = (unsigned char*)pVertices + i * VertexStride;
		const Vector3& vertex = m_VertexElems.GetPosition(pVertex);
		if (verts.insert(std::make_pair(boost::fusion::make_tuple(vertex.x, vertex.y, vertex.z), (int)verts.size() + 1)).second)
		{
			ofs << "v " << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
		}
	}

	for (int i = 0; i < GetNumVertices(); i++)
	{
		unsigned char* pVertex = (unsigned char*)pVertices + i * VertexStride;
		const Vector2& texcoord = m_VertexElems.GetTexcoord(pVertex);
		if (texcoords.insert(std::make_pair(boost::fusion::make_tuple(texcoord.x, texcoord.y, 1.0f), (int)texcoords.size() + 1)).second)
		{
			ofs << "vt " << texcoord.x << " " << texcoord.y << std::endl;
		}
	}

	for (int i = 0; i < GetNumVertices(); i++)
	{
		unsigned char* pVertex = (unsigned char*)pVertices + i * VertexStride;
		const Vector3& normal = m_VertexElems.GetNormal(pVertex);
		if (normals.insert(std::make_pair(boost::fusion::make_tuple(normal.x, normal.y, normal.z), (int)normals.size() + 1)).second)
		{
			ofs << "vn " << normal.x << " " << normal.y << " " << normal.z << std::endl;
		}
	}

	VOID* pIndices = LockIndexBuffer();
	for (int i = 0; i < m_AttribTable.size(); i++)
	{
		ofs << "g " << "aaa" << i << std::endl;

		D3DXATTRIBUTERANGE& rang = m_AttribTable[i];
		for (int j = rang.FaceStart; j < rang.FaceStart + rang.FaceCount; j++)
		{
			ofs << "f";
			for (int k = 0; k < 3; k++)
			{
				int vertex_i;
				if (GetOptions() & D3DXMESH_32BIT)
					vertex_i = *((DWORD*)pIndices + j * 3 + k);
				else
					vertex_i = *((WORD*)pIndices + j * 3 + k);
				unsigned char* pVertex = (unsigned char*)pVertices + vertex_i * VertexStride;
				const Vector3& vertex = m_VertexElems.GetPosition(pVertex);
				const Vector2& texcoord = m_VertexElems.GetTexcoord(pVertex);
				const Vector3& normal = m_VertexElems.GetNormal(pVertex);
				ofs << " " << verts[boost::fusion::make_tuple(vertex.x, vertex.y, vertex.z)]
					<< "/" << texcoords[boost::fusion::make_tuple(texcoord.x, texcoord.y, 1.0f)]
					<< "/" << normals[boost::fusion::make_tuple(normal.x, normal.y, normal.z)];
			}
			ofs << std::endl;
		}
	}

	UnlockVertexBuffer();
	UnlockIndexBuffer();
}

void OgreMesh::Transform(const Matrix4 & trans)
{
	VOID * pVertices = LockVertexBuffer();
	DWORD NumVertices = GetNumVertices();
	DWORD VertexStride = GetNumBytesPerVertex();
	for (int vertex_i = 0; vertex_i < (int)NumVertices; vertex_i++)
	{
		for (int UsageIndex = 0; UsageIndex < 1; UsageIndex++)
		{
			unsigned char * pVertex = (unsigned char *)pVertices + vertex_i * VertexStride;
			if (m_VertexElems.elems[D3DDECLUSAGE_POSITION][UsageIndex].Type == D3DDECLTYPE_FLOAT3)
			{
				Vector3 & Position = m_VertexElems.GetPosition(pVertex, UsageIndex);
				Position = Position.transformCoord(trans);
			}

			if (m_VertexElems.elems[D3DDECLUSAGE_NORMAL][UsageIndex].Type == D3DDECLTYPE_FLOAT3)
			{
				Vector3 & Normal = m_VertexElems.GetNormal(pVertex, UsageIndex);
				Normal = Normal.transformNormal(trans);
			}

			if (m_VertexElems.elems[D3DDECLUSAGE_BINORMAL][UsageIndex].Type == D3DDECLTYPE_FLOAT3)
			{
				Vector3 & Binormal = m_VertexElems.GetBinormal(pVertex, UsageIndex);
				Binormal = Binormal.transformNormal(trans);
			}

			if (m_VertexElems.elems[D3DDECLUSAGE_TANGENT][UsageIndex].Type == D3DDECLTYPE_FLOAT3)
			{
				Vector3 & Tangent = m_VertexElems.GetTangent(pVertex, UsageIndex);
				Tangent = Tangent.transformNormal(trans);
			}
		}
	}
}

AABB OgreMesh::CalculateAABB(DWORD AttribId)
{
	const D3DXATTRIBUTERANGE& rang = m_AttribTable[AttribId];
	void* pVertices = LockVertexBuffer(D3DLOCK_READONLY);
	DWORD VertexStride = GetNumBytesPerVertex();
	AABB ret = AABB::Invalid();
	for (DWORD i = rang.VertexStart; i < rang.VertexStart + rang.VertexCount; i++)
	{
		unsigned char* pVertex = (unsigned char*)pVertices + i * VertexStride;
		const Vector3 & vertex = m_VertexElems.GetPosition(pVertex);
		ret.unionSelf(vertex);
	}
	UnlockVertexBuffer();
	return ret;
}

ProgressiveMesh::ProgressiveMesh(OgreMesh* Mesh, DWORD NumAttribs)
	: m_Mesh(Mesh)
	, m_NumAttribs(Min(Mesh->GetNumAttributes(), NumAttribs))
	, m_Verts(Mesh->GetNumVertices())
{
	VOID* pIndices = m_Mesh->LockIndexBuffer(D3DLOCK_READONLY);
	for (int i = 0; i < m_Mesh->GetNumFaces(); i++)
	{
		PMTriangle tri = { {
			m_Mesh->GetOptions() & D3DXMESH_32BIT ? *((DWORD*)pIndices + i * 3 + 0) : *((WORD*)pIndices + i * 3 + 0),
			m_Mesh->GetOptions() & D3DXMESH_32BIT ? *((DWORD*)pIndices + i * 3 + 1) : *((WORD*)pIndices + i * 3 + 1),
			m_Mesh->GetOptions() & D3DXMESH_32BIT ? *((DWORD*)pIndices + i * 3 + 2) : *((WORD*)pIndices + i * 3 + 2) }, -1 };
		m_Tris.push_back(tri);
	}
	m_Mesh->UnlockIndexBuffer();

	for (int i = 0; i < m_NumAttribs; i++)
	{
		const D3DXATTRIBUTERANGE& rang = m_Mesh->m_AttribTable[i];
		for (int j = rang.FaceStart; j < rang.FaceStart + rang.FaceCount; j++)
		{
			_ASSERT(m_Tris[j].AttribId < 0);
			m_Tris[j].AttribId = i;

			for (int k = 0; k < _countof(m_Tris[j].vi); k++)
			{
				PMVertex& pmv = m_Verts[m_Tris[j].vi[k]];
				_ASSERT(pmv.tris.end() == std::find(pmv.tris.begin(), pmv.tris.end(), j));
				pmv.tris.push_back(j);
			}
		}
	}

	typedef std::map<boost::tuple<float, float, float>, boost::shared_ptr<std::vector<Plane> > > pos_plane_map;
	pos_plane_map ppmap;

	VOID* pVertices = m_Mesh->LockVertexBuffer();
	std::vector<PMVertex>::iterator vert_iter = m_Verts.begin();
	for (; vert_iter != m_Verts.end(); vert_iter++)
	{
		const Vector3& pos = m_Mesh->m_VertexElems.GetPosition((unsigned char*)pVertices + std::distance(m_Verts.begin(), vert_iter) * m_Mesh->GetNumBytesPerVertex());
		std::pair<pos_plane_map::iterator, bool> res = ppmap.insert(std::make_pair(
			boost::make_tuple(pos.x, pos.y, pos.z), boost::shared_ptr<std::vector<Plane> >(new std::vector<Plane>)));
		vert_iter->planes = res.first->second;

		std::vector<int>::iterator tri_iter = vert_iter->tris.begin();
		for (; tri_iter != vert_iter->tris.end(); tri_iter++)
		{
			const Vector3 v[3] = {
				m_Mesh->m_VertexElems.GetPosition((unsigned char*)pVertices + m_Tris[*tri_iter].vi[0] * m_Mesh->GetNumBytesPerVertex()),
				m_Mesh->m_VertexElems.GetPosition((unsigned char*)pVertices + m_Tris[*tri_iter].vi[1] * m_Mesh->GetNumBytesPerVertex()),
				m_Mesh->m_VertexElems.GetPosition((unsigned char*)pVertices + m_Tris[*tri_iter].vi[2] * m_Mesh->GetNumBytesPerVertex()) };

			vert_iter->planes->push_back(Plane::FromTriangle(v[0], v[1], v[2]));
		}
	}
	m_Mesh->UnlockVertexBuffer();

	ppmap.clear(); // ! release plane.use_count

	for (vert_iter = m_Verts.begin(); vert_iter != m_Verts.end(); vert_iter++)
	{
		UpdateCollapseCost(vert_iter);
	}
}

void ProgressiveMesh::UpdateCollapseCost(std::vector<PMVertex>::iterator vert_iter)
{
	vert_iter->neighbors.clear();
	std::vector<int>::iterator tri_iter = vert_iter->tris.begin();
	for (; tri_iter != vert_iter->tris.end(); tri_iter++)
	{
		PMTriangle& tri = m_Tris[*tri_iter];
		for (int i = 0; i < _countof(tri.vi); i++)
		{
			if (tri.vi[i] != std::distance(m_Verts.begin(), vert_iter))
			{
				std::pair<std::map<int, int>::iterator, bool> res = vert_iter->neighbors.insert(std::make_pair(tri.vi[i], 1));
				if (!res.second)
				{
					res.first->second++;
				}
			}
		}
	}

	vert_iter->isBorder = vert_iter->neighbors.end() != std::find_if(vert_iter->neighbors.begin(),
		vert_iter->neighbors.end(), boost::lambda::bind(&std::map<int, int>::value_type::second, boost::lambda::_1) <= 1);
	vert_iter->collapsecost = FLT_MAX;
	vert_iter->collapseto = -1;
	VOID* pVertices = m_Mesh->LockVertexBuffer();
	std::map<int, int>::iterator nei_iter = vert_iter->neighbors.begin();
	for (; nei_iter != vert_iter->neighbors.end(); nei_iter++)
	{
		PMVertex& neivert = m_Verts[nei_iter->first];
		// https://github.com/OGRECave/ogre/blob/v1-8-1/OgreMain/src/OgreProgressiveMesh.cpp#L1083
		// merged border only collapse to higher merged border
		if (!vert_iter->isBorder || nei_iter->second <= 1 && vert_iter->planes.use_count() <= neivert.planes.use_count())
		{
			const Vector3& pos = m_Mesh->m_VertexElems.GetPosition((unsigned char*)pVertices + nei_iter->first * m_Mesh->GetNumBytesPerVertex());
			float cost = 0;
			std::vector<Plane>::iterator plane_iter = vert_iter->planes->begin();
			for (; plane_iter != vert_iter->planes->end(); plane_iter++)
			{
				cost += fabsf(plane_iter->DistanceToPoint(pos));
			}

			if (cost < vert_iter->collapsecost)
			{
				vert_iter->collapsecost = cost;
				vert_iter->collapseto = nei_iter->first;
			}
		}
	}
	m_Mesh->UnlockVertexBuffer();
}

void ProgressiveMesh::Collapse(int numCollapses)
{
	for (int Collapse = 0; Collapse < numCollapses; Collapse++)
	{
		float bestCost = FLT_MAX;
		int collapseverti = -1;
		std::vector<PMVertex>::iterator vert_iter = m_Verts.begin();
		for (; vert_iter != m_Verts.end(); vert_iter++)
		{
			if (vert_iter->collapsecost < bestCost)
			{
				collapseverti = std::distance(m_Verts.begin(), vert_iter);
				bestCost = vert_iter->collapsecost;
			}
		}

		if (collapseverti < 0)
		{
			break;
		}

		PMVertex& collapsevert = m_Verts[collapseverti];
		std::vector<int>::iterator tri_iter = collapsevert.tris.begin();
		for (; tri_iter != collapsevert.tris.end(); tri_iter++)
		{
			m_Tris.reserve(m_Tris.size() + 1);  // ! avoid dangling reference
			PMTriangle& tri = m_Tris[*tri_iter];
			_ASSERT(tri.AttribId >= 0);
			int* viend = tri.vi + _countof(tri.vi);
			if (std::find(&tri.vi[0], viend, collapsevert.collapseto) != viend)
			{
				for (int i = 0; i < _countof(tri.vi); i++)
				{
					if (tri.vi[i] != collapseverti)
					{
						PMVertex& neivert = m_Verts[tri.vi[i]];
						std::vector<int>::iterator rem_tri_iter = std::find(neivert.tris.begin(), neivert.tris.end(), *tri_iter);
						_ASSERT(rem_tri_iter != neivert.tris.end());
						neivert.tris.erase(rem_tri_iter);
					}
				}
			}
			else
			{
				PMTriangle new_tri(tri);
				_ASSERT(new_tri.AttribId >= 0);
				int* new_viend = new_tri.vi + _countof(new_tri.vi);
				int* replace_vi = std::find(&new_tri.vi[0], new_viend, collapseverti);
				_ASSERT(replace_vi != new_viend);
				*replace_vi = collapsevert.collapseto;
				m_Tris.push_back(new_tri);
				int new_trii = m_Tris.size() - 1;

				for (int i = 0; i < _countof(tri.vi); i++)
				{
					if (tri.vi[i] != collapseverti)
					{
						PMVertex& neivert = m_Verts[tri.vi[i]];
						std::vector<int>::iterator rem_tri_iter = std::find(neivert.tris.begin(), neivert.tris.end(), *tri_iter);
						_ASSERT(rem_tri_iter != neivert.tris.end());
						*rem_tri_iter = new_trii;
					}
				}

				PMVertex& collapsetovert = m_Verts[collapsevert.collapseto];
				_ASSERT(collapsetovert.tris.end() == std::find(collapsetovert.tris.begin(), collapsetovert.tris.end(), new_trii));
				collapsetovert.tris.push_back(new_trii);
			}
			tri.AttribId = -1;
		}
		collapsevert.collapsecost = FLT_MAX;

		std::map<int, int>::iterator nei_iter = collapsevert.neighbors.begin();
		for (; nei_iter != collapsevert.neighbors.end(); nei_iter++)
		{
			std::vector<PMVertex>::iterator nei_vert_iter = m_Verts.begin() + nei_iter->first;
			UpdateCollapseCost(nei_vert_iter);
		}
	}
}

DWORD ProgressiveMesh::GetNumFaces(void)
{
	return std::count_if(m_Tris.begin(), m_Tris.end(), boost::lambda::bind(&PMTriangle::AttribId, boost::lambda::_1) >= 0);
}
