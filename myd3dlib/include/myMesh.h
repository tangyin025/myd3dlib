
#pragma once

namespace my
{
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

	class VertexElement
	{
	public:
		WORD Stream;
		WORD Offset;
		BYTE Type;
		BYTE Method;
		BYTE Usage;
		BYTE UsageIndex;

	public:
		VertexElement(WORD _Stream, WORD _Offset, BYTE _Type, BYTE _Method, BYTE _Usage, BYTE _UsageIndex)
			: Stream(_Stream)
			, Offset(_Offset)
			, Type(_Type)
			, Method(_Method)
			, Usage(_Usage)
			, UsageIndex(_UsageIndex)
		{
		}

		static VertexElement End(WORD _Stream = 0xFF, WORD _Offset = 0, BYTE _UsageIndex = 0)
		{
			return VertexElement(0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0);
		}

		static VertexElement Position(WORD _Stream, WORD _Offset, BYTE _UsageIndex = 0)
		{
			return VertexElement(_Stream, _Offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, _UsageIndex);
		}

		static VertexElement Normal(WORD _Stream, WORD _Offset, BYTE _UsageIndex = 0)
		{
			return VertexElement(_Stream, _Offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, _UsageIndex);
		}

		static VertexElement Tangent(WORD _Stream, WORD _Offset, BYTE _UsageIndex = 0)
		{
			return VertexElement(_Stream, _Offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, _UsageIndex);
		}

		static VertexElement Binormal(WORD _Stream, WORD _Offset, BYTE _UsageIndex = 0)
		{
			return VertexElement(_Stream, _Offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, _UsageIndex);
		}

		static VertexElement Color(WORD _Stream, WORD _Offset, BYTE _UsageIndex = 0)
		{
			return VertexElement(_Stream, _Offset, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, _UsageIndex);
		}

		static VertexElement Texcoord(WORD _Stream, WORD _Offset, BYTE _UsageIndex = 0)
		{
			return VertexElement(_Stream, _Offset, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, _UsageIndex);
		}
	};

	typedef std::vector<VertexElement> VertexElementList;

	class OgreMesh;

	typedef boost::shared_ptr<OgreMesh> OgreMeshPtr;

	class OgreMesh : public Mesh
	{
	protected:
		OgreMesh(ID3DXMesh * pMesh)
			: Mesh(pMesh)
		{
		}

		static WORD CalculateD3DDeclTypeSize(int type);

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
