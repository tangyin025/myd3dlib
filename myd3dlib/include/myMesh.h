
#pragma once

namespace my
{
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

	class D3DVERTEXELEMENT9Set : public std::set<D3DVERTEXELEMENT9, D3DVERTEXELEMENT9Less>
	{
	public:
		typedef Vector3 PositionType;

		static D3DVERTEXELEMENT9 CreatePositionElement(WORD Stream, WORD Offset, BYTE UsageIndex = 0, BYTE Method = D3DDECLMETHOD_DEFAULT)
		{
			D3DVERTEXELEMENT9 ret = {Stream, Offset, D3DDECLTYPE_FLOAT3, Method, D3DDECLUSAGE_POSITION, UsageIndex};
			return ret;
		}

		PositionType & GetPosition(void * pVertex, BYTE UsageIndex = 0) const
		{
			const_iterator elem_iter = find(CreatePositionElement(0, 0, UsageIndex));
			_ASSERT(elem_iter != end());

			return *(PositionType *)((unsigned char *)pVertex + elem_iter->Offset);
		}

		void SetPosition(void * pVertex, const PositionType & Position, BYTE UsageIndex = 0) const
		{
			GetPosition(pVertex, UsageIndex) = Position;
		}

		typedef Vector3 NormalType;

		static D3DVERTEXELEMENT9 CreateNormalElement(WORD Stream, WORD Offset, BYTE UsageIndex = 0, BYTE Method = D3DDECLMETHOD_DEFAULT)
		{
			D3DVERTEXELEMENT9 ret = {Stream, Offset, D3DDECLTYPE_FLOAT3, Method, D3DDECLUSAGE_NORMAL, UsageIndex};
			return ret;
		}

		NormalType & GetNormal(void * pVertex, BYTE UsageIndex = 0) const
		{
			const_iterator elem_iter = find(CreateNormalElement(0, 0, UsageIndex));
			_ASSERT(elem_iter != end());

			return *(NormalType *)((unsigned char *)pVertex + elem_iter->Offset);
		}

		void SetNormal(void * pVertex, const NormalType & Normal, BYTE UsageIndex = 0) const
		{
			GetNormal(pVertex, UsageIndex) = Normal;
		}

		typedef Vector2 TexcoordType;

		static D3DVERTEXELEMENT9 CreateTexcoordElement(WORD Stream, WORD Offset, BYTE UsageIndex = 0, BYTE Method = D3DDECLMETHOD_DEFAULT)
		{
			D3DVERTEXELEMENT9 ret = {Stream, Offset, D3DDECLTYPE_FLOAT3, Method, D3DDECLUSAGE_TEXCOORD, UsageIndex};
			return ret;
		}

		TexcoordType & GetTexcoord(void * pVertex, BYTE UsageIndex = 0) const
		{
			const_iterator elem_iter = find(CreateTexcoordElement(0, 0, UsageIndex));
			_ASSERT(elem_iter != end());

			return *(TexcoordType *)((unsigned char *)pVertex + elem_iter->Offset);
		}

		void SetTexcoord(void * pVertex, const TexcoordType & Texcoord, BYTE UsageIndex = 0) const
		{
			GetTexcoord(pVertex, UsageIndex) = Texcoord;
		}

		typedef D3DCOLOR BlendIndicesType;

		static D3DVERTEXELEMENT9 CreateBlendIndicesElement(WORD Stream, WORD Offset, BYTE UsageIndex = 0, BYTE Method = D3DDECLMETHOD_DEFAULT)
		{
			D3DVERTEXELEMENT9 ret = {Stream, Offset, D3DDECLTYPE_UBYTE4, Method, D3DDECLUSAGE_BLENDINDICES, UsageIndex};
			return ret;
		}

		BlendIndicesType & GetBlendIndices(void * pVertex, BYTE UsageIndex = 0) const
		{
			const_iterator elem_iter = find(CreateBlendIndicesElement(0, 0, UsageIndex));
			_ASSERT(elem_iter != end());

			return *(BlendIndicesType *)((unsigned char *)pVertex + elem_iter->Offset);
		}

		void SetBlendIndices(void * pVertex, const BlendIndicesType & BlendIndices, BYTE UsageIndex = 0) const
		{
			GetBlendIndices(pVertex, UsageIndex) = BlendIndices;
		}

		typedef Vector4 BlendWeightsType;

		static D3DVERTEXELEMENT9 CreateBlendWeightsElement(WORD Stream, WORD Offset, BYTE UsageIndex = 0, BYTE Method = D3DDECLMETHOD_DEFAULT)
		{
			D3DVERTEXELEMENT9 ret = {Stream, Offset, D3DDECLTYPE_FLOAT4, Method, D3DDECLUSAGE_BLENDWEIGHT, UsageIndex};
			return ret;
		}

		BlendWeightsType & GetBlendWeights(void * pVertex, BYTE UsageIndex = 0) const
		{
			const_iterator elem_iter = find(CreateBlendWeightsElement(0, 0, UsageIndex));
			_ASSERT(elem_iter != end());

			return *(BlendWeightsType *)((unsigned char *)pVertex + elem_iter->Offset);
		}

		void SetBlendWeights(void * pVertex, const BlendWeightsType & BlendWeights, BYTE UsageIndex = 0) const
		{
			GetBlendWeights(pVertex, UsageIndex) = BlendWeights;
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

		std::vector<D3DVERTEXELEMENT9> BuildVertexElementList(void)
		{
			std::vector<D3DVERTEXELEMENT9> ret;
			const_iterator elem_iter = begin();
			for(; elem_iter != end(); elem_iter++)
			{
				ret.push_back(*elem_iter);
			}

			D3DVERTEXELEMENT9 elem_end = {0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0};
			ret.push_back(elem_end);
			return ret;
		}
	};

	class VertexBuffer : public DeviceRelatedObjectBase
	{
	public:

		CComPtr<IDirect3DDevice9> m_Device;

		CComPtr<IDirect3DVertexBuffer9> m_VertexBuffer;

		std::vector<unsigned char> m_MemVertexBuffer;

		D3DVERTEXELEMENT9Set m_VertexElemSet;

		CComPtr<IDirect3DVertexDeclaration9> m_VertexDecl;

		WORD m_vertexStride;

		UINT m_NumVertices;

	public:
		VertexBuffer(LPDIRECT3DDEVICE9 pDevice, const D3DVERTEXELEMENT9Set & VertexElemSet);

		void OnD3D9ResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		void OnD3D9LostDevice(void);

		void OnD3D9DestroyDevice(void);

		void UpdateVertexBuffer(void);

		void ResizeVertexBufferLength(UINT NumVertices);

		void SetPosition(int Index, const D3DVERTEXELEMENT9Set::PositionType & Position, BYTE UsageIndex = 0);

		void SetNormal(int Index, const D3DVERTEXELEMENT9Set::NormalType & Position, BYTE UsageIndex = 0);

		void SetTexcoord(int Index, const D3DVERTEXELEMENT9Set::TexcoordType & Texcoord, BYTE UsageIndex = 0);

		void SetBlendIndices(int Index, const D3DVERTEXELEMENT9Set::BlendIndicesType & BlendIndices, BYTE UsageIndex = 0);

		void SetBlendWeights(int Index, const D3DVERTEXELEMENT9Set::BlendWeightsType & BlendWeights, BYTE UsageIndex = 0);
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
