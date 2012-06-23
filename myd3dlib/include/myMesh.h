
#pragma once

#include <d3d9.h>
#include <set>
#include <myMath.h>
#include <vector>
#include <atlbase.h>
#include <boost/shared_ptr.hpp>
#include "mySingleton.h"
#include "myException.h"

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
		ElementType & GetCustomType(void * pVertex, WORD Stream, D3DDECLUSAGE Usage, BYTE UsageIndex) const
		{
			const_iterator elem_iter = find(CreateCustomElement(Stream, Usage, UsageIndex));
			_ASSERT(elem_iter != end());

			return *(ElementType *)((unsigned char *)pVertex + elem_iter->Offset);
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

		UINT GetVertexStride(DWORD Stream) const;

		CComPtr<IDirect3DVertexDeclaration9> CreateVertexDeclaration(LPDIRECT3DDEVICE9 pDevice) const;
	};

	class VertexBuffer;

	typedef boost::shared_ptr<VertexBuffer> VertexBufferPtr;

	class VertexBuffer : public DeviceRelatedObjectBase
	{
	//protected:
	public:
		CComPtr<IDirect3DDevice9> m_Device;

		CComPtr<IDirect3DVertexBuffer9> m_VertexBuffer;

		std::vector<unsigned char> m_MemVertexBuffer;

		D3DVERTEXELEMENT9Set m_VertexElemSet;

		WORD m_vertexStride;

		UINT m_NumVertices;

		WORD m_Stream;

		VertexBuffer(LPDIRECT3DDEVICE9 pDevice, const D3DVERTEXELEMENT9Set & VertexElemSet, WORD Stream = 0);

	public:
		static VertexBufferPtr CreateVertexBuffer(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const D3DVERTEXELEMENT9Set & VertexElemSet,
			WORD Stream = 0);

		void OnResetDevice(void);

		void OnLostDevice(void);

		void OnDestroyDevice(void);

		void UpdateVertexBuffer(void);

		void ResizeVertexBufferLength(UINT NumVertices);

		template <typename ElementType>
		void SetCustomType(UINT Index, const ElementType & Value, D3DDECLUSAGE Usage, BYTE UsageIndex)
		{
			_ASSERT(Index < m_NumVertices && m_MemVertexBuffer.size() == m_NumVertices * m_vertexStride);

			m_VertexElemSet.SetCustomType<ElementType>(&m_MemVertexBuffer[Index * m_vertexStride], m_Stream, Usage, UsageIndex, Value);
		}

		void SetPosition(UINT Index, const D3DVERTEXELEMENT9Set::PositionType & Position, BYTE UsageIndex = 0);

		void SetBinormal(UINT Index, const D3DVERTEXELEMENT9Set::BinormalType & Binormal, BYTE UsageIndex = 0);

		void SetTangent(UINT Index, const D3DVERTEXELEMENT9Set::TangentType & Tangent, BYTE UsageIndex = 0);

		void SetNormal(UINT Index, const D3DVERTEXELEMENT9Set::NormalType & Normal, BYTE UsageIndex = 0);

		void SetTexcoord(UINT Index, const D3DVERTEXELEMENT9Set::TexcoordType & Texcoord, BYTE UsageIndex = 0);

		void SetBlendIndices(UINT Index, const D3DVERTEXELEMENT9Set::BlendIndicesType & BlendIndices, BYTE UsageIndex = 0);

		void SetBlendWeights(UINT Index, const D3DVERTEXELEMENT9Set::BlendWeightsType & BlendWeights, BYTE UsageIndex = 0);
	};

	class IndexBuffer;

	typedef boost::shared_ptr<IndexBuffer> IndexBufferPtr;

	class IndexBuffer : public DeviceRelatedObjectBase
	{
	public:
		CComPtr<IDirect3DIndexBuffer9> m_IndexBuffer;

	protected:
		typedef std::vector<unsigned int> UIntList;

		UIntList m_MemIndexBuffer;

		CComPtr<IDirect3DDevice9> m_Device;

		IndexBuffer(LPDIRECT3DDEVICE9 pDevice);

	public:
		static IndexBufferPtr CreateIndexBuffer(LPDIRECT3DDEVICE9 pD3DDevice);

		void OnResetDevice(void);

		void OnLostDevice(void);

		void OnDestroyDevice(void);

		void UpdateIndexBuffer(void);

		void ResizeIndexBufferLength(UINT NumIndices);

		void SetIndex(UINT Index, unsigned int IndexValue);
	};

	class Mesh;

	typedef boost::shared_ptr<Mesh> MeshPtr;

	class Mesh : public DeviceRelatedObject<ID3DXMesh>
	{
	protected:
		Mesh(ID3DXMesh * pMesh)
			: DeviceRelatedObject(pMesh)
		{
		}

	public:
		static MeshPtr CreateMesh(
			LPDIRECT3DDEVICE9 pD3DDevice,
			DWORD NumFaces,
			DWORD NumVertices,
			CONST LPD3DVERTEXELEMENT9 pDeclaration,
			DWORD Options = D3DXMESH_MANAGED);

		static MeshPtr CreateMeshFVF(
			LPDIRECT3DDEVICE9 pD3DDevice,
			DWORD NumFaces,
			DWORD NumVertices,
			DWORD FVF,
			DWORD Options = D3DXMESH_MANAGED);

		static MeshPtr CreateMeshFromX(
			LPDIRECT3DDEVICE9 pD3DDevice,
			LPCSTR pFilename,
			DWORD Options = D3DXMESH_MANAGED,
			LPD3DXBUFFER * ppAdjacency = NULL,
			LPD3DXBUFFER * ppMaterials = NULL,
			LPD3DXBUFFER * ppEffectInstances = NULL,
			DWORD * pNumMaterials = NULL);

		static MeshPtr CreateMeshFromXInMemory(
			LPDIRECT3DDEVICE9 pD3DDevice,
			LPCVOID Memory,
			DWORD SizeOfMemory,
			DWORD Options = D3DXMESH_MANAGED,
			LPD3DXBUFFER * ppAdjacency = NULL,
			LPD3DXBUFFER * ppMaterials = NULL,
			LPD3DXBUFFER * ppEffectInstances = NULL,
			DWORD * pNumMaterials = NULL);

		static MeshPtr CreateMeshFromOgreXml(
			LPDIRECT3DDEVICE9 pd3dDevice,
			LPCSTR pFilename,
			bool bComputeTangentFrame = true,
			DWORD dwMeshOptions = D3DXMESH_MANAGED);

		static MeshPtr CreateMeshFromOgreXmlInMemory(
			LPDIRECT3DDEVICE9 pd3dDevice,
			LPCSTR pSrcData,
			UINT srcDataLen,
			bool bComputeTangentFrame = true,
			DWORD dwMeshOptions = D3DXMESH_MANAGED);

		static MeshPtr CreateBox(
			LPDIRECT3DDEVICE9 pd3dDevice,
			FLOAT Width = 1.0f,
			FLOAT Height = 1.0f,
			FLOAT Depth = 1.0f,
			LPD3DXBUFFER * ppAdjacency = NULL);

		static MeshPtr CreateCylinder(
			LPDIRECT3DDEVICE9 pd3dDevice,
			FLOAT Radius1 = 1.0f,
			FLOAT Radius2 = 1.0f,
			FLOAT Length = 2.0f,
			UINT Slices = 20,
			UINT Stacks = 1,
			LPD3DXBUFFER * ppAdjacency = NULL);

		static MeshPtr CreatePolygon(
			LPDIRECT3DDEVICE9 pDevice,
			FLOAT Length = 1.0f,
			UINT Sides = 5,
			LPD3DXBUFFER * ppAdjacency = NULL);

		static MeshPtr CreateSphere(
			LPDIRECT3DDEVICE9 pDevice,
			FLOAT Radius = 1.0f,
			UINT Slices = 20,
			UINT Stacks = 20,
			LPD3DXBUFFER * ppAdjacency = NULL);

		static MeshPtr CreateTeapot(
			LPDIRECT3DDEVICE9 pDevice,
			LPD3DXBUFFER * ppAdjacency = NULL);

		static MeshPtr CreateTorus(
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
};
