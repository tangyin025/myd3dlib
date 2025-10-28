// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include "myMath.h"
#include <vector>
#include <atlbase.h>
#include "mySingleton.h"

namespace rapidxml
{
	template <typename T> class xml_node;
}

namespace my
{
	class D3DVertexElement
	{
	public:
		WORD Offset;

		D3DDECLTYPE Type;

		D3DDECLMETHOD Method;

		D3DVertexElement(void)
			: Offset(0)
			, Type(D3DDECLTYPE_UNUSED)
			, Method(D3DDECLMETHOD_DEFAULT)
		{
		}

		D3DVertexElement(WORD _Offset, D3DDECLTYPE _Type, D3DDECLMETHOD _Method)
			: Offset(_Offset)
			, Type(_Type)
			, Method(_Method)
		{
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(Offset);
			ar & BOOST_SERIALIZATION_NVP(Type);
			ar & BOOST_SERIALIZATION_NVP(Method);
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

		D3DVertexElementSet(D3DVERTEXELEMENT9* elems);

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(elems);
		}

		void InsertVertexElement(WORD Offset, D3DDECLTYPE Type, D3DDECLUSAGE Usage, BYTE UsageIndex, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT);

		template <typename ElementType>
		ElementType * GetVertexValue(void * pVertex, D3DDECLUSAGE Usage, BYTE UsageIndex) const
		{
			return (ElementType *)((unsigned char *)pVertex + elems[Usage][UsageIndex].Offset);
		}

		template <typename ElementType>
		void SetVertexValue(void * pVertex, D3DDECLUSAGE Usage, BYTE UsageIndex, const ElementType & Value) const
		{
			*GetVertexValue<ElementType>(pVertex, Usage, UsageIndex) = Value;
		}

		std::vector<D3DVERTEXELEMENT9> BuildVertexElementList(WORD Stream) const;

		unsigned int CalcTextureCoords(void) const;

		void InsertPositionElement(WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT);

		Vector3 & GetPosition(void * pVertex, BYTE UsageIndex = 0) const;

		void SetPosition(void * pVertex, const Vector3 & Position, BYTE UsageIndex = 0) const;

		void InsertBlendWeightElement(WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT);

		Vector4 & GetBlendWeight(void * pVertex, BYTE UsageIndex = 0) const;

		void SetBlendWeight(void * pVertex, const Vector4 & BlendWeight, BYTE UsageIndex = 0) const;

		void InsertBlendIndicesElement(WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT);

		DWORD & GetBlendIndices(void * pVertex, BYTE UsageIndex = 0) const;

		void SetBlendIndices(void * pVertex, const DWORD & BlendIndices, BYTE UsageIndex = 0) const;

		void InsertNormalElement(WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT);

		Vector3 & GetNormal(void * pVertex, BYTE UsageIndex = 0) const;

		void SetNormal(void * pVertex, const Vector3 & Normal, BYTE UsageIndex = 0) const;

		void InsertTexcoordElement(WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT);

		Vector2 & GetTexcoord(void * pVertex, BYTE UsageIndex = 0) const;

		void SetTexcoord(void * pVertex, const Vector2 & Texcoord, BYTE UsageIndex = 0) const;

		void InsertTangentElement(WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT);

		Vector3 & GetTangent(void * pVertex, BYTE UsageIndex = 0) const;

		void SetTangent(void * pVertex, const Vector3 & Tangent, BYTE UsageIndex = 0) const;

		void InsertBinormalElement(WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT);

		Vector3 & GetBinormal(void * pVertex, BYTE UsageIndex = 0) const;

		void SetBinormal(void * pVertex, const Vector3 & Binormal, BYTE UsageIndex = 0) const;

		void InsertColorElement(WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT);

		D3DCOLOR & GetColor(void * pVertex, BYTE UsageIndex = 0) const;

		void SetColor(void * pVertex, const D3DCOLOR & Color, BYTE UsageIndex = 0) const;
	};

	class VertexBuffer : public D3DDeviceResource<IDirect3DVertexBuffer9>
	{
	public:
		VertexBuffer(void)
		{
		}

		virtual void OnResetDevice(void);

		virtual void OnLostDevice(void);

		void Create(IDirect3DVertexBuffer9 * ptr);

		void CreateVertexBuffer(
			UINT Length,
			DWORD Usage = 0,
			DWORD FVF = 0,
			D3DPOOL Pool = D3DPOOL_DEFAULT);

		void SaveVertexBuffer(const char* path);

		D3DVERTEXBUFFER_DESC GetDesc(void);

		void * Lock(UINT OffsetToLock = 0, UINT SizeToLock = 0, DWORD Flags = 0);

		void Unlock(void);
	};

	typedef boost::shared_ptr<VertexBuffer> VertexBufferPtr;

	class IndexBuffer : public D3DDeviceResource<IDirect3DIndexBuffer9>
	{
	public:
		IndexBuffer(void)
		{
		}

		void Create(IDirect3DIndexBuffer9 * ptr);

		void CreateIndexBuffer(
			UINT Length,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_INDEX16,
			D3DPOOL Pool = D3DPOOL_DEFAULT);

		D3DINDEXBUFFER_DESC GetDesc(void);

		void * Lock(UINT OffsetToLock, UINT SizeToLock, DWORD Flags = 0);

		void Unlock(void);
	};

	typedef boost::shared_ptr<IndexBuffer> IndexBufferPtr;

	class Mesh : public D3DDeviceResource<ID3DXMesh>
	{
	public:
		Mesh(void)
		{
		}

		void Create(ID3DXMesh * pMesh);

		void CreateMesh(
			DWORD NumFaces,
			DWORD NumVertices,
			CONST LPD3DVERTEXELEMENT9 pDeclaration,
			DWORD Options = D3DXMESH_MANAGED);

		void CreateMeshFVF(
			DWORD NumFaces,
			DWORD NumVertices,
			DWORD FVF,
			DWORD Options = D3DXMESH_MANAGED);

		void CreateMeshFromX(
			LPCTSTR pFilename,
			DWORD Options = D3DXMESH_MANAGED,
			LPD3DXBUFFER * ppAdjacency = NULL,
			LPD3DXBUFFER * ppMaterials = NULL,
			LPD3DXBUFFER * ppEffectInstances = NULL,
			DWORD * pNumMaterials = NULL);

		void CreateMeshFromXInMemory(
			LPCVOID Memory,
			DWORD SizeOfMemory,
			DWORD Options = D3DXMESH_MANAGED,
			LPD3DXBUFFER * ppAdjacency = NULL,
			LPD3DXBUFFER * ppMaterials = NULL,
			LPD3DXBUFFER * ppEffectInstances = NULL,
			DWORD * pNumMaterials = NULL);

		void CreateBox(
			FLOAT Width = 1.0f,
			FLOAT Height = 1.0f,
			FLOAT Depth = 1.0f,
			LPD3DXBUFFER * ppAdjacency = NULL);

		void CreateCylinder(
			FLOAT Radius1 = 1.0f,
			FLOAT Radius2 = 1.0f,
			FLOAT Length = 2.0f,
			UINT Slices = 20,
			UINT Stacks = 1,
			LPD3DXBUFFER * ppAdjacency = NULL);

		void CreatePolygon(
			FLOAT Length = 1.0f,
			UINT Sides = 5,
			LPD3DXBUFFER * ppAdjacency = NULL);

		void CreateSphere(
			FLOAT Radius = 1.0f,
			UINT Slices = 20,
			UINT Stacks = 20,
			LPD3DXBUFFER * ppAdjacency = NULL);

		void CreateTeapot(
			LPD3DXBUFFER * ppAdjacency = NULL);

		void CreateTorus(
			FLOAT InnerRadius = 0.5f,
			FLOAT OuterRadius = 1.5f,
			UINT Sides = 20,
			UINT Rings = 20,
			LPD3DXBUFFER * ppAdjacency = NULL);

		CComPtr<ID3DXMesh> CloneMesh(DWORD Options, CONST D3DVERTEXELEMENT9 * pDeclaration);

		CComPtr<ID3DXMesh> CloneMeshFVF(DWORD Options, DWORD FVF);

		CComPtr<ID3DXMesh> CleanMesh(D3DXCLEANTYPE CleanType, const DWORD *pAdjacencyIn, DWORD *pAdjacencyOut);

		CComPtr<ID3DXMesh> SimplifyMesh(
			const DWORD *pAdjacency,
			DWORD MinValue,
			DWORD Options = D3DXMESHSIMP_FACE,
			const D3DXATTRIBUTEWEIGHTS *pVertexAttributeWeights = NULL,
			const FLOAT *pVertexWeights = NULL);

		void ConvertAdjacencyToPointReps(CONST DWORD * pAdjacency, DWORD * pPRep);

		void ConvertPointRepsToAdjacency(CONST DWORD* pPRep, DWORD* pAdjacency);

		void DrawSubset(DWORD AttribId);

		void GenerateAdjacency(FLOAT Epsilon, DWORD * pAdjacency);

		void GetAttributeTable(D3DXATTRIBUTERANGE * pAttribTable, DWORD * pAttribTableSize);

		DWORD GetNumAttributes(void);

		void GetDeclaration(D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE]);

		CComPtr<IDirect3DDevice9> GetDevice(void);

		DWORD GetFVF(void);

		CComPtr<IDirect3DIndexBuffer9> GetIndexBuffer(void);

		DWORD GetNumBytesPerVertex(void);

		DWORD GetNumFaces(void);

		DWORD GetNumVertices(void);

		DWORD GetOptions(void);

		CComPtr<IDirect3DVertexBuffer9> GetVertexBuffer(void);

		LPVOID LockIndexBuffer(DWORD Flags = 0);

		LPVOID LockVertexBuffer(DWORD Flags = 0);

		void UnlockIndexBuffer(void);

		void UnlockVertexBuffer(void);

		void UpdateSemantics(D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE]);

		DWORD * LockAttributeBuffer(DWORD Flags = 0);

		CComPtr<ID3DXMesh> Optimize(
			DWORD Flags,
			CONST DWORD * pAdjacencyIn,
			DWORD * pAdjacencyOut,
			DWORD * pFaceRemap = NULL,
			LPD3DXBUFFER * ppVertexRemap = NULL);

		void OptimizeInplace(DWORD Flags,
			CONST DWORD * pAdjacencyIn,
			DWORD * pAdjacencyOut,
			DWORD * pFaceRemap = NULL,
			LPD3DXBUFFER * ppVertexRemap = NULL);

		void SetAttributeTable(CONST D3DXATTRIBUTERANGE * pAttribTable, DWORD cAttribTableSize);

		void UnlockAttributeBuffer(void);

		DWORD GetAttributeIdFromInternalFaceIndex(unsigned int face_i);

		static void ComputeDualQuaternionSkinnedVertices(
			void * pDstVertices,
			DWORD NumVerts,
			DWORD DstVertexStride,
			const D3DVertexElementSet & DstVertexElems,
			void * pSrcVertices,
			DWORD SrcVertexStride,
			const D3DVertexElementSet & SrcVertexElems,
			const Matrix4 * DualQuaternions,
			DWORD DualQuaternionSize);

		static void ComputeNormalFrame(
			void * pVertices,
			DWORD NumVerts,
			DWORD VertexStride,
			const void * pIndices,
			bool bIndices16,
			DWORD NumFaces,
			const D3DVertexElementSet & VertexElems);

		static void ComputeTangentFrame(
			void * pVertices,
			DWORD NumVerts,
			DWORD VertexStride,
			const void * pIndices,
			bool bIndices16,
			DWORD NumFaces,
			const D3DVertexElementSet & VertexElems);

		static RayResult RayTest(
			const Ray& ray,
			void* pVertices,
			DWORD NumVerts,
			DWORD VertexStride,
			void* pIndices,
			bool bIndices16,
			DWORD FaceStart,
			DWORD NumFaces,
			const D3DVertexElementSet& VertexElems);

		static bool FrustumTest(
			const Frustum& frustum,
			void* pVertices,
			DWORD NumVerts,
			DWORD VertexStride,
			void* pIndices,
			bool bIndices16,
			DWORD FaceStart,
			DWORD NumFaces,
			const D3DVertexElementSet& VertexElems);
	};

	typedef boost::shared_ptr<Mesh> MeshPtr;

	class ProgressiveMesh;

	class OgreMesh : public Mesh
	{
	public:
		D3DVertexElementSet m_VertexElems;

		std::vector<D3DXATTRIBUTERANGE> m_AttribTable;

		CComPtr<IDirect3DVertexDeclaration9> m_Decl;

		VertexBuffer m_Vb;

		IndexBuffer m_Ib;

		//std::vector<DWORD> m_Adjacency;

	public:
		OgreMesh(void)
		{
		}

		void CreateMeshFromOgreXmlInFile(
			LPCTSTR pFilename,
			bool bComputeTangentFrame = true,
			DWORD dwMeshOptions = D3DXMESH_MANAGED,
			unsigned int reserveVertices = 0,
			unsigned int reserveFaces = 0);

		void CreateMeshFromOgreXmlInMemory(
			LPSTR pSrcData,
			UINT srcDataLen,
			bool bComputeTangentFrame,
			DWORD dwMeshOptions,
			unsigned int reserveVertices,
			unsigned int reserveFaces);

		void CreateMeshFromOgreXml(
			const rapidxml::xml_node<char> * node_root,
			bool bComputeTangentFrame,
			DWORD dwMeshOptions,
			unsigned int reserveVertices,
			unsigned int reserveFaces);

		//void CreateMeshFromObjInFile(
		//	LPCTSTR pFilename,
		//	bool bComputeTangentFrame = true,
		//	DWORD dwMeshOptions = D3DXMESH_MANAGED);

		//void CreateMeshFromObjInStream(
		//	std::istream & is,
		//	bool bComputeTangentFrame = true,
		//	DWORD dwMeshOptions = D3DXMESH_MANAGED);

		void CreateMeshFromOther(OgreMesh* other, DWORD AttribId, const Matrix4& trans, const Matrix4& uv_trans, unsigned int reserveVertices, unsigned int reserveFaces);

		void AppendMesh(OgreMesh* other, DWORD AttribId, const Matrix4& trans, const Matrix4& uv_trans);

		void CombineMesh(OgreMesh* other, DWORD AttribId, const Matrix4& trans, const Matrix4& uv_trans);

		const D3DXATTRIBUTERANGE& AppendToAttrib(const D3DXATTRIBUTERANGE& rang, OgreMesh* other, DWORD AttribId, const Matrix4& trans, const Matrix4& uv_trans);

		void AppendProgressiveMesh(ProgressiveMesh* pmesh);

		void SaveOgreMesh(LPCTSTR path, bool useSharedGeom);

		boost::shared_ptr<OgreMesh> Optimize(DWORD Flags);

		void OptimizeInplace(DWORD Flags);

		boost::shared_ptr<OgreMesh> SimplifyMesh(DWORD MinValue, DWORD Options);

		void SaveObj(const char* path);

		void Transform(const Matrix4 & trans);

		AABB CalculateAABB(DWORD AttribId);
	};

	typedef boost::shared_ptr<OgreMesh> OgreMeshPtr;

	class ProgressiveMesh
	{
	public:
		OgreMesh* m_Mesh;

		const DWORD m_NumAttribs;

		struct PMTriangle
		{
			int vi[3];
			int AttribId;
		};

		std::vector<PMTriangle> m_Tris;

		struct PMVertex
		{
			std::vector<int> tris;
			boost::shared_ptr<std::vector<Plane> > planes;
			std::map<int, int> neighbors;
			bool isBorder;
			float collapsecost;
			int collapseto;
		};

		std::vector<PMVertex> m_Verts;

	public:
		ProgressiveMesh(OgreMesh* Mesh, DWORD NumAttribs);

		void UpdateCollapseCost(std::vector<PMVertex>::iterator vert_iter);

		void Collapse(int numCollapses);

		DWORD GetNumFaces(void);
	};
}
