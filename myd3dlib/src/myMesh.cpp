#include "myMesh.h"
#include "myCollision.h"
#include "myResource.h"
#include "myDxutApp.h"
#include "libc.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

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
	LPCSTR pFilename,
	DWORD Options,
	LPD3DXBUFFER * ppAdjacency,
	LPD3DXBUFFER * ppMaterials,
	LPD3DXBUFFER * ppEffectInstances,
	DWORD * pNumMaterials)
{
	LPD3DXMESH pMesh = NULL;
	hr = D3DXLoadMeshFromXA(
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

void Mesh::ComputeDualQuaternionSkinnedVertices(
	void * pDstVertices,
	DWORD NumVerts,
	DWORD DstVertexStride,
	const D3DVertexElementSet & DstVertexElems,
	void * pSrcVertices,
	DWORD SrcVertexStride,
	const D3DVertexElementSet & SrcVertexElems,
	const TransformList & dualQuaternionList)
{
	for (unsigned int i = 0; i < NumVerts; i++)
	{
		unsigned char * pDstVertex = (unsigned char *)pDstVertices + i * DstVertexStride;
		unsigned char * pSrcVertex = (unsigned char *)pSrcVertices + i * SrcVertexStride;
		DstVertexElems.SetPosition(pDstVertex,
			dualQuaternionList.TransformVertexWithDualQuaternionList(
				SrcVertexElems.GetPosition(pSrcVertex),
				SrcVertexElems.GetBlendIndices(pSrcVertex),
				SrcVertexElems.GetBlendWeight(pSrcVertex)));
	}
}

void Mesh::ComputeNormalFrame(
	void * pVertices,
	DWORD NumVerts,
	DWORD VertexStride,
	void * pIndices,
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

		FNormals[face_i] = (v2 - v1).cross(v3 - v1);
		_ASSERT(Vector3::zero != FNormals[face_i]);
		FNormals[face_i].normalizeSelf();
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

		VertexElems.GetNormal(pv1) += FNormals[face_i] * Vector3::Angle(v3 - v1, v2 - v1);
		VertexElems.GetNormal(pv2) += FNormals[face_i] * Vector3::Angle(v3 - v2, v1 - v2);
		VertexElems.GetNormal(pv3) += FNormals[face_i] * Vector3::Angle(v1 - v3, v2 - v3);
	}

	for (unsigned int vert_i = 0; vert_i < NumVerts; vert_i++)
	{
		unsigned char * pVertex = (unsigned char *)pVertices + vert_i * VertexStride;
		VertexElems.GetNormal(pVertex).normalizeSelf();
	}
}

void Mesh::ComputeTangentFrame(
	void * pVertices,
	DWORD NumVerts,
	DWORD VertexStride,
	void * pIndices,
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

		float r = 1.0F / (s1 * t2 - s2 * t1);
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
		VertexElems.SetTangent(pVertex, (t - n * n.dot(t)).normalize(), 0);
	}
}

RayResult Mesh::RayTest(
	const Ray& ray,
	void* pVertices,
	DWORD NumVerts,
	DWORD VertexStride,
	void* pIndices,
	bool bIndices16,
	DWORD NumFaces,
	const D3DVertexElementSet& VertexElems)
{
	RayResult ret(false, FLT_MAX);
	for (unsigned int face_i = 0; face_i < NumFaces; face_i++)
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
	DWORD NumFaces,
	const D3DVertexElementSet& VertexElems)
{
	for (unsigned int face_i = 0; face_i < NumFaces; face_i++)
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
	const std::string & sub_mesh_name,
	bool bComputeTangentFrame,
	DWORD dwMeshOptions)
{
	CachePtr cache = FileIStream::Open(pFilename)->GetWholeCache();
	cache->push_back(0);

	CreateMeshFromOgreXmlInMemory((char *)&(*cache)[0], cache->size(), sub_mesh_name, bComputeTangentFrame, dwMeshOptions);
}

void OgreMesh::CreateMeshFromOgreXmlInMemory(
	LPSTR pSrcData,
	UINT srcDataLen,
	const std::string & sub_mesh_name,
	bool bComputeTangentFrame,
	DWORD dwMeshOptions)
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

	CreateMeshFromOgreXml(&doc, sub_mesh_name, bComputeTangentFrame, dwMeshOptions);
}

void OgreMesh::CreateMeshFromOgreXml(
	const rapidxml::xml_node<char> * node_root,
	const std::string & sub_mesh_name,
	bool bComputeTangentFrame,
	DWORD dwMeshOptions)
{
	DEFINE_XML_NODE_SIMPLE(mesh, root);
	DEFINE_XML_NODE_SIMPLE(submeshes, mesh);
	DEFINE_XML_NODE_SIMPLE(submesh, submeshes);
	rapidxml::xml_node<char>* node_sharedgeometry = node_mesh->first_node("sharedgeometry");
	if (node_sharedgeometry)
	{
		_ASSERT(sub_mesh_name.empty());
		rapidxml::xml_node<char> * node_boneassignments = node_mesh->first_node("boneassignments");
		CreateMeshFromOgreXmlNodes(node_sharedgeometry, node_boneassignments, node_submesh, true, bComputeTangentFrame, dwMeshOptions);
		return;
	}

	DEFINE_XML_NODE_SIMPLE(submeshnames, mesh);
	DEFINE_XML_NODE_SIMPLE(submeshname, submeshnames);
	for (; node_submesh != NULL && node_submeshname != NULL; node_submesh = node_submesh->next_sibling(), node_submeshname = node_submeshname->next_sibling())
	{
		DEFINE_XML_ATTRIBUTE_SIMPLE(name, submeshname);
		DEFINE_XML_ATTRIBUTE_INT_SIMPLE(index, submeshname);
		if (sub_mesh_name == attr_name->value())
		{
			DEFINE_XML_NODE_SIMPLE(geometry, submesh);
			rapidxml::xml_node<char> * node_boneassignments = node_submesh->first_node("boneassignments");
			CreateMeshFromOgreXmlNodes(node_geometry, node_boneassignments, node_submesh, false, bComputeTangentFrame, dwMeshOptions);
			return;
		}
	}

	THROW_CUSEXCEPTION(str_printf("cannot find sub mesh: %s", sub_mesh_name.c_str()));
}

void OgreMesh::CreateMeshFromOgreXmlNodes(
	const rapidxml::xml_node<char> * node_geometry,
	const rapidxml::xml_node<char> * node_boneassignments,
	const rapidxml::xml_node<char> * node_submesh,
	const bool bUseSharedGeometry,
	bool bComputeTangentFrame,
	DWORD dwMeshOptions)
{
	DEFINE_XML_ATTRIBUTE_INT_SIMPLE(vertexcount, geometry);
	DEFINE_XML_NODE_SIMPLE(vertexbuffer, geometry);
	DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(positions, vertexbuffer);
	DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(normals, vertexbuffer);
	DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(colours_diffuse, vertexbuffer);
	//DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(colours_specular, vertexbuffer);
	DEFINE_XML_ATTRIBUTE_INT_SIMPLE(texture_coords, vertexbuffer);

	if((dwMeshOptions & ~D3DXMESH_32BIT) && vertexcount >= USHRT_MAX)
	{
		//THROW_CUSEXCEPTION("facecount overflow ( >= 65535 )");
		dwMeshOptions |= D3DXMESH_32BIT;
	}

	if(!positions)
	{
		THROW_CUSEXCEPTION("cannot process non-position vertex");
	}

	m_VertexElems.InsertPositionElement(0);
	WORD offset = sizeof(Vector3);

	if(normals || bComputeTangentFrame)
	{
		m_VertexElems.InsertNormalElement(offset);
		offset += sizeof(Vector3);
	}

	if(bComputeTangentFrame)
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

	for(int i = 0; i < texture_coords; i++)
	{
		m_VertexElems.InsertTexcoordElement(offset, i);
		offset += sizeof(Vector2);
	}

	WORD indicesOffset = 0, weightsOffset = 0;
	if(node_boneassignments != NULL && node_boneassignments->first_node("vertexboneassignment"))
	{
		m_VertexElems.InsertBlendIndicesElement(offset);
		offset += sizeof(DWORD);

		m_VertexElems.InsertBlendWeightElement(offset);
		offset += sizeof(Vector4);
	}

	int facecount = 0;
	const rapidxml::xml_node<char> * node_submesh_iter = node_submesh;
	for(; node_submesh_iter != NULL; node_submesh_iter = bUseSharedGeometry ? node_submesh_iter->next_sibling() : NULL)
	{
		DEFINE_XML_NODE_SIMPLE(faces, submesh_iter);
		DEFINE_XML_ATTRIBUTE_INT_SIMPLE(count, faces);
		facecount += count;
	}

	std::vector<D3DVERTEXELEMENT9> velist = m_VertexElems.BuildVertexElementList(0);
	D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
	velist.push_back(ve_end);

	ResourceMgr::getSingleton().EnterDeviceSection();
	CreateMesh(facecount, vertexcount, (D3DVERTEXELEMENT9 *)&velist[0], dwMeshOptions);
	ResourceMgr::getSingleton().LeaveDeviceSection();

	ResourceMgr::getSingleton().EnterDeviceSection();
	VOID * pVertices = LockVertexBuffer();
	ResourceMgr::getSingleton().LeaveDeviceSection();
	DEFINE_XML_NODE_SIMPLE(vertex, vertexbuffer);
	for(int vertex_i = 0; node_vertex != NULL && vertex_i < vertexcount; node_vertex = node_vertex->next_sibling(), vertex_i++)
	{
		unsigned char * pVertex = (unsigned char *)pVertices + vertex_i * offset;
		if(positions)
		{
			DEFINE_XML_NODE_SIMPLE(position, vertex);
			Vector3 & Position = m_VertexElems.GetPosition(pVertex);
			rapidxml::xml_attribute<char> * attr_tmp;
			DEFINE_XML_ATTRIBUTE_FLOAT(Position.x, attr_tmp, node_position, x);
			DEFINE_XML_ATTRIBUTE_FLOAT(Position.y, attr_tmp, node_position, y);
			DEFINE_XML_ATTRIBUTE_FLOAT(Position.z, attr_tmp, node_position, z);
		}

		if(normals)
		{
			DEFINE_XML_NODE_SIMPLE(normal, vertex);
			Vector3 & Normal = m_VertexElems.GetNormal(pVertex);
			rapidxml::xml_attribute<char> * attr_tmp;
			DEFINE_XML_ATTRIBUTE_FLOAT(Normal.x, attr_tmp, node_normal, x);
			DEFINE_XML_ATTRIBUTE_FLOAT(Normal.y, attr_tmp, node_normal, y);
			DEFINE_XML_ATTRIBUTE_FLOAT(Normal.z, attr_tmp, node_normal, z);
		}

		if (colours_diffuse)
		{
			DEFINE_XML_NODE_SIMPLE(colour_diffuse, vertex);
			DEFINE_XML_ATTRIBUTE_SIMPLE(value, colour_diffuse);
			const char * color_value = attr_value->value();
			std::vector<std::string> color_set;
			boost::algorithm::split(color_set, color_value, boost::is_any_of(" "), boost::algorithm::token_compress_off);
			D3DXCOLOR Color = D3DCOLOR_ARGB(
				(int)(boost::lexical_cast<float>(color_set[3]) * 255),
				(int)(boost::lexical_cast<float>(color_set[0]) * 255),
				(int)(boost::lexical_cast<float>(color_set[1]) * 255),
				(int)(boost::lexical_cast<float>(color_set[2]) * 255));
			m_VertexElems.SetColor(pVertex, Color);
		}

		rapidxml::xml_node<char> * node_texcoord = node_vertex->first_node("texcoord");
		for(int i = 0; i < texture_coords && node_texcoord != NULL; i++, node_texcoord = node_texcoord->next_sibling())
		{
			Vector2 & Texcoord = m_VertexElems.GetTexcoord(pVertex, i);
			rapidxml::xml_attribute<char> * attr_tmp;
			DEFINE_XML_ATTRIBUTE_FLOAT(Texcoord.x, attr_tmp, node_texcoord, u);
			DEFINE_XML_ATTRIBUTE_FLOAT(Texcoord.y, attr_tmp, node_texcoord, v);
		}

		if(node_boneassignments != NULL && node_boneassignments->first_node("vertexboneassignment"))
		{
			m_VertexElems.SetBlendIndices(pVertex, 0);
			m_VertexElems.SetBlendWeight(pVertex, Vector4::zero);
		}
	}

	if(node_boneassignments != NULL)
	{
		rapidxml::xml_node<char> * node_vertexboneassignment = node_boneassignments->first_node("vertexboneassignment");
		for(; node_vertexboneassignment != NULL; node_vertexboneassignment = node_vertexboneassignment->next_sibling())
		{
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(vertexindex, vertexboneassignment);
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(boneindex, vertexboneassignment);
			DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(weight, vertexboneassignment);

			if(vertexindex >= vertexcount)
			{
				THROW_CUSEXCEPTION(str_printf("invalid vertex index: %d", vertexindex));
			}

			if(boneindex >= 0xff)
			{
				THROW_CUSEXCEPTION(str_printf("invalid bone index: %d", boneindex));
			}

			unsigned char * pVertex = (unsigned char *)pVertices + vertexindex * offset;
			unsigned char * pIndices = (unsigned char *)&m_VertexElems.GetBlendIndices(pVertex);
			float * pWeights = (float *)&m_VertexElems.GetBlendWeight(pVertex);

			int i = 0;
			for(; i < D3DVertexElementSet::MAX_BONE_INDICES; i++)
			{
				if(pWeights[i] == 0)
				{
					pIndices[i] = boneindex;
					pWeights[i] = weight;
					break;
				}
			}

			if(i >= D3DVertexElementSet::MAX_BONE_INDICES)
			{
				THROW_CUSEXCEPTION("too much bone assignment");
			}
		}
	}
	ResourceMgr::getSingleton().EnterDeviceSection();
	UnlockVertexBuffer();
	ResourceMgr::getSingleton().LeaveDeviceSection();

	ResourceMgr::getSingleton().EnterDeviceSection();
	VOID * pIndices = LockIndexBuffer();
	DWORD * pAttrBuffer = LockAttributeBuffer();
	ResourceMgr::getSingleton().LeaveDeviceSection();
	int submesh_i = 0;
	node_submesh_iter = node_submesh;
	for(int face_i = 0; node_submesh_iter != NULL; node_submesh_iter = bUseSharedGeometry ? node_submesh_iter->next_sibling() : NULL, submesh_i++)
	{
		DEFINE_XML_ATTRIBUTE_SIMPLE(material, submesh_iter);
		DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(use32bitindexes, submesh_iter);
		DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(usesharedvertices, submesh_iter);
		if(usesharedvertices != bUseSharedGeometry)
		{
			THROW_CUSEXCEPTION("invalid usesharedvertices");
		}

		rapidxml::xml_attribute<char> * attr_operationtype = node_submesh_iter->first_attribute("operationtype");
		if (attr_operationtype && 0 != _stricmp(attr_operationtype->value(), "triangle_list"))
		{
			THROW_CUSEXCEPTION("!triangle_list");
		}

		DEFINE_XML_NODE_SIMPLE(faces, submesh_iter);
		DEFINE_XML_ATTRIBUTE_INT_SIMPLE(count, faces);

		DEFINE_XML_NODE_SIMPLE(face, faces);
		for(; node_face != NULL && face_i < facecount; node_face = node_face->next_sibling(), face_i++)
		{
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(v1, face);
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(v2, face);
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(v3, face);

			if(dwMeshOptions & D3DXMESH_32BIT)
			{
				*((DWORD *)pIndices + face_i * 3 + 0) = v1;
				*((DWORD *)pIndices + face_i * 3 + 1) = v2;
				*((DWORD *)pIndices + face_i * 3 + 2) = v3;
			}
			else
			{
				*((WORD *)pIndices + face_i * 3 + 0) = v1;
				*((WORD *)pIndices + face_i * 3 + 1) = v2;
				*((WORD *)pIndices + face_i * 3 + 2) = v3;
			}
			pAttrBuffer[face_i] = submesh_i;
		}

		m_MaterialNameList.push_back(attr_material->value());
	}
	ResourceMgr::getSingleton().EnterDeviceSection();
	UnlockAttributeBuffer();
	UnlockIndexBuffer();
	ResourceMgr::getSingleton().LeaveDeviceSection();

	std::vector<DWORD> adjacency(GetNumFaces() * 3);
	ResourceMgr::getSingleton().EnterDeviceSection();
	GenerateAdjacency((float)EPSILON_E6, &adjacency[0]);
	ResourceMgr::getSingleton().LeaveDeviceSection();
	if(bComputeTangentFrame)
	{
		//DWORD dwOptions = D3DXTANGENT_GENERATE_IN_PLACE;
		//if(!normals)
		//	dwOptions |= D3DXTANGENT_CALCULATE_NORMALS;
		//hr = D3DXComputeTangentFrameEx(
		//	m_ptr, D3DDECLUSAGE_TEXCOORD, 0, D3DDECLUSAGE_TANGENT, 0, D3DX_DEFAULT, 0, D3DDECLUSAGE_NORMAL, 0, dwOptions, &m_Adjacency[0], -1.01f, -0.01f, -1.01f, NULL, NULL);
		//if(FAILED(hr))
		//{
		//	THROW_D3DEXCEPTION(hr);
		//}
		ResourceMgr::getSingleton().EnterDeviceSection();
		VOID * pVertices = LockVertexBuffer();
		VOID * pIndices = LockIndexBuffer();
		ResourceMgr::getSingleton().LeaveDeviceSection();
		ComputeTangentFrame(
			pVertices, GetNumVertices(), GetNumBytesPerVertex(), pIndices, !(dwMeshOptions & D3DXMESH_32BIT), GetNumFaces(), m_VertexElems);
		ResourceMgr::getSingleton().EnterDeviceSection();
		UnlockVertexBuffer();
		UnlockIndexBuffer();
		ResourceMgr::getSingleton().LeaveDeviceSection();
	}
	m_Adjacency.resize(adjacency.size());
	ResourceMgr::getSingleton().EnterDeviceSection();
	OptimizeInplace(D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, &adjacency[0], &m_Adjacency[0], NULL, NULL);
	ResourceMgr::getSingleton().LeaveDeviceSection();

	DWORD AttribTblCount = 0;
	ResourceMgr::getSingleton().EnterDeviceSection();
	GetAttributeTable(NULL, &AttribTblCount);
	m_AttribTable.resize(AttribTblCount);
	GetAttributeTable(&m_AttribTable[0], &AttribTblCount);
	ResourceMgr::getSingleton().LeaveDeviceSection();
}

void OgreMesh::SaveOgreMesh(const char * path)
{
	std::ofstream ofs(path);
	ofs << "<mesh>\n";
	ofs << "\t<sharedgeometry vertexcount=\"" << GetNumVertices() << "\">\n";
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
	void * pVertices = LockVertexBuffer();
	DWORD VertexStride = GetNumBytesPerVertex();
	// write vertex data
	for (DWORD i=0; i < GetNumVertices(); i++)
	{
		ofs << "\t\t\t<vertex>\n";
		unsigned char * pVertex = (unsigned char *)pVertices + i * VertexStride;
		const Vector3 vertex = m_VertexElems.GetPosition(pVertex);
		//write vertex position
		ofs << "\t\t\t\t<position x=\"" << vertex.x << "\" y=\"" << vertex.y << "\" " << "z=\"" << vertex.z << "\"/>\n";
		//write vertex normal
		if (normals)
		{
			const Vector3 normal = m_VertexElems.GetNormal(pVertex);
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
			for (DWORD j=0; j<texture_coords; j++)
			{
				const Vector2 texcoord = m_VertexElems.GetTexcoord(pVertex, (BYTE)j);
				ofs << "\t\t\t\t<texcoord u=\"" << texcoord.x << "\" v=\"" << texcoord.y << "\"/>\n";
			}
		}
		ofs << "\t\t\t</vertex>\n";
	}
	ofs << "\t\t</vertexbuffer>\n";
	ofs << "\t</sharedgeometry>\n";
	DWORD submeshes = 0;
	GetAttributeTable(NULL, &submeshes);
	std::vector<D3DXATTRIBUTERANGE> rang(submeshes);
	GetAttributeTable(&rang[0], &submeshes);
	VOID * pIndices = LockIndexBuffer();
	// write submeshes data
	ofs << "\t<submeshes>\n";
	for (DWORD i=0; i < submeshes; i++)
	{
		// Start submesh description
		ofs << "\t\t<submesh ";
		// Write material name
		ofs << "material=\"" << m_MaterialNameList[rang[i].AttribId] << "\" ";
		DWORD use32bitindexes = GetOptions() & D3DXMESH_32BIT;
		// Write use32bitIndexes flag
		ofs << "use32bitindexes=\"";
		if (use32bitindexes)
			ofs << "true";
		else
			ofs << "false";
		ofs << "\" ";
		// Write operation type flag
		ofs << "usesharedvertices=\"true\" operationtype=\"triangle_list\">\n";

		// Write submesh polygons
		ofs << "\t\t\t<faces count=\"" << rang[i].FaceCount << "\">\n";
		for (DWORD face_i=rang[i].FaceStart; face_i<rang[i].FaceStart+rang[i].FaceCount; face_i++)
		{
			if (use32bitindexes)
			{
				ofs << "\t\t\t\t<face v1=\"" << *((DWORD *)pIndices + face_i * 3 + 0) << "\" v2=\"" << *((DWORD *)pIndices + face_i * 3 + 1) << "\" v3=\"" << *((DWORD *)pIndices + face_i * 3 + 2) << "\"/>\n";
			}
			else
			{
				ofs << "\t\t\t\t<face v1=\"" << *((WORD *)pIndices + face_i * 3 + 0) << "\" v2=\"" << *((WORD *)pIndices + face_i * 3 + 1) << "\" v3=\"" << *((WORD *)pIndices + face_i * 3 + 2) << "\"/>\n";
			}
		}
		ofs << "\t\t\t</faces>\n";
		ofs << "\t\t</submesh>\n";
	}
	UnlockIndexBuffer();
	ofs << "\t</submeshes>\n";
	// write skeleton link
	ofs << "\t<skeletonlink name=\"\"/>\n";
	// Write shared geometry bone assignments
	ofs << "\t<boneassignments>\n";
	if (m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
	{
		for (DWORD i=0; i<GetNumVertices(); i++)
		{
			unsigned char * pVertex = (unsigned char *)pVertices + i * VertexStride;
			for (int j=0; j<D3DVertexElementSet::MAX_BONE_INDICES; j++)
			{
				if (m_VertexElems.GetBlendWeight(pVertex)[j] > 0.001)
				{
					ofs << "\t\t<vertexboneassignment vertexindex=\"" << i << "\" boneindex=\"" << (int)((unsigned char *)&m_VertexElems.GetBlendIndices(pVertex))[j] << "\" weight=\"" << m_VertexElems.GetBlendWeight(pVertex)[j] <<"\"/>\n";
				}
			}
		}
	}
	UnlockVertexBuffer();
	ofs << "\t</boneassignments>\n";
	// write submesh names
	ofs << "\t<submeshnames>\n";
	ofs << "\t</submeshnames>\n";
	// end mesh description
	ofs << "</mesh>\n";
}

void OgreMesh::SaveSimplifiedOgreMesh(const char * path, DWORD MinValue, DWORD Options)
{
	OgreMeshPtr simplified_mesh(new OgreMesh());
	simplified_mesh->Create(SimplifyMesh(&m_Adjacency[0], MinValue, Options).Detach());
	simplified_mesh->m_Adjacency = m_Adjacency;
	simplified_mesh->m_MaterialNameList = m_MaterialNameList;
	simplified_mesh->m_VertexElems = m_VertexElems;
	simplified_mesh->SaveOgreMesh(path);
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

UINT OgreMesh::GetMaterialNum(void) const
{
	return m_MaterialNameList.size();
}

const std::string & OgreMesh::GetMaterialName(DWORD AttribId) const
{
	return m_MaterialNameList[AttribId];
}

AABB OgreMesh::CalculateAABB(DWORD AttribId)
{
	const D3DXATTRIBUTERANGE& att = m_AttribTable[AttribId];
	void* pVertices = LockVertexBuffer(D3DLOCK_READONLY);
	DWORD VertexStride = GetNumBytesPerVertex();
	AABB ret(FLT_MAX, -FLT_MAX);
	for (DWORD i = 0; i < att.VertexCount; i++)
	{
		unsigned char* pVertex = (unsigned char*)pVertices + att.VertexStart * VertexStride + i * VertexStride;
		const Vector3 vertex = m_VertexElems.GetPosition(pVertex);
		ret.unionSelf(vertex);
	}
	UnlockVertexBuffer();
	return ret;
}
