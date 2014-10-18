#pragma once

#include "myMath.h"
#include <vector>
#include <atlbase.h>
#include <boost/shared_ptr.hpp>
#include "mySingleton.h"
#include "rapidxml.hpp"

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

		void InsertVertexElement(WORD Offset, D3DDECLTYPE Type, D3DDECLUSAGE Usage, BYTE UsageIndex, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT);

		template <typename ElementType>
		ElementType & GetVertexValue(void * pVertex, D3DDECLUSAGE Usage, BYTE UsageIndex) const
		{
			return *(ElementType *)((unsigned char *)pVertex + elems[Usage][UsageIndex].Offset);
		}

		template <typename ElementType>
		void SetVertexValue(void * pVertex, D3DDECLUSAGE Usage, BYTE UsageIndex, const ElementType & Value) const
		{
			GetVertexValue<ElementType>(pVertex, Usage, UsageIndex) = Value;
		}

		std::vector<D3DVERTEXELEMENT9> BuildVertexElementList(WORD Stream);

		unsigned int CalcTextureCoords(void);

		void InsertPositionElement(WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT);

		Vector3 & GetPosition(void * pVertex, BYTE UsageIndex = 0);

		void SetPosition(void * pVertex, const Vector3 & Position, BYTE UsageIndex = 0) const;

		void InsertBlendWeightElement(WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT);

		Vector4 & GetBlendWeight(void * pVertex, BYTE UsageIndex = 0);

		void SetBlendWeight(void * pVertex, const Vector4 & BlendWeight, BYTE UsageIndex = 0) const;

		void InsertBlendIndicesElement(WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT);

		DWORD & GetBlendIndices(void * pVertex, BYTE UsageIndex = 0);

		void SetBlendIndices(void * pVertex, const DWORD & BlendIndices, BYTE UsageIndex = 0) const;

		void InsertNormalElement(WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT);

		Vector3 & GetNormal(void * pVertex, BYTE UsageIndex = 0);

		void SetNormal(void * pVertex, const Vector3 & Normal, BYTE UsageIndex = 0) const;

		void InsertTexcoordElement(WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT);

		Vector2 & GetTexcoord(void * pVertex, BYTE UsageIndex = 0);

		void SetTexcoord(void * pVertex, const Vector2 & Texcoord, BYTE UsageIndex = 0) const;

		void InsertTangentElement(WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT);

		Vector3 & GetTangent(void * pVertex, BYTE UsageIndex = 0);

		void SetTangent(void * pVertex, const Vector3 & Tangent, BYTE UsageIndex = 0) const;

		void InsertBinormalElement(WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT);

		Vector3 & GetBinormal(void * pVertex, BYTE UsageIndex = 0);

		void SetBinormal(void * pVertex, const Vector3 & Binormal, BYTE UsageIndex = 0) const;

		void InsertColorElement(WORD Offset, BYTE UsageIndex = 0, D3DDECLMETHOD Method = D3DDECLMETHOD_DEFAULT);

		D3DCOLOR & GetColor(void * pVertex, BYTE UsageIndex = 0);

		void SetColor(void * pVertex, const D3DCOLOR & Color, BYTE UsageIndex = 0) const;
	};

	class VertexBuffer : public DeviceRelatedObject<IDirect3DVertexBuffer9>
	{
	public:
		VertexBuffer(void)
		{
		}

		void Create(IDirect3DVertexBuffer9 * ptr);

		void CreateVertexBuffer(
			LPDIRECT3DDEVICE9 pDevice,
			UINT Length,
			DWORD Usage = 0,
			DWORD FVF = 0,
			D3DPOOL Pool = D3DPOOL_DEFAULT);

		D3DVERTEXBUFFER_DESC GetDesc(void);

		void * Lock(UINT OffsetToLock, UINT SizeToLock, DWORD Flags = 0);

		void Unlock(void);
	};

	typedef boost::shared_ptr<VertexBuffer> VertexBufferPtr;

	class IndexBuffer : public DeviceRelatedObject<IDirect3DIndexBuffer9>
	{
	public:
		IndexBuffer(void)
		{
		}

		void Create(IDirect3DIndexBuffer9 * ptr);

		void CreateIndexBuffer(
			LPDIRECT3DDEVICE9 pDevice,
			UINT Length,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_INDEX16,
			D3DPOOL Pool = D3DPOOL_DEFAULT);

		D3DINDEXBUFFER_DESC GetDesc(void);

		void * Lock(UINT OffsetToLock, UINT SizeToLock, DWORD Flags = 0);

		void Unlock(void);
	};

	typedef boost::shared_ptr<IndexBuffer> IndexBufferPtr;

	class Mesh : public DeviceRelatedObject<ID3DXMesh>
	{
	public:
		Mesh(void)
		{
		}

		void Create(ID3DXMesh * pMesh);

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

		CComPtr<ID3DXMesh> CloneMesh(DWORD Options, CONST D3DVERTEXELEMENT9 * pDeclaration, LPDIRECT3DDEVICE9 pDevice);

		CComPtr<ID3DXMesh> CloneMeshFVF(DWORD Options, DWORD FVF, LPDIRECT3DDEVICE9 pDevice);

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
			DWORD * pFaceRemap,
			LPD3DXBUFFER * ppVertexRemap = NULL);

		void OptimizeInplace(DWORD Flags,
			CONST DWORD * pAdjacencyIn,
			DWORD * pAdjacencyOut,
			DWORD * pFaceRemap,
			LPD3DXBUFFER * ppVertexRemap = NULL);

		void SetAttributeTable(CONST D3DXATTRIBUTERANGE * pAttribTable, DWORD cAttribTableSize);

		void UnlockAttributeBuffer(void);
	};

	typedef boost::shared_ptr<Mesh> MeshPtr;

	class OgreMesh : public Mesh
	{
	public:
		D3DVertexElementSet m_VertexElems;

		std::vector<std::string> m_MaterialNameList;

		AABB m_aabb;

		std::vector<DWORD> m_Adjacency;

	public:
		OgreMesh(void)
		{
		};

		void CreateMeshFromOgreXmlInFile(
			LPDIRECT3DDEVICE9 pd3dDevice,
			LPCTSTR pFilename,
			bool bComputeTangentFrame = true,
			DWORD dwMeshOptions = D3DXMESH_MANAGED);

		void CreateMeshFromOgreXmlInMemory(
			LPDIRECT3DDEVICE9 pd3dDevice,
			LPSTR pSrcData,
			UINT srcDataLen,
			bool bComputeTangentFrame = true,
			DWORD dwMeshOptions = D3DXMESH_MANAGED);

		void CreateMeshFromOgreXml(
			LPDIRECT3DDEVICE9 pd3dDevice,
			const rapidxml::xml_node<char> * node_root,
			bool bComputeTangentFrame = true,
			DWORD dwMeshOptions = D3DXMESH_MANAGED);

		void CreateMeshFromOgreXmlNodes(
			LPDIRECT3DDEVICE9 pd3dDevice,
			const rapidxml::xml_node<char> * node_geometry,
			const rapidxml::xml_node<char> * node_boneassignments,
			const rapidxml::xml_node<char> * node_submesh,
			const bool bUseSharedGeometry = true,
			bool bComputeTangentFrame = true,
			DWORD dwMeshOptions = D3DXMESH_MANAGED);

		void SaveMesh(std::ostream & ostr);

		void ComputeTangentFrame(void);

		UINT GetMaterialNum(void) const;

		const std::string & GetMaterialName(DWORD AttribId) const;
	};

	typedef boost::shared_ptr<OgreMesh> OgreMeshPtr;

	class OgreMeshSet : public DeviceRelatedObject<ID3DXMesh>, public std::vector<OgreMeshPtr>
	{
	public:
		OgreMeshSet(void)
		{
		}

		virtual void OnResetDevice(void);

		virtual void OnLostDevice(void);

		virtual void OnDestroyDevice(void);

		void CreateMeshSetFromOgreXmlInFile(
			LPDIRECT3DDEVICE9 pd3dDevice,
			LPCTSTR pFilename,
			bool bComputeTangentFrame = true,
			DWORD dwMeshSetOptions = D3DXMESH_MANAGED);

		void CreateMeshSetFromOgreXmlInMemory(
			LPDIRECT3DDEVICE9 pd3dDevice,
			LPSTR pSrcData,
			UINT srcDataLen,
			bool bComputeTangentFrame = true,
			DWORD dwMeshSetOptions = D3DXMESH_MANAGED);

		void CreateMeshSetFromOgreXml(
			LPDIRECT3DDEVICE9 pd3dDevice,
			const rapidxml::xml_node<char> * node_root,
			bool bComputeTangentFrame = true,
			DWORD dwMeshOptions = D3DXMESH_MANAGED);
	};

	typedef boost::shared_ptr<OgreMeshSet> OgreMeshSetPtr;
}
