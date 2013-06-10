#pragma once

#include <d3d9.h>
#include <set>
#include "myMath.h"
#include <vector>
#include <atlbase.h>
#include <boost/shared_ptr.hpp>
#include "mySingleton.h"

namespace my
{
	struct D3DVERTEXELEMENT9Less
	{
		bool operator ()(const D3DVERTEXELEMENT9 & lhs, const D3DVERTEXELEMENT9 & rhs) const
		{
			if(lhs.Stream == rhs.Stream)
			{
				if(lhs.Usage == rhs.Usage)
				{
					return lhs.UsageIndex < rhs.UsageIndex;
				}
				return lhs.Usage < rhs.Usage;
			}
			return lhs.Stream < rhs.Stream;
		}
	};

	class D3DVERTEXELEMENT9Set : public std::set<D3DVERTEXELEMENT9, D3DVERTEXELEMENT9Less>
	{
	public:
		static const int MAX_BONE_INDICES = 4;

		std::pair<iterator, bool> insert(const value_type & val)
		{
			_ASSERT(end() == find(val));

			return std::set<D3DVERTEXELEMENT9, D3DVERTEXELEMENT9Less>::insert(val);
		}

		static D3DVERTEXELEMENT9 CreateCustomElement(WORD Stream, D3DDECLUSAGE Usage, BYTE UsageIndex, WORD Offset = 0, D3DDECLTYPE Type = D3DDECLTYPE_UNUSED, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT)
		{
			D3DVERTEXELEMENT9 ret = {Stream, Offset, Type, Method, Usage, UsageIndex};
			return ret;
		}

		template <typename ElementType>
		ElementType & GetCustomType(void * pVertex, DWORD Offset) const
		{
			return *(ElementType *)((unsigned char *)pVertex + Offset);
		}

		template <typename ElementType>
		ElementType & GetCustomType(void * pVertex, WORD Stream, D3DDECLUSAGE Usage, BYTE UsageIndex) const
		{
			const_iterator elem_iter = find(CreateCustomElement(Stream, Usage, UsageIndex));
			_ASSERT(elem_iter != end());

			return GetCustomType<ElementType>(pVertex, elem_iter->Offset);
		}

		template <typename ElementType>
		ElementType & SetCustomType(void * pVertex, WORD Stream, D3DDECLUSAGE Usage, BYTE UsageIndex, const ElementType & value) const
		{
			return GetCustomType<ElementType>(pVertex, Stream, Usage, UsageIndex) = value;
		}

		typedef Vector3 PositionType;

		static D3DVERTEXELEMENT9 CreatePositionElement(WORD Stream, WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT)
		{
			return CreateCustomElement(Stream, D3DDECLUSAGE_POSITION, UsageIndex, Offset, D3DDECLTYPE_FLOAT3, Method);
		}

		PositionType & GetPosition(void * pVertex, WORD Stream = 0, BYTE UsageIndex = 0) const
		{
			return GetCustomType<PositionType>(pVertex, Stream, D3DDECLUSAGE_POSITION, UsageIndex);
		}

		void SetPosition(void * pVertex, const PositionType & Position, WORD Stream = 0, BYTE UsageIndex = 0) const
		{
			GetPosition(pVertex, Stream, UsageIndex) = Position;
		}

		typedef Vector3 BinormalType;

		static D3DVERTEXELEMENT9 CreateBinormalElement(WORD Stream, WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT)
		{
			return CreateCustomElement(Stream, D3DDECLUSAGE_BINORMAL, UsageIndex, Offset, D3DDECLTYPE_FLOAT3, Method);
		}

		BinormalType & GetBinormal(void * pVertex, WORD Stream = 0, BYTE UsageIndex = 0) const
		{
			return GetCustomType<BinormalType>(pVertex, Stream, D3DDECLUSAGE_BINORMAL, UsageIndex);
		}

		void SetBinormal(void * pVertex, const BinormalType & Binormal, WORD Stream = 0, BYTE UsageIndex = 0) const
		{
			GetBinormal(pVertex, Stream, UsageIndex) = Binormal;
		}

		typedef Vector3 TangentType;

		static D3DVERTEXELEMENT9 CreateTangentElement(WORD Stream, WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT)
		{
			return CreateCustomElement(Stream, D3DDECLUSAGE_TANGENT, UsageIndex, Offset, D3DDECLTYPE_FLOAT3, Method);
		}

		TangentType & GetTangent(void * pVertex, WORD Stream = 0, BYTE UsageIndex = 0) const
		{
			return GetCustomType<TangentType>(pVertex, Stream, D3DDECLUSAGE_TANGENT, UsageIndex);
		}

		void SetTangent(void * pVertex, const TangentType & Tangent, WORD Stream = 0, BYTE UsageIndex = 0) const
		{
			GetTangent(pVertex, Stream, UsageIndex) = Tangent;
		}

		typedef Vector3 NormalType;

		static D3DVERTEXELEMENT9 CreateNormalElement(WORD Stream, WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT)
		{
			return CreateCustomElement(Stream, D3DDECLUSAGE_NORMAL, UsageIndex, Offset, D3DDECLTYPE_FLOAT3, Method);
		}

		NormalType & GetNormal(void * pVertex, WORD Stream = 0, BYTE UsageIndex = 0) const
		{
			return GetCustomType<NormalType>(pVertex, Stream, D3DDECLUSAGE_NORMAL, UsageIndex);
		}

		void SetNormal(void * pVertex, const NormalType & Normal, WORD Stream = 0, BYTE UsageIndex = 0) const
		{
			GetNormal(pVertex, Stream, UsageIndex) = Normal;
		}

		typedef Vector2 TexcoordType;

		static D3DVERTEXELEMENT9 CreateTexcoordElement(WORD Stream, WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT)
		{
			return CreateCustomElement(Stream, D3DDECLUSAGE_TEXCOORD, UsageIndex, Offset, D3DDECLTYPE_FLOAT2, Method);
		}

		TexcoordType & GetTexcoord(void * pVertex, WORD Stream = 0, BYTE UsageIndex = 0) const
		{
			return GetCustomType<TexcoordType>(pVertex, Stream, D3DDECLUSAGE_TEXCOORD, UsageIndex);
		}

		void SetTexcoord(void * pVertex, const TexcoordType & Texcoord, WORD Stream = 0, BYTE UsageIndex = 0) const
		{
			GetTexcoord(pVertex, Stream, UsageIndex) = Texcoord;
		}

		typedef D3DCOLOR ColorType;

		static D3DVERTEXELEMENT9 CreateColorElement(WORD Stream, WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT)
		{
			return CreateCustomElement(Stream, D3DDECLUSAGE_COLOR, UsageIndex, Offset, D3DDECLTYPE_D3DCOLOR, Method);
		}

		ColorType & GetColor(void * pVertex, WORD Stream = 0, BYTE UsageIndex = 0) const
		{
			return GetCustomType<ColorType>(pVertex, Stream, D3DDECLUSAGE_COLOR, UsageIndex);
		}

		void SetColor(void * pVertex, const ColorType & Color, WORD Stream = 0, BYTE UsageIndex = 0) const
		{
			GetColor(pVertex, Stream, UsageIndex) = Color;
		}

		typedef D3DCOLOR BlendIndicesType;

		static D3DVERTEXELEMENT9 CreateBlendIndicesElement(WORD Stream, WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT)
		{
			_ASSERT(sizeof(BlendIndicesType) / sizeof(unsigned char) == MAX_BONE_INDICES);

			return CreateCustomElement(Stream, D3DDECLUSAGE_BLENDINDICES, UsageIndex, Offset, D3DDECLTYPE_UBYTE4, Method);
		}

		BlendIndicesType & GetBlendIndices(void * pVertex, WORD Stream = 0, BYTE UsageIndex = 0) const
		{
			return GetCustomType<BlendIndicesType>(pVertex, Stream, D3DDECLUSAGE_BLENDINDICES, UsageIndex);
		}

		void SetBlendIndices(void * pVertex, const BlendIndicesType & BlendIndices, WORD Stream = 0, BYTE UsageIndex = 0) const
		{
			GetBlendIndices(pVertex, Stream, UsageIndex) = BlendIndices;
		}

		typedef Vector4 BlendWeightsType;

		static D3DVERTEXELEMENT9 CreateBlendWeightsElement(WORD Stream, WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT)
		{
			return CreateCustomElement(Stream, D3DDECLUSAGE_BLENDWEIGHT, UsageIndex, Offset, D3DDECLTYPE_FLOAT4, Method);
		}

		BlendWeightsType & GetBlendWeights(void * pVertex, WORD Stream = 0, BYTE UsageIndex = 0) const
		{
			return GetCustomType<BlendWeightsType>(pVertex, Stream, D3DDECLUSAGE_BLENDWEIGHT, UsageIndex);
		}

		void SetBlendWeights(void * pVertex, const BlendWeightsType & BlendWeights, WORD Stream = 0, BYTE UsageIndex = 0) const
		{
			GetBlendWeights(pVertex, Stream, UsageIndex) = BlendWeights;
		}

		static WORD CalculateElementUsageSize(BYTE Usage)
		{
			switch(Usage)
			{
			case D3DDECLUSAGE_POSITION:
				return sizeof(PositionType);

			case D3DDECLUSAGE_NORMAL:
				return sizeof(NormalType);

			case D3DDECLUSAGE_TEXCOORD:
				return sizeof(TexcoordType);

			case D3DDECLUSAGE_BLENDINDICES:
				return sizeof(BlendIndicesType);

			case D3DDECLUSAGE_BLENDWEIGHT:
				return sizeof(BlendWeightsType);
			}

			_ASSERT(false);
			return 0;
		}

		std::vector<D3DVERTEXELEMENT9> BuildVertexElementList(void) const;

		std::vector<D3DVERTEXELEMENT9> BuildVertexElementList(WORD OverideStream) const;

		UINT CalculateVertexStride(DWORD Stream = 0) const;

		CComPtr<IDirect3DVertexDeclaration9> CreateVertexDeclaration(LPDIRECT3DDEVICE9 pDevice) const;
	};

	class D3DVertexElement
	{
	public:
		BYTE Offset;

		D3DDECLTYPE Type;

		D3DDECLMETHOD Method;

		D3DVertexElement(void)
			: Offset(0)
			, Type(D3DDECLTYPE_UNUSED)
			, Method(D3DDECLMETHOD_DEFAULT)
		{
		}

		D3DVertexElement(BYTE _Offset, D3DDECLTYPE _Type, D3DDECLMETHOD _Method)
			: Offset(_Offset)
			, Type(_Type)
			, Method(_Method)
		{
		}
	};

	class D3DVertexElementSet
	{
	public:
		static const int MAX_BONE_INDICES = 4;

		static const int MAX_USAGE = D3DDECLUSAGE_SAMPLE+1;

		static const int MAX_USAGE_INDEX = 8;

		D3DVertexElement elems[MAX_USAGE][MAX_USAGE_INDEX];

	public:
		D3DVertexElementSet(void)
		{
		}

		void InsertVertexElement(BYTE Offset, D3DDECLTYPE Type, D3DDECLUSAGE Usage, BYTE UsageIndex, D3DDECLMETHOD Method)
		{
			_ASSERT(Type != D3DDECLTYPE_UNUSED);

			_ASSERT(Usage < MAX_USAGE && UsageIndex < MAX_USAGE_INDEX);

			elems[Usage][UsageIndex] = D3DVertexElement(Offset, Type, Method);
		}

		template <typename ElementType>
		ElementType & GetVertexElement(void * pVertex, D3DDECLUSAGE Usage, BYTE UsageIndex) const
		{
			return *(ElementType *)((unsigned char *)pVertex + elems[Usage][UsageIndex].Offset);
		}

		template <typename ElementType>
		void SetVertexElement(void * pVertex, D3DDECLUSAGE Usage, BYTE UsageIndex, const ElementType & Value) const
		{
			GetVertexElement<ElementType>(pVertex, Usage, UsageIndex) = Value;
		}

		std::vector<D3DVERTEXELEMENT9> BuildVertexElementList(WORD Stream)
		{
			std::vector<D3DVERTEXELEMENT9> ret;
			for(unsigned int Usage = 0; Usage < MAX_USAGE; Usage++)
			{
				for(unsigned int UsageIndex = 0; UsageIndex < MAX_USAGE_INDEX; UsageIndex++)
				{
					const D3DVertexElement & elem = elems[Usage][UsageIndex];
					if(elem.Type != D3DDECLTYPE_UNUSED)
					{
						D3DVERTEXELEMENT9 ve = {Stream, elem.Offset, elem.Type, elem.Method, Usage, UsageIndex};
						ret.push_back(ve);
					}
				}
			}
		}

		void InsertPositionElement(BYTE Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT)
		{
			InsertVertexElement(Offset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION, UsageIndex, Method);
		}

		Vector3 & GetPosition(void * pVertex, BYTE UsageIndex = 0)
		{
			_ASSERT(elems[D3DDECLUSAGE_POSITION][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

			return GetVertexElement<Vector3>(pVertex, D3DDECLUSAGE_POSITION, UsageIndex);
		}

		void SetPosition(void * pVertex, const Vector3 & Position, BYTE UsageIndex = 0) const
		{
			_ASSERT(elems[D3DDECLUSAGE_POSITION][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

			return SetVertexElement<Vector3>(pVertex, D3DDECLUSAGE_POSITION, UsageIndex, Position);
		}

		void InsertBlendWeightElement(BYTE Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT)
		{
			InsertVertexElement(Offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_BLENDWEIGHT, UsageIndex, Method);
		}

		Vector4 & GetBlendWeight(void * pVertex, BYTE UsageIndex = 0)
		{
			_ASSERT(elems[D3DDECLUSAGE_BLENDWEIGHT][UsageIndex].Type == D3DDECLTYPE_FLOAT4);

			return GetVertexElement<Vector4>(pVertex, D3DDECLUSAGE_BLENDWEIGHT, UsageIndex);
		}

		void SetBlendWeight(void * pVertex, const Vector4 & BlendWeight, BYTE UsageIndex = 0) const
		{
			_ASSERT(elems[D3DDECLUSAGE_BLENDWEIGHT][UsageIndex].Type == D3DDECLTYPE_FLOAT4);

			return SetVertexElement<Vector4>(pVertex, D3DDECLUSAGE_BLENDWEIGHT, UsageIndex, BlendWeight);
		}

		void InsertBlendIndicesElement(BYTE Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT)
		{
			InsertVertexElement(Offset, D3DDECLTYPE_UBYTE4, D3DDECLUSAGE_BLENDINDICES, UsageIndex, Method);
		}

		DWORD & GetBlendIndices(void * pVertex, BYTE UsageIndex = 0)
		{
			_ASSERT(elems[D3DDECLUSAGE_BLENDINDICES][UsageIndex].Type == D3DDECLTYPE_UBYTE4);

			return GetVertexElement<DWORD>(pVertex, D3DDECLUSAGE_BLENDINDICES, UsageIndex);
		}

		void SetBlendIndices(void * pVertex, const DWORD & BlendIndices, BYTE UsageIndex = 0) const
		{
			_ASSERT(elems[D3DDECLUSAGE_BLENDINDICES][UsageIndex].Type == D3DDECLTYPE_UBYTE4);

			return SetVertexElement<DWORD>(pVertex, D3DDECLUSAGE_BLENDINDICES, UsageIndex, BlendIndices);
		}

		void InsertNormalElement(BYTE Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT)
		{
			InsertVertexElement(Offset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL, UsageIndex, Method);
		}

		Vector3 & GetNormal(void * pVertex, BYTE UsageIndex = 0)
		{
			_ASSERT(elems[D3DDECLUSAGE_NORMAL][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

			return GetVertexElement<Vector3>(pVertex, D3DDECLUSAGE_NORMAL, UsageIndex);
		}

		void SetNormal(void * pVertex, const Vector3 & Normal, BYTE UsageIndex = 0) const
		{
			_ASSERT(elems[D3DDECLUSAGE_NORMAL][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

			return SetVertexElement<Vector3>(pVertex, D3DDECLUSAGE_NORMAL, UsageIndex, Normal);
		}

		void InsertTexcoordElement(BYTE Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT)
		{
			InsertVertexElement(Offset, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, UsageIndex, Method);
		}

		Vector2 & GetTexcoord(void * pVertex, BYTE UsageIndex = 0)
		{
			_ASSERT(elems[D3DDECLUSAGE_TEXCOORD][UsageIndex].Type == D3DDECLTYPE_FLOAT2);

			return GetVertexElement<Vector2>(pVertex, D3DDECLUSAGE_TEXCOORD, UsageIndex);
		}

		void SetTexcoord(void * pVertex, const Vector2 & Texcoord, BYTE UsageIndex = 0) const
		{
			_ASSERT(elems[D3DDECLUSAGE_TEXCOORD][UsageIndex].Type == D3DDECLTYPE_FLOAT2);

			return SetVertexElement<Vector2>(pVertex, D3DDECLUSAGE_TEXCOORD, UsageIndex, Texcoord);
		}

		void InsertTangentElement(BYTE Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT)
		{
			InsertVertexElement(Offset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_TANGENT, UsageIndex, Method);
		}

		Vector3 & GetTangent(void * pVertex, BYTE UsageIndex = 0)
		{
			_ASSERT(elems[D3DDECLUSAGE_TANGENT][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

			return GetVertexElement<Vector3>(pVertex, D3DDECLUSAGE_TANGENT, UsageIndex);
		}

		void SetTangent(void * pVertex, const Vector3 & Tangent, BYTE UsageIndex = 0) const
		{
			_ASSERT(elems[D3DDECLUSAGE_TANGENT][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

			return SetVertexElement<Vector3>(pVertex, D3DDECLUSAGE_TANGENT, UsageIndex, Tangent);
		}

		void InsertBinormalElement(BYTE Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT)
		{
			InsertVertexElement(Offset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_BINORMAL, UsageIndex, Method);
		}

		Vector3 & GetBinormal(void * pVertex, BYTE UsageIndex = 0)
		{
			_ASSERT(elems[D3DDECLUSAGE_BINORMAL][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

			return GetVertexElement<Vector3>(pVertex, D3DDECLUSAGE_BINORMAL, UsageIndex);
		}

		void SetBinormal(void * pVertex, const Vector3 & Binormal, BYTE UsageIndex = 0) const
		{
			_ASSERT(elems[D3DDECLUSAGE_BINORMAL][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

			return SetVertexElement<Vector3>(pVertex, D3DDECLUSAGE_BINORMAL, UsageIndex, Binormal);
		}

		void InsertColorElement(BYTE Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT)
		{
			InsertVertexElement(Offset, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR, UsageIndex, Method);
		}

		D3DCOLOR & GetColor(void * pVertex, BYTE UsageIndex = 0)
		{
			_ASSERT(elems[D3DDECLUSAGE_COLOR][UsageIndex].Type == D3DDECLTYPE_D3DCOLOR);

			return GetVertexElement<D3DCOLOR>(pVertex, D3DDECLUSAGE_COLOR, UsageIndex);
		}

		void SetColor(void * pVertex, const D3DCOLOR & Color, BYTE UsageIndex = 0) const
		{
			_ASSERT(elems[D3DDECLUSAGE_COLOR][UsageIndex].Type == D3DDECLTYPE_D3DCOLOR);

			return SetVertexElement<D3DCOLOR>(pVertex, D3DDECLUSAGE_COLOR, UsageIndex, Color);
		}
	};

	class VertexBuffer : public DeviceRelatedObject<IDirect3DVertexBuffer9>
	{
	public:
		VertexBuffer(void)
		{
		}

		void Create(IDirect3DVertexBuffer9 * ptr)
		{
			_ASSERT(!m_ptr);

			m_ptr = ptr;
		}

		void CreateVertexBuffer(
			LPDIRECT3DDEVICE9 pDevice,
			UINT Length,
			DWORD Usage = 0,
			DWORD FVF = 0,
			D3DPOOL Pool = D3DPOOL_DEFAULT);

		D3DVERTEXBUFFER_DESC GetDesc(void)
		{
			D3DVERTEXBUFFER_DESC desc;
			V(m_ptr->GetDesc(&desc));
			return desc;
		}

		void * Lock(UINT OffsetToLock, UINT SizeToLock, DWORD Flags = 0)
		{
			void * ret;
			V(m_ptr->Lock(OffsetToLock, SizeToLock, &ret, Flags));
			return ret;
		}

		void Unlock(void)
		{
			V(m_ptr->Unlock());
		}
	};

	typedef boost::shared_ptr<VertexBuffer> VertexBufferPtr;

	class IndexBuffer : public DeviceRelatedObject<IDirect3DIndexBuffer9>
	{
	public:
		IndexBuffer(void)
		{
		}

		void Create(IDirect3DIndexBuffer9 * ptr)
		{
			_ASSERT(!m_ptr);

			m_ptr = ptr;
		}

		void CreateIndexBuffer(
			LPDIRECT3DDEVICE9 pDevice,
			UINT Length,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_INDEX32,
			D3DPOOL Pool = D3DPOOL_DEFAULT);

		D3DINDEXBUFFER_DESC GetDesc(void)
		{
			D3DINDEXBUFFER_DESC desc;
			V(m_ptr->GetDesc(&desc));
			return desc;
		}

		void * Lock(UINT OffsetToLock, UINT SizeToLock, DWORD Flags = 0)
		{
			void * ret;
			V(m_ptr->Lock(OffsetToLock, SizeToLock, &ret, Flags));
			return ret;
		}

		void Unlock(void)
		{
			V(m_ptr->Unlock());
		}
	};

	typedef boost::shared_ptr<IndexBuffer> IndexBufferPtr;

	class Mesh : public DeviceRelatedObject<ID3DXMesh>
	{
	public:
		Mesh(void)
		{
		}

		void Create(ID3DXMesh * pMesh)
		{
			_ASSERT(!m_ptr);

			m_ptr = pMesh;
		}

		void CreateMesh(
			LPDIRECT3DDEVICE9 pD3DDevice,
			DWORD NumFaces,
			DWORD NumVertices,
			CONST LPD3DVERTEXELEMENT9 pDeclaration,
			DWORD Options = D3DXMESH_MANAGED);

		void CreateMeshFVF(
			LPDIRECT3DDEVICE9 pD3DDevice,
			DWORD NumFaces,
			DWORD NumVertices,
			DWORD FVF,
			DWORD Options = D3DXMESH_MANAGED);

		void CreateMeshFromX(
			LPDIRECT3DDEVICE9 pD3DDevice,
			LPCSTR pFilename,
			DWORD Options = D3DXMESH_MANAGED,
			LPD3DXBUFFER * ppAdjacency = NULL,
			LPD3DXBUFFER * ppMaterials = NULL,
			LPD3DXBUFFER * ppEffectInstances = NULL,
			DWORD * pNumMaterials = NULL);

		void CreateMeshFromXInMemory(
			LPDIRECT3DDEVICE9 pD3DDevice,
			LPCVOID Memory,
			DWORD SizeOfMemory,
			DWORD Options = D3DXMESH_MANAGED,
			LPD3DXBUFFER * ppAdjacency = NULL,
			LPD3DXBUFFER * ppMaterials = NULL,
			LPD3DXBUFFER * ppEffectInstances = NULL,
			DWORD * pNumMaterials = NULL);

		void CreateBox(
			LPDIRECT3DDEVICE9 pd3dDevice,
			FLOAT Width = 1.0f,
			FLOAT Height = 1.0f,
			FLOAT Depth = 1.0f,
			LPD3DXBUFFER * ppAdjacency = NULL);

		void CreateCylinder(
			LPDIRECT3DDEVICE9 pd3dDevice,
			FLOAT Radius1 = 1.0f,
			FLOAT Radius2 = 1.0f,
			FLOAT Length = 2.0f,
			UINT Slices = 20,
			UINT Stacks = 1,
			LPD3DXBUFFER * ppAdjacency = NULL);

		void CreatePolygon(
			LPDIRECT3DDEVICE9 pDevice,
			FLOAT Length = 1.0f,
			UINT Sides = 5,
			LPD3DXBUFFER * ppAdjacency = NULL);

		void CreateSphere(
			LPDIRECT3DDEVICE9 pDevice,
			FLOAT Radius = 1.0f,
			UINT Slices = 20,
			UINT Stacks = 20,
			LPD3DXBUFFER * ppAdjacency = NULL);

		void CreateTeapot(
			LPDIRECT3DDEVICE9 pDevice,
			LPD3DXBUFFER * ppAdjacency = NULL);

		void CreateTorus(
			LPDIRECT3DDEVICE9 pDevice,
			FLOAT InnerRadius = 0.5f,
			FLOAT OuterRadius = 1.5f,
			UINT Sides = 20,
			UINT Rings = 20,
			LPD3DXBUFFER * ppAdjacency = NULL);

		CComPtr<ID3DXMesh> CloneMesh(DWORD Options, CONST D3DVERTEXELEMENT9 * pDeclaration, LPDIRECT3DDEVICE9 pDevice)
		{
			CComPtr<ID3DXMesh> CloneMesh;
			V(m_ptr->CloneMesh(Options, pDeclaration, pDevice, &CloneMesh));
			return CloneMesh;
		}

		CComPtr<ID3DXMesh> CloneMeshFVF(DWORD Options, DWORD FVF, LPDIRECT3DDEVICE9 pDevice)
		{
			CComPtr<ID3DXMesh> CloneMesh;
			V(m_ptr->CloneMeshFVF(Options, FVF, pDevice, &CloneMesh));
			return CloneMesh;
		}

		void ConvertAdjacencyToPointReps(CONST DWORD * pAdjacency, DWORD * pPRep)
		{
			V(m_ptr->ConvertAdjacencyToPointReps(pAdjacency, pPRep));
		}

		void ConvertPointRepsToAdjacency(CONST DWORD* pPRep, DWORD* pAdjacency)
		{
			V(m_ptr->ConvertPointRepsToAdjacency(pPRep, pAdjacency));
		}

		void DrawSubset(DWORD AttribId)
		{
			V(m_ptr->DrawSubset(AttribId));
		}

		void GenerateAdjacency(FLOAT Epsilon, DWORD * pAdjacency)
		{
			V(m_ptr->GenerateAdjacency(Epsilon, pAdjacency));
		}

		void GetAttributeTable(D3DXATTRIBUTERANGE * pAttribTable, DWORD * pAttribTableSize)
		{
			V(m_ptr->GetAttributeTable(pAttribTable, pAttribTableSize));
		}

		void GetDeclaration(D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE])
		{
			V(m_ptr->GetDeclaration(Declaration));
		}

		CComPtr<IDirect3DDevice9> GetDevice(void)
		{
			CComPtr<IDirect3DDevice9> Device;
			V(m_ptr->GetDevice(&Device));
			return Device;
		}

		DWORD GetFVF(void)
		{
			return m_ptr->GetFVF();
		}

		CComPtr<IDirect3DIndexBuffer9> GetIndexBuffer(void)
		{
			CComPtr<IDirect3DIndexBuffer9> IndexBuffer;
			V(m_ptr->GetIndexBuffer(&IndexBuffer));
			return IndexBuffer;
		}

		DWORD GetNumBytesPerVertex(void)
		{
			return m_ptr->GetNumBytesPerVertex();
		}

		DWORD GetNumFaces(void)
		{
			return m_ptr->GetNumFaces();
		}

		DWORD GetNumVertices(void)
		{
			return m_ptr->GetNumVertices();
		}

		DWORD GetOptions(void)
		{
			return m_ptr->GetOptions();
		}

		CComPtr<IDirect3DVertexBuffer9> GetVertexBuffer(void)
		{
			CComPtr<IDirect3DVertexBuffer9> VertexBuffer;
			V(m_ptr->GetVertexBuffer(&VertexBuffer));
			return VertexBuffer;
		}

		LPVOID LockIndexBuffer(DWORD Flags = 0)
		{
			LPVOID pData = NULL;
			if(FAILED(hr = m_ptr->LockIndexBuffer(Flags, &pData)))
			{
				THROW_D3DEXCEPTION(hr);
			}
			return pData;
		}

		LPVOID LockVertexBuffer(DWORD Flags = 0)
		{
			LPVOID pData = NULL;
			if(FAILED(hr = m_ptr->LockVertexBuffer(Flags, &pData)))
			{
				THROW_D3DEXCEPTION(hr);
			}
			return pData;
		}

		void UnlockIndexBuffer(void)
		{
			V(m_ptr->UnlockIndexBuffer());
		}

		void UnlockVertexBuffer(void)
		{
			V(m_ptr->UnlockVertexBuffer());
		}

		void UpdateSemantics(D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE])
		{
			V(m_ptr->UpdateSemantics(Declaration));
		}

		DWORD * LockAttributeBuffer(DWORD Flags = 0)
		{
			DWORD * pData = NULL;
			if(FAILED(hr = m_ptr->LockAttributeBuffer(Flags, &pData)))
			{
				THROW_D3DEXCEPTION(hr);
			}
			return pData;
		}

		CComPtr<ID3DXMesh> Optimize(
			DWORD Flags,
			CONST DWORD * pAdjacencyIn,
			DWORD * pAdjacencyOut,
			DWORD * pFaceRemap,
			LPD3DXBUFFER * ppVertexRemap = NULL)
		{
			CComPtr<ID3DXMesh> OptMesh;
			V(m_ptr->Optimize(Flags, pAdjacencyIn, pAdjacencyOut, pFaceRemap, ppVertexRemap, &OptMesh));
			return OptMesh;
		}

		void OptimizeInplace(DWORD Flags,
			CONST DWORD * pAdjacencyIn,
			DWORD * pAdjacencyOut,
			DWORD * pFaceRemap,
			LPD3DXBUFFER * ppVertexRemap = NULL)
		{
			V(m_ptr->OptimizeInplace(Flags, pAdjacencyIn, pAdjacencyOut, pFaceRemap, ppVertexRemap));
		}

		void SetAttributeTable(CONST D3DXATTRIBUTERANGE * pAttribTable, DWORD cAttribTableSize)
		{
			V(m_ptr->SetAttributeTable(pAttribTable, cAttribTableSize));
		}

		void UnlockAttributeBuffer(void)
		{
			V(m_ptr->UnlockAttributeBuffer());
		}
	};

	typedef boost::shared_ptr<Mesh> MeshPtr;

	class OgreMesh : public Mesh
	{
	public:
		D3DVERTEXELEMENT9Set m_VertexElemSet;

		std::vector<std::string> m_MaterialNameList;

	public:
		OgreMesh(void)
		{
		};

		void CreateMeshFromOgreXml(
			LPDIRECT3DDEVICE9 pd3dDevice,
			LPCSTR pFilename,
			bool bComputeTangentFrame = true,
			DWORD dwMeshOptions = D3DXMESH_MANAGED);

		void CreateMeshFromOgreXmlInMemory(
			LPDIRECT3DDEVICE9 pd3dDevice,
			LPCSTR pSrcData,
			UINT srcDataLen,
			bool bComputeTangentFrame = true,
			DWORD dwMeshOptions = D3DXMESH_MANAGED);

		UINT GetMaterialNum(void) const
		{
			return m_MaterialNameList.size();
		}

		const std::string & GetMaterialName(DWORD AttribId) const
		{
			return m_MaterialNameList[AttribId];
		}

		void ComputeTangentFrame(void);
	};

	typedef boost::shared_ptr<OgreMesh> OgreMeshPtr;
};
