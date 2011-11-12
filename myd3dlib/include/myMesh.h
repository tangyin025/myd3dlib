
#pragma once

namespace my
{
	class VertexElement
	{
	protected:
		WORD Stream;
		D3DDECLTYPE Type;
		D3DDECLMETHOD Method;
		D3DDECLUSAGE Usage;
		BYTE UsageIndex;
		WORD ElementSize;

	public:
		VertexElement(WORD _Stream, D3DDECLTYPE _Type, D3DDECLMETHOD _Method, D3DDECLUSAGE _Usage, BYTE _UsageIndex, WORD _ElementSize)
			: Stream(_Stream)
			, Type(_Type)
			, Method(_Method)
			, Usage(_Usage)
			, UsageIndex(_UsageIndex)
			, ElementSize(_ElementSize)
		{
		}

		D3DVERTEXELEMENT9 BuildD3DVertexElement(WORD Offset) const
		{
			D3DVERTEXELEMENT9 ret = {Stream, Offset, Type, Method, Usage, UsageIndex};
			return ret;
		}

		virtual WORD GetElementSize(void) const
		{
			return ElementSize;
		}

	public:
		typedef Vector3 PositionType;

		static VertexElement CreatePositionElement(WORD _Stream = 0, BYTE _UsageIndex = 0, D3DDECLMETHOD _Method = D3DDECLMETHOD_DEFAULT)
		{
			return VertexElement(_Stream, D3DDECLTYPE_FLOAT3, _Method, D3DDECLUSAGE_POSITION, _UsageIndex, sizeof(PositionType));
		}

		typedef Vector3 NormalType;

		static VertexElement CreateNormalElement(WORD _Stream = 0, BYTE _UsageIndex = 0, D3DDECLMETHOD _Method = D3DDECLMETHOD_DEFAULT)
		{
			return VertexElement(_Stream, D3DDECLTYPE_FLOAT3, _Method, D3DDECLUSAGE_NORMAL, _UsageIndex, sizeof(NormalType));
		}

		typedef Vector2 TexcoordType;

		static VertexElement CreateTexcoordElement(WORD _Stream = 0, BYTE _UsageIndex = 0, D3DDECLMETHOD _Method = D3DDECLMETHOD_DEFAULT)
		{
			return VertexElement(_Stream, D3DDECLTYPE_FLOAT2, _Method, D3DDECLUSAGE_TEXCOORD, _UsageIndex, sizeof(TexcoordType));
		}

		typedef unsigned char IndicesType[4];

		static VertexElement CreateIndicesElement(WORD _Stream = 0, BYTE _UsageIndex = 0, D3DDECLMETHOD _Method = D3DDECLMETHOD_DEFAULT)
		{
			return VertexElement(_Stream, D3DDECLTYPE_UBYTE4, _Method, D3DDECLUSAGE_BLENDINDICES, _UsageIndex, sizeof(IndicesType));
		}

		typedef float WeightsType[4];

		static VertexElement CreateWeightsElement(WORD _Stream = 0, BYTE _UsageIndex = 0, D3DDECLMETHOD _Method = D3DDECLMETHOD_DEFAULT)
		{
			return VertexElement(_Stream, D3DDECLTYPE_FLOAT4, _Method, D3DDECLUSAGE_BLENDWEIGHT, _UsageIndex, sizeof(WeightsType));
		}
	};

	class VertexElementList : public std::vector<VertexElement>
	{
	};

	class VertexBuffer : public DeviceRelatedObjectBase
	{
	public:
		struct D3DVERTEXELEMENT9Less
		{
			bool operator ()(const D3DVERTEXELEMENT9 & lhs, const D3DVERTEXELEMENT9 & rhs) const
			{
				if(lhs.Usage == rhs.Usage)
				{
					return lhs.UsageIndex < rhs.UsageIndex;
				}
				return lhs.Usage < rhs.Usage;
			}
		};

		typedef std::set<D3DVERTEXELEMENT9, D3DVERTEXELEMENT9Less> D3DVERTEXELEMENT9Set;

		CComPtr<IDirect3DDevice9> m_Device;

		CComPtr<IDirect3DVertexBuffer9> m_VertexBuffer;

		std::vector<unsigned char> m_MemVertexBuffer;

		D3DVERTEXELEMENT9Set m_VertexElemSet;

		CComPtr<IDirect3DVertexDeclaration9> m_VertexDecl;

		WORD m_vertexStride;

		UINT m_NumVertices;

	public:
		VertexBuffer(LPDIRECT3DDEVICE9 pDevice, const VertexElementList & vertexElemList);

		void OnD3D9ResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		void OnD3D9LostDevice(void);

		void OnD3D9DestroyDevice(void);

		void UpdateVertexBuffer(void);

		void ResizeVertexBufferLength(UINT NumVertices);

		void SetPosition(int Index, const VertexElement::PositionType & Position, BYTE UsageIndex = 0);

		void SetNormal(int Index, const VertexElement::NormalType & Position, BYTE UsageIndex = 0);

		void SetTexcoord(int Index, const VertexElement::TexcoordType & Texcoord, BYTE UsageIndex = 0);

		void SetBlendIndices(int Index, int SubIndex, unsigned char BlendIndex, BYTE UsageIndex = 0);

		void SetBlendWeights(int Index, int SubIndex, float BlendWeight, BYTE UsageIndex = 0);
	};

	typedef boost::shared_ptr<VertexBuffer> VertexBufferPtr;

	class IndexBuffer : public DeviceRelatedObjectBase
	{
	public:
		typedef std::vector<unsigned int> UIntList;

		UIntList m_MemIndexBuffer;

		CComPtr<IDirect3DDevice9> m_Device;

		CComPtr<IDirect3DIndexBuffer9> m_IndexBuffer;

	public:
		IndexBuffer(LPDIRECT3DDEVICE9 pDevice);

		void OnD3D9ResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		void OnD3D9LostDevice(void);

		void OnD3D9DestroyDevice(void);

		void UpdateIndexBuffer(void);

		void ResizeIndexBufferLength(UINT NumIndices);

		void SetIndex(int Index, unsigned int IndexValue);
	};

	typedef boost::shared_ptr<IndexBuffer> IndexBufferPtr;

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

		CComPtr<IDirect3DIndexBuffer9> GetIndexBuffer(LPDIRECT3DINDEXBUFFER9 * ppIB)
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
			V(m_ptr->LockIndexBuffer(Flags, &pData));
			return pData;
		}

		LPVOID LockVertexBuffer(DWORD Flags = 0)
		{
			LPVOID pData = NULL;
			V(m_ptr->LockVertexBuffer(Flags, &pData));
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
			V(m_ptr->LockAttributeBuffer(Flags, &pData));
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

	class OgreVertexElement
	{
	public:
		WORD Stream;
		WORD Offset;
		BYTE Type;
		BYTE Method;
		BYTE Usage;
		BYTE UsageIndex;

	public:
		OgreVertexElement(WORD _Stream, WORD _Offset, BYTE _Type, BYTE _Method, BYTE _Usage, BYTE _UsageIndex)
			: Stream(_Stream)
			, Offset(_Offset)
			, Type(_Type)
			, Method(_Method)
			, Usage(_Usage)
			, UsageIndex(_UsageIndex)
		{
		}

		static OgreVertexElement End(WORD _Stream = 0xFF, WORD _Offset = 0, BYTE _UsageIndex = 0)
		{
			return OgreVertexElement(0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0);
		}

		static OgreVertexElement Position(WORD _Stream, WORD _Offset, BYTE _UsageIndex = 0)
		{
			return OgreVertexElement(_Stream, _Offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, _UsageIndex);
		}

		static OgreVertexElement Normal(WORD _Stream, WORD _Offset, BYTE _UsageIndex = 0)
		{
			return OgreVertexElement(_Stream, _Offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, _UsageIndex);
		}

		static OgreVertexElement Tangent(WORD _Stream, WORD _Offset, BYTE _UsageIndex = 0)
		{
			return OgreVertexElement(_Stream, _Offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, _UsageIndex);
		}

		static OgreVertexElement Binormal(WORD _Stream, WORD _Offset, BYTE _UsageIndex = 0)
		{
			return OgreVertexElement(_Stream, _Offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, _UsageIndex);
		}

		static OgreVertexElement Color(WORD _Stream, WORD _Offset, BYTE _UsageIndex = 0)
		{
			return OgreVertexElement(_Stream, _Offset, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, _UsageIndex);
		}

		static OgreVertexElement Texcoord(WORD _Stream, WORD _Offset, BYTE _UsageIndex = 0)
		{
			return OgreVertexElement(_Stream, _Offset, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, _UsageIndex);
		}

		static OgreVertexElement BlendIndices(WORD _Stream, WORD _Offset, BYTE _UsageIndex = 0)
		{
			return OgreVertexElement(_Stream, _Offset, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, _UsageIndex);
		}

		static OgreVertexElement BlendWeights(WORD _Stream, WORD _Offset, BYTE _UsageIndex = 0)
		{
			return OgreVertexElement(_Stream, _Offset, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, _UsageIndex);
		}
	};

	typedef std::vector<OgreVertexElement> OgreVertexElementList;

	class OgreMesh;

	typedef boost::shared_ptr<OgreMesh> OgreMeshPtr;

	class OgreMesh : public Mesh
	{
	public:
		static const int MAX_BONE_INDICES = 4;

	protected:
		static WORD CalculateD3DDeclTypeSize(int type);

		OgreMesh(ID3DXMesh * pMesh)
			: Mesh(pMesh)
		{
		}

	public:
		static OgreMeshPtr CreateOgreMesh(
			LPDIRECT3DDEVICE9 pd3dDevice,
			LPCSTR pSrcData,
			UINT srcDataLen,
			DWORD dwMeshOptions = D3DXMESH_MANAGED);

		static OgreMeshPtr CreateOgreMeshFromFile(
			LPDIRECT3DDEVICE9 pDevice,
			LPCTSTR pFilename,
			DWORD dwMeshOptions = D3DXMESH_MANAGED);
	};
};
